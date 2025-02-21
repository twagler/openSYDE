//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Resize rectangle undo command (implementation)

   Resize rectangle undo command

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SebUnoResizeRectangleCommand.hpp"
#include "C_GiBiRectBaseGroup.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace std;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor

   \param[in,out] opc_Scene   Pointer to currently active scene
   \param[in]     orc_Ids     Affected unique IDs
   \param[in]     orc_OldPos  Old position
   \param[in]     orc_OldSize Old size
   \param[in]     orc_NewPos  New position
   \param[in]     orc_NewSize New size
   \param[in]     opc_Parent  Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SebUnoResizeRectangleCommand::C_SebUnoResizeRectangleCommand(QGraphicsScene * const opc_Scene,
                                                               const std::vector<uint64_t> & orc_Ids,
                                                               const QPointF & orc_OldPos, const QSizeF & orc_OldSize,
                                                               const QPointF & orc_NewPos, const QSizeF & orc_NewSize,
                                                               QUndoCommand * const opc_Parent) :
   C_SebUnoBaseCommand(opc_Scene, orc_Ids, "Resize one rectangle based drawing element", opc_Parent),
   mc_OldPos(orc_OldPos),
   mc_OldSize(orc_OldSize),
   mc_NewPos(orc_NewPos),
   mc_NewSize(orc_NewSize)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SebUnoResizeRectangleCommand::~C_SebUnoResizeRectangleCommand()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Undo move
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SebUnoResizeRectangleCommand::undo(void)
{
   vector<QGraphicsItem *> c_Items = this->m_GetSceneItems();
   for (uint32_t u32_ItId = 0; u32_ItId < c_Items.size(); ++u32_ItId)
   {
      m_UndoSingle(c_Items[u32_ItId]);
   }
   QUndoCommand::undo();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Redo move
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SebUnoResizeRectangleCommand::redo(void)
{
   vector<QGraphicsItem *> c_Items = this->m_GetSceneItems();
   for (uint32_t u32_ItId = 0; u32_ItId < c_Items.size(); ++u32_ItId)
   {
      m_RedoSingle(c_Items[u32_ItId]);
   }
   QUndoCommand::redo();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Undo for one item

   \param[in,out] opc_Item Item to perform action on
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SebUnoResizeRectangleCommand::m_UndoSingle(QGraphicsItem * const opc_Item) const
{
   if (opc_Item != NULL)
   {
      C_GiBiRectBaseGroup * pc_RectBase;

      pc_RectBase = dynamic_cast<C_GiBiRectBaseGroup *>(opc_Item);
      if (pc_RectBase != NULL)
      {
         pc_RectBase->ApplySizeChange(this->mc_OldPos, this->mc_OldSize);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Redo for one item

   \param[in,out] opc_Item Item to perform action on
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SebUnoResizeRectangleCommand::m_RedoSingle(QGraphicsItem * const opc_Item) const
{
   if (opc_Item != NULL)
   {
      C_GiBiRectBaseGroup * pc_RectBase;

      pc_RectBase = dynamic_cast<C_GiBiRectBaseGroup *>(opc_Item);
      if (pc_RectBase != NULL)
      {
         pc_RectBase->ApplySizeChange(this->mc_NewPos, this->mc_NewSize);
      }
   }
}
