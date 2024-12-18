//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget to store and manage multiple data pool list entries (implementation)

   Widget to store and manage multiple data pool list entries

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <iostream>

#include <QDrag>
#include <QDragMoveEvent>
#include <QScrollBar>

#include "C_SdNdeDpListsTreeWidget.hpp"
#include "stwerrors.hpp"
#include "C_SdClipBoardHelper.hpp"
#include "C_SdNdeDpListTableView.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_GtGetText.hpp"
#include "C_SdNdeDpUtil.hpp"
#include "C_Uti.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_OscLoggingHandler.hpp"
#include "TglUtils.hpp"
#include "C_SdUtil.hpp"
#include "C_UsHandler.hpp"
#include "C_OgeWiCustomMessage.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListsTreeWidget::C_SdNdeDpListsTreeWidget(QWidget * const opc_Parent) :
   QTreeWidget(opc_Parent),
   mu32_NodeIndex(0),
   mu32_DataPoolIndex(0),
   mpc_ContextMenu(NULL),
   mpc_ActionAdd(NULL),
   mpc_ActionMoveUp(NULL),
   mpc_ActionMoveDown(NULL),
   mq_AllowAdd(true),
   mq_AllowMoveUp(true),
   mq_AllowMoveDown(true),
   mq_InitialUserSettings(true)
{
   //UI Settings
   this->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
   this->setDragEnabled(false);
   this->setDragDropMode(QAbstractItemView::NoDragDrop);
   this->setDefaultDropAction(Qt::TargetMoveAction);
   this->setAlternatingRowColors(true);
   this->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
   this->setAutoExpandDelay(500);
   this->setIndentation(0);
   this->setRootIsDecorated(false);
   this->setAnimated(true);
   this->setHeaderHidden(true);
   this->setAcceptDrops(true);
   this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

   this->setItemDelegate(&this->mc_Delegate);

   //Custom context menu
   m_SetupContextMenu();

   // configure the scrollbar to stop resizing the widget when showing or hiding the scrollbar
   this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   this->verticalScrollBar()->hide();

   //Avoid styling table inside
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->verticalScrollBar(), "C_SdNdeDpListsTreeWidget", true);
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->horizontalScrollBar(), "C_SdNdeDpListsTreeWidget", true);

   // Deactivate custom context menu of scroll bar
   this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
   this->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

   //Connect
   connect(this, &C_SdNdeDpListsTreeWidget::collapsed, this, &C_SdNdeDpListsTreeWidget::m_OnCollapse);
   connect(this, &C_SdNdeDpListsTreeWidget::expanded, this, &C_SdNdeDpListsTreeWidget::m_OnExpand);
   connect(&this->mc_UndoManager, &C_SdNdeUnoDataPoolManager::SigChanged, this,
           &C_SdNdeDpListsTreeWidget::m_HandleChanged);
   connect(&this->mc_ModelViewManager, &C_SdNdeDpListModelViewManager::SigDataSetCountChange, this,
           &C_SdNdeDpListsTreeWidget::m_UpdateDataSetCount);
   connect(&this->mc_ModelViewManager, &C_SdNdeDpListModelViewManager::SigDataSetErrorChange, this,
           &C_SdNdeDpListsTreeWidget::m_HandleDataSetErrorChange);

   connect(this->verticalScrollBar(), &QScrollBar::rangeChanged, this,
           &C_SdNdeDpListsTreeWidget::m_ScrollBarRangeChanged);
   //Error handling
   connect(&this->mc_ErrorManager, &C_SdNdeDpListErrorManager::SigErrorChange, this,
           &C_SdNdeDpListsTreeWidget::SigErrorChange);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListsTreeWidget::~C_SdNdeDpListsTreeWidget(void)
{
   m_StoreUserSettings();

   //element cleanup handled by Qt engine; just NULLing here to have a defined state
   mpc_ContextMenu    = NULL;
   mpc_ActionAdd      = NULL;
   mpc_ActionMoveUp   = NULL;
   mpc_ActionMoveDown = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set new data pool

   \param[in]  oru32_NodeIndex      Node index
   \param[in]  oru32_DataPoolIndex  Data pool index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::SetDataPool(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex)
{
   const uint16_t u16_TimerId = osc_write_log_performance_start();

   if (this->mq_InitialUserSettings == true)
   {
      this->mq_InitialUserSettings = false;
   }
   else
   {
      //Might be problematic if deleting/adding new datapools
      m_StoreUserSettings();
   }
   this->mu32_NodeIndex = oru32_NodeIndex;
   this->mu32_DataPoolIndex = oru32_DataPoolIndex;
   this->clear();
   this->mc_UndoManager.clear();
   this->mc_ModelViewManager.Clear();
   this->mc_ErrorManager.Init(this->mu32_NodeIndex, this->mu32_DataPoolIndex);
   this->mc_Delegate.SetDataPool(this->mu32_NodeIndex, this->mu32_DataPoolIndex);
   this->mc_Delegate.SetMaximumHeight(this->height());
   m_InitFromData();
   m_RestoreUserSettings();

   osc_write_log_performance_stop(
      u16_TimerId, static_cast<QString>("Switch list tree to Datapool %1").arg(
         this->mu32_DataPoolIndex).toStdString().c_str());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add new list
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::AddEntry(void)
{
   //Top level
   const int32_t s32_Index = this->topLevelItemCount();
   QTreeWidgetItem * const pc_Item = new QTreeWidgetItem(static_cast<int32_t>(QTreeWidgetItem::ItemType::UserType));

   this->addTopLevelItem(pc_Item);

   m_InitialItemConfigure(pc_Item, s32_Index);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update widgets from data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::UpdateUi(void)
{
   const C_OscNodeDataPool * const pc_OscDataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
      this->mu32_NodeIndex, this->mu32_DataPoolIndex);

   if (pc_OscDataPool != NULL)
   {
      if (static_cast<uint32_t>(this->topLevelItemCount()) == pc_OscDataPool->c_Lists.size())
      {
         // update every list because of error handling
         for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
         {
            QTreeWidgetItem * const pc_HeaderItem = this->topLevelItem(s32_It);

            if (pc_HeaderItem != NULL)
            {
               //Header

               C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
                  dynamic_cast<C_SdNdeDpListHeaderWidget *>(this->itemWidget(pc_HeaderItem, 0));

               //Update content
               if (pc_HeaderWidget == NULL)
               {
                  m_InitialItemConfigure(pc_HeaderItem, static_cast<uint32_t>(s32_It));
               }
               else
               {
                  pc_HeaderWidget->SetIndex(static_cast<uint32_t>(s32_It));
               }
               //Table
               if (pc_HeaderItem->childCount() > 0)
               {
                  QTreeWidgetItem * const pc_TableItem = pc_HeaderItem->child(0);
                  if (pc_TableItem != NULL)
                  {
                     C_SdNdeDpListTableWidget * const pc_TableWidget =
                        dynamic_cast<C_SdNdeDpListTableWidget * const>(this->itemWidget(pc_TableItem, 0));

                     //Update content
                     if (pc_TableWidget != NULL)
                     {
                        pc_TableWidget->SetList(this->mu32_NodeIndex, this->mu32_DataPoolIndex, s32_It);
                     }
                  }
               }
            }
         }
      }
   }
   //Do item layout to update the item sizes (Size hint!)
   this->doItemsLayout();
   //Signals
   Q_EMIT (this->SigSizeChange());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert new row at specified row index

   \param[in]  oru32_TargetRow   Row index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::InsertRowWithoutData(const uint32_t & oru32_TargetRow)
{
   QTreeWidgetItem * const pc_Item = new QTreeWidgetItem(static_cast<int32_t>(QTreeWidgetItem::ItemType::UserType));

   this->insertTopLevelItem(oru32_TargetRow, pc_Item);
   m_InitialItemConfigure(pc_Item, oru32_TargetRow);
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy tree item to clipboard
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::Copy(void) const
{
   const uint16_t u16_TimerId = osc_write_log_performance_start();
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->Copy();
   }
   else
   {
      const C_OscNodeDataPool * const pc_DataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
         this->mu32_NodeIndex,
         this->mu32_DataPoolIndex);
      if (pc_DataPool != NULL)
      {
         std::vector<C_OscNodeDataPoolList> c_OscContentVec;
         std::vector<C_PuiSdNodeDataPoolList> c_UiContentVec;
         std::vector<uint32_t> c_SelectedIndices = m_GetSelectedIndices();

         //Sort to have "correct" copy order
         C_SdUtil::h_SortIndicesAscending(c_SelectedIndices);
         //Reserve
         c_OscContentVec.resize(c_SelectedIndices.size());
         c_UiContentVec.resize(c_SelectedIndices.size());
         for (uint32_t u32_ItIndex = 0; u32_ItIndex < c_SelectedIndices.size(); ++u32_ItIndex)
         {
            tgl_assert(C_PuiSdHandler::h_GetInstance()->GetDataPoolList(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                                        c_SelectedIndices[u32_ItIndex],
                                                                        c_OscContentVec[u32_ItIndex],
                                                                        c_UiContentVec[u32_ItIndex]) == C_NO_ERR);
         }
         C_SdClipBoardHelper::h_StoreDataPoolLists(c_OscContentVec, c_UiContentVec, pc_DataPool->e_Type);
      }
   }
   osc_write_log_performance_stop(u16_TimerId, "Copy datapool list");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy tree item to clipboard and delete it afterwards
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::Cut(void)
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->Cut();
   }
   else
   {
      Copy();
      Delete();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paste tree item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::Paste(void)
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->Paste();
   }
   else
   {
      //Last selected index
      const uint32_t u32_LastIndex = m_GetOneAfterHighestSelected();
      this->mc_UndoManager.DoPaste(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this, u32_LastIndex);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete tree item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::Delete(void)
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->Delete();
   }
   else
   {
      const std::vector<uint32_t> c_Indices = m_GetSelectedIndices();
      this->mc_UndoManager.DoDeleteList(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this,
                                        c_Indices);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert triggered via action
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::InsertAction(void)
{
   Insert();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert tree item

   \param[in]  orq_SetFocus   Optional flag if inital focus should be set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::Insert(const bool & orq_SetFocus)
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->Insert(orq_SetFocus);
   }
   else
   {
      if (this->mq_AllowAdd == true)
      {
         std::vector<uint32_t> c_Indices;
         const uint32_t u32_TargetIndex = m_GetOneAfterHighestSelected();
         c_Indices.push_back(u32_TargetIndex);

         this->mc_UndoManager.DoAddList(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this,
                                        c_Indices);
         //Set initial focus
         if (orq_SetFocus == true)
         {
            QTreeWidgetItem * const pc_TreeWidgetItem =
               this->itemFromIndex(this->model()->index(static_cast<int32_t>(u32_TargetIndex), 0));
            tgl_assert(pc_TreeWidgetItem != NULL);
            if (pc_TreeWidgetItem != NULL)
            {
               C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
                  dynamic_cast<C_SdNdeDpListHeaderWidget * >(this->itemWidget(pc_TreeWidgetItem, 0));
               tgl_assert((pc_HeaderWidget != NULL) && (pc_TreeWidgetItem->isSelected() == true));
               if (pc_HeaderWidget != NULL)
               {
                  pc_HeaderWidget->SetEditFocus();
               }
            }
         }
      }
      else
      {
         C_OgeWiCustomMessage c_MessageBox(this, C_OgeWiCustomMessage::E_Type::eERROR);
         c_MessageBox.SetDescription(static_cast<QString>(C_GtGetText::h_GetText(
                                                             "Only %1 lists allowed per Datapool.")).arg(
                                        C_OscNode::hu32_MAX_NUMBER_OF_LISTS_PER_DATA_POOL));
         c_MessageBox.SetCustomMinHeight(180, 180);
         c_MessageBox.Execute();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move selected item up by one slot
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::DoMoveUp(void)
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->DoMoveUp();
   }
   else
   {
      if (this->mq_AllowMoveUp == true)
      {
         bool q_AllowMove = true;

         std::vector<uint32_t> c_TargetIndices;
         std::vector<uint32_t> c_SourceIndices = m_GetSelectedIndices();
         c_TargetIndices.resize(c_SourceIndices.size());

         //Sort to have "correct" move order
         C_SdUtil::h_SortIndicesAscending(c_SourceIndices);
         for (uint32_t u32_ItItem = 0; u32_ItItem < c_TargetIndices.size(); ++u32_ItItem)
         {
            //Check if not first
            if (c_SourceIndices[u32_ItItem] > 0)
            {
               c_TargetIndices[u32_ItItem] = c_SourceIndices[u32_ItItem] - 1;
            }
            else
            {
               //No move
               q_AllowMove = false;
            }
         }
         if (q_AllowMove == true)
         {
            m_Move(c_SourceIndices, c_TargetIndices);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move selected item down by one slot
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::DoMoveDown(void)
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->DoMoveDown();
   }
   else
   {
      if (this->mq_AllowMoveDown == true)
      {
         bool q_AllowMove = true;

         std::vector<uint32_t> c_TargetIndices;
         std::vector<uint32_t> c_SourceIndices = m_GetSelectedIndices();
         const uint32_t u32_LastIndex = static_cast<uint32_t>(this->topLevelItemCount() - 1);
         c_TargetIndices.resize(c_SourceIndices.size());

         //Sort to have "correct" move order
         C_SdUtil::h_SortIndicesAscending(c_SourceIndices);
         for (uint32_t u32_ItItem = 0; u32_ItItem < c_TargetIndices.size(); ++u32_ItItem)
         {
            //Check if not last
            if (c_SourceIndices[u32_ItItem] < u32_LastIndex)
            {
               c_TargetIndices[u32_ItItem] = c_SourceIndices[u32_ItItem] + 1;
            }
            else
            {
               //No move
               q_AllowMove = false;
            }
         }
         if (q_AllowMove == true)
         {
            m_Move(c_SourceIndices, c_TargetIndices);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enter name edit
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::Edit(void) const
{
   C_SdNdeDpListTableWidget * const pc_Table = m_GetActiveTable();

   if (pc_Table != NULL)
   {
      pc_Table->Edit();
   }
   else
   {
      const std::vector<uint32_t> c_Selection = m_GetSelectedIndices();
      if (c_Selection.size() == 1)
      {
         const QModelIndex c_Index = this->model()->index(c_Selection[0], 0);
         QTreeWidgetItem * const pc_ItemWidget = this->itemFromIndex(c_Index);
         if (pc_ItemWidget != NULL)
         {
            C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
               dynamic_cast<C_SdNdeDpListHeaderWidget * >(this->itemWidget(pc_ItemWidget, 0));
            if (pc_HeaderWidget != NULL)
            {
               pc_HeaderWidget->SetEditFocus();
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle pop up request
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::PopUp(void) const
{
   QTreeWidgetItem * const pc_Table = m_GetActiveTableTreeWidget();

   if (pc_Table != NULL)
   {
      //Pop up header
      const QModelIndex c_Index = this->indexFromItem(pc_Table);
      if (c_Index.parent().isValid() == true)
      {
         C_SdNdeDpListHeaderWidget * const pc_Widget =
            dynamic_cast<C_SdNdeDpListHeaderWidget * const>(this->indexWidget(c_Index.parent()));

         if (pc_Widget != NULL)
         {
            pc_Widget->PopUp();
         }
      }
   }
   else
   {
      const std::vector<uint32_t> c_Selection = m_GetSelectedIndices();
      if (c_Selection.size() == 1)
      {
         const QModelIndex c_Index = this->model()->index(c_Selection[0], 0);
         QTreeWidgetItem * const pc_ItemWidget = this->itemFromIndex(c_Index);
         if (pc_ItemWidget != NULL)
         {
            C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
               dynamic_cast<C_SdNdeDpListHeaderWidget * >(this->itemWidget(pc_ItemWidget, 0));
            if (pc_HeaderWidget != NULL)
            {
               pc_HeaderWidget->PopUp();
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check node data pool full

   \return
   true  Data pool full
   false else
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SdNdeDpListsTreeWidget::CheckDataPoolFull(void) const
{
   bool q_Retval = false;
   const C_OscNodeDataPool * const pc_DataPool =
      C_PuiSdHandler::h_GetInstance()->GetOscDataPool(this->mu32_NodeIndex,
                                                      this->mu32_DataPoolIndex);

   if (pc_DataPool != NULL)
   {
      if (pc_DataPool->c_Lists.size() >= C_OscNode::hu32_MAX_NUMBER_OF_LISTS_PER_DATA_POOL)
      {
         q_Retval = true;
      }
   }
   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Function to open a concrete datapool list or dataelement

   \param[in]  os32_ListIndex          Optional list index (if not used set to -1)
   \param[in]  os32_DataElementIndex   Optional data element index (if not used set to -1)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::OpenDetail(const int32_t os32_ListIndex, const int32_t os32_DataElementIndex)
{
   QTreeWidgetItem * const pc_Item = this->topLevelItem(os32_ListIndex);
   const QList<QTreeWidgetItem *> & rc_SelectedItems = this->selectedItems();

   QList<QTreeWidgetItem *>::const_iterator c_ItItem;

   // deselect and collapse all previous lists
   for (c_ItItem = rc_SelectedItems.begin(); c_ItItem != rc_SelectedItems.end(); ++c_ItItem)
   {
      (*c_ItItem)->setSelected(false);
   }

   if (pc_Item != NULL)
   {
      if (os32_DataElementIndex < 0)
      {
         pc_Item->setSelected(true);
      }
      this->m_OnExpandRequestedIndex(os32_ListIndex, true);

      // scroll to the end first to see the expanded list always too
      this->scrollToBottom();
      this->scrollToItem(pc_Item);

      if (os32_DataElementIndex >= 0)
      {
         C_SdNdeDpListTableWidget * const pc_TableWidget =
            dynamic_cast<C_SdNdeDpListTableWidget * const>(this->itemWidget(pc_Item->child(0), 0));

         if (pc_TableWidget != NULL)
         {
            pc_TableWidget->SelectDataElement(os32_DataElementIndex);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Register model change
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::UpdateModels(void)
{
   this->mc_ModelViewManager.UpdateModels();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle error change signal of any list item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::HandleErrorChange(void)
{
   this->mc_ErrorManager.OnErrorChange();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten drop event slot

   Here: Skip old item, find new item and add widget (widget is not copied)

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::dropEvent(QDropEvent * const opc_Event)
{
   const C_SdNdeDpListsTreeWidget * const pc_Tree =
      dynamic_cast<const C_SdNdeDpListsTreeWidget * const>(opc_Event->source());

   if (pc_Tree != NULL)
   {
      bool q_AllowedMoveAction = false;
      const DropIndicatorPosition e_DropIndicator = dropIndicatorPosition();

      //Only allow drop between items
      switch (e_DropIndicator)
      {
      case QAbstractItemView::AboveItem:
         q_AllowedMoveAction = true;
         break;
      case QAbstractItemView::BelowItem:
         q_AllowedMoveAction = true;
         break;
      case QAbstractItemView::OnItem:
         q_AllowedMoveAction = true;
         break;
      case QAbstractItemView::OnViewport:
      default:
         break;
      }

      if (q_AllowedMoveAction == true)
      {
         const std::vector<uint32_t> c_SelectedIndices = this->m_GetSelectedIndices();
         std::vector<uint32_t> c_TargetIndices;
         int32_t s32_TargetRow = static_cast<int32_t>(this->indexAt(opc_Event->position().toPoint()).row());
         mh_AdaptDropTargetIndex(c_SelectedIndices, e_DropIndicator, s32_TargetRow);
         c_TargetIndices.resize(c_SelectedIndices.size());
         for (uint32_t u32_ItItem = 0; u32_ItItem < c_TargetIndices.size(); ++u32_ItItem)
         {
            c_TargetIndices[u32_ItItem] = static_cast<uint32_t>(s32_TargetRow) + u32_ItItem;
         }
         QTreeWidget::dropEvent(opc_Event);
         this->mc_UndoManager.DoMoveList(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                         this,
                                         c_SelectedIndices,
                                         c_TargetIndices);
      }
      else
      {
         setDropIndicatorShown(false); // Manually hide the drop indicator
      }
   }
   else
   {
      opc_Event->ignore();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten drag enter event slot

   Here: Accept table item type

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::dragEnterEvent(QDragEnterEvent * const opc_Event)
{
   const C_SdNdeDpListsTreeWidget * const pc_Tree =
      dynamic_cast<const C_SdNdeDpListsTreeWidget * const>(opc_Event->source());

   if (pc_Tree != NULL)
   {
      //Ignore list drag
      const int32_t s32_Delay = this->autoExpandDelay();
      this->setAutoExpandDelay(-1);
      QTreeWidget::dragEnterEvent(opc_Event);
      opc_Event->accept();
      this->setAutoExpandDelay(s32_Delay);
   }
   else
   {
      const C_SdNdeDpListTableView * const pc_Table =
         dynamic_cast<const C_SdNdeDpListTableView * const>(opc_Event->source());

      if (pc_Table != NULL)
      {
         QTreeWidget::dragEnterEvent(opc_Event);
         opc_Event->accept();
      }
      else
      {
         opc_Event->ignore();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten drag move event slot

   Here: selective deactivation of autoexpand feature

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::dragMoveEvent(QDragMoveEvent * const opc_Event)
{
   const C_SdNdeDpListsTreeWidget * const pc_Tree =
      dynamic_cast<const C_SdNdeDpListsTreeWidget * const>(opc_Event->source());

   if (pc_Tree != NULL)
   {
      //Ignore list drag
      const int32_t s32_Delay = this->autoExpandDelay();
      this->setAutoExpandDelay(-1);
      QTreeWidget::dragMoveEvent(opc_Event);
      opc_Event->accept();
      this->setAutoExpandDelay(s32_Delay);
   }
   else
   {
      const C_SdNdeDpListTableView * const pc_Table =
         dynamic_cast<const C_SdNdeDpListTableView * const>(opc_Event->source());

      if (pc_Table != NULL)
      {
         QTreeWidget::dragMoveEvent(opc_Event);
         opc_Event->ignore();
      }
      else
      {
         opc_Event->ignore();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten selection changed event slot

   Here: minimize last selected item & signal selection change

   \param[in]  orc_Selected      New selected items
   \param[in]  orc_Deselected    Last selected items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::selectionChanged(const QItemSelection & orc_Selected,
                                                const QItemSelection & orc_Deselected)
{
   bool q_IsTopLevel = true;
   const QModelIndexList c_DeselectedIndexList = orc_Deselected.indexes();
   const QModelIndexList c_SelectedIndexList = orc_Selected.indexes();

   QTreeWidget::selectionChanged(orc_Selected, orc_Deselected);

   //Check if top level
   for (QModelIndexList::const_iterator c_ItIndex = c_SelectedIndexList.begin();
        c_ItIndex != c_SelectedIndexList.end();
        ++c_ItIndex)
   {
      if (c_ItIndex->parent().isValid() == true)
      {
         q_IsTopLevel = false;
      }
   }

   //If top level collapse
   if ((q_IsTopLevel == true) && (c_SelectedIndexList.size() > 0))
   {
      m_ClearTableSelection();
   }
   m_CheckActions();
   //Notify selection on top level
   for (QModelIndexList::const_iterator c_ItIndex = c_DeselectedIndexList.begin();
        c_ItIndex != c_DeselectedIndexList.end();
        ++c_ItIndex)
   {
      C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
         dynamic_cast<C_SdNdeDpListHeaderWidget * >(this->itemWidget(this->itemFromIndex(*c_ItIndex), 0));
      if (pc_HeaderWidget != NULL)
      {
         pc_HeaderWidget->NotifySelection(false);
      }
   }
   for (QModelIndexList::const_iterator c_ItIndex = c_SelectedIndexList.begin();
        c_ItIndex != c_SelectedIndexList.end();
        ++c_ItIndex)
   {
      C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
         dynamic_cast<C_SdNdeDpListHeaderWidget * >(this->itemWidget(this->itemFromIndex(*c_ItIndex), 0));
      if (pc_HeaderWidget != NULL)
      {
         pc_HeaderWidget->NotifySelection(true);
      }
   }

   //Notify parent
   Q_EMIT this->SigSelectionChanged(m_GetSelectedIndices().size(), true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: handle list paste

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::keyPressEvent(QKeyEvent * const opc_Event)
{
   bool q_CallOrig = true;

   //Debug commands
   if (((opc_Event->modifiers().testFlag(Qt::ShiftModifier) == true) &&
        (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)) &&
       (opc_Event->modifiers().testFlag(Qt::AltModifier) == true))
   {
      if (static_cast<Qt::Key>(opc_Event->key()) == Qt::Key_O)
      {
         //Undo redo command view
         this->mc_UndoManager.ToggleCommandDisplay();
      }
   }
   //Standard commands
   switch (opc_Event->key())
   {
   case Qt::Key_C:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         Copy();
      }
      break;
   case Qt::Key_X:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         Cut();
      }
      break;
   case Qt::Key_V:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         Paste();
      }
      break;
   case Qt::Key_Z:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         this->mc_UndoManager.DoUndo();
      }
      break;
   case Qt::Key_Y:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         this->mc_UndoManager.DoRedo();
      }
      break;
   case Qt::Key_Delete:
      Delete();
      break;
   case Qt::Key_BracketRight: // Qt::Key_BracketRight matches the "Not-Num-Plus"-Key
   case Qt::Key_Plus:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         Insert(false);
      }
      break;
   case Qt::Key_Up:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         DoMoveUp();
      }
      break;
   case Qt::Key_Down:
      if (opc_Event->modifiers().testFlag(Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         DoMoveDown();
      }
      break;
   case Qt::Key_F2:
      Edit();
      q_CallOrig = false;
      break;
   case Qt::Key_F11:
      PopUp();
      break;
   default:
      //Nothing to do
      break;
   }

   if (q_CallOrig == true)
   {
      QTreeWidget::keyPressEvent(opc_Event);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle expand request

   \param[in]  opc_Item    Affected tree item widget
   \param[in]  oq_Expand   true: expand
                           false collapse
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_OnExpandRequestedHeader(const C_SdNdeDpListHeaderWidget * const opc_Item,
                                                         const bool oq_Expand)
{
   int32_t s32_It = 0UL;

   for (; s32_It < this->topLevelItemCount(); ++s32_It)
   {
      if (this->itemWidget(this->topLevelItem(s32_It), 0) == opc_Item)
      {
         break;
      }
   }
   m_OnExpandRequestedIndex(s32_It, oq_Expand);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle expand request

   \param[in]  os32_Index  Index
   \param[in]  oq_Expand   true: expand
                           false collapse
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_OnExpandRequestedIndex(const int32_t os32_Index, const bool oq_Expand)
{
   if (oq_Expand == true)
   {
      //Do not use the item interface "expandItem" (does not work)
      this->expand(this->indexFromItem(this->topLevelItem(os32_Index)));
   }
   else
   {
      //Do not use the item interface "collapseItem" (does not work)
      this->collapse(this->indexFromItem(this->topLevelItem(os32_Index)));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set up new tree item widget

   \param[in,out]  opc_Item      Tree item widget to set up
   \param[in]      os32_Index    According index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_InitialItemConfigure(QTreeWidgetItem * const opc_Item, const int32_t os32_Index)
{
   C_SdNdeDpListHeaderWidget * const pc_ListItem = new C_SdNdeDpListHeaderWidget(NULL, this,
                                                                                 &this->mc_UndoManager,
                                                                                 &this->mc_ModelViewManager,
                                                                                 this->mu32_NodeIndex,
                                                                                 this->mu32_DataPoolIndex,
                                                                                 os32_Index);

   connect(pc_ListItem, &C_SdNdeDpListHeaderWidget::SigSave, this,
           &C_SdNdeDpListsTreeWidget::SigSave);
   connect(pc_ListItem, &C_SdNdeDpListHeaderWidget::SigSaveAs, this,
           &C_SdNdeDpListsTreeWidget::SigSaveAs);
   connect(pc_ListItem, &C_SdNdeDpListHeaderWidget::SigExpand, this,
           &C_SdNdeDpListsTreeWidget::m_OnExpandRequestedHeader);
   //Connect error change
   connect(pc_ListItem, &C_SdNdeDpListHeaderWidget::SigErrorChange, this,
           &C_SdNdeDpListsTreeWidget::HandleErrorChange);
   //Selection handling
   connect(pc_ListItem, &C_SdNdeDpListHeaderWidget::SigExclusiveSelection, this,
           &C_SdNdeDpListsTreeWidget::m_HandleExclusiveListSelection);

   this->setItemWidget(opc_Item, 0, pc_ListItem);
   {
      //Table
      QTreeWidgetItem * const pc_Table =
         new QTreeWidgetItem(opc_Item, static_cast<int32_t>(QTreeWidgetItem::ItemType::UserType));
      C_SdNdeDpListTableWidget * const pc_TableWidget = new C_SdNdeDpListTableWidget(NULL, this,
                                                                                     &this->mc_UndoManager,
                                                                                     false);

      //Set the current model view manager!
      pc_TableWidget->SetModelViewManager(&this->mc_ModelViewManager);

      //Connect error change
      connect(pc_TableWidget, &C_SdNdeDpListTableWidget::SigErrorChangePossible, pc_ListItem,
              &C_SdNdeDpListHeaderWidget::UpdateUi);
      //Update header on internal usage change
      connect(pc_TableWidget, &C_SdNdeDpListTableWidget::SigSizeChangePossible, this,
              &C_SdNdeDpListsTreeWidget::UpdateUi);
      //Table selection
      connect(pc_TableWidget, &C_SdNdeDpListTableWidget::SigSelectionChanged, this,
              &C_SdNdeDpListsTreeWidget::m_HandleTableSelection);
      //Table button status
      connect(pc_TableWidget, &C_SdNdeDpListTableWidget::SigButtonChange, this,
              &C_SdNdeDpListsTreeWidget::m_OnButtonChange);
      //Reset after pop up
      connect(pc_ListItem, &C_SdNdeDpListHeaderWidget::SigUpdateTable, pc_TableWidget,
              &C_SdNdeDpListTableWidget::Reset);

      this->setItemWidget(pc_Table, 0, pc_TableWidget);
   } //lint !e429  //no memory leak because setItemWidget takes ownership
}    //lint !e429  //no memory leak because setItemWidget takes ownership

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move item in tree

   \param[in]  orc_SourceIndices  Source index
   \param[in]  orc_TargetIndices  Target index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_Move(const std::vector<uint32_t> & orc_SourceIndices,
                                      const std::vector<uint32_t> & orc_TargetIndices)
{
   const uint16_t u16_TimerId = osc_write_log_performance_start();

   this->mc_UndoManager.DoMoveList(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this,
                                   orc_SourceIndices, orc_TargetIndices);
   // for update of table sizes:
   this->UpdateModels();
   osc_write_log_performance_stop(u16_TimerId, "Move list");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init from data pool data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_InitFromData(void)
{
   const C_OscNodeDataPool * const pc_OscDataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
      this->mu32_NodeIndex,
      this->mu32_DataPoolIndex);

   if (pc_OscDataPool != NULL)
   {
      for (uint32_t u32_It = 0; u32_It < pc_OscDataPool->c_Lists.size(); ++u32_It)
      {
         this->AddEntry();
         {
            QTreeWidgetItem * const pc_HeaderItem = this->topLevelItem(u32_It);

            if (pc_HeaderItem != NULL)
            {
               C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
                  dynamic_cast<C_SdNdeDpListHeaderWidget * const>(this->itemWidget(pc_HeaderItem, 0));

               if (pc_HeaderWidget != NULL)
               {
                  //Init
                  pc_HeaderWidget->SetIndex(u32_It);
               }
               //Table
               if (pc_HeaderItem->childCount() > 0)
               {
                  QTreeWidgetItem * const pc_TableItem = pc_HeaderItem->child(0);
                  if (pc_TableItem != NULL)
                  {
                     C_SdNdeDpListTableWidget * const pc_TableWidget =
                        dynamic_cast<C_SdNdeDpListTableWidget * const>(this->itemWidget(pc_TableItem, 0));

                     if (pc_TableWidget != NULL)
                     {
                        pc_TableWidget->SetList(this->mu32_NodeIndex, this->mu32_DataPoolIndex, u32_It);
                     }
                  }
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Show custom context menu

   \param[in]  orc_Pos  Local context menu position
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_OnCustomContextMenuRequested(const QPoint & orc_Pos)
{
   const QModelIndex c_Current = this->indexAt(orc_Pos);

   if ((c_Current.isValid() == true) && (c_Current.parent().isValid() == false))
   {
      this->mpc_ContextMenu->popup(this->mapToGlobal(orc_Pos));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init context menu entries
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_SetupContextMenu(void)
{
   this->mpc_ContextMenu = new C_OgeContextMenu(this);
   this->mpc_ActionAdd = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Add new List"),
                                                          static_cast<int32_t>(Qt::CTRL) +
                                                          static_cast<int32_t>(Qt::Key_Plus),
                                                          this,
                                                          &C_SdNdeDpListsTreeWidget::InsertAction);

   this->mpc_ContextMenu->addSeparator();

   this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Cut"),
                                    static_cast<int32_t>(Qt::CTRL) + static_cast<int32_t>(Qt::Key_X),
                                    this,
                                    &C_SdNdeDpListsTreeWidget::Cut);
   this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Copy"),
                                    static_cast<int32_t>(Qt::CTRL) + static_cast<int32_t>(Qt::Key_C),
                                    this,
                                    &C_SdNdeDpListsTreeWidget::Copy);
   this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Paste"),
                                    static_cast<int32_t>(Qt::CTRL) + static_cast<int32_t>(Qt::Key_V),
                                    this,
                                    &C_SdNdeDpListsTreeWidget::Paste);

   this->mpc_ContextMenu->addSeparator();

   this->mpc_ActionMoveUp = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Move Up"),
                                                             static_cast<int32_t>(Qt::CTRL) +
                                                                 static_cast<int32_t>(Qt::Key_Up),
                                                             this,
                                                             &C_SdNdeDpListsTreeWidget::DoMoveUp);
   this->mpc_ActionMoveDown = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                                  "Move Down"),
                                                               static_cast<int32_t>(Qt::CTRL) +
                                                                   static_cast<int32_t>(Qt::Key_Down),
                                                               this,
                                                               &C_SdNdeDpListsTreeWidget::DoMoveDown);

   this->mpc_ContextMenu->addSeparator();

   this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Delete"),
                                    static_cast<int32_t>(Qt::Key_Delete),
                                    this,
                                    &C_SdNdeDpListsTreeWidget::Delete);

   this->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(this, &C_SdNdeDpListsTreeWidget::customContextMenuRequested, this,
           &C_SdNdeDpListsTreeWidget::m_OnCustomContextMenuRequested);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle collapse event

   \param[in]  orc_Index   Collapsed item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_OnCollapse(const QModelIndex & orc_Index) const
{
   C_SdNdeDpListHeaderWidget * const pc_Widget =
      dynamic_cast<C_SdNdeDpListHeaderWidget * const>(this->indexWidget(orc_Index));

   if (pc_Widget != NULL)
   {
      pc_Widget->RegisterExpandOrCollapse(false);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle expand event

   \param[in]  orc_Index   Expanded item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_OnExpand(const QModelIndex & orc_Index) const
{
   C_SdNdeDpListHeaderWidget * const pc_Widget =
      dynamic_cast<C_SdNdeDpListHeaderWidget * const>(this->indexWidget(orc_Index));

   if (pc_Widget != NULL)
   {
      pc_Widget->RegisterExpandOrCollapse(true);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle error change signal of any data set item

   \param[in]  oru32_NodeIndex      Node index
   \param[in]  oru32_DataPoolIndex  Data pool index
   \param[in]  oru32_ListIndex      List index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_HandleDataSetErrorChange(const uint32_t & oru32_NodeIndex,
                                                          const uint32_t & oru32_DataPoolIndex,
                                                          const uint32_t & oru32_ListIndex) const
{
   QTreeWidgetItem * const pc_HeaderItem = this->topLevelItem(oru32_ListIndex);

   tgl_assert((oru32_NodeIndex == this->mu32_NodeIndex) && (oru32_DataPoolIndex == this->mu32_DataPoolIndex));

   if (pc_HeaderItem != NULL)
   {
      C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
         dynamic_cast<C_SdNdeDpListHeaderWidget * const>(this->itemWidget(pc_HeaderItem, 0));
      if (pc_HeaderWidget != NULL)
      {
         pc_HeaderWidget->CheckError();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle table selection change

   \param[in]  oru32_ListIndex   Source list
   \param[in]  oru32_Count       Number of selected items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_HandleTableSelection(const uint32_t & oru32_ListIndex, const uint32_t & oru32_Count)
{
   if (oru32_Count > 0)
   {
      this->clearSelection();
      m_ClearTableSelection(static_cast<int32_t>(oru32_ListIndex));
   }
   Q_EMIT this->SigSelectionChanged(oru32_Count, false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get selected indices

   \return
   Selected indices
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint32_t> C_SdNdeDpListsTreeWidget::m_GetSelectedIndices(void) const
{
   std::vector<uint32_t> c_Retval;
   const QList<QTreeWidgetItem *> c_SelectedItems = this->selectedItems();

   for (QList<QTreeWidgetItem *>::const_iterator c_ItSelectedItem = c_SelectedItems.begin();
        c_ItSelectedItem != c_SelectedItems.end(); ++c_ItSelectedItem)
   {
      QTreeWidgetItem * const pc_Item = *c_ItSelectedItem;
      if (pc_Item->parent() == NULL)
      {
         c_Retval.push_back(this->indexOfTopLevelItem(pc_Item));
      }
   }
   C_Uti::h_Uniqueify(c_Retval);
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Forward signal

   \param[in]  orq_AddActive        Push button add active
   \param[in]  orq_CutActive        Push button cut active
   \param[in]  orq_CopyActive       Push button copy active
   \param[in]  orq_PasteActive      Push button paste active
   \param[in]  orq_DeleteActive     Push button delete active
   \param[in]  orq_MoveDownActive   Push button move down active
   \param[in]  orq_MoveUpActive     Push button move up active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_OnButtonChange(const bool & orq_AddActive, const bool & orq_CutActive,
                                                const bool & orq_CopyActive, const bool & orq_PasteActive,
                                                const bool & orq_DeleteActive, const bool & orq_MoveDownActive,
                                                const bool & orq_MoveUpActive)
{
   Q_EMIT this->SigButtonChange(orq_AddActive, orq_CutActive, orq_CopyActive, orq_PasteActive, orq_DeleteActive,
                                orq_MoveDownActive, orq_MoveUpActive);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Recheck if actions statuses are still up to date
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_CheckActions(void)
{
   const std::vector<uint32_t> c_SelectedIndices = m_GetSelectedIndices();

   if (c_SelectedIndices.size() > 0)
   {
      bool q_AllowAdd = true;
      bool q_AllowMoveUp;
      bool q_AllowMoveDown;

      //Add
      if (CheckDataPoolFull() == true)
      {
         q_AllowAdd = false;
      }
      this->mq_AllowAdd = q_AllowAdd;
      //Move down & up
      q_AllowMoveUp = true;
      q_AllowMoveDown = true;
      for (uint32_t u32_ItSelectedIndex = 0UL; u32_ItSelectedIndex < c_SelectedIndices.size(); ++u32_ItSelectedIndex)
      {
         if (c_SelectedIndices[u32_ItSelectedIndex] == 0UL)
         {
            q_AllowMoveUp = false;
         }
         if (c_SelectedIndices[u32_ItSelectedIndex] == static_cast<uint32_t>(this->topLevelItemCount() - 1))
         {
            q_AllowMoveDown = false;
         }
      }
      //Actions
      this->mq_AllowMoveDown = q_AllowMoveDown;
      if (this->mpc_ActionMoveDown != NULL)
      {
         this->mpc_ActionMoveDown->setEnabled(q_AllowMoveDown);
      }
      this->mq_AllowMoveUp = q_AllowMoveUp;
      if (this->mpc_ActionMoveUp != NULL)
      {
         this->mpc_ActionMoveUp->setEnabled(q_AllowMoveUp);
      }
      //Buttons
      Q_EMIT this->SigButtonChange(true, true, true, true, true, q_AllowMoveDown, q_AllowMoveUp);
   }
   else
   {
      //Buttons
      Q_EMIT this->SigButtonChange(true, false, false, false, false, false, false);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get active table

   \param[in]  orq_IgnoreSelectedItems    Optional indicator if selected item size should be ignored

   \return
   NULL No active table
   else Active table
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListTableWidget * C_SdNdeDpListsTreeWidget::m_GetActiveTable(const bool & orq_IgnoreSelectedItems)
const
{
   C_SdNdeDpListTableWidget * pc_Retval = NULL;
   bool q_Continue = true;

   //If requested by user ignore list size
   if (orq_IgnoreSelectedItems == false)
   {
      if (m_GetSelectedIndices().size() > 0)
      {
         q_Continue = false;
      }
   }
   if (q_Continue == true)
   {
      //Search for collapsed list
      for (int32_t s32_ItList = 0; s32_ItList < static_cast<int32_t>(this->topLevelItemCount()); ++s32_ItList)
      {
         const QTreeWidgetItem * const pc_ListItem = this->topLevelItem(static_cast<int32_t>(s32_ItList));
         if (pc_ListItem != NULL)
         {
            if (pc_ListItem->isExpanded())
            {
               QTreeWidgetItem * const pc_TableWidgetItem = pc_ListItem->child(0);
               if (pc_TableWidgetItem != NULL)
               {
                  C_SdNdeDpListTableWidget * const pc_Table =
                     dynamic_cast<C_SdNdeDpListTableWidget *>(this->itemWidget(pc_TableWidgetItem, 0));
                  if ((pc_Table != NULL) && (pc_Table->IsSelected() == true))
                  {
                     pc_Retval = pc_Table;
                  }
               }
            }
         }
      }
   }
   return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get active table tree widget item

   \param[in]  orq_IgnoreSelectedItems    Optional indicator if selected item size should be ignored

   \return
   NULL No active table tree widget item
   Else Active table tree widget item
*/
//----------------------------------------------------------------------------------------------------------------------
QTreeWidgetItem * C_SdNdeDpListsTreeWidget::m_GetActiveTableTreeWidget(const bool & orq_IgnoreSelectedItems) const
{
   QTreeWidgetItem * pc_Retval = NULL;
   bool q_Continue = true;

   //If requested by user ignore list size
   if (orq_IgnoreSelectedItems == false)
   {
      if (m_GetSelectedIndices().size() > 0)
      {
         q_Continue = false;
      }
   }
   if (q_Continue == true)
   {
      //Search for collapsed list
      for (int32_t s32_ItList = 0; s32_ItList < static_cast<int32_t>(this->topLevelItemCount()); ++s32_ItList)
      {
         const QTreeWidgetItem * const pc_ListItem = this->topLevelItem(static_cast<int32_t>(s32_ItList));
         if (pc_ListItem != NULL)
         {
            if (pc_ListItem->isExpanded())
            {
               QTreeWidgetItem * const pc_TableWidgetItem = pc_ListItem->child(0);
               if (pc_TableWidgetItem != NULL)
               {
                  C_SdNdeDpListTableWidget * const pc_Table =
                     dynamic_cast<C_SdNdeDpListTableWidget *>(this->itemWidget(pc_TableWidgetItem, 0));
                  if ((pc_Table != NULL) && (pc_Table->IsSelected() == true))
                  {
                     pc_Retval = pc_TableWidgetItem;
                  }
               }
            }
         }
      }
   }
   return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt target position of drop action

   \param[in]      orc_SelectedIndices          Selected indices
   \param[in]      ore_DropIndicatorPosition    Drop indicator position
   \param[in,out]  ors32_TargetPosition         Target position (Requirement: valid initialization)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::mh_AdaptDropTargetIndex(const std::vector<uint32_t> & orc_SelectedIndices,
                                                       const QAbstractItemView::DropIndicatorPosition & ore_DropIndicatorPosition,
                                                       int32_t & ors32_TargetPosition)
{
   int32_t s32_Offset = 0;
   bool q_FirstSelected = false;

   //If drag below inserted adapt target row
   for (uint32_t u32_ItSelected = 0; u32_ItSelected < orc_SelectedIndices.size(); ++u32_ItSelected)
   {
      if (static_cast<int32_t>(orc_SelectedIndices[u32_ItSelected]) <= ors32_TargetPosition)
      {
         ++s32_Offset;
      }
      if (orc_SelectedIndices[u32_ItSelected] == 0)
      {
         q_FirstSelected = true;
      }
   }
   //Check if first item is dropped on first item
   //  -> no index adaptations necessary
   if ((q_FirstSelected == true) && (ors32_TargetPosition == 0))
   {
      //No adaptation necessary
   }
   else
   {
      ors32_TargetPosition -= s32_Offset;
      //If indicator below item insert at next index
      if (ore_DropIndicatorPosition != QAbstractItemView::AboveItem)
      {
         ++ors32_TargetPosition;
      }
   }
   //Always check if it is positive
   ors32_TargetPosition = std::max(ors32_TargetPosition, static_cast<int32_t>(0L));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle change detected
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_HandleChanged(void)
{
   Q_EMIT this->SigChanged();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle data set size change

   \param[in]  oru32_NodeIndex      Node index
   \param[in]  oru32_DataPoolIndex  Data pool index
   \param[in]  oru32_ListIndex      List index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_UpdateDataSetCount(const uint32_t & oru32_NodeIndex,
                                                    const uint32_t & oru32_DataPoolIndex,
                                                    const uint32_t & oru32_ListIndex) const
{
   if ((this->mu32_NodeIndex == oru32_NodeIndex) && (this->mu32_DataPoolIndex == oru32_DataPoolIndex))
   {
      QTreeWidgetItem * const pc_TreeWidgetItem = this->topLevelItem(static_cast<uint32_t>(oru32_ListIndex));
      if (pc_TreeWidgetItem != NULL)
      {
         C_SdNdeDpListHeaderWidget * const pc_HeaderWidget =
            dynamic_cast<C_SdNdeDpListHeaderWidget * const>(this->indexWidget(this->indexFromItem(
                                                                                 pc_TreeWidgetItem)));
         if (pc_HeaderWidget != NULL)
         {
            pc_HeaderWidget->UpdateDataSetCount();
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_ScrollBarRangeChanged(const int32_t os32_Min, const int32_t os32_Max) const
{
   // manual showing and hiding of the scrollbar to stop resizing the parent widget when showing or hiding the scrollbar
   if ((os32_Min == 0) && (os32_Max == 0))
   {
      this->verticalScrollBar()->hide();
   }
   else
   {
      this->verticalScrollBar()->show();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get index of element after highest selected element

   \return
   Index of element after highest selected element
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SdNdeDpListsTreeWidget::m_GetOneAfterHighestSelected(void) const
{
   uint32_t u32_Retval = 0;
   const std::vector<uint32_t> c_SelectedItems = this->m_GetSelectedIndices();

   if (c_SelectedItems.size() > 0)
   {
      for (uint32_t u32_ItSelected = 0; u32_ItSelected < c_SelectedItems.size(); ++u32_ItSelected)
      {
         const uint32_t u32_Cur = c_SelectedItems[u32_ItSelected];
         u32_Retval = std::max(u32_Retval, u32_Cur);
      }
      ++u32_Retval;
   }
   else
   {
      if (this->topLevelItemCount() >= 0)
      {
         u32_Retval = static_cast<uint32_t>(this->topLevelItemCount());
      }
   }
   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Disable all table selections

   \param[in]  ors32_Exception   Optional parameter to allow one table to not have its selection cleared
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_ClearTableSelection(const int32_t & ors32_Exception) const
{
   for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
   {
      if (ors32_Exception != s32_It)
      {
         QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
         if (pc_TopLevelItem != NULL)
         {
            QTreeWidgetItem * const pc_TableWidgetItem = pc_TopLevelItem->child(0);
            if (pc_TableWidgetItem != NULL)
            {
               C_SdNdeDpListTableWidget * const pc_Table =
                  dynamic_cast<C_SdNdeDpListTableWidget * const>(this->itemWidget(pc_TableWidgetItem, 0));

               //Clear table selection
               if (pc_Table != NULL)
               {
                  pc_Table->ClearSelection();
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle exclusive list selection

   \param[in]  oru32_ListIndex   List index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_HandleExclusiveListSelection(const uint32_t & oru32_ListIndex) const
{
   for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
   {
      QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
      if (pc_TopLevelItem != NULL)
      {
         if (static_cast<int32_t>(oru32_ListIndex) != s32_It)
         {
            pc_TopLevelItem->setSelected(false);
         }
         else
         {
            pc_TopLevelItem->setSelected(true);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Store user settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_StoreUserSettings(void) const
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
   const C_OscNodeDataPool * const pc_Datapool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(this->mu32_NodeIndex,
                                                                                                 this->mu32_DataPoolIndex);

   if ((pc_Node != NULL) && (pc_Datapool != NULL))
   {
      std::vector<QString> c_ExpandedListNames;
      std::vector<QString> c_SelectedListNames;
      std::vector<QString> c_SelectedVariableNames;

      //Expanded items
      for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
      {
         QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
         if ((pc_TopLevelItem != NULL) && (pc_TopLevelItem->isExpanded() == true))
         {
            const uint32_t u32_ListIndex = static_cast<uint32_t>(s32_It);
            if (u32_ListIndex < pc_Datapool->c_Lists.size())
            {
               const C_OscNodeDataPoolList & rc_List = pc_Datapool->c_Lists[u32_ListIndex];
               c_ExpandedListNames.emplace_back(rc_List.c_Name.c_str());
            }
         }
      }
      C_UsHandler::h_GetInstance()->SetProjSdNodeDatapoolOpenListNames(pc_Node->c_Properties.c_Name.c_str(),
                                                                       pc_Datapool->c_Name.c_str(),
                                                                       c_ExpandedListNames);

      //Selected items
      for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
      {
         QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
         if (pc_TopLevelItem != NULL)
         {
            if (pc_TopLevelItem->isSelected() == true)
            {
               const uint32_t u32_ListIndex = static_cast<uint32_t>(s32_It);
               //Lists
               if (u32_ListIndex < pc_Datapool->c_Lists.size())
               {
                  const C_OscNodeDataPoolList & rc_List = pc_Datapool->c_Lists[u32_ListIndex];
                  c_SelectedListNames.emplace_back(rc_List.c_Name.c_str());
               }
            }
            else
            {
               QTreeWidgetItem * const pc_TableWidgetItem = pc_TopLevelItem->child(0);
               //Variables
               if (pc_TableWidgetItem != NULL)
               {
                  C_SdNdeDpListTableWidget * const pc_Table =
                     dynamic_cast<C_SdNdeDpListTableWidget * const>(this->itemWidget(pc_TableWidgetItem, 0));

                  //Check table selection
                  if (pc_Table != NULL)
                  {
                     const std::vector<QString> c_CurSelectedVariableNames = pc_Table->GetSelectedVariableNames();
                     if (c_CurSelectedVariableNames.size() > 0)
                     {
                        const uint32_t u32_ListIndex = static_cast<uint32_t>(s32_It);
                        c_SelectedVariableNames = c_CurSelectedVariableNames;
                        if (u32_ListIndex < pc_Datapool->c_Lists.size())
                        {
                           const C_OscNodeDataPoolList & rc_List = pc_Datapool->c_Lists[u32_ListIndex];
                           c_SelectedListNames.emplace_back(rc_List.c_Name.c_str());
                        }
                     }
                  }
               }
            }
         }
      }
      C_UsHandler::h_GetInstance()->SetProjSdNodeDatapoolSelectedListNames(pc_Node->c_Properties.c_Name.c_str(),
                                                                           pc_Datapool->c_Name.c_str(),
                                                                           c_SelectedListNames);
      C_UsHandler::h_GetInstance()->SetProjSdNodeDatapoolSelectedVariableNames(pc_Node->c_Properties.c_Name.c_str(),
                                                                               pc_Datapool->c_Name.c_str(),
                                                                               c_SelectedVariableNames);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Restore user settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListsTreeWidget::m_RestoreUserSettings(void)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
   const C_OscNodeDataPool * const pc_Datapool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(this->mu32_NodeIndex,
                                                                                                 this->mu32_DataPoolIndex);

   if ((pc_Node != NULL) && (pc_Datapool != NULL))
   {
      const C_UsNode c_Node = C_UsHandler::h_GetInstance()->GetProjSdNode(pc_Node->c_Properties.c_Name.c_str());
      const C_UsNodeDatapool c_Datapool = c_Node.GetDatapool(pc_Datapool->c_Name.c_str());
      const std::vector<QString> & rc_ExpandedListNames = c_Datapool.GetExpandedListNames();
      const std::vector<QString> & rc_SelectedListNames = c_Datapool.GetSelectedListNames();
      const std::vector<QString> & rc_SelectedVariableNames = c_Datapool.GetSelectedVariableNames();

      //Expanded items
      for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
      {
         QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
         if (pc_TopLevelItem != NULL)
         {
            const uint32_t u32_ListIndex = static_cast<uint32_t>(s32_It);
            if (u32_ListIndex < pc_Datapool->c_Lists.size())
            {
               const C_OscNodeDataPoolList & rc_List = pc_Datapool->c_Lists[u32_ListIndex];
               const QString c_ListName = rc_List.c_Name.c_str();
               bool q_Expand = false;
               for (uint32_t u32_ItName = 0; u32_ItName < rc_ExpandedListNames.size(); ++u32_ItName)
               {
                  if (c_ListName.compare(rc_ExpandedListNames[u32_ItName]) == 0)
                  {
                     q_Expand = true;
                     break;
                  }
               }
               pc_TopLevelItem->setExpanded(q_Expand);
            }
         }
      }

      //Selected items
      if ((rc_SelectedVariableNames.size() > 0) && (rc_SelectedListNames.size() == 1))
      {
         //Variables
         for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
         {
            QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
            if (pc_TopLevelItem != NULL)
            {
               const uint32_t u32_ListIndex = static_cast<uint32_t>(s32_It);
               if (u32_ListIndex < pc_Datapool->c_Lists.size())
               {
                  const C_OscNodeDataPoolList & rc_List = pc_Datapool->c_Lists[u32_ListIndex];
                  const QString c_ListName = rc_List.c_Name.c_str();
                  bool q_Selection = false;
                  for (uint32_t u32_ItName = 0; u32_ItName < rc_SelectedListNames.size(); ++u32_ItName)
                  {
                     if (c_ListName.compare(rc_SelectedListNames[u32_ItName]) == 0)
                     {
                        q_Selection = true;
                        break;
                     }
                  }
                  if (q_Selection == true)
                  {
                     QTreeWidgetItem * const pc_TableWidgetItem = pc_TopLevelItem->child(0);
                     if (pc_TableWidgetItem != NULL)
                     {
                        C_SdNdeDpListTableWidget * const pc_Table =
                           dynamic_cast<C_SdNdeDpListTableWidget * const>(this->itemWidget(pc_TableWidgetItem,
                                                                                           0));

                        //Check table selection
                        if (pc_Table != NULL)
                        {
                           pc_Table->SetSelectedVariableNames(rc_SelectedVariableNames);
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {
         //Lists
         for (int32_t s32_It = 0; s32_It < this->topLevelItemCount(); ++s32_It)
         {
            QTreeWidgetItem * const pc_TopLevelItem = this->topLevelItem(s32_It);
            if (pc_TopLevelItem != NULL)
            {
               const uint32_t u32_ListIndex = static_cast<uint32_t>(s32_It);
               if (u32_ListIndex < pc_Datapool->c_Lists.size())
               {
                  const C_OscNodeDataPoolList & rc_List = pc_Datapool->c_Lists[u32_ListIndex];
                  const QString c_ListName = rc_List.c_Name.c_str();
                  bool q_Selection = false;
                  for (uint32_t u32_ItName = 0; u32_ItName < rc_SelectedListNames.size(); ++u32_ItName)
                  {
                     if (c_ListName.compare(rc_SelectedListNames[u32_ItName]) == 0)
                     {
                        q_Selection = true;
                        //If selected, scroll there
                        this->scrollToItem(pc_TopLevelItem);
                        break;
                     }
                  }
                  pc_TopLevelItem->setSelected(q_Selection);
               }
            }
         }
      }
   }
}
