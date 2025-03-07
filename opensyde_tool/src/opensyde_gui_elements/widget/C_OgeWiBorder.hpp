//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Simple widget with a border (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEWIBORDER_HPP
#define C_OGEWIBORDER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QWidget>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeWiBorder :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_OgeWiBorder(QWidget * const opc_Parent = NULL);

protected:
   void paintEvent(QPaintEvent * const opc_Event) override;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
