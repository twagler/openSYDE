//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label with custom context menu (implementation)

   Label with custom context menu in openSYDE style.
   This is only needed for labels with text interaction flag
   Qt::TextSelectableByMouse.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QApplication>
#include <QClipboard>

#include "C_OgeLabContextMenuBase.hpp"
#include "stwtypes.hpp"
#include "C_GtGetText.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;

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
C_OgeLabContextMenuBase::C_OgeLabContextMenuBase(QWidget * const opc_Parent) :
   C_OgeLabToolTipBase(opc_Parent),
   mpc_ContextMenu(NULL)
{
   m_InitContextMenu();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize custom context menu functionality
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabContextMenuBase::m_InitContextMenu(void)
{
   this->setContextMenuPolicy(Qt::CustomContextMenu);
   this->mpc_ContextMenu = new C_OgeContextMenu(this);
   connect(this, &C_OgeLabContextMenuBase::customContextMenuRequested, this,
           &C_OgeLabContextMenuBase::m_OnCustomContextMenuRequested);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Setup context menu entries
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabContextMenuBase::m_SetupContextMenu(void)
{
   // reset to empty menu
   this->mpc_ContextMenu->clear();

   // add the actions
   this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Copy"),
                                    static_cast<int32_t>(Qt::CTRL) + static_cast<int32_t>(Qt::Key_C),
                                    this,
                                    &C_OgeLabContextMenuBase::m_Copy);

   this->mpc_ContextMenu->addSeparator();

   this->mpc_ContextMenu->addAction(C_GtGetText::h_GetText("Select All"),
                                    static_cast<int32_t>(Qt::CTRL) + static_cast<int32_t>(Qt::Key_A),
                                    this,
                                    &C_OgeLabContextMenuBase::m_SelectAll);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Show custom context menu

   \param[in] orc_Pos Local context menu position
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabContextMenuBase::m_OnCustomContextMenuRequested(const QPoint & orc_Pos)
{
   // show context menu only if selectable
   if (this->textInteractionFlags().testFlag(Qt::TextSelectableByMouse) == true)
   {
      m_SetupContextMenu(); // setup the custom menu here to have real is-selectable information

      const QPoint c_PosGlobal = this->mapToGlobal(orc_Pos);
      this->mpc_ContextMenu->popup(c_PosGlobal);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy the selected text to clipboard.

   QLabel has no copy action.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabContextMenuBase::m_Copy() const
{
   QClipboard * const pc_Clipboard = QApplication::clipboard();

   pc_Clipboard->setText(this->selectedText());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Select the hole text of the label.

   QLabel has no selectAll action.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabContextMenuBase::m_SelectAll()
{
   this->setSelection(0, this->text().length());
}
