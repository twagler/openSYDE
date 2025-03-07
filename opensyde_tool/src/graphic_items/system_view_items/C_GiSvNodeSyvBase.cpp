//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Offers system view specific visualization and functionality of a node. (implementation)

   Offers system view specific visualization and functionality of a node.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_GtGetText.hpp"
#include "C_PuiSvHandler.hpp"
#include "C_GiSvNodeSyvBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
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

   \param[in]      ou32_ViewIndex   Index of system view
   \param[in]      ors32_NodeIndex  Index of data element in system view
   \param[in]      oru64_Id         Unique ID
   \param[in]      orf64_Width      Width of node
   \param[in]      orf64_Height     Height of node
   \param[in,out]  opc_Parent       Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_GiSvNodeSyvBase::C_GiSvNodeSyvBase(const uint32_t ou32_ViewIndex, const int32_t & ors32_NodeIndex,
                                     const uint64_t & oru64_Id, const float64_t & orf64_Width,
                                     const float64_t & orf64_Height, QGraphicsItem * const opc_Parent) :
   C_GiNode(ors32_NodeIndex, oru64_Id, orf64_Width, orf64_Height, opc_Parent),
   mu32_ViewIndex(ou32_ViewIndex),
   mq_ViewConnected(false)
{
   this->setFlag(QGraphicsItem::ItemIsMovable, false);
   this->setFlag(QGraphicsItem::ItemIsSelectable, false);

   this->m_SetDrawBorder(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the node connected state

   \param[in]  oq_Connected   Flag if connected or not
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvNodeSyvBase::SetViewConnected(const bool oq_Connected)
{
   this->mq_ViewConnected = oq_Connected;
   this->SetDrawWhiteFilter(!oq_Connected);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Function for initially loading internal data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvNodeSyvBase::LoadData(void)
{
   C_GiNode::LoadData();

   // get the view connected flag
   if (this->ms32_Index >= 0)
   {
      const C_PuiSvData * const pc_SvData = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);

      if (pc_SvData != NULL)
      {
         this->SetViewConnected(pc_SvData->GetNodeStatusDisplayedAsActive(static_cast<uint32_t>(this->ms32_Index)));
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get current error state and update error tooltip accordingly

   \return
   True  Error detected
   False No error detected
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_GiSvNodeSyvBase::m_UpdateError(void)
{
   const bool q_Retval = C_PuiSvHandler::h_GetInstance()->GetErrorNode(static_cast<uint32_t>(this->ms32_Index));

   if (q_Retval == true)
   {
      this->mc_ErrorText = C_GtGetText::h_GetText("For further details switch to SYSTEM DEFINITION");
   }
   else
   {
      this->mc_ErrorText = "";
   }
   return q_Retval;
}
