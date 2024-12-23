//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for selecting a system definition node for DCF/EDS file import (implementation)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QFileInfo>

#include "stwerrors.hpp"
#include "TglUtils.hpp"
#include "C_GtGetText.hpp"
#include "C_Uti.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_SdUtil.hpp"
#include "C_PuiSdUtil.hpp"
#include "C_CieDcfEdsImportNodeSelectWidget.hpp"
#include "ui_C_CieDcfEdsImportNodeSelectWidget.h"
#include "C_OgeWiCustomMessage.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out] orc_Parent            Reference to parent
   \param[in]     orc_FilePath          File Path to display
   \param[in]     ou32_BusIndex         Bus index
*/
//----------------------------------------------------------------------------------------------------------------------
C_CieDcfEdsImportNodeSelectWidget::C_CieDcfEdsImportNodeSelectWidget(
   stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent, const QString & orc_FilePath,
   const uint32_t ou32_BusIndex) :
   QWidget(&orc_Parent),
   mpc_Ui(new Ui::C_CieDcfEdsImportNodeSelectWidget),
   mrc_ParentDialog(orc_Parent)
{
   this->mpc_Ui->setupUi(this);

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   // initialize content
   this->InitStaticNames();
   this->mpc_Ui->pc_LabPath->setText(orc_FilePath);
   //lint -e{1938} //we don't create global objects of this class; no race conditions can occur
   this->mpc_Ui->pc_LabPath->setText(
      C_Uti::h_MinimizePath(orc_FilePath, C_Uti::h_GetFontPixel(mc_STYLE_GUIDE_FONT_REGULAR_13), 500, 0));
   this->mpc_Ui->pc_LabPath->SetToolTipInformation(static_cast<QFileInfo>(orc_FilePath).completeSuffix().toUpper() +
                                                   C_GtGetText::h_GetText(" Configuration File"),
                                                   C_GtGetText::h_GetText(orc_FilePath.toStdString().c_str()));
   mrc_ParentDialog.SetSubTitle(static_cast<QFileInfo>(orc_FilePath).completeSuffix().toUpper() +
                                C_GtGetText::h_GetText(" File"));

   if (static_cast<QFileInfo>(orc_FilePath).completeSuffix().toUpper() == "DCF")
   {
      const QSize c_SIZE(717, 375);
      mrc_ParentDialog.SetSize(c_SIZE);
   }
   else
   {
      const QSize c_SIZE(720, 375);
      mrc_ParentDialog.SetSize(c_SIZE);
   }

   this->m_FillUpComboBox(ou32_BusIndex);

   //Connects
   connect(this->mpc_Ui->pc_PushButtonOk, &stw::opensyde_gui_elements::C_OgePubDialog::clicked, this,
           &C_CieDcfEdsImportNodeSelectWidget::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &stw::opensyde_gui_elements::C_OgePubDialog::clicked, this,
           &C_CieDcfEdsImportNodeSelectWidget::m_CancelClicked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_CieDcfEdsImportNodeSelectWidget::~C_CieDcfEdsImportNodeSelectWidget(void)
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieDcfEdsImportNodeSelectWidget::InitStaticNames(void)
{
   // window title
   this->mrc_ParentDialog.SetTitle(C_GtGetText::h_GetText("Import Messages"));
   this->mrc_ParentDialog.SetSubTitle(C_GtGetText::h_GetText("EDS/DCF File"));

   // labels
   this->mpc_Ui->pc_LabelHeading->setText(C_GtGetText::h_GetText("Configuration"));
   this->mpc_Ui->pc_LabelNode->setText(C_GtGetText::h_GetText("Assignee"));
   this->mpc_Ui->pc_LabelNode->SetToolTipInformation(
      C_GtGetText::h_GetText("Assignee"),
      C_GtGetText::h_GetText("Select a node from Network Topology. All messages get imported to this node."));
   this->mpc_Ui->pc_LabPathTitle->setText(C_GtGetText::h_GetText("Path:"));

   // buttons
   this->mpc_Ui->pc_PushButtonCancel->setText(C_GtGetText::h_GetText("Cancel"));
   this->mpc_Ui->pc_PushButtonOk->setText(C_GtGetText::h_GetText("Continue"));
}

//-------------------------------------------------------------------------------------------f---------------------------
/*! \brief   Get user selected node.

   \param[out] oru32_NodeIndex         index of selected node
   \param[out] oru32_InterfaceIndex    index of interface for selected node

   \return
   C_NO_ERR    everything fine
   C_RANGE     something went wrong
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CieDcfEdsImportNodeSelectWidget::GetNodeSelection(uint32_t & oru32_NodeIndex,
                                                            uint32_t & oru32_InterfaceIndex) const
{
   int32_t s32_Return = C_RANGE;
   const int32_t s32_CurrentIndex = this->mpc_Ui->pc_CbxNode->currentIndex();

   if ((s32_CurrentIndex >= 0) && (this->mc_InterfaceIndexes.size() == this->mc_NodeIndexes.size()))
   {
      if (s32_CurrentIndex < static_cast<int32_t>(this->mc_NodeIndexes.size()))
      {
         oru32_NodeIndex = this->mc_NodeIndexes[s32_CurrentIndex];
         oru32_InterfaceIndex = this->mc_InterfaceIndexes[s32_CurrentIndex];
         s32_Return = C_NO_ERR;
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: Handle specific enter key cases

   \param[in,out] opc_KeyEvent Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieDcfEdsImportNodeSelectWidget::keyPressEvent(QKeyEvent * const opc_KeyEvent)
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
/*! \brief   User clicked on ok button.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieDcfEdsImportNodeSelectWidget::m_OkClicked(void) const
{
   this->mrc_ParentDialog.accept();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   User clicked on cancel button.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieDcfEdsImportNodeSelectWidget::m_CancelClicked(void) const
{
   this->mrc_ParentDialog.reject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize ComboBox for node selection.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieDcfEdsImportNodeSelectWidget::m_FillUpComboBox(const uint32_t ou32_BusIndex)
{
   std::vector<QString> c_Names;

   // cleanup
   this->mpc_Ui->pc_CbxNode->clear();

   // get node and interface names
   C_PuiSdHandler::h_GetInstance()->GetOscSystemDefinitionConst().GetNodeIndexesOfBus(ou32_BusIndex,
                                                                                      this->mc_NodeIndexes,
                                                                                      this->mc_InterfaceIndexes);

   tgl_assert(C_SdUtil::h_GetNames(this->mc_NodeIndexes, this->mc_InterfaceIndexes, c_Names, false) == C_NO_ERR);

   for (std::vector<QString>::const_iterator c_NodeIt = c_Names.begin(); c_NodeIt != c_Names.end(); ++c_NodeIt)
   {
      this->mpc_Ui->pc_CbxNode->addItem(*c_NodeIt);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Node change on user request

   \param[in] ou32_NodeIndex         index of selected node
   \param[in] ou32_InterfaceIndex    index of interface for selected node
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieDcfEdsImportNodeSelectWidget::NodeIdToBeChanged(const uint32_t ou32_NodeIndex,
                                                          const uint32_t ou32_InterfaceIndex)
{
   C_SdUtil::h_NodeIdToBeChanged(ou32_NodeIndex, ou32_InterfaceIndex, this);
}
