//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Collection of utility function for message signals (implementation)

   Collection of utility function for message signals

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "TglUtils.hpp"
#include "C_OscCanUtil.hpp"
#include "C_SclChecksums.hpp"
#include "C_CamGenSigUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert message data array to vector

   \param[in]  orc_Message    message data

   \return
   Converted message data of DLC length
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> C_CamGenSigUtil::h_ConvertRawDataFormat(const C_CamProMessageData & orc_Message)
{
   std::vector<uint8_t> c_Retval;
   c_Retval.reserve(orc_Message.u16_Dlc);
   //Copy each byte
   for (uint16_t u16_It = 0; u16_It < orc_Message.u16_Dlc; ++u16_It)
   {
      c_Retval.push_back(orc_Message.c_Bytes[u16_It]);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decode raw value into C_OscNodeDataPoolContent

   Based on: openSYDE structure

   \param[in]  orc_Raw        Raw value
   \param[in]  orc_Signal     Signal description
   \param[in]  orc_SignalMin  Signal min value structure (only used for init)

   \return
   Value as C_OscNodeDataPoolContent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeDataPoolContent C_CamGenSigUtil::h_DecodeRawToContentSignal(const std::vector<uint8_t> & orc_Raw,
                                                                     const C_OscCanSignal & orc_Signal,
                                                                     const C_OscNodeDataPoolContent & orc_SignalMin)
{
   C_OscNodeDataPoolContent c_Retval = orc_SignalMin;
   //Convert to byte array
   uint8_t au8_Bytes[8];

   for (uint8_t u8_ItByte = 0U; u8_ItByte < 8U; ++u8_ItByte)
   {
      if (static_cast<uint32_t>(u8_ItByte) < orc_Raw.size())
      {
         au8_Bytes[u8_ItByte] = orc_Raw[static_cast<uint32_t>(u8_ItByte)];
      }
      else
      {
         au8_Bytes[u8_ItByte] = 0U;
      }
   }
   //Use core function
   C_OscCanUtil::h_GetSignalValue(au8_Bytes, orc_Signal, c_Retval);

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decode signal value to raw

   \param[in,out]  orc_Raw       Raw CAN message values
   \param[in]      orc_Signal    Signal description
   \param[in]      orc_Value     Signal value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenSigUtil::h_DecodeSignalValueToRaw(std::vector<uint8_t> & orc_Raw, const C_OscCanSignal & orc_Signal,
                                               const C_OscNodeDataPoolContent & orc_Value)
{
   //Convert to byte array
   uint8_t au8_Bytes[8];

   //Step 1: zero all SIGNAL bits (required by core function)
   for (uint16_t u16_ItBit = 0U; u16_ItBit < orc_Signal.u16_ComBitLength; ++u16_ItBit)
   {
      C_CamGenSigUtil::h_SetBit(orc_Signal.GetDataBytesBitPosOfSignalBit(u16_ItBit), orc_Raw, false);
   }

   //Step 2: convert all bytes to array
   for (uint8_t u8_ItByte = 0U; u8_ItByte < 8U; ++u8_ItByte)
   {
      if (static_cast<uint32_t>(u8_ItByte) < orc_Raw.size())
      {
         au8_Bytes[u8_ItByte] = orc_Raw[static_cast<uint32_t>(u8_ItByte)];
      }
      else
      {
         au8_Bytes[u8_ItByte] = 0U;
      }
   }
   //Step 3: use core function
   C_OscCanUtil::h_SetSignalValue(au8_Bytes, orc_Signal, orc_Value);
   //Step 4: write back to vector
   for (uint8_t u8_ItByte = 0U; (static_cast<uint32_t>(u8_ItByte) < orc_Raw.size()) && (u8_ItByte < 8U); ++u8_ItByte)
   {
      orc_Raw[static_cast<uint32_t>(u8_ItByte)] = au8_Bytes[u8_ItByte];
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert DBC to opensyde signal

   Warning: u32_ComDataElementIndex is invalid

   \param[in]  orc_Input   DBC input

   \return
   opensyde signal
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanSignal C_CamGenSigUtil::h_ConvertDbcToOsy(const C_CieConverter::C_CieCanSignal & orc_Input)
{
   C_OscCanSignal c_Retval;

   //Extract necessary openSYDE structure from DBC structure
   c_Retval.u16_ComBitLength = orc_Input.u16_ComBitLength;
   c_Retval.u16_ComBitStart = orc_Input.u16_ComBitStart;
   c_Retval.e_ComByteOrder = orc_Input.e_ComByteOrder;
   c_Retval.e_MultiplexerType = orc_Input.e_MultiplexerType;
   c_Retval.u16_MultiplexValue = orc_Input.u16_MultiplexValue;
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert DBC to opensyde signal

   Warning: e_Access & q_DiagEventCall & q_NvmValueChanged & q_IsValid & u32_NvmStartAddress are invalid

   \param[in]  orc_Input   DBC input

   \return
   opensyde signal
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeDataPoolListElement C_CamGenSigUtil::h_ConvertDbcToOsy(const C_CieConverter::C_CieDataPoolElement & orc_Input)
{
   C_OscNodeDataPoolListElement c_Retval;

   c_Retval.c_Name = orc_Input.c_Name;
   c_Retval.c_Comment = orc_Input.c_Comment;
   c_Retval.c_MinValue = orc_Input.c_MinValue;
   c_Retval.c_MaxValue = orc_Input.c_MaxValue;
   c_Retval.f64_Factor = orc_Input.f64_Factor;
   c_Retval.f64_Offset = orc_Input.f64_Offset;
   c_Retval.c_Unit = orc_Input.c_Unit;
   c_Retval.c_DataSetValues = orc_Input.c_DataSetValues;
   c_Retval.c_Value = orc_Input.c_MinValue;
   c_Retval.c_NvmValue = orc_Input.c_MinValue;
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert DBC to opensyde message

   Warning: u32_TimeoutMs & u16_DelayTimeMs & signals are all not filled in

   \param[in]  orc_Input   DBC input

   \return
   opensyde message
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanMessage C_CamGenSigUtil::h_ConvertDbcToOsy(const C_CieConverter::C_CieCanMessage & orc_Input)
{
   C_OscCanMessage c_Retval;

   c_Retval.c_Name = orc_Input.c_Name;
   c_Retval.c_Comment = orc_Input.c_Comment;
   c_Retval.u32_CanId = orc_Input.u32_CanId;
   c_Retval.q_IsExtended = orc_Input.q_IsExtended;
   c_Retval.u16_Dlc = orc_Input.u16_Dlc;
   c_Retval.e_TxMethod = orc_Input.e_TxMethod;
   c_Retval.u32_CycleTimeMs = orc_Input.u32_CycleTimeMs;
   //Both are not contained in DBC
   c_Retval.u32_TimeoutMs = 0UL;
   c_Retval.u16_DelayTimeMs = 0UL;
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get a single bit from the input byte vector

   \param[in]  ou32_BitPos    Bit position
   \param[in]  orc_Bytes      Bytes to use as data

   \return
   Bit value
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamGenSigUtil::h_GetBit(const uint32_t ou32_BitPos, const std::vector<uint8_t> & orc_Bytes)
{
   bool q_Retval = false;

   tgl_assert(ou32_BitPos < 64UL);
   if (ou32_BitPos < 64UL)
   {
      //Which byte (CAN)
      const uint16_t u16_InBytePos = static_cast<uint16_t>(ou32_BitPos / 8U);
      if (u16_InBytePos < orc_Bytes.size())
      {
         const uint8_t u8_Byte = orc_Bytes[u16_InBytePos];
         //Which bit (CAN)
         const uint8_t u8_Mask = 0x1U << (ou32_BitPos % 8U);
         q_Retval = (u8_Byte & u8_Mask) > 0U;
      }
   }
   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set a single bit in the output byte vector

   \param[in]      ou32_BitPos   Bit position
   \param[in,out]  orc_Bytes     Bytes to change
   \param[in]      oq_Value      Bit value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamGenSigUtil::h_SetBit(const uint32_t ou32_BitPos, std::vector<uint8_t> & orc_Bytes, const bool oq_Value)
{
   tgl_assert(ou32_BitPos < 64UL);
   if (ou32_BitPos < 64UL)
   {
      //Which byte (output)
      const uint16_t u16_OutBytePos = static_cast<uint16_t>(ou32_BitPos / 8U);
      if (u16_OutBytePos < orc_Bytes.size())
      {
         //Which bit (output)
         const uint8_t u8_Mask = 0x1U << (ou32_BitPos % 8U);
         if (oq_Value == true)
         {
            //Apply 1
            orc_Bytes[u16_OutBytePos] = orc_Bytes[u16_OutBytePos] | u8_Mask;
         }
         else
         {
            //Apply 0
            const uint8_t u8_InvertedMask = ~u8_Mask;
            orc_Bytes[u16_OutBytePos] = orc_Bytes[u16_OutBytePos] & u8_InvertedMask;
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculate message hash

   \param[in]  orc_Message       Message
   \param[in]  orc_DatapoolPart  Datapool part

   \return
   Message hash
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_CamGenSigUtil::h_CalcMessageHash(const C_OscCanMessage & orc_Message,
                                            const std::vector<C_OscNodeDataPoolListElement> & orc_DatapoolPart)
{
   uint32_t u32_Retval = 0UL;
   const uint32_t u32_Size = orc_DatapoolPart.size();

   orc_Message.CalcHash(u32_Retval, true);
   stw::scl::C_SclChecksums::CalcCRC32(&u32_Size, sizeof(u32_Size), u32_Retval);
   for (uint32_t u32_ItDpPart = 0UL; u32_ItDpPart < orc_DatapoolPart.size(); ++u32_ItDpPart)
   {
      orc_DatapoolPart[u32_ItDpPart].CalcHash(u32_Retval);
   }
   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calc message hash

   \param[in]  orc_Message    Message
   \param[in]  orc_List       List

   \return
   Message hash
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_CamGenSigUtil::h_CalcMessageHash(const C_OscCanMessage & orc_Message, const C_OscNodeDataPoolList & orc_List)
{
   uint32_t u32_Retval;

   std::vector<C_OscNodeDataPoolListElement> c_DatapoolPart;

   //Fill
   c_DatapoolPart.reserve(orc_Message.c_Signals.size());
   for (uint32_t u32_ItSignal = 0UL; u32_ItSignal < orc_Message.c_Signals.size(); ++u32_ItSignal)
   {
      const C_OscCanSignal & rc_Signal = orc_Message.c_Signals[u32_ItSignal];
      if (rc_Signal.u32_ComDataElementIndex < orc_List.c_Elements.size())
      {
         const C_OscNodeDataPoolListElement & rc_Element =
            orc_List.c_Elements[rc_Signal.u32_ComDataElementIndex];
         c_DatapoolPart.push_back(rc_Element);
      }
   }

   //Hash
   u32_Retval = C_CamGenSigUtil::h_CalcMessageHash(orc_Message, c_DatapoolPart);

   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calc message hash

   \param[in]  orc_Message    Message

   \return
   Message hash
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_CamGenSigUtil::h_CalcMessageHash(const C_CieConverter::C_CieCanMessage & orc_Message)
{
   uint32_t u32_Retval;
   C_OscCanMessage c_Message = C_CamGenSigUtil::h_ConvertDbcToOsy(orc_Message);

   std::vector<C_OscNodeDataPoolListElement> c_DatapoolPart;

   //Fill
   c_Message.c_Signals.clear();
   c_Message.c_Signals.reserve(orc_Message.c_Signals.size());
   c_DatapoolPart.reserve(orc_Message.c_Signals.size());
   for (uint32_t u32_ItSig = 0UL; u32_ItSig < orc_Message.c_Signals.size(); ++u32_ItSig)
   {
      const C_CieConverter::C_CieCanSignal & rc_Signal = orc_Message.c_Signals[u32_ItSig];
      c_Message.c_Signals.push_back(C_CamGenSigUtil::h_ConvertDbcToOsy(rc_Signal));
      c_DatapoolPart.push_back(C_CamGenSigUtil::h_ConvertDbcToOsy(rc_Signal.c_Element));
   }

   //Hash
   u32_Retval = C_CamGenSigUtil::h_CalcMessageHash(c_Message, c_DatapoolPart);

   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamGenSigUtil::C_CamGenSigUtil(void)
{
}
