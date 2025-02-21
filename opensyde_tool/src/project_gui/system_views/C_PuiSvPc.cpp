//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view PC item data element (implementation)

   System view PC item data element

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "constants.hpp"
#include "C_SclChecksums.hpp"
#include "C_PuiUtil.hpp"
#include "C_PuiSvPc.hpp"

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
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSvPc::C_PuiSvPc(void) :
   C_PuiBsBox(),
   me_CanDllType(ePEAK),
   mc_CustomCanDllPath("")
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(&this->me_CanDllType, sizeof(this->me_CanDllType), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->mc_CustomCanDllPath.toStdString().c_str(),
                                       this->mc_CustomCanDllPath.length(), oru32_HashValue);
   this->mc_ConnectionData.CalcHash(oru32_HashValue);

   C_PuiBsBox::CalcHash(oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get connection UI data

   \return
   Current connection UI data
*/
//----------------------------------------------------------------------------------------------------------------------
const C_PuiBsLineBase & C_PuiSvPc::GetConnectionData(void) const
{
   return this->mc_ConnectionData;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set connection UI data

   \param[in]  orc_Value   New connection UI data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetConnectionData(const C_PuiBsLineBase & orc_Value)
{
   this->mc_ConnectionData = orc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get CAN Dll type. (PEAK = 0,Vector = 1, Other = 2)

   \return   CAN Dll type
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSvPc::E_CanDllType C_PuiSvPc::GetCanDllType() const
{
   return this->me_CanDllType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the CAN DLL path

   \return
   CAN DLL path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiSvPc::GetCanDll(void) const
{
   QString c_Return;

   switch (this->me_CanDllType)
   {
   case ePEAK:
      c_Return = stw::opensyde_gui::mc_DLL_PATH_PEAK;
      break;
   case eVECTOR:
      c_Return = stw::opensyde_gui::mc_DLL_PATH_VECTOR;
      break;
   case eOTHER:
      c_Return = this->mc_CustomCanDllPath;
      break;
   default:
      c_Return = stw::opensyde_gui::mc_DLL_PATH_PEAK;
      break;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get custom CAN DLL Path

   \return Custom CAN DLL Path string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiSvPc::GetCustomCanDllPath() const
{
   return this->mc_CustomCanDllPath;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the absolute CAN DLL path or empty string if the path is empty

   \return
   Absolute CAN DLL path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiSvPc::GetCanDllAbsolute(void) const
{
   QString c_Return = this->GetCanDll();

   if (this->GetCanDll().isEmpty() == false)
   {
      // resolve variables and make absolute if it is relative
      c_Return = C_PuiUtil::h_GetResolvedAbsPathFromExe(this->GetCanDll());
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set DLL type. See get for type definition.

   \param[in]  oe_Type  CAN DLL type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetCanDllType(const C_PuiSvPc::E_CanDllType oe_Type)
{
   this->me_CanDllType = oe_Type;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set path for the CAN DLL

   \param[in]  orc_Path    Path for the CAN DLL
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetCustomCanDllPath(const QString & orc_Path)
{
   this->mc_CustomCanDllPath = orc_Path;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set UI box

   \param[in]  orc_Box  New UI box
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetBox(const C_PuiBsBox & orc_Box)
{
   this->c_UiPosition = orc_Box.c_UiPosition;
   this->f64_Height = orc_Box.f64_Height;
   this->f64_Width = orc_Box.f64_Width;
   this->f64_ZetOrder = orc_Box.f64_ZetOrder;
}
