//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for showing already shared datapools (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEDPSELECTORADDLISTWIDGET_HPP
#define C_SDNDEDPSELECTORADDLISTWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QListWidget>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeDpSelectorAddListWidget :
   public QListWidget
{
   Q_OBJECT

public:
   explicit C_SdNdeDpSelectorAddListWidget(QWidget * const opc_Parent = NULL);
   ~C_SdNdeDpSelectorAddListWidget(void) override;

private:
   //Avoid call
   C_SdNdeDpSelectorAddListWidget(const C_SdNdeDpSelectorAddListWidget &);
   C_SdNdeDpSelectorAddListWidget & operator =(const C_SdNdeDpSelectorAddListWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
