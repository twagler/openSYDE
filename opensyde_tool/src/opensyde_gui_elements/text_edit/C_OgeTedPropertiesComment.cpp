//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Text edit field for property comment (implementation)

   Text edit field for property comment.
   This class does not contain any functionality,
   but needs to exist, to have a unique group,
   to apply a specific stylesheet for.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeTedPropertiesComment.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;

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

C_OgeTedPropertiesComment::C_OgeTedPropertiesComment(QWidget * const opc_Parent) :
   C_OgeTedToolTipBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten focus out event slot

   Here: clear text selection and emit signal

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTedPropertiesComment::focusOutEvent(QFocusEvent * const opc_Event)
{
   QTextCursor c_TempTextCursor;

   if (opc_Event->reason() != Qt::PopupFocusReason)
   {
      c_TempTextCursor = this->textCursor();

      c_TempTextCursor.clearSelection();
      this->setTextCursor(c_TempTextCursor);
      Q_EMIT (this->SigEditingFinished());
   }

   QTextEdit::focusOutEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Overwritten key press event to emulate key 'Tab', 'Return' and 'Enter' behaviour as user confirmation.

   \param[in]       opc_KeyEvent     Qt key event to catch
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTedPropertiesComment::keyPressEvent(QKeyEvent * const opc_KeyEvent)
{
   if (opc_KeyEvent != NULL)
   {
      C_OgeTedContextMenuBase::keyPressEvent(opc_KeyEvent);
      if ((opc_KeyEvent->key() == Qt::Key_Tab) ||
          (opc_KeyEvent->key() == Qt::Key_Return) ||
          (opc_KeyEvent->key() == Qt::Key_Enter))
      {
         Q_EMIT (this->SigCommentConfirmed());
      }
   }
}
