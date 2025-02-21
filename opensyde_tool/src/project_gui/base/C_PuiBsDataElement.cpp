//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for graphics items with any data elements (implementation)

   Base class for graphics items with any data elements

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_PuiBsDataElement.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in]  ors32_Index    Index of connected data item
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiBsDataElement::C_PuiBsDataElement(const int32_t & ors32_Index) :
   ms32_Index(ors32_Index)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiBsDataElement::~C_PuiBsDataElement(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get index of data item

   \return
   Index of data item in specific data array
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_PuiBsDataElement::GetIndex(void) const
{
   return this->ms32_Index;
}
