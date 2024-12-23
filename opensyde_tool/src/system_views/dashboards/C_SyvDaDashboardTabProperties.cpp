//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Properties dialog for dashboard tab properties

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SyvDaDashboardTabProperties.hpp"
#include "ui_C_SyvDaDashboardTabProperties.h"
#include "C_GtGetText.hpp"
#include "C_PuiSvData.hpp"
#include "C_PuiSvHandler.hpp"
#include "stwerrors.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_OgeWiCustomMessage.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;
using namespace stw::errors;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out]  orc_Parent             Reference to parent
   \param[in]      orc_Name               Name of the bus for the title
   \param[in]      ou32_DashboardIndex    Index of dashboard for editing (not relevant if oq_NewDashboard is true)
   \param[in]      ou32_ViewIndex         Index of current view
   \param[in]      oq_NewDashboard        Flag if new dashboard for creation ()
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaDashboardTabProperties::C_SyvDaDashboardTabProperties(C_OgePopUpDialog & orc_Parent, const QString & orc_Name,
                                                             const uint32_t ou32_DashboardIndex,
                                                             const uint32_t ou32_ViewIndex,
                                                             const bool oq_NewDashboard) :
   QWidget(&orc_Parent),
   mpc_Ui(new Ui::C_SyvDaDashboardTabProperties()),
   mpc_ParentDialog(&orc_Parent),
   mu32_DashboardIndex(ou32_DashboardIndex),
   mu32_ViewIndex(ou32_ViewIndex),
   mq_NewDashboard(oq_NewDashboard)
{
   //  UI
   mpc_Ui->setupUi(this);

   // register the widget for showing
   this->mpc_ParentDialog->SetWidget(this);

   // set main title
   this->mpc_ParentDialog->SetTitle(orc_Name);

   //BEFORE load
   InitStaticNames();

   this->mpc_Ui->pc_RbCommonDashboard->setEnabled(oq_NewDashboard);
   this->mpc_Ui->pc_RbChartDashboard->setEnabled(oq_NewDashboard);

   // init name and comment
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);
   if (pc_View != NULL)
   {
      if (this->mq_NewDashboard == false)
      {
         const C_PuiSvDashboard * const pc_Dashboard = pc_View->GetDashboard(this->mu32_DashboardIndex);
         if (pc_Dashboard != NULL)
         {
            this->mpc_Ui->pc_LineEditName->setText(pc_Dashboard->GetName());
            this->mpc_Ui->pc_TedComment->setText(pc_Dashboard->GetComment());

            if (pc_Dashboard->GetType() == C_PuiSvDashboard::eSCENE)
            {
               this->mpc_Ui->pc_RbCommonDashboard->setChecked(true);
               this->mpc_Ui->pc_RbChartDashboard->setChecked(false);
            }
            else
            {
               this->mpc_Ui->pc_RbCommonDashboard->setChecked(false);
               this->mpc_Ui->pc_RbChartDashboard->setChecked(true);
            }
         }
      }
      else
      {
         // New dashboard, get a unique name
         this->mpc_Ui->pc_LineEditName->setText(pc_View->GetUniqueDashboardName("Dashboard"));
         this->mpc_Ui->pc_TedComment->setText("");

         this->mpc_Ui->pc_RbCommonDashboard->setChecked(oq_NewDashboard);
      }
   }

   // connects
   connect(this->mpc_Ui->pc_BushButtonOk, &QPushButton::clicked, this, &C_SyvDaDashboardTabProperties::m_OkClicked);
   connect(this->mpc_Ui->pc_BushButtonCancel, &QPushButton::clicked,
           this, &C_SyvDaDashboardTabProperties::m_CancelClicked);
   connect(this->mpc_Ui->pc_LineEditName, &QLineEdit::textChanged, this,
           &C_SyvDaDashboardTabProperties::m_CheckDashboardTabName);
   connect(this->mpc_Ui->pc_RbCommonDashboard, &QRadioButton::clicked,
           this, &C_SyvDaDashboardTabProperties::m_SetCommonDashboardType);
   connect(this->mpc_Ui->pc_RbChartDashboard, &QRadioButton::clicked,
           this, &C_SyvDaDashboardTabProperties::m_SetChartDashboardType);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{1540}  no memory leak because of the parent of mpc_ParentDialog and the Qt memory management
C_SyvDaDashboardTabProperties::~C_SyvDaDashboardTabProperties(void)
{
   delete mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardTabProperties::InitStaticNames(void)
{
   this->mpc_ParentDialog->SetSubTitle(C_GtGetText::h_GetText("Properties"));
   this->mpc_Ui->pc_BushButtonOk->setText(C_GtGetText::h_GetText("OK"));
   this->mpc_Ui->pc_BushButtonCancel->setText(C_GtGetText::h_GetText("Cancel"));
   this->mpc_Ui->pc_TedComment->setPlaceholderText(C_GtGetText::h_GetText("Add your comment here ..."));
   this->mpc_Ui->pc_LabelName->setText(C_GtGetText::h_GetText("Name"));
   this->mpc_Ui->pc_LabelComment->setText(C_GtGetText::h_GetText("Comment"));
   this->mpc_Ui->pc_LabelHintComment->setText(
      C_GtGetText::h_GetText("Note: Comment is shown in tooltip on tab hover."));
   this->mpc_Ui->pc_LabelType->setText(C_GtGetText::h_GetText("Type"));
   this->mpc_Ui->pc_RbCommonDashboard->setText(C_GtGetText::h_GetText("Common Dashboard"));
   this->mpc_Ui->pc_RbChartDashboard->setText(C_GtGetText::h_GetText("Chart Dashboard"));

   //Tool tips
   this->mpc_Ui->pc_LabelName->SetToolTipInformation(C_GtGetText::h_GetText("Name"),
                                                     C_GtGetText::h_GetText(
                                                        "Unique Dashboard tab name. Unique within a view."));

   this->mpc_Ui->pc_LabelComment->SetToolTipInformation(C_GtGetText::h_GetText("Comment"),
                                                        C_GtGetText::h_GetText("Comment for this Dashboard tab, that "
                                                                               "is shown in tooltip on tab hover."));
   this->mpc_Ui->pc_RbCommonDashboard->SetToolTipInformation(C_GtGetText::h_GetText("Common Dashboard"),
                                                             C_GtGetText::h_GetText("Empty Dashboard"));
   this->mpc_Ui->pc_RbChartDashboard->SetToolTipInformation(C_GtGetText::h_GetText("Chart Dashboard"),
                                                            C_GtGetText::h_GetText("Dashboard with a chart"));
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get name of dashboard tab.

   \return
   name of dashboard tab
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SyvDaDashboardTabProperties::GetDashboardTabName(void) const
{
   return this->mpc_Ui->pc_LineEditName->text();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get comment of dashboard tab.

   \return
   name of dashboard tab
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SyvDaDashboardTabProperties::GetDashboardTabComment(void) const
{
   return this->mpc_Ui->pc_TedComment->toPlainText();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get tab type of dashboard tab.

   \return
   type of dashboard tab
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSvDashboard::E_TabType C_SyvDaDashboardTabProperties::GetDashboardTabType() const
{
   C_PuiSvDashboard::E_TabType e_TabType = C_PuiSvDashboard::eSCENE;

   if (this->mpc_Ui->pc_RbChartDashboard->isChecked() == true)
   {
      e_TabType = C_PuiSvDashboard::eCHART;
   }

   return e_TabType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: Handle specific enter key cases

   \param[in,out]  opc_KeyEvent  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardTabProperties::keyPressEvent(QKeyEvent * const opc_KeyEvent)
{
   bool q_CallOrg = true;

   //Handle all enter key cases manually
   if ((opc_KeyEvent->key() == Qt::Key_Enter) || (opc_KeyEvent->key() == Qt::Key_Return))
   {
      if (((opc_KeyEvent->modifiers().testFlag(Qt::ControlModifier) == true) &&
           (opc_KeyEvent->modifiers().testFlag(Qt::AltModifier) == false)) &&
          (opc_KeyEvent->modifiers().testFlag(Qt::ShiftModifier) == false))
      {
         m_OkClicked();
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
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardTabProperties::m_OkClicked(void)
{
   if (this->m_CheckDashboardTabName() == true)
   {
      if (this->mpc_ParentDialog != NULL)
      {
         this->mpc_ParentDialog->accept();
      }
   }
   else
   {
      C_OgeWiCustomMessage c_MessageBox(this, C_OgeWiCustomMessage::E_Type::eERROR);
      c_MessageBox.SetHeading(C_GtGetText::h_GetText("Dashboard rename"));
      c_MessageBox.SetDescription(C_GtGetText::h_GetText(
                                     "This Dashboard name already exists. Choose another name."));
      c_MessageBox.SetCustomMinHeight(180, 180);
      c_MessageBox.Execute();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Cancel button
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardTabProperties::m_CancelClicked(void)
{
   if (this->mpc_ParentDialog != NULL)
   {
      this->mpc_ParentDialog->reject();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check Dashboard name

   Check if Dashboard name is valid (i.e. unique) and if not color line edit font red
   and show error tooltip.

   \retval   True    Dashboard name is unique
   \retval   False   Dashboard name is duplicated
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SyvDaDashboardTabProperties::m_CheckDashboardTabName(void) const
{
   bool q_ValidName = false;
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);

   if (pc_View != NULL)
   {
      const QString c_NewName = this->mpc_Ui->pc_LineEditName->text();
      const uint32_t * pu32_DashboardIndex = NULL;

      if (this->mq_NewDashboard == false)
      {
         // Only in case of an existing dashboard a valid index exists
         pu32_DashboardIndex = &this->mu32_DashboardIndex;
      }

      if (pc_View->CheckDashboardName(c_NewName, pu32_DashboardIndex) == true)
      {
         q_ValidName = true;
      }

      // red font if not valid
      C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_LineEditName, "Valid", q_ValidName);

      // show error tooltip if invalid and no tooltip else
      if (q_ValidName == true)
      {
         this->mpc_Ui->pc_LineEditName->SetToolTipInformation(C_GtGetText::h_GetText(""),
                                                              C_GtGetText::h_GetText(""),
                                                              C_NagToolTip::eDEFAULT);
      }
      else
      {
         const QString c_Heading = C_GtGetText::h_GetText("Dashboard Name");
         const QString c_Content = C_GtGetText::h_GetText("Name is already in use.");
         this->mpc_Ui->pc_LineEditName->SetToolTipInformation(c_Heading, c_Content, C_NagToolTip::eERROR);
      }
   }

   return q_ValidName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot of common dashboard radio button

   \param[in]  oq_IsChecked   Checked state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardTabProperties::m_SetCommonDashboardType(const bool oq_IsChecked) const
{
   this->mpc_Ui->pc_RbCommonDashboard->setChecked(oq_IsChecked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot of chart dashboard radio button

   \param[in]  oq_IsChecked   Checked state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardTabProperties::m_SetChartDashboardType(const bool oq_IsChecked) const
{
   this->mpc_Ui->pc_RbChartDashboard->setChecked(oq_IsChecked);
}
