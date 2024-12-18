//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data pool list elements data set view (implementation)

   Data pool list elements data set view

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QScrollBar>
#include <QHeaderView>
#include <QMimeData>
#include <QDrag>
#include "C_SdNdeDpListDataSetView.hpp"
#include "C_GtGetText.hpp"
#include "C_PuiSdHandler.hpp"
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_SdClipBoardHelper.hpp"
#include "C_Uti.hpp"
#include "C_SdNdeDpUtil.hpp"
#include "C_SdNdeSingleHeaderView.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_SdUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
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

   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListDataSetView::C_SdNdeDpListDataSetView(QWidget * const opc_Parent) :
   C_TblViewScroll(opc_Parent),
   mu32_NodeIndex(0),
   mu32_DataPoolIndex(0),
   mu32_ListIndex(0),
   mc_UndoManager(),
   mc_Delegate(),
   mpc_ModelViewManager(NULL),
   mpc_LabelCorner(NULL),
   mq_AllowMoveLeft(true),
   mq_AllowMoveRight(true)
{
   //UI Settings
   this->setSortingEnabled(false);
   this->setGridStyle(Qt::NoPen);
   this->setShowGrid(false);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectColumns);
   this->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
   this->setAlternatingRowColors(true);
   this->setDragDropMode(QAbstractItemView::NoDragDrop);
   this->setDefaultDropAction(Qt::DropAction::MoveAction);
   this->setDragEnabled(false);
   this->setLineWidth(0);
   this->setFrameShadow(QAbstractItemView::Shadow::Plain);
   this->setFrameShape(QAbstractItemView::Shape::NoFrame);
   this->setEditTriggers(
      QAbstractItemView::DoubleClicked /* | QAbstractItemView::CurrentChanged*/ | QAbstractItemView::AnyKeyPressed |
      QAbstractItemView::EditKeyPressed);
   //Consider all elements for resize
   this->setVerticalHeader(new C_SdNdeSingleHeaderView(Qt::Vertical));
   this->verticalHeader()->setResizeContentsPrecision(-1);
   //Row Height
   this->setHorizontalHeader(new C_SdNdeSingleHeaderView(Qt::Horizontal));
   this->horizontalHeader()->setResizeContentsPrecision(-1);
   this->horizontalHeader()->setDefaultSectionSize(175);
   this->horizontalHeader()->setFixedHeight(35);
   //Corner button
   this->setCornerButtonEnabled(false);

   this->setItemDelegate(&mc_Delegate);

   //Hover event
   this->setMouseTracking(true);

   //Avoid styling table inside
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->verticalScrollBar(), "C_SdNdeDpListDataSetView", true);
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->horizontalScrollBar(), "C_SdNdeDpListDataSetView", true);

   //Corner button label
   {
      this->mpc_LabelCorner = new QLabel(this);
      this->mpc_LabelCorner->setAlignment(Qt::AlignCenter);
      this->mpc_LabelCorner->setAttribute(Qt::WA_TransparentForMouseEvents);
      this->mpc_LabelCorner->setText(C_GtGetText::h_GetText("Dataset #"));

      connect(
         this->verticalHeader(), &QHeaderView::geometriesChanged, this,
         &C_SdNdeDpListDataSetView::m_UpdateCornerButton);
      connect(
         this->horizontalHeader(), &QHeaderView::geometriesChanged, this,
         &C_SdNdeDpListDataSetView::m_UpdateCornerButton);
   }

   connect(&(this->mc_Delegate), &C_SdNdeDpListDataSetDelegate::SigTedConfirmed, this,
           &C_SdNdeDpListDataSetView::m_CommentConfirmed);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{1540}  no memory leak because of the parent and the Qt memory management or never took ownership
C_SdNdeDpListDataSetView::~C_SdNdeDpListDataSetView(void)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      this->mpc_ModelViewManager->UnRegisterDataSetView(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                        this->mu32_ListIndex, this);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Specify associated list

   \param[in] oru32_NodeIndex     Node index
   \param[in] oru32_DataPoolIndex Node data pool index
   \param[in] oru32_ListIndex     Node data pool list index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::SetList(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
                                       const uint32_t & oru32_ListIndex)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      this->mpc_ModelViewManager->UnRegisterDataSetView(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                        this->mu32_ListIndex, this);
   }
   this->mu32_NodeIndex = oru32_NodeIndex;
   this->mu32_DataPoolIndex = oru32_DataPoolIndex;
   this->mu32_ListIndex = oru32_ListIndex;
   m_UpdateModelView();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if equal

   \param[in] oru32_NodeIndex     Node index
   \param[in] oru32_DataPoolIndex Node data pool index
   \param[in] oru32_ListIndex     Node data pool list index

   \return
   True  Match
   False No match
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SdNdeDpListDataSetView::Equals(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
                                      const uint32_t & oru32_ListIndex) const
{
   bool q_Retval;

   if (((this->mu32_NodeIndex == oru32_NodeIndex) && (this->mu32_DataPoolIndex == oru32_DataPoolIndex)) &&
       (this->mu32_ListIndex == oru32_ListIndex))
   {
      q_Retval = true;
   }
   else
   {
      q_Retval = false;
   }
   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy tree item to clipboard
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::Copy(void) const
{
   std::vector<uint32_t> c_SelectedIndices = m_GetSelectedIndices();

   if (c_SelectedIndices.size() > 0)
   {
      std::vector<C_OscNodeDataPoolDataSet> c_OscNames;
      std::vector<std::vector<C_OscNodeDataPoolContent> > c_OscDataSetValues;

      c_OscNames.resize(c_SelectedIndices.size());
      c_OscDataSetValues.resize(c_SelectedIndices.size());

      //Sort to have "correct" copy order
      C_SdUtil::h_SortIndicesAscending(c_SelectedIndices);
      for (uint32_t u32_ItSelectedIndex = 0; u32_ItSelectedIndex < c_SelectedIndices.size(); ++u32_ItSelectedIndex)
      {
         tgl_assert(C_PuiSdHandler::h_GetInstance()->GetDataPoolListDataSet(this->mu32_NodeIndex,
                                                                            this->mu32_DataPoolIndex,
                                                                            this->mu32_ListIndex,
                                                                            c_SelectedIndices[
                                                                               u32_ItSelectedIndex],
                                                                            c_OscNames[u32_ItSelectedIndex],
                                                                            c_OscDataSetValues[
                                                                               u32_ItSelectedIndex]) == C_NO_ERR);
      }

      C_SdClipBoardHelper::h_StoreDataPoolListDataSetsToClipBoard(c_OscNames, c_OscDataSetValues);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy tree item to clipboard and delete it afterwards
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::Cut(void)
{
   Copy();
   Delete();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paste tree item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::Paste(void)
{
   const uint32_t u32_LastIndex = m_GetOneAfterHighestSelected();

   this->mc_UndoManager.DoPaste(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                this->mpc_ModelViewManager,
                                u32_LastIndex);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete tree item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::Delete(void)
{
   const std::vector<uint32_t> c_SelectedIndices = m_GetSelectedIndices();

   if (c_SelectedIndices.size() > 0)
   {
      this->mc_UndoManager.DoDeleteElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                            this->mpc_ModelViewManager,
                                            c_SelectedIndices);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert tree item

   \param[in] orq_SetFocus Optional flag if inital focus should be set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::Insert(const bool & orq_SetFocus)
{
   std::vector<uint32_t> c_Indices;
   const uint32_t u32_Index = m_GetOneAfterHighestSelected();
   c_Indices.push_back(u32_Index);

   this->mc_UndoManager.DoAddElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                      this->mpc_ModelViewManager,
                                      c_Indices);
   if (orq_SetFocus == true)
   {
      this->edit(this->model()->index(0, static_cast<int32_t>(u32_Index)));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move selected item up by one slot
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::DoMoveLeft(void)
{
   if (this->mq_AllowMoveLeft == true)
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

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move selected item down by one slot
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::DoMoveRight(void)
{
   if (this->mq_AllowMoveRight == true)
   {
      if (this->mpc_ModelViewManager != NULL)
      {
         bool q_AllowMove = true;

         std::vector<uint32_t> c_TargetIndices;
         std::vector<uint32_t> c_SourceIndices = m_GetSelectedIndices();
         const uint32_t u32_LastIndex =
            static_cast<uint32_t>(this->mpc_ModelViewManager->GetDataSetModel(this->mu32_NodeIndex,
                                                                              this->
                                                                              mu32_DataPoolIndex,
                                                                              this->
                                                                              mu32_ListIndex)->columnCount() -
                                  1);
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
/*! \brief   GetUndocommand and take ownership

   Internal undo command is reseted

   \return
   NULL No changes
   Else Undocommand accumulating all changes
*/
//----------------------------------------------------------------------------------------------------------------------
QUndoCommand * C_SdNdeDpListDataSetView::TakeUndoCommand(void)
{
   return this->mc_UndoManager.TakeUndoCommand();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Do data change

   \param[in] oru32_DataPoolListDataSetIndex Node data pool list data set index
   \param[in] orc_NewData                    New data
   \param[in] ore_DataChangeType             Data change type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::OnDataChange(const uint32_t & oru32_DataPoolListDataSetIndex,
                                            const QVariant & orc_NewData,
                                            const C_SdNdeDpUtil::E_DataSetDataChangeType & ore_DataChangeType)
{
   this->mc_UndoManager.DoDataChangeElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                             this->mu32_ListIndex,
                                             this->mpc_ModelViewManager, oru32_DataPoolListDataSetIndex, orc_NewData,
                                             ore_DataChangeType);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set model view manager

   \param[in,out] opc_Value Model view manager
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::SetModelViewManager(C_SdNdeDpListModelViewManager * const opc_Value)
{
   this->mpc_ModelViewManager = opc_Value;
   m_UpdateModelView();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle data set count change

   \param[in] ors32_NewColumnCount New column count
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::OnColumnCountChange(const int32_t & ors32_NewColumnCount)
{
   Q_EMIT this->SigColumnCountChange(ors32_NewColumnCount);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten drop event slot

   Here: Move elements

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::dropEvent(QDropEvent * const opc_Event)
{
   C_SdNdeDpListDataSetView * const pc_SourceTable =
      dynamic_cast<C_SdNdeDpListDataSetView * const>(opc_Event->source());

   if (pc_SourceTable != NULL)
   {
      const QMimeData * const pc_MimeData = opc_Event->mimeData();
      if (pc_MimeData != NULL)
      {
         if (this->mpc_ModelViewManager != NULL)
         {
            C_SdNdeDpListDataSetModel * const pc_Model = this->mpc_ModelViewManager->GetDataSetModel(
               this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
            if (pc_Model != NULL)
            {
               if (pc_Model->mimeTypes().size() > 2)
               {
                  if ((pc_MimeData->hasFormat(pc_Model->mimeTypes().at(1)) == true) &&
                      (pc_MimeData->hasFormat(pc_Model->mimeTypes().at(2)) == true))
                  {
                     std::vector<C_OscNodeDataPoolDataSet> c_OscNames;
                     std::vector<std::vector<C_OscNodeDataPoolContent> > c_OscDataSetValues;
                     const QString c_Content = pc_MimeData->data(pc_Model->mimeTypes().at(1));
                     //Insert indices
                     if (C_SdClipBoardHelper::h_LoadToDataPoolListDataSetsFromString(c_OscNames, c_OscDataSetValues,
                                                                                     c_Content) == C_NO_ERR)
                     {
                        std::vector<uint32_t> c_SourceIndices;
                        uint32_t u32_TargetCol;
                        const QString c_IndicesString = pc_MimeData->data(pc_Model->mimeTypes().at(2));
                        std::vector<uint32_t> c_NewIndices;
                        const QModelIndex c_Index = this->indexAt(opc_Event->position().toPoint());
                        //Target row
                        if (c_Index.isValid())
                        {
                           u32_TargetCol = static_cast<uint32_t>(c_Index.column()) + 1UL;
                        }
                        else
                        {
                           u32_TargetCol = pc_Model->columnCount();
                        }
                        //Calculate target indices
                        c_NewIndices.resize(c_OscNames.size());
                        for (uint32_t u32_ItNewIndex = 0; u32_ItNewIndex < c_NewIndices.size(); ++u32_ItNewIndex)
                        {
                           c_NewIndices[u32_ItNewIndex] = u32_TargetCol + u32_ItNewIndex;
                        }
                        //Get indices
                        if (C_SdClipBoardHelper::h_LoadIndicesFromString(c_SourceIndices,
                                                                         c_IndicesString) == C_NO_ERR)
                        {
                           if (pc_SourceTable == this)
                           {
                              //Effective move
                              this->mc_UndoManager.DoMoveElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                                  this->mu32_ListIndex,
                                                                  this->mpc_ModelViewManager,
                                                                  c_SourceIndices,
                                                                  c_NewIndices, true);
                           }
                           else
                           {
                              //Add here
                              this->mc_UndoManager.DoAddSpecificElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                                         this->mu32_ListIndex,
                                                                         this->mpc_ModelViewManager,
                                                                         c_NewIndices,
                                                                         c_OscNames, c_OscDataSetValues);
                              //Delete indices in source
                              pc_SourceTable->m_DeleteIndices(c_SourceIndices);
                           }
                        }
                        //Accept event
                        opc_Event->accept();
                     }
                  }
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten mouse move event slot

   Here: Track mouse hover

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::mouseMoveEvent(QMouseEvent * const opc_Event)
{
   int32_t s32_HoveredCol = -1;
   const QModelIndex c_HoveredIndex = this->indexAt(opc_Event->pos());

   C_TblViewScroll::mouseMoveEvent(opc_Event);

   if (c_HoveredIndex.isValid())
   {
      s32_HoveredCol = c_HoveredIndex.column();
   }
   if (this->mc_Delegate.SetHoveredCol(s32_HoveredCol) == true)
   {
      this->update();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten mouse leave event slot

   Here: Handle hover effect change

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::leaveEvent(QEvent * const opc_Event)
{
   C_TblViewScroll::leaveEvent(opc_Event);
   if (this->mc_Delegate.SetHoveredCol(-1) == true)
   {
      this->update();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten selection changed event slot

   Here: Register selection change

   \param[in] orc_Selected   Selected items
   \param[in] orc_Deselected Deselected items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::selectionChanged(const QItemSelection & orc_Selected,
                                                const QItemSelection & orc_Deselected)
{
   std::vector<uint32_t> c_SelectedIndices;
   C_TblViewScroll::selectionChanged(orc_Selected, orc_Deselected);

   c_SelectedIndices = C_SdNdeDpUtil::h_ConvertVector(this->selectedIndexes(), false);
   C_Uti::h_Uniqueify(c_SelectedIndices);
   Q_EMIT this->SigSelectionChanged(c_SelectedIndices.size());
   m_CheckActions(c_SelectedIndices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten start drag event

   Here: start drag manually (for custom preview)

   \param[in] oc_SupportedActions Supported actions
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::startDrag(const Qt::DropActions oc_SupportedActions)
{
   const QModelIndexList c_SelectedItems = this->selectedIndexes();
   const std::vector<uint32_t> c_ReallySelectedItems = this->m_GetSelectedIndices();

   if (c_ReallySelectedItems.size() > 0)
   {
      //Manual drag
      QDrag * const pc_Drag = new QDrag(this);

      pc_Drag->setMimeData(this->model()->mimeData(c_SelectedItems));
      pc_Drag->exec(oc_SupportedActions, this->defaultDropAction());
   } //lint !e429  //no memory leak because of the parent of pc_Drag and the Qt memory management
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete specified indices

   \param[in] orc_Indices Indices
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::m_DeleteIndices(const std::vector<uint32_t> & orc_Indices)
{
   std::vector<uint32_t> c_Indices = orc_Indices;
   C_Uti::h_Uniqueify(c_Indices);
   this->mc_UndoManager.DoDeleteElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                         this->mpc_ModelViewManager,
                                         c_Indices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move item in tree

   \param[in] orc_SourceIndices Source index
   \param[in] orc_TargetIndices Target index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::m_Move(const std::vector<uint32_t> & orc_SourceIndices,
                                      const std::vector<uint32_t> & orc_TargetIndices)
{
   this->mc_UndoManager.DoMoveElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                       this->mpc_ModelViewManager,
                                       orc_SourceIndices, orc_TargetIndices, false);
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get selected indices

   \return
   Selected indices
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint32_t> C_SdNdeDpListDataSetView::m_GetSelectedIndices(void) const
{
   std::vector<uint32_t> c_Retval;
   QModelIndexList c_SelectedItems = this->selectedIndexes();

   c_Retval.reserve(c_SelectedItems.size());
   for (QModelIndexList::const_iterator c_ItSelectedItem = c_SelectedItems.begin();
        c_ItSelectedItem != c_SelectedItems.end(); ++c_ItSelectedItem)
   {
      const QModelIndex & rc_Item = *c_ItSelectedItem;
      c_Retval.push_back(rc_Item.column());
   }
   C_Uti::h_Uniqueify(c_Retval);
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Recheck if actions statuses are still up to date

   \param[in] orc_SelectedIndices Selected indices
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::m_CheckActions(const std::vector<uint32_t> & orc_SelectedIndices)
{
   if (orc_SelectedIndices.size() > 0)
   {
      if (this->mpc_ModelViewManager != NULL)
      {
         C_SdNdeDpListDataSetModel * const pc_Model = this->mpc_ModelViewManager->GetDataSetModel(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
         if (pc_Model != NULL)
         {
            bool q_AllowMoveLeft;
            bool q_AllowMoveRight;

            //Move left & right
            q_AllowMoveLeft = true;
            q_AllowMoveRight = true;
            for (uint32_t u32_ItSelectedIndex = 0; u32_ItSelectedIndex < orc_SelectedIndices.size();
                 ++u32_ItSelectedIndex)
            {
               if (orc_SelectedIndices[u32_ItSelectedIndex] == 0)
               {
                  q_AllowMoveLeft = false;
               }
               if (orc_SelectedIndices[u32_ItSelectedIndex] == static_cast<uint32_t>(pc_Model->columnCount() - 1))
               {
                  q_AllowMoveRight = false;
               }
            }
            //Actions
            this->mq_AllowMoveRight = q_AllowMoveRight;
            this->mq_AllowMoveLeft = q_AllowMoveLeft;
            //Buttons
            Q_EMIT this->SigButtonChange(true, true, true, true, true, q_AllowMoveLeft, q_AllowMoveRight);
         }
      }
   }
   else
   {
      //Default
      this->mq_AllowMoveRight = false;
      this->mq_AllowMoveLeft = false;
      Q_EMIT this->SigButtonChange(true, false, false, true, false, false, false);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update model
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::m_UpdateModelView(void)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      C_SdNdeDpListDataSetModel * const pc_Model = this->mpc_ModelViewManager->GetDataSetModel(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex,
         this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         this->setModel(pc_Model);
         this->mc_Delegate.SetModel(pc_Model);
         m_CheckActions(this->m_GetSelectedIndices());
         //Row height seems to be reset on model change
         this->setRowHeight(C_SdNdeDpListDataSetModel::h_EnumToRow(C_SdNdeDpListDataSetModel::eNAME), 35);
         this->setRowHeight(C_SdNdeDpListDataSetModel::h_EnumToRow(C_SdNdeDpListDataSetModel::eCOMMENT), 110);
      }
   }
   if (this->mpc_ModelViewManager != NULL)
   {
      this->mpc_ModelViewManager->RegisterDataSetView(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                      this->mu32_ListIndex, this);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update corner button size
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::m_UpdateCornerButton(void)
{
   tgl_assert(this->mpc_LabelCorner != NULL);
   if (this->mpc_LabelCorner != NULL)
   {
      this->mpc_LabelCorner->setGeometry(0, 0, this->verticalHeader()->width(), this->horizontalHeader()->height());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get index of element after highest selected element

   \return
   Index of element after highest selected element
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SdNdeDpListDataSetView::m_GetOneAfterHighestSelected(void)
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
      if (this->mpc_ModelViewManager != NULL)
      {
         //Add at end
         const int32_t s32_ColCount = this->mpc_ModelViewManager->GetDataSetModel(this->mu32_NodeIndex,
                                                                                  this->mu32_DataPoolIndex,
                                                                                  this->mu32_ListIndex)->columnCount();
         if (s32_ColCount >= 0)
         {
            u32_Retval = static_cast<uint32_t>(s32_ColCount);
         }
      }
   }
   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot to set focus of comment field after user confirms with defined keys of KeyPressEvent in C_OgeTedTable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListDataSetView::m_CommentConfirmed()
{
   this->setFocus();
}
