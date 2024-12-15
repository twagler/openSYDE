//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Generic line edit for table edit (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_TBLEDITLINEEDITBASE_HPP
#define C_TBLEDITLINEEDITBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QVariant>
#include "C_OgeLeToolTipBase.hpp"

#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_TblEditLineEditBase :
   public stw::opensyde_gui_elements::C_OgeLeToolTipBase
{
public:
   C_TblEditLineEditBase(QWidget * const opc_Parent = NULL);

   void SetFromVariant(const QVariant & orc_DisplayValue, const QVariant & orc_EditValue);
   void SetMinFromVariant(const QVariant & orc_Value);
   void SetMaxFromVariant(const QVariant & orc_Value);
   int32_t GetValueAsVariant(QVariant & orc_Value, QString & orc_ErrorDescription) const;

private:
   QMetaType::Type me_Type;
   QString mc_MinValue;
   QString mc_MaxValue;

   void m_UpdateToolTip(void);
   static float64_t mh_GetStringAsFloat(const QString & orc_Value, bool & orq_Worked);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
