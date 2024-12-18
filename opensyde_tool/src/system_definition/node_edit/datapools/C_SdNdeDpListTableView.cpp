//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data pool list elements table item (implementation)

   Data pool list elements table item

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QHeaderView>
#include <QMimeData>
#include <QDrag>
#include <QScrollBar>
#include <QPointer>

#include "C_SdNdeDpListTableView.hpp"
#include "C_GtGetText.hpp"
#include "C_PuiSdHandler.hpp"
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_SdClipBoardHelper.hpp"
#include "C_Uti.hpp"
#include "C_SdNdeDpUtil.hpp"
#include "C_SdNdeSingleHeaderView.hpp"
#include "C_OgePopUpDialog.hpp"
#include "C_SdNdeDpListArrayEditWidget.hpp"
#include "constants.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_SdUtil.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_UsHandler.hpp"
#include "C_SdNdeDpListCommentDialog.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_elements;
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
C_SdNdeDpListTableView::C_SdNdeDpListTableView(QWidget * const opc_Parent) :
   C_TblViewScroll(opc_Parent),
   mu32_NodeIndex(0),
   mu32_DataPoolIndex(0),
   mu32_ListIndex(0),
   ms32_LastIndicatorSize(-1),
   mc_UndoManager(),
   mc_Delegate(),
   mpc_ContextMenu(NULL),
   mpc_ModelViewManager(NULL),
   mpc_ActionMoveUp(NULL),
   mpc_ActionMoveDown(NULL),
   mpc_ActionAdd(NULL),
   mpc_ActionCut(NULL),
   mpc_ActionCopy(NULL),
   mpc_ActionPaste(NULL),
   mpc_ActionDelete(NULL),
   mpc_ActionEditComment(NULL),
   mq_AllowMoveUp(true),
   mq_AllowMoveDown(true),
   mq_AllowAdd(true)
{
   //UI Settings
   this->setCornerButtonEnabled(false);
   this->setSortingEnabled(false);
   this->setGridStyle(Qt::NoPen);
   this->setShowGrid(false);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
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
   this->setHorizontalHeader(new C_SdNdeSingleHeaderView(Qt::Horizontal));
   this->horizontalHeader()->setResizeContentsPrecision(-1);
   this->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
   this->horizontalHeader()->setFixedHeight(40);

   //Row Height
   this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
   this->verticalHeader()->setMinimumSectionSize(30);
   this->verticalHeader()->setMaximumSectionSize(30);
   this->verticalHeader()->setDefaultSectionSize(30);

   // Icon
   this->setIconSize(QSize(16, 16));

   //Hide vertical header
   this->verticalHeader()->hide();

   this->setItemDelegate(&mc_Delegate);

   //Custom context menu
   m_SetupContextMenu();

   //Hover event
   this->setMouseTracking(true);

   //Avoid styling table inside
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->verticalScrollBar(), "C_SdNdeDpListTableView", true);
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->horizontalScrollBar(), "C_SdNdeDpListTableView", true);

   //Connects
   connect(&mc_Delegate, &C_SdNdeDpListTableDelegate::SigLinkClicked, this,
           &C_SdNdeDpListTableView::m_HandleLinkClicked);
   connect(this->horizontalHeader(), &QHeaderView::sectionResized, this,
           &C_SdNdeDpListTableView::m_OnColumnResize);
   connect(&mc_Delegate, &C_SdNdeDpListTableDelegate::SigTedConfirmed, this,
           &C_SdNdeDpListTableView::m_CommentConfirmed);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListTableView::~C_SdNdeDpListTableView(void)
{
   m_HandleColumnStateSave();
   if (this->mpc_ModelViewManager != NULL)
   {
      this->mpc_ModelViewManager->UnRegisterElementView(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                        this->mu32_ListIndex, this);
   }

   //cleanup handled by Qt engine; just NULLing here
   mpc_ContextMenu       = NULL;
   mpc_ModelViewManager  = NULL;
   mpc_ActionMoveUp      = NULL;
   mpc_ActionMoveDown    = NULL;
   mpc_ActionAdd         = NULL;
   mpc_ActionCut         = NULL;
   mpc_ActionCopy        = NULL;
   mpc_ActionPaste       = NULL;
   mpc_ActionDelete      = NULL;
   mpc_ActionEditComment = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Specify associated list

   \param[in]  oru32_NodeIndex      Node index
   \param[in]  oru32_DataPoolIndex  Node data pool index
   \param[in]  oru32_ListIndex      Node data pool list index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::SetList(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
                                     const uint32_t & oru32_ListIndex)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      this->mpc_ModelViewManager->UnRegisterElementView(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                        this->mu32_ListIndex, this);
   }
   mu32_NodeIndex = oru32_NodeIndex;
   mu32_DataPoolIndex = oru32_DataPoolIndex;
   mu32_ListIndex = oru32_ListIndex;
   Reset();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if equal

   \param[in]  oru32_NodeIndex      Node index
   \param[in]  oru32_DataPoolIndex  Node data pool index
   \param[in]  oru32_ListIndex      Node data pool list index

   \return
   True  Match
   False No match
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SdNdeDpListTableView::Equals(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
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
void C_SdNdeDpListTableView::Copy(void) const
{
   const C_OscNodeDataPool * const pc_Datapool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(this->mu32_NodeIndex,
                                                                                                 this->mu32_DataPoolIndex);

   if (pc_Datapool != NULL)
   {
      std::vector<C_OscNodeDataPoolListElement> c_OscContentVec;
      std::vector<C_PuiSdNodeDataPoolListElement> c_UiContentVec;
      C_OscNodeDataPoolListElement c_OscContent;
      C_PuiSdNodeDataPoolListElement c_UiContent;

      std::vector<uint32_t> c_SelectedIndices = m_GetSelectedIndices();
      c_OscContentVec.reserve(c_SelectedIndices.size());
      c_UiContentVec.reserve(c_SelectedIndices.size());

      //Sort to have "correct" copy order
      C_SdUtil::h_SortIndicesAscending(c_SelectedIndices);
      for (uint32_t u32_ItIndex = 0; u32_ItIndex < c_SelectedIndices.size(); ++u32_ItIndex)
      {
         if (C_PuiSdHandler::h_GetInstance()->GetDataPoolListElement(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                                     this->mu32_ListIndex,
                                                                     c_SelectedIndices[u32_ItIndex],
                                                                     c_OscContent,
                                                                     c_UiContent) == C_NO_ERR)
         {
            c_OscContentVec.push_back(c_OscContent);
            c_UiContentVec.push_back(c_UiContent);
         }
      }
      C_SdClipBoardHelper::h_StoreDataPoolListElementsToClipBoard(c_OscContentVec, c_UiContentVec, pc_Datapool->e_Type);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy tree item to clipboard and delete it afterwards
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::Cut(void)
{
   Copy();
   Delete();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paste tree item
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::Paste(void)
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
void C_SdNdeDpListTableView::Delete(void)
{
   const std::vector<uint32_t> c_Indices = m_GetSelectedIndices();

   this->mc_UndoManager.DoDeleteElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                         this->mpc_ModelViewManager,
                                         c_Indices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert triggered via action
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::InsertAction(void)
{
   Insert(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Edit comment in editor
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_EditCommentInEditor()
{
   const QPointer<C_OgePopUpDialog> c_Dialog = new C_OgePopUpDialog(this, this);
   C_SdNdeDpListCommentDialog * const pc_ArrayEditWidget =
      new C_SdNdeDpListCommentDialog(*c_Dialog, this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                     this->mu32_ListIndex);

   const int32_t s32_SelectedRow = this->selectedIndexes().at(0).row();
   const QModelIndex c_NameIndex = this->model()->index(s32_SelectedRow, 3);
   const QModelIndex c_CommentIndex = this->model()->index(s32_SelectedRow, 4);
   const QString c_Text =
      static_cast<QString>(C_GtGetText::h_GetText("Data Element \"%1\" ")).arg(this->model()->data(
                                                                                  c_NameIndex).toString());

   pc_ArrayEditWidget->SetTitle(c_Text);

   c_Dialog->SetSize(QSize(800, 550));
   pc_ArrayEditWidget->GetCommentToEditor(this->model()->data(c_CommentIndex).toString());

   if (c_Dialog->exec() == static_cast<int32_t>(QDialog::Accepted))
   {
      const QVariant c_Data = pc_ArrayEditWidget->GetComment();
      this->model()->setData(c_CommentIndex, c_Data);
   }

   if (c_Dialog != NULL)
   {
      c_Dialog->HideOverlay();
      c_Dialog->deleteLater();
   }
} //lint !e429  //no memory leak because of the parent of pc_ImportDialog and the Qt memory management

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert tree item

   \param[in]  orq_SetFocus   Optional flag if inital focus should be set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::Insert(const bool & orq_SetFocus)
{
   if (mq_AllowAdd == true)
   {
      std::vector<uint32_t> c_Indices;
      c_Indices.push_back(m_GetOneAfterHighestSelected());

      this->mc_UndoManager.DoAddElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                         this->mpc_ModelViewManager, c_Indices);
      if (orq_SetFocus == true)
      {
         this->EditInitial();
      }
   }
   else
   {
      QString c_Text;
      C_OgeWiCustomMessage c_MessageBox(this, C_OgeWiCustomMessage::E_Type::eERROR);
      const C_OscNodeDataPool * const pc_DataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex);
      if (pc_DataPool != NULL)
      {
         c_Text = C_PuiSdHandlerNodeLogic::h_GetElementTypeName(pc_DataPool->e_Type) + "s";
      }
      else
      {
         c_Text = C_GtGetText::h_GetText("Data elements");
      }
      c_MessageBox.SetDescription(static_cast<QString>(C_GtGetText::h_GetText("Only %1 %2 allowed per list.")).arg(
                                     C_OscNode::hu32_MAX_NUMBER_OF_ELEMENTS_PER_LIST).arg(c_Text));
      c_MessageBox.SetCustomMinHeight(180, 180);
      c_MessageBox.Execute();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move selected item up by one slot
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::DoMoveUp(void)
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

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move selected item down by one slot
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::DoMoveDown(void)
{
   if (this->mq_AllowMoveDown == true)
   {
      if (this->mpc_ModelViewManager != NULL)
      {
         C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
         if (pc_Model != NULL)
         {
            bool q_AllowMove = true;

            std::vector<uint32_t> c_TargetIndices;
            std::vector<uint32_t> c_SourceIndices = m_GetSelectedIndices();
            const uint32_t u32_LastIndex = static_cast<uint32_t>(pc_Model->rowCount() - 1);
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
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enter name edit
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::EditInitial(void)
{
   const std::vector<uint32_t> c_Selection = this->m_GetSelectedIndices();

   if (c_Selection.size() == 1)
   {
      if (this->mpc_ModelViewManager != NULL)
      {
         C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
         if (pc_Model != NULL)
         {
            this->edit(this->model()->index(c_Selection[0], pc_Model->EnumToColumn(
                                               C_SdNdeDpListTableModel::eNAME)));
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set undo stack

   \param[in,out]  opc_Value  Undo stack
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::SetUndoStack(C_SdNdeUnoDataPoolManager * const opc_Value)
{
   this->mc_UndoManager.SetUndoStack(opc_Value);
   //Undo
   this->mc_Delegate.SetUndoStack(opc_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Forward signal
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::OnErrorChangePossible(void)
{
   Q_EMIT this->SigErrorChangePossible();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Forward signal
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::OnSizeChangePossible(void)
{
   m_HandleColumnStateSave();
   Q_EMIT this->SigSizeChangePossible();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Do data change

   \param[in]  oru32_DataPoolListElementIndex   Node data pool list element index
   \param[in]  orc_NewData                      New data
   \param[in]  ore_DataChangeType               Data change type
   \param[in]  oru32_ArrayIndex                 Optional array index
   \param[in]  ors32_DataSetIndex               Optional data set index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::OnDataChangeElements(const uint32_t & oru32_DataPoolListElementIndex,
                                                  const QVariant & orc_NewData,
                                                  const C_SdNdeDpUtil::E_ElementDataChangeType & ore_DataChangeType,
                                                  const uint32_t & oru32_ArrayIndex, const int32_t & ors32_DataSetIndex)
{
   this->mc_UndoManager.DoDataChangeElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                             this->mpc_ModelViewManager, oru32_DataPoolListElementIndex, orc_NewData,
                                             ore_DataChangeType, oru32_ArrayIndex, ors32_DataSetIndex);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: handle list paste

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::keyPressEvent(QKeyEvent * const opc_Event)
{
   bool q_CallOrig = true;

   switch (opc_Event->key())
   {
   case Qt::Key_C:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         Copy();
         opc_Event->accept();
      }
      break;
   case Qt::Key_X:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         Cut();
         opc_Event->accept();
      }
      break;
   case Qt::Key_V:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         Paste();
         opc_Event->accept();
      }
      break;
   case Qt::Key_Delete:
      q_CallOrig = false;
      Delete();
      opc_Event->accept();
      break;
   case Qt::Key_BracketRight: // Qt::Key_BracketRight matches the "Not-Num-Plus"-Key
   case Qt::Key_Plus:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         Insert(false);
         opc_Event->accept();
      }
      break;
   case Qt::Key_Up:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         DoMoveUp();
         opc_Event->accept();
      }
      break;
   case Qt::Key_Down:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == true)
      {
         q_CallOrig = false;
         DoMoveDown();
         opc_Event->accept();
      }
      break;
   case Qt::Key_F10:
      if (C_Uti::h_CheckKeyModifier(opc_Event->modifiers(), Qt::ControlModifier) == false)
      {
         q_CallOrig = false;
         this->m_EditCommentInEditor();
         opc_Event->accept();
      }
      break;
   default:
      //Nothing to do
      break;
   }

   if (q_CallOrig == true)
   {
      C_TblViewScroll::keyPressEvent(opc_Event);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten drop event slot

   Here: Move elements

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::dropEvent(QDropEvent * const opc_Event)
{
   C_SdNdeDpListTableView * const pc_SourceTable =
      dynamic_cast<C_SdNdeDpListTableView * const>(opc_Event->source());

   if (pc_SourceTable != NULL)
   {
      const QMimeData * const pc_MimeData = opc_Event->mimeData();
      if (pc_MimeData != NULL)
      {
         if (this->mpc_ModelViewManager != NULL)
         {
            C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
               this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
            if (pc_Model != NULL)
            {
               if (pc_Model->mimeTypes().size() > 2)
               {
                  if ((pc_MimeData->hasFormat(pc_Model->mimeTypes().at(1)) == true) &&
                      (pc_MimeData->hasFormat(pc_Model->mimeTypes().at(2)) == true))
                  {
                     std::vector<C_OscNodeDataPoolListElement> c_OscContent;
                     std::vector<C_PuiSdNodeDataPoolListElement> c_UiContent;
                     const QString c_Content = pc_MimeData->data(pc_Model->mimeTypes().at(1));
                     //Insert indices
                     if (C_SdClipBoardHelper::h_LoadToDataPoolListElementsFromString(c_OscContent, c_UiContent,
                                                                                     c_Content) == C_NO_ERR)
                     {
                        std::vector<uint32_t> c_SourceIndices;
                        uint32_t u32_TargetRow;
                        const QString c_IndicesString = pc_MimeData->data(pc_Model->mimeTypes().at(2));
                        std::vector<uint32_t> c_NewIndices;
                        const QModelIndex c_Index = this->indexAt(opc_Event->position().toPoint());
                        //Target row
                        if (c_Index.isValid())
                        {
                           u32_TargetRow = static_cast<uint32_t>(c_Index.row()) + 1UL;
                        }
                        else
                        {
                           u32_TargetRow = pc_Model->rowCount();
                        }
                        //Calculate target indices
                        c_NewIndices.resize(c_OscContent.size());
                        for (uint32_t u32_ItNewIndex = 0; u32_ItNewIndex < c_NewIndices.size(); ++u32_ItNewIndex)
                        {
                           c_NewIndices[u32_ItNewIndex] = u32_TargetRow + u32_ItNewIndex;
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
                                                                         c_OscContent, c_UiContent);
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
/*! \brief   Overwritten selection changed event slot

   Here: Register selection change

   \param[in]  orc_Selected      Selected items
   \param[in]  orc_Deselected    Deselected items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::selectionChanged(const QItemSelection & orc_Selected,
                                              const QItemSelection & orc_Deselected)
{
   std::vector<uint32_t> c_SelectedIndices;
   C_TblViewScroll::selectionChanged(orc_Selected, orc_Deselected);

   c_SelectedIndices = C_SdNdeDpUtil::h_ConvertVector(this->selectedIndexes());
   C_Uti::h_Uniqueify(c_SelectedIndices);
   Q_EMIT this->SigSelectionChanged(c_SelectedIndices.size());
   m_CheckActions(c_SelectedIndices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten start drag event

   Here: start drag manually (for custom preview)

   \param[in]  oc_SupportedActions  Supported actions
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::startDrag(const Qt::DropActions oc_SupportedActions)
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
/*! \brief   Set model view manager

   \param[in,out]  opc_Value  Model view manager
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::SetModelViewManager(C_SdNdeDpListModelViewManager * const opc_Value)
{
   this->mpc_ModelViewManager = opc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Function to select a concrete dataelement

   \param[in]  os32_DataElementIndex   Data element index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::SelectDataElement(const int32_t os32_DataElementIndex)
{
   this->clearSelection();
   this->selectRow(os32_DataElementIndex);
   this->setFocus();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Select data elements in a specific range

   \param[in]  ou32_FirstRow  First row to select
   \param[in]  ou32_LastRow   Last row to select
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::SelectRange(const uint32_t ou32_FirstRow, const uint32_t ou32_LastRow) const
{
   const QModelIndex c_TopLeft = this->model()->index(ou32_FirstRow, 0);
   const QModelIndex c_BottomRight = this->model()->index(ou32_LastRow, this->model()->columnCount() - 1);
   const QItemSelection c_Selection(c_TopLeft, c_BottomRight);

   this->selectionModel()->select(c_Selection, QItemSelectionModel::Select);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if table is selected

   \return
   True  Selected
   False Not selected
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SdNdeDpListTableView::IsSelected(void) const
{
   return (this->selectedIndexes().size() > 0);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update selection for new data set column

   \param[in]  os32_DataSetColumn   Index of new data set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::UpdateSelectionForNewDataSetColumn(const int32_t os32_DataSetColumn)
{
   const QModelIndexList c_SelectedIndices = this->selectedIndexes();

   if ((c_SelectedIndices.size() > 0) && (this->mpc_ModelViewManager != NULL))
   {
      C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         const int32_t s32_DataSetOffset = pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eDATA_SET);
         std::vector<uint32_t> c_Rows;
         //Get all rows
         for (uint32_t u32_ItIndex = 0; static_cast<int32_t>(u32_ItIndex) < c_SelectedIndices.size(); ++u32_ItIndex)
         {
            const QModelIndex & rc_CurIndex = c_SelectedIndices[u32_ItIndex];
            c_Rows.push_back(rc_CurIndex.row());
         }
         //Make rows unique
         C_Uti::h_Uniqueify(c_Rows);
         //Select all rows (again)
         for (uint32_t u32_ItRow = 0; u32_ItRow < c_Rows.size(); ++u32_ItRow)
         {
            //this->selectRow(c_Rows[u32_ItRow]);
            this->selectionModel()->select(pc_Model->index(c_Rows[u32_ItRow],
                                                           static_cast<int32_t>(s32_DataSetOffset + os32_DataSetColumn)),
                                           QItemSelectionModel::Select);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Register model data reset
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::Reset(void)
{
   m_UpdateModelView();
   m_HandleColumnChange();
   m_CheckContextMenuText();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set selected variable names

   \param[in]  orc_VariableNames    New selected variable names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::SetSelectedVariableNames(const std::vector<QString> & orc_VariableNames)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         const C_OscNodeDataPoolList * const pc_List = C_PuiSdHandler::h_GetInstance()->GetOscDataPoolList(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);

         if (pc_List != NULL)
         {
            for (uint32_t u32_ItElement = 0; u32_ItElement < pc_List->c_Elements.size(); ++u32_ItElement)
            {
               const C_OscNodeDataPoolListElement & rc_Element = pc_List->c_Elements[u32_ItElement];
               for (uint32_t u32_ItVarName = 0; u32_ItVarName < orc_VariableNames.size(); ++u32_ItVarName)
               {
                  const QString & rc_VariableName = orc_VariableNames[u32_ItVarName];
                  if (rc_VariableName.compare(rc_Element.c_Name.c_str()) == 0)
                  {
                     //Match
                     this->selectRow(u32_ItElement);
                     //If selected, scroll there (first column)
                     this->scrollTo(pc_Model->index(u32_ItElement, 0));
                     break;
                  }
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get selected variable names

   \return
   Current selected variable names
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<QString> C_SdNdeDpListTableView::GetSelectedVariableNames(void) const
{
   std::vector<QString> c_Retval;
   const C_OscNodeDataPoolList * const pc_List = C_PuiSdHandler::h_GetInstance()->GetOscDataPoolList(
      this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
   if (pc_List != NULL)
   {
      const std::vector<uint32_t> c_Indices = this->m_GetSelectedIndices();
      c_Retval.reserve(c_Indices.size());
      for (uint32_t u32_ItVar = 0; u32_ItVar < c_Indices.size(); ++u32_ItVar)
      {
         const uint32_t u32_VarIndex = c_Indices[u32_ItVar];
         if (u32_VarIndex < pc_List->c_Elements.size())
         {
            const C_OscNodeDataPoolListElement & rc_Element = pc_List->c_Elements[u32_VarIndex];
            c_Retval.emplace_back(rc_Element.c_Name.c_str());
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete specified indices

   \param[in]  orc_Indices    Indices
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_DeleteIndices(const std::vector<uint32_t> & orc_Indices)
{
   std::vector<uint32_t> c_Indices = orc_Indices;
   C_Uti::h_Uniqueify(c_Indices);
   this->mc_UndoManager.DoDeleteElements(this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex,
                                         this->mpc_ModelViewManager,
                                         c_Indices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move item in tree

   \param[in]  orc_SourceIndices  Source index
   \param[in]  orc_TargetIndices  Target index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_Move(const std::vector<uint32_t> & orc_SourceIndices,
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
std::vector<uint32_t> C_SdNdeDpListTableView::m_GetSelectedIndices(void) const
{
   std::vector<uint32_t> c_Retval;
   QModelIndexList c_SelectedItems = this->selectedIndexes();

   c_Retval.reserve(c_SelectedItems.size());
   for (QModelIndexList::const_iterator c_ItSelectedItem = c_SelectedItems.begin();
        c_ItSelectedItem != c_SelectedItems.end(); ++c_ItSelectedItem)
   {
      const QModelIndex & rc_Item = *c_ItSelectedItem;
      c_Retval.push_back(rc_Item.row());
   }
   C_Uti::h_Uniqueify(c_Retval);
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init context menu entries
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_SetupContextMenu(void)
{
   this->mpc_ContextMenu = new C_OgeContextMenu(this);

   this->mpc_ActionAdd = this->mpc_ContextMenu->addAction("", this, &C_SdNdeDpListTableView::InsertAction,
                                                          static_cast<int32_t>(Qt::CTRL) +
                                                          static_cast<int32_t>(Qt::Key_Plus));
   this->mpc_ActionEditComment = this->mpc_ContextMenu->addAction("Edit Comment in Editor", this,
                                                                  &C_SdNdeDpListTableView::m_EditCommentInEditor,
                                                                  static_cast<int32_t>(Qt::Key_F10));

   this->mpc_ContextMenu->addSeparator();

   m_CheckContextMenuText();

   this->mpc_ActionCut = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                             "Cut"), this, &C_SdNdeDpListTableView::Cut,
                                                          static_cast<int32_t>(Qt::CTRL) +
                                                          static_cast<int32_t>(Qt::Key_X));
   this->mpc_ActionCopy = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                              "Copy"), this, &C_SdNdeDpListTableView::Copy,
                                                           static_cast<int32_t>(Qt::CTRL) +
                                                           static_cast<int32_t>(Qt::Key_C));
   this->mpc_ActionPaste = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                               "Paste"), this, &C_SdNdeDpListTableView::Paste,
                                                            static_cast<int32_t>(Qt::CTRL) +
                                                            static_cast<int32_t>(Qt::Key_V));

   this->mpc_ContextMenu->addSeparator();

   this->mpc_ActionMoveUp = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                                "Move Up"), this,
                                                             &C_SdNdeDpListTableView::DoMoveUp,
                                                             static_cast<int32_t>(Qt::CTRL) +
                                                             static_cast<int32_t>(Qt::Key_Up));
   this->mpc_ActionMoveDown = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                                  "Move Down"), this,
                                                               &C_SdNdeDpListTableView::DoMoveDown,
                                                               static_cast<int32_t>(Qt::CTRL) +
                                                               static_cast<int32_t>(Qt::Key_Down));

   this->mpc_ContextMenu->addSeparator();

   this->mpc_ActionDelete = this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText(
                                                                "Delete"), this, &C_SdNdeDpListTableView::Delete,
                                                             static_cast<int32_t>(Qt::Key_Delete));

   this->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(this, &C_SdNdeDpListTableView::customContextMenuRequested, this,
           &C_SdNdeDpListTableView::m_OnCustomContextMenuRequested);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Show custom context menu

   \param[in]  orc_Pos  Local context menu position
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_OnCustomContextMenuRequested(const QPoint & orc_Pos)
{
   this->mpc_ContextMenu->popup(this->viewport()->mapToGlobal(orc_Pos));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle column size change so hide necessary columns
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_HandleColumnChange(void)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         C_OscNodeDataPool::E_Type e_DataPoolType;
         //Show all columns
         for (uint32_t u32_Col = 0; u32_Col < static_cast<uint32_t>(pc_Model->columnCount()); ++u32_Col)
         {
            this->showColumn(u32_Col);
         }
         //Handle data pool type
         C_PuiSdHandler::h_GetInstance()->GetDataPoolType(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                          e_DataPoolType);
         if (e_DataPoolType == C_OscNodeDataPool::E_Type::eNVM)
         {
            this->hideColumn(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eEVENT_CALL));
         }
         else // DIAG Datapool
         {
            this->hideColumn(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eADDRESS));
            this->hideColumn(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eDATA_SIZE));
            //no extra handling for COMM & HAL Datapools because they do not use this list visualization
         }
         m_HandleColumnStateRestore();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Recheck if actions statuses are still up to date

   \param[in]  orc_SelectedIndices  Selected indices
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_CheckActions(const std::vector<uint32_t> & orc_SelectedIndices)
{
   if (orc_SelectedIndices.size() > 0)
   {
      bool q_AllowAdd;
      bool q_AllowMoveUp;
      bool q_AllowMoveDown;

      //Add
      if (static_cast<uint32_t>(this->model()->rowCount()) < C_OscNode::hu32_MAX_NUMBER_OF_ELEMENTS_PER_LIST)
      {
         q_AllowAdd = true;
      }
      else
      {
         q_AllowAdd = false;
      }
      //Move down & up
      q_AllowMoveUp = true;
      q_AllowMoveDown = true;
      if (this->mpc_ModelViewManager != NULL)
      {
         C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
         if (pc_Model != NULL)
         {
            for (uint32_t u32_ItSelectedIndex = 0; u32_ItSelectedIndex < orc_SelectedIndices.size();
                 ++u32_ItSelectedIndex)
            {
               if (orc_SelectedIndices[u32_ItSelectedIndex] == 0)
               {
                  q_AllowMoveUp = false;
               }
               if (orc_SelectedIndices[u32_ItSelectedIndex] == static_cast<uint32_t>(pc_Model->rowCount() - 1))
               {
                  q_AllowMoveDown = false;
               }
            }
         }
      }
      //Actions
      this->mq_AllowMoveDown = q_AllowMoveDown;
      if (this->mpc_ActionMoveDown != NULL)
      {
         this->mpc_ActionMoveDown->setEnabled(q_AllowMoveDown);
      }
      this->mq_AllowAdd = q_AllowAdd;
      this->mq_AllowMoveUp = q_AllowMoveUp;
      if (this->mpc_ActionMoveUp != NULL)
      {
         this->mpc_ActionMoveUp->setEnabled(q_AllowMoveUp);
      }
      //Simple Activation
      if (this->mpc_ActionCut != NULL)
      {
         this->mpc_ActionCut->setEnabled(true);
      }
      if (this->mpc_ActionCopy != NULL)
      {
         this->mpc_ActionCopy->setEnabled(true);
      }
      if (this->mpc_ActionPaste != NULL)
      {
         this->mpc_ActionPaste->setEnabled(true);
      }
      if (this->mpc_ActionDelete != NULL)
      {
         this->mpc_ActionDelete->setEnabled(true);
      }
      //Buttons
      Q_EMIT this->SigButtonChange(true, true, true, true, true, q_AllowMoveDown, q_AllowMoveUp);
   }
   else
   {
      //Simple Deactivation
      if (this->mpc_ActionCut != NULL)
      {
         this->mpc_ActionCut->setEnabled(false);
      }
      if (this->mpc_ActionCopy != NULL)
      {
         this->mpc_ActionCopy->setEnabled(false);
      }
      if (this->mpc_ActionPaste != NULL)
      {
         this->mpc_ActionPaste->setEnabled(false);
      }
      if (this->mpc_ActionDelete != NULL)
      {
         this->mpc_ActionDelete->setEnabled(false);
      }
      this->mq_AllowMoveDown = false;
      if (this->mpc_ActionMoveDown != NULL)
      {
         this->mpc_ActionMoveDown->setEnabled(this->mq_AllowMoveDown);
      }
      this->mq_AllowMoveUp = false;
      if (this->mpc_ActionMoveUp != NULL)
      {
         this->mpc_ActionMoveUp->setEnabled(this->mq_AllowMoveUp);
      }
      //Buttons
      Q_EMIT this->SigButtonChange(true, false, false, false, false, false, false);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update model
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_UpdateModelView(void)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex,
         this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         //Update necesssary!
         pc_Model->Reset();
         this->setModel(pc_Model);
         this->mc_Delegate.SetModel(pc_Model);
      }
   }
   if (this->mpc_ModelViewManager != NULL)
   {
      this->mpc_ModelViewManager->RegisterElementView(this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                                                      this->mu32_ListIndex, this);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle link clicked

   \param[in]  orc_Index   Index of clicked link
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_HandleLinkClicked(const QModelIndex & orc_Index)
{
   if (orc_Index.isValid() == true)
   {
      tgl_assert(this->mpc_ModelViewManager != NULL);
      if (this->mpc_ModelViewManager != NULL)
      {
         //Model
         C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
            this->mu32_NodeIndex,
            this->mu32_DataPoolIndex,
            this->mu32_ListIndex);
         if (pc_Model != NULL)
         {
            int32_t s32_DataSetIndex = -1;
            C_SdNdeDpUtil::E_ArrayEditType e_ArrayEditType;
            const C_SdNdeDpListTableModel::E_Columns e_Column = pc_Model->ColumnToEnum(
               orc_Index.column(), &s32_DataSetIndex);
            if (e_Column == C_SdNdeDpListTableModel::eMIN)
            {
               e_ArrayEditType = C_SdNdeDpUtil::eARRAY_EDIT_MIN;
            }
            else if (e_Column == C_SdNdeDpListTableModel::eMAX)
            {
               e_ArrayEditType = C_SdNdeDpUtil::eARRAY_EDIT_MAX;
            }
            else
            {
               e_ArrayEditType = C_SdNdeDpUtil::eARRAY_EDIT_DATA_SET;
            }
            const uint32_t u32_ElementIndex = static_cast<uint32_t>(orc_Index.row());
            const uint32_t u32_DataSetIndex = static_cast<uint32_t>(s32_DataSetIndex);
            C_OscNodeDataPoolListElement c_OscElement;
            C_PuiSdNodeDataPoolListElement c_UiElement;
            const QPointer<C_OgePopUpDialog> c_Dialog = new C_OgePopUpDialog(this, this);

            if (C_PuiSdHandler::h_GetInstance()->GetDataPoolListElement(this->mu32_NodeIndex,
                                                                        this->mu32_DataPoolIndex,
                                                                        this->mu32_ListIndex, u32_ElementIndex,
                                                                        c_OscElement,
                                                                        c_UiElement) == C_NO_ERR)
            {
               C_SdNdeDpListArrayEditWidget * const pc_ArrayEditWidget =
                  new C_SdNdeDpListArrayEditWidget(
                     *c_Dialog,
                     this->mu32_NodeIndex, this->mu32_DataPoolIndex,
                     this->mu32_ListIndex, u32_ElementIndex, e_ArrayEditType, u32_DataSetIndex);

               pc_ArrayEditWidget->SetModelViewManager(this->mpc_ModelViewManager);
               //Resize
               c_Dialog->SetSize(QSize(871, 318));
               if (c_Dialog->exec() == static_cast<int32_t>(QDialog::Accepted))
               {
                  //Register undo
                  QUndoCommand * const pc_UndoCommand = pc_ArrayEditWidget->TakeUndoCommand();
                  if (pc_UndoCommand != NULL)
                  {
                     //Undo all because push automatically redos
                     pc_UndoCommand->undo();
                     this->mc_UndoManager.DoPush(pc_UndoCommand);
                  }
               }
               else
               {
                  if (c_Dialog != NULL)
                  {
                     //Revert changes
                     C_PuiSdHandler::h_GetInstance()->SetDataPoolListElement(this->mu32_NodeIndex,
                                                                             this->mu32_DataPoolIndex,
                                                                             this->mu32_ListIndex, u32_ElementIndex,
                                                                             c_OscElement, c_UiElement);
                  }
               }
            } //lint !e429  //no memory leak because of the parent of pc_ArrayEditWidget and the Qt memory management
            if (c_Dialog != NULL)
            {
               c_Dialog->HideOverlay();
               c_Dialog->deleteLater();
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reinitialize context menu text
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_CheckContextMenuText(void)
{
   if (this->mpc_ActionAdd != NULL)
   {
      QString c_Text;
      const C_OscNodeDataPool * const pc_DataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
         this->mu32_NodeIndex,
         this->mu32_DataPoolIndex);

      if (pc_DataPool != NULL)
      {
         c_Text = C_PuiSdHandlerNodeLogic::h_GetElementTypeName(pc_DataPool->e_Type);
      }
      else
      {
         c_Text = C_GtGetText::h_GetText("Data Element");
      }

      this->mpc_ActionAdd->setText(C_GtGetText::h_GetText("Add new ") + c_Text);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   React to column resize

   Currently deactivated but kept if necessary in future
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_OnColumnResize(void)
{
   tgl_assert(this->mpc_ModelViewManager != NULL);
   if (this->mpc_ModelViewManager != NULL)
   {
      //Model
      C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
         this->mu32_NodeIndex,
         this->mu32_DataPoolIndex,
         this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         const int32_t s32_ColAuto =
            this->columnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eAUTO_MIN_MAX));
         const int32_t s32_ColEvent = this->columnWidth(pc_Model->EnumToColumn(
                                                           C_SdNdeDpListTableModel::eEVENT_CALL));
         if (this->ms32_LastIndicatorSize < std::max(s32_ColAuto, s32_ColEvent))
         {
            QString c_Style;
            this->ms32_LastIndicatorSize = std::max(s32_ColAuto, s32_ColEvent);

            //Stylesheet to make complete checkbox area interact-able
            c_Style = static_cast<QString>("stw--opensyde_gui--C_SdNdeDpListTableView::indicator {"
                                           "width: %1px;"
                                           "height: 30px;"
                                           "}").arg(this->ms32_LastIndicatorSize);
            this->setStyleSheet(c_Style);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get index of element after highest selected element

   \return
   Index of element after highest selected element
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SdNdeDpListTableView::m_GetOneAfterHighestSelected(void)
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
         const C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
         if (pc_Model != NULL)
         {
            const int32_t s32_RowCount = pc_Model->rowCount();
            if (s32_RowCount >= 0)
            {
               u32_Retval = static_cast<uint32_t>(s32_RowCount);
            }
         }
      }
   }
   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle column state store
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_HandleColumnStateSave(void) const
{
   if (this->model() != NULL)
   {
      const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
      const C_OscNodeDataPool * const pc_NodeDataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex);
      const C_OscNodeDataPoolList * const pc_NodeDataPoolList = C_PuiSdHandler::h_GetInstance()->GetOscDataPoolList(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);

      if (((pc_Node != NULL) && (pc_NodeDataPool != NULL)) && (pc_NodeDataPoolList != NULL))
      {
         const std::vector<int32_t> c_ColumnWidths = this->m_GetColumnWidths();
         C_UsHandler::h_GetInstance()->SetProjSdNodeDatapoolListColumnSizes(
            pc_Node->c_Properties.c_Name.c_str(), pc_NodeDataPool->c_Name.c_str(),
            pc_NodeDataPoolList->c_Name.c_str(), c_ColumnWidths);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle column state restore
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_HandleColumnStateRestore(void)
{
   if (this->mpc_ModelViewManager != NULL)
   {
      C_SdNdeDpListTableModel * const pc_Model = this->mpc_ModelViewManager->GetElementModel(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);
      if (pc_Model != NULL)
      {
         bool q_UserSettingsApplied = false;
         const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
         const C_OscNodeDataPool * const pc_NodeDataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex);
         const C_OscNodeDataPoolList * const pc_NodeDataPoolList = C_PuiSdHandler::h_GetInstance()->GetOscDataPoolList(
            this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex);

         if (((pc_Node != NULL) && (pc_NodeDataPool != NULL)) && (pc_NodeDataPoolList != NULL))
         {
            const C_UsNode c_UserNode =
               C_UsHandler::h_GetInstance()->GetProjSdNode(pc_Node->c_Properties.c_Name.c_str());
            const C_UsNodeDatapool c_UserDataPool = c_UserNode.GetDatapool(pc_NodeDataPool->c_Name.c_str());
            const C_UsNodeDatapoolList c_UserList = c_UserDataPool.GetOtherList(pc_NodeDataPoolList->c_Name.c_str());
            const std::vector<int32_t> & rc_ColumnWidths = c_UserList.GetColumnWidths();
            q_UserSettingsApplied = this->m_SetColumnWidths(rc_ColumnWidths);
         }
         if (q_UserSettingsApplied == false)
         {
            //Table size
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eINVALID), 45);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eICON), 20);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eINDEX), 50);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eNAME), 225);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eCOMMENT), 248);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eVALUE_TYPE), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eARRAY_SIZE), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eAUTO_MIN_MAX), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eMIN), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eMAX), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eFACTOR), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eOFFSET), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eUNIT), 40);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eACCESS), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eDATA_SIZE), 85);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eADDRESS), 85);
            //this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eDATA_SET), 150);
            this->setColumnWidth(pc_Model->EnumToColumn(C_SdNdeDpListTableModel::eEVENT_CALL), 85);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot to set focus of comment field after user confirms with defined keys of KeyPressEvent in C_OgeTedTable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListTableView::m_CommentConfirmed(void)
{
   this->setFocus();
}
