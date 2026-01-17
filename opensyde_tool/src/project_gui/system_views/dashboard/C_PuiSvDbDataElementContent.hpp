//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Holds datapool element content and its timestamp (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUISVDBDATAELEMENTCONTENT_HPP
#define C_PUISVDBDATAELEMENTCONTENT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <QtGlobal>

#include "C_OscNodeDataPoolContent.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiSvDbDataElementContent :
   public stw::opensyde_core::C_OscNodeDataPoolContent
{
public:
   C_PuiSvDbDataElementContent(void);
   C_PuiSvDbDataElementContent(const C_PuiSvDbDataElementContent & orc_Source);
   C_PuiSvDbDataElementContent(const stw::opensyde_core::C_OscNodeDataPoolContent & orc_Source);
   C_PuiSvDbDataElementContent & operator = (const C_PuiSvDbDataElementContent & orc_Source) &; //lint !e1511 //we want
                                                                                                // to
                                                                                                // hide the base func.

   void SetTimeStamp(const qint64 os64_Timestamp);
   qint64 GetTimeStamp(void) const;

private:
   qint64 ms64_TimeStamp;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
