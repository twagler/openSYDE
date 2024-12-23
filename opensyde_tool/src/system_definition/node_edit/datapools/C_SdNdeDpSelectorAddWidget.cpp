//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for adding a new Datapool and configuring if it is a stand alone or shared Datapool

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SdNdeDpSelectorAddWidget.hpp"
#include "ui_C_SdNdeDpSelectorAddWidget.h"

#include "stwerrors.hpp"
#include "TglUtils.hpp"
#include "C_GtGetText.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_SdNdeDpUtil.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_PuiProject.hpp"
#include "C_Uti.hpp"
#include "C_UsHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] orc_Parent           Reference to parent
   \param[in]     ou32_NodeIndex       Node index
   \param[in,out] orc_OscDataPool      Reference to the actual core datapool object
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpSelectorAddWidget::C_SdNdeDpSelectorAddWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                                       const uint32_t ou32_NodeIndex,
                                                       C_OscNodeDataPool & orc_OscDataPool) :
   QWidget(&orc_Parent),
   mpc_Ui(new Ui::C_SdNdeDpSelectorAddWidget),
   mrc_ParentDialog(orc_Parent),
   mu32_NodeIndex(ou32_NodeIndex),
   mrc_OscDataPool(orc_OscDataPool),
   mc_RamViewFilePath("")
{
   this->mpc_Ui->setupUi(this);

   // Init widget
   InitStaticNames();
   this->m_InitFromData();
   this->m_DisableSharedSection();
   this->m_OnSharedDataPoolChanged();

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this, &C_SdNdeDpSelectorAddWidget::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this,
           &C_SdNdeDpSelectorAddWidget::m_CancelClicked);

   connect(this->mpc_Ui->pc_RadioButtonStandAlone, &stw::opensyde_gui_elements::C_OgeRabProperties::toggled, this,
           &C_SdNdeDpSelectorAddWidget::m_DisableSharedSection);
   connect(this->mpc_Ui->pc_RadioButtonShared, &stw::opensyde_gui_elements::C_OgeRabProperties::toggled, this,
           &C_SdNdeDpSelectorAddWidget::m_EnableSharedSection);
   connect(this->mpc_Ui->pc_RadiButtonRamViewImport, &stw::opensyde_gui_elements::C_OgeRabProperties::toggled, this,
           &C_SdNdeDpSelectorAddWidget::m_DisableSharedSection);

   //lint -e{929}  Qt interface
   connect(this->mpc_Ui->pc_ComboBoxSharedDatapool,
           static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
           &C_SdNdeDpSelectorAddWidget::m_OnSharedDataPoolChanged);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpSelectorAddWidget::~C_SdNdeDpSelectorAddWidget()
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::InitStaticNames(void) const
{
   this->mrc_ParentDialog.SetTitle(C_GtGetText::h_GetText("Datapools"));
   this->mrc_ParentDialog.SetSubTitle(C_GtGetText::h_GetText("Add new Datapool"));
   this->mpc_Ui->pc_LabelHeading->setText(C_GtGetText::h_GetText("Select Type"));

   this->mpc_Ui->pc_RadioButtonStandAlone->setText(C_GtGetText::h_GetText("Stand-alone Datapool"));
   this->mpc_Ui->pc_RadioButtonShared->setText(C_GtGetText::h_GetText("Shared Datapool"));
   this->mpc_Ui->pc_RadiButtonRamViewImport->setText(C_GtGetText::h_GetText("Import from RAMView Project"));

   this->mpc_Ui->pc_PushButtonOk->setText(C_GtGetText::h_GetText("Continue"));
   this->mpc_Ui->pc_PushButtonCancel->setText(C_GtGetText::h_GetText("Cancel"));
   this->mpc_Ui->pc_LabelSharedDatapool->setText(C_GtGetText::h_GetText("Share with"));
   this->mpc_Ui->pc_LabelSharedDatapoolInfo->setText(C_GtGetText::h_GetText("Already shared with:"));

   //tooltips
   this->mpc_Ui->pc_RadioButtonStandAlone->SetToolTipInformation(C_GtGetText::h_GetText("Stand-alone Datapool"),
                                                                 C_GtGetText::h_GetText(
                                                                    "Default type. A Datapool without relationship to other Datapools."));

   this->mpc_Ui->pc_RadioButtonShared->SetToolTipInformation(C_GtGetText::h_GetText("Shared Datapool"),
                                                             C_GtGetText::h_GetText(
                                                                "A Datapool with relationship to other Datapools.\n"
                                                                "Datapool configuration and properties are synchronized"
                                                                "within shared Datapools."));
   this->mpc_Ui->pc_RadiButtonRamViewImport->SetToolTipInformation(C_GtGetText::h_GetText(
                                                                      "Import from RAMView Project"),
                                                                   C_GtGetText::h_GetText(
                                                                      "A Datapool that gets imported from a RAMView "
                                                                      "project by loading Datapool lists from a *.def file."));

   this->mpc_Ui->pc_LabelSharedDatapool->SetToolTipInformation(C_GtGetText::h_GetText("Share with"),
                                                               C_GtGetText::h_GetText("Select share partner Datapool."));

   this->mpc_Ui->pc_LabelSharedDatapoolInfo->SetToolTipInformation(C_GtGetText::h_GetText("Already shared with"),
                                                                   C_GtGetText::h_GetText(
                                                                      "List of Datapools which are already shared with "
                                                                      "selected shared partner Datapool."));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns the result of the dialog.

   \param[out]  orc_SharedDatapoolId   Datapool ID of selected shared datapool partner to the new datapool
                                       Will be filled if at least one shareable datapool is available
   \param[out]  orc_RamViewFilePath    RAMView project file name (*.def)

   \return
   Dialog result: Stand-alone Datapool or shared Datapool or Datapool to import from a RAMView project
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpSelectorAddWidget::E_SelectionResult C_SdNdeDpSelectorAddWidget::GetDialogResult(
   C_OscNodeDataPoolId & orc_SharedDatapoolId, QString & orc_RamViewFilePath) const
{
   E_SelectionResult e_Result = eSTANDALONE;

   if (this->mpc_Ui->pc_RadioButtonStandAlone->isChecked())
   {
      e_Result = eSTANDALONE;
   }
   else if (this->mpc_Ui->pc_RadioButtonShared->isChecked())
   {
      e_Result = eSHARED;
      this->m_GetSelectedSharedDatapool(orc_SharedDatapoolId);
   }
   else if (this->mpc_Ui->pc_RadiButtonRamViewImport->isChecked())
   {
      e_Result = eRAMVIEWIMPORT;
      orc_RamViewFilePath = this->mc_RamViewFilePath;
   }
   else
   {
      // No button checked can not happen
      tgl_assert(false);
   }

   return e_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: Handle specific enter key cases

   \param[in,out]  opc_KeyEvent  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::keyPressEvent(QKeyEvent * const opc_KeyEvent)
{
   bool q_CallOrg = true;

   //Handle all enter key cases manually
   if ((opc_KeyEvent->key() == Qt::Key_Enter) || (opc_KeyEvent->key() == Qt::Key_Return))
   {
      if (((opc_KeyEvent->modifiers().testFlag(Qt::ControlModifier) == true) &&
           (opc_KeyEvent->modifiers().testFlag(Qt::AltModifier) == false)) &&
          (opc_KeyEvent->modifiers().testFlag(Qt::ShiftModifier) == false))
      {
         this->m_OkClicked();
      }
      else
      {
         q_CallOrg = false;
      }
   }
   if (q_CallOrg == true)
   {
      QWidget::keyPressEvent(opc_KeyEvent);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init dynamic content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_InitFromData(void)
{
   // Fill the combo box with all datapools of the same type beginning with datapools
   // of the same node
   const C_OscNode * pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
   uint32_t u32_DatapoolCounter;
   uint32_t u32_NodeCounter;
   C_OscNodeDataPoolId c_DatapoolId;

   // Add datapools of current node
   if (pc_Node != NULL)
   {
      c_DatapoolId.u32_NodeIndex = this->mu32_NodeIndex;

      for (u32_DatapoolCounter = 0U; u32_DatapoolCounter < pc_Node->c_DataPools.size(); ++u32_DatapoolCounter)
      {
         const C_OscNodeDataPool & rc_Datapool = pc_Node->c_DataPools[u32_DatapoolCounter];

         if (rc_Datapool.e_Type == this->mrc_OscDataPool.e_Type)
         {
            const QString c_DatapoolName = rc_Datapool.c_Name.c_str();
            this->mpc_Ui->pc_ComboBoxSharedDatapool->addItem(c_DatapoolName);

            // Save the unique identification for the datapool
            c_DatapoolId.u32_DataPoolIndex = u32_DatapoolCounter;
            this->mc_AvailableDatapools[c_DatapoolName] = c_DatapoolId;
         }
      }
   }

   // Add datapools of all other nodes
   for (u32_NodeCounter = 0U; u32_NodeCounter < C_PuiSdHandler::h_GetInstance()->GetOscNodesSize(); ++u32_NodeCounter)
   {
      if (u32_NodeCounter != this->mu32_NodeIndex)
      {
         pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(u32_NodeCounter);

         if (pc_Node != NULL)
         {
            const QString c_NodeName = static_cast<QString>(pc_Node->c_Properties.c_Name.c_str()) + "::";

            c_DatapoolId.u32_NodeIndex = u32_NodeCounter;

            for (u32_DatapoolCounter = 0U; u32_DatapoolCounter < pc_Node->c_DataPools.size(); ++u32_DatapoolCounter)
            {
               const C_OscNodeDataPool & rc_Datapool = pc_Node->c_DataPools[u32_DatapoolCounter];

               if (rc_Datapool.e_Type == this->mrc_OscDataPool.e_Type)
               {
                  const QString c_Text = c_NodeName + static_cast<QString>(rc_Datapool.c_Name.c_str());
                  this->mpc_Ui->pc_ComboBoxSharedDatapool->addItem(c_Text);

                  // Save the unique identification for the datapool
                  c_DatapoolId.u32_DataPoolIndex = u32_DatapoolCounter;
                  this->mc_AvailableDatapools[c_Text] = c_DatapoolId;
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_OkClicked(void)
{
   bool q_Continue = true;

   if (this->mpc_Ui->pc_RadiButtonRamViewImport->isChecked() == true)
   {
      QString c_Folder = C_UsHandler::h_GetInstance()->GetLastKnownRamViewProjectPath();

      // use project folder path if no path is known
      if (c_Folder.isEmpty() == true)
      {
         c_Folder = C_PuiProject::h_GetInstance()->GetFolderPath();
      }

      // default to exe if path is empty (i.e. project is not saved yet)
      if (c_Folder.isEmpty() == true)
      {
         c_Folder = C_Uti::h_GetExePath();
      }

      mc_RamViewFilePath = C_OgeWiUtil::h_GetOpenFileName(this, C_GtGetText::h_GetText("Select RAMView project"),
                                                          c_Folder, "*.def", "*.def");

      // return to dialog if user canceled file selection
      if (mc_RamViewFilePath.isEmpty() == true)
      {
         q_Continue = false;
      }
      else
      {
         // remember user setting
         C_UsHandler::h_GetInstance()->SetLastKnownRamViewProjectPath(mc_RamViewFilePath);
      }
   }

   if (q_Continue == true)
   {
      this->mrc_ParentDialog.accept();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Cancel button
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_CancelClicked(void)
{
   this->mrc_ParentDialog.reject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot for change to radio button Stand-alone
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_DisableSharedSection(void) const
{
   this->mpc_Ui->pc_ComboBoxSharedDatapool->setEnabled(false);
   this->mpc_Ui->pc_LabelSharedDatapool->setEnabled(false);
   this->mpc_Ui->pc_LabelSharedDatapoolInfo->setEnabled(false);
   this->mpc_Ui->pc_ListWidgetSharedDatapoolInfo->setEnabled(false);

   if (this->mpc_Ui->pc_ComboBoxSharedDatapool->count() == 0)
   {
      this->mpc_Ui->pc_RadioButtonShared->setEnabled(false);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot for change to radio button Share
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_EnableSharedSection(void) const
{
   this->mpc_Ui->pc_ComboBoxSharedDatapool->setEnabled(true);
   this->mpc_Ui->pc_ListWidgetSharedDatapoolInfo->setEnabled(true);
   this->mpc_Ui->pc_LabelSharedDatapool->setEnabled(true);
   this->mpc_Ui->pc_LabelSharedDatapoolInfo->setEnabled(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot for change of the combo box with the selected shared datapool
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_OnSharedDataPoolChanged(void) const
{
   const C_PuiSdSharedDatapools & rc_SharedDatapools = C_PuiSdHandler::h_GetInstance()->GetSharedDatapoolsConst();
   C_OscNodeDataPoolId c_SelectedDatapoolId;
   uint32_t u32_SharedGroup = 0U;

   // Remove previous results
   this->mpc_Ui->pc_ListWidgetSharedDatapoolInfo->clear();

   // Search for already shared datapools with the selected datapool and update the list
   this->m_GetSelectedSharedDatapool(c_SelectedDatapoolId);

   // c_SelectedDatapoolId is only valid if the combo box has at lest one entry
   if ((this->mpc_Ui->pc_ComboBoxSharedDatapool->count() > 0) &&
       (rc_SharedDatapools.IsSharedDatapool(c_SelectedDatapoolId, &u32_SharedGroup) == true))
   {
      // Datapool is already shared
      std::vector<QString> c_SharedDatapoolGroup;
      uint32_t u32_DatapoolCounter;

      tgl_assert(C_SdNdeDpUtil::h_GetSharedDatapoolGroup(u32_SharedGroup, c_SelectedDatapoolId,
                                                         this->mu32_NodeIndex,
                                                         c_SharedDatapoolGroup) == C_NO_ERR);

      for (u32_DatapoolCounter = 0U; u32_DatapoolCounter < c_SharedDatapoolGroup.size(); ++u32_DatapoolCounter)
      {
         this->mpc_Ui->pc_ListWidgetSharedDatapoolInfo->addItem(c_SharedDatapoolGroup[u32_DatapoolCounter]);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the result of the dialog if a shared datapool is selected

   \param[out]  orc_SharedDatapoolId   Datapool ID of selected shared datapool partner to the new datapool
                                       Will be filled if at least one shareable datapool is available
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpSelectorAddWidget::m_GetSelectedSharedDatapool(C_OscNodeDataPoolId & orc_SharedDatapoolId) const
{
   if (this->mpc_Ui->pc_ComboBoxSharedDatapool->count() > 0)
   {
      const std::map<QString, stw::opensyde_core::C_OscNodeDataPoolId>::const_iterator c_ItDatapool =
         this->mc_AvailableDatapools.find(this->mpc_Ui->pc_ComboBoxSharedDatapool->currentText());

      tgl_assert(c_ItDatapool != this->mc_AvailableDatapools.end());
      if (c_ItDatapool != this->mc_AvailableDatapools.end())
      {
         orc_SharedDatapoolId = c_ItDatapool->second;
      }
   }
}
