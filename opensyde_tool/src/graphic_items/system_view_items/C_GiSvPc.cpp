//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Offers visualization and functionality of a PC. (implementation)

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cmath>
#include <windows.h>

#include <QGraphicsView>

#include "gitypes.hpp"
#include "C_GiSvPc.hpp"
#include "C_GtGetText.hpp"
#include "C_PuiSvHandler.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_OgePopUpDialog.hpp"
#include "C_GiCustomFunctions.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_SyvSeDllConfigurationDialog.hpp"
#include "C_OscSystemBus.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const uint32_t C_GiSvPc::mhu32_SCALE_CATEGORY_0 = 3U; // no scaling -> default
const uint32_t C_GiSvPc::mhu32_SCALE_CATEGORY_1 = 0U;
const uint32_t C_GiSvPc::mhu32_SCALE_CATEGORY_2 = 1U;
const uint32_t C_GiSvPc::mhu32_SCALE_CATEGORY_3 = 2U;

const float64_t C_GiSvPc::mhaf64_SCALE_MIN_WIDTH_NODE[3] =
{
   250.0, 350.0, 450.0
};
const float64_t C_GiSvPc::mhaf64_SCALE_MIN_HEIGHT_NODE[3] =
{
   165.0, 230.0, 300.0
};

const float64_t C_GiSvPc::mhf64_INIT_SIZE_OF_PC = 150.0;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in] ou64_UniqueId  Unique item ID
   \param[in] ou32_ViewIndex View index
*/
//----------------------------------------------------------------------------------------------------------------------
C_GiSvPc::C_GiSvPc(const uint64_t ou64_UniqueId, const uint32_t ou32_ViewIndex) :
   C_GiImageGroupWithoutData(ou64_UniqueId, "", true),
   C_GiBiCustomToolTip(),
   C_PuiSvDbDataElement(ou32_ViewIndex, 0, 0, C_PuiSvDbDataElement::eUNKNOWN),
   mq_Connected(false)
{
   QRectF c_SizeImage;

   //Image
   if (C_GiSvPc::mh_GetIsLaptop() == true)
   {
      this->m_LateImageInit("://images/system_views/Laptop.svg");
   }
   else
   {
      this->m_LateImageInit("://images/system_views/Rechner.svg");
   }
   this->SetEditMode(false);

   //Allow hover events for tool tip hide
   this->setAcceptHoverEvents(true);

   //Conflict icon
   this->m_DetectIconSize();
   this->m_InitConflictIcon();
   //lint -e{1938}  static const is guaranteed preinitialized before main
   this->m_UpdateItems((std::max(mhf64_MIN_WIDTH_IMAGE, this->mpc_SvgGraphicsItem->f64_Width) - mhf64_MIN_WIDTH_IMAGE),
                       (std::max(mhf64_MIN_HEIGHT_IMAGE,
                                 this->mpc_SvgGraphicsItem->f64_Height) - mhf64_MIN_HEIGHT_IMAGE),
                       true);

   // Scale the image to a init size and keep the aspect ratio
   c_SizeImage = this->mpc_SvgGraphicsItem->GetSizeRect();
   //lint -e{1938}  static const is guaranteed preinitialized before main
   this->ApplySizeChange(QPointF(20.0, 20.0),
                         QSizeF(mhf64_INIT_SIZE_OF_PC,
                                c_SizeImage.height() / (c_SizeImage.width() / mhf64_INIT_SIZE_OF_PC)));
   //lint -e{1566}  no memory leak because of the parent of mpc_ConflictIcon and the Qt memory management
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{1540}  no memory leak because of the parent of mpc_ConflictIcon and the Qt memory management
C_GiSvPc::~C_GiSvPc(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Find closest point in shape to scene position

   \param[in]  orc_ScenePoint Scene position
   \param[out] orc_Closest    Closest point in shape
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::FindClosestPoint(const QPointF & orc_ScenePoint, QPointF & orc_Closest) const
{
   const QRectF c_Bounding = this->mpc_SvgGraphicsItem->sceneBoundingRect();
   bool q_HorizontalOk = false;
   bool q_VerticalOk = false;

   //X
   if (c_Bounding.topLeft().x() <= orc_ScenePoint.x())
   {
      if (c_Bounding.bottomRight().x() >= orc_ScenePoint.x())
      {
         //X OK
         orc_Closest.setX(orc_ScenePoint.x());
         q_HorizontalOk = true;
      }
      else
      {
         orc_Closest.setX(c_Bounding.bottomRight().x());
      }
   }
   else
   {
      orc_Closest.setX(c_Bounding.topLeft().x());
   }
   //Y
   if (c_Bounding.topLeft().y() <= orc_ScenePoint.y())
   {
      if (c_Bounding.bottomRight().y() >= orc_ScenePoint.y())
      {
         //Y OK
         orc_Closest.setY(orc_ScenePoint.y());
         q_VerticalOk = true;
      }
      else
      {
         orc_Closest.setY(c_Bounding.bottomRight().y());
      }
   }
   else
   {
      orc_Closest.setY(c_Bounding.topLeft().y());
   }
   //Align necessary
   if ((q_HorizontalOk == true) && (q_VerticalOk == true))
   {
      //Evaluate which border is the closest one
      const float64_t f64_HorizontalRightDist = std::abs(orc_ScenePoint.x() - c_Bounding.bottomRight().x());
      const float64_t f64_HorizontalLeftDist = std::abs(orc_ScenePoint.x() - c_Bounding.topLeft().x());
      const float64_t f64_VerticalBottomDist = std::abs(orc_ScenePoint.y() - c_Bounding.bottomRight().y());
      const float64_t f64_VerticalTopDist = std::abs(orc_ScenePoint.y() - c_Bounding.topLeft().y());
      float64_t f64_SmallestHorizontalDist;
      float64_t f64_SmallestVerticalDist;
      if (f64_HorizontalRightDist <= f64_HorizontalLeftDist)
      {
         f64_SmallestHorizontalDist = f64_HorizontalRightDist;
      }
      else
      {
         f64_SmallestHorizontalDist = f64_HorizontalLeftDist;
      }
      if (f64_VerticalTopDist <= f64_VerticalBottomDist)
      {
         f64_SmallestVerticalDist = f64_VerticalTopDist;
      }
      else
      {
         f64_SmallestVerticalDist = f64_VerticalBottomDist;
      }
      //Align to closest border
      if (f64_SmallestHorizontalDist <= f64_SmallestVerticalDist)
      {
         if (orc_ScenePoint.x() < (c_Bounding.topLeft().x() + (c_Bounding.width() / 2.0)))
         {
            orc_Closest.setX(c_Bounding.topLeft().x());
         }
         else
         {
            orc_Closest.setX(c_Bounding.bottomRight().x());
         }
      }
      else
      {
         if (orc_ScenePoint.y() < (c_Bounding.topLeft().y() + (c_Bounding.height() / 2.0)))
         {
            orc_Closest.setY(c_Bounding.topLeft().y());
         }
         else
         {
            orc_Closest.setY(c_Bounding.bottomRight().y());
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the type of this item

   \return  ID
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_GiSvPc::type(void) const
{
   return ms32_GRAPHICS_ITEM_PC;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open communication dialog

   Depends on CAN or Ethernet.
   Ethernet does not offer a communication dialog yet.

   \return
   true     Ok was clicked
   false    Cancel was clicked or the PC is connected to an Ethernet bus
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_GiSvPc::OpenDialog(void) const
{
   bool q_Retval = false;

   if (this->mq_Connected == false)
   {
      const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);
      bool q_OpenDialog = false;

      if ((pc_View != NULL) && (pc_View->GetOscPcData().GetConnected() == true))
      {
         const C_OscSystemBus * const pc_Bus =
            C_PuiSdHandler::h_GetInstance()->GetOscBus(pc_View->GetOscPcData().GetBusIndex());

         if (pc_Bus != NULL)
         {
            if (pc_Bus->e_Type == C_OscSystemBus::eCAN)
            {
               // Dialog only relevant if a CAN bus is connected
               q_OpenDialog  = true;
            }
         }
      }

      if ((q_OpenDialog == true) && (pc_View != NULL))
      {
         const uint32_t u32_BusIndex = pc_View->GetOscPcData().GetBusIndex();

         //Find connected interface of target node to found bus index
         const C_OscSystemBus * const pc_Bus = C_PuiSdHandler::h_GetInstance()->GetOscBus(u32_BusIndex);

         if (pc_Bus != NULL)
         {
            if (pc_Bus->e_Type == C_OscSystemBus::eCAN)
            {
               q_Retval = this->m_OpenCanDllDialog();
            }
            // Ethernet has not a own dialog yet -> Do nothing
         }
      }
      else
      {
         QGraphicsView * const pc_GraphicsView = this->scene()->views().at(0);
         C_OgeWiCustomMessage c_Message(pc_GraphicsView);
         c_Message.SetHeading(C_GtGetText::h_GetText("Ethernet settings"));
         c_Message.SetDescription(C_GtGetText::h_GetText("Setup your Ethernet adapter settings in Windows system "
                                                         "network configuration."));
         c_Message.SetCustomMinHeight(180, 180);
         c_Message.Execute();
      }
   }
   else
   {
      QGraphicsView * const pc_GraphicsView = this->scene()->views().at(0);
      C_OgeWiCustomMessage c_Message(pc_GraphicsView);
      c_Message.SetHeading(C_GtGetText::h_GetText("Configure PC Interface settings"));
      c_Message.SetDescription(C_GtGetText::h_GetText("Not available while being connected."));
      c_Message.SetCustomMinHeight(180, 180);
      c_Message.Execute();
   }

   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set connection change

   \param[in] oq_Connected Flag if connected
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::SetConnected(const bool oq_Connected)
{
   this->mq_Connected = oq_Connected;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Function for initially loading internal data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::LoadData(void)
{
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);

   if (pc_View != NULL)
   {
      C_PuiSvPc c_PcData = pc_View->GetPuiPcData();
      C_GiCustomFunctions::h_AdaptMouseRangePos(c_PcData.c_UiPosition);
      this->LoadBasicData(c_PcData);
      //Overwrite z order
      this->SetZetValueCustom(mf64_ZORDER_MAX - 1.0);
      // And adapt size
      this->ApplySizeChange(c_PcData.c_UiPosition, QSizeF(c_PcData.f64_Width, c_PcData.f64_Height));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Function for updating internal data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::UpdateData(void)
{
   C_PuiBsBox c_Data;

   this->UpdateBasicData(c_Data);
   C_PuiSvHandler::h_GetInstance()->SetViewPcBox(this->mu32_ViewIndex, c_Data);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete data in system views
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{9175} intentionally no functionality in this implementation
void C_GiSvPc::DeleteData(void)
{
   //Not allowed
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Change PC item to edit mode

   \param[in] oq_EditMode Edit mode flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::SetEditMode(const bool oq_EditMode)
{
   if (oq_EditMode == true)
   {
      this->setFlag(QGraphicsItem::ItemIsMovable, true);
      this->setFlag(QGraphicsItem::ItemIsSelectable, true);
      this->SetDefaultCursor(Qt::SizeAllCursor);
   }
   else
   {
      this->setFlag(QGraphicsItem::ItemIsMovable, false);
      this->setFlag(QGraphicsItem::ItemIsSelectable, false);
      this->SetDefaultCursor(Qt::ArrowCursor);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Generate hint to display as tool tip.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::GenerateHint()
{
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);

   if (pc_View != NULL)
   {
      QString c_ToolTipContent;
      const C_PuiSvPc c_PcData = pc_View->GetPuiPcData();

      // content
      c_ToolTipContent += C_GtGetText::h_GetText("CAN Interface: ");
      switch (c_PcData.GetCanDllType())
      {
      case C_PuiSvPc::ePEAK:
         c_ToolTipContent += "PEAK";
         break;
      case C_PuiSvPc::eVECTOR:
         c_ToolTipContent += "Vector";
         break;
      case C_PuiSvPc::eOTHER:
         c_ToolTipContent += static_cast<QString>(C_GtGetText::h_GetText("Other (%1)")).arg(
            c_PcData.GetCustomCanDllPath());
         break;
      default:
         break;
      }

      c_ToolTipContent += C_GtGetText::h_GetText("\nDouble click on PC to enter CAN interface settings.");
      this->SetDefaultToolTipContent(c_ToolTipContent);

      // heading
      this->SetDefaultToolTipHeading(C_GtGetText::h_GetText("openSYDE Client PC"));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resize update slot

   \param[in] of64_DiffWidth  Width difference
   \param[in] of64_DiffHeight Height difference
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::m_ResizeUpdateItems(const float64_t of64_DiffWidth, const float64_t of64_DiffHeight)
{
   this->m_UpdateItems(of64_DiffWidth, of64_DiffHeight, false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overridden mouse press event.

   Here: hide tool tip

   \param[in,out]    opc_Event Event   identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::mousePressEvent(QGraphicsSceneMouseEvent * const opc_Event)
{
   C_GiImageGroupWithoutData::mousePressEvent(opc_Event);

   // hide tooltip
   Q_EMIT (this->SigHideToolTip());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overridden hover leave event.

   Here: hide tool tip

   \param[in,out]    opc_Event Event   identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::hoverLeaveEvent(QGraphicsSceneHoverEvent * const opc_Event)
{
   C_GiImageGroupWithoutData::hoverLeaveEvent(opc_Event);

   // hide tooltip
   Q_EMIT (this->SigHideToolTip());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open CAN DLL dialog

   \return
   true     Ok was clicked
   false    Cancel was clicked
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_GiSvPc::m_OpenCanDllDialog(void) const
{
   bool q_Retval = false;
   const C_PuiSvData * const pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);

   if (pc_View != NULL)
   {
      const C_OscViewPc & rc_OscPcData = pc_View->GetOscPcData();
      const C_PuiSvPc & rc_PcData = pc_View->GetPuiPcData();
      QGraphicsView * const pc_GraphicsView = this->scene()->views().at(0);
      const QPointer<C_OgePopUpDialog> c_DllDialog = new C_OgePopUpDialog(pc_GraphicsView, pc_GraphicsView);
      C_SyvSeDllConfigurationDialog * const pc_DllWidget = new C_SyvSeDllConfigurationDialog(*c_DllDialog);

      // Resize
      const QSize c_SIZE(700, 490);
      c_DllDialog->SetSize(c_SIZE);

      // Initialize the data
      pc_DllWidget->SetDllType(rc_PcData.GetCanDllType());
      pc_DllWidget->SetCustomDllPath(rc_PcData.GetCustomCanDllPath());
      // Bitrate
      if (rc_OscPcData.GetConnected() == true)
      {
         const C_OscSystemBus * const pc_Bus = C_PuiSdHandler::h_GetInstance()->GetOscBus(rc_OscPcData.GetBusIndex());

         if (pc_Bus != NULL)
         {
            pc_DllWidget->SetBitrate(pc_Bus->u64_BitRate);
         }
      }

      if (c_DllDialog->exec() == static_cast<int32_t>(QDialog::Accepted))
      {
         // Update the data
         C_PuiSvHandler::h_GetInstance()->SetViewPcCanDll(this->mu32_ViewIndex,
                                                          pc_DllWidget->GetDllType(), pc_DllWidget->GetCustomDllPath());
         q_Retval = true;
      }

      if (c_DllDialog != NULL)
      {
         c_DllDialog->HideOverlay();
         c_DllDialog->deleteLater();
      }
   } //lint !e429  //no memory leak because of the parent of pc_DllWidget and the Qt memory management
   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current platform is laptop

   \return
   True  Laptop
   False PC
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_GiSvPc::mh_GetIsLaptop(void)
{
   SYSTEM_POWER_STATUS c_PowerStatus;
   const int32_t s32_Success = GetSystemPowerStatus(&c_PowerStatus);
   bool q_Return = false;

   if (s32_Success > 0)
   {
      if ((c_PowerStatus.BatteryFlag != 128) && // No system battery
          (c_PowerStatus.BatteryFlag != 255))   // Unknown status—unable to read the battery flag information
      {
         q_Return = true;
      }
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize conflict icon
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::m_InitConflictIcon(void)
{
   const float64_t f64_PosHorizontal = (this->mpc_SvgGraphicsItem->boundingRect().width() - 33.0) - // static offset for
                                                                                                    // correct
                                                                                                    // position
                                       (static_cast<float64_t>(this->ms32_IconSize) - 24.0);        // offset of scaled

   // icon

   // create the conflict icon
   this->mpc_ConflictIcon = new C_GiRectPixmap(QRectF(f64_PosHorizontal, 9.0,
                                                      static_cast<float64_t>(this->ms32_IconSize),
                                                      static_cast<float64_t>(this->ms32_IconSize)));
   this->mpc_ConflictIcon->SetSvg("://images/Error_iconV2.svg");

   // set the position of the icon
   this->mpc_ConflictIcon->setParentItem(this->mpc_SvgGraphicsItem);

   // the icon will be shown if a conflict is detected
   this->mpc_ConflictIcon->setVisible(false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect and update current icon size
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::m_DetectIconSize(void)
{
   const uint32_t u32_ScaleCategory = this->m_GetScaleCategory();

   switch (u32_ScaleCategory)
   {
   case mhu32_SCALE_CATEGORY_0:
      this->ms32_IconSize = 24;
      break;
   case mhu32_SCALE_CATEGORY_1:
      this->ms32_IconSize = 36;
      break;
   case mhu32_SCALE_CATEGORY_2:
      this->ms32_IconSize = 48;
      break;
   case mhu32_SCALE_CATEGORY_3:
      this->ms32_IconSize = 72;
      break;
   default:
      // fallback
      this->ms32_IconSize = 24;
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update items to resize

   \param[in] of64_DiffWidth  Width difference
   \param[in] of64_DiffHeight Height difference
   \param[in] oq_Initial      Initial flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_GiSvPc::m_UpdateItems(const float64_t of64_DiffWidth, const float64_t of64_DiffHeight, const bool oq_Initial)
{
   const int32_t s32_OldIconSize = this->ms32_IconSize;

   // update the scale category
   this->m_DetectIconSize();

   Q_UNUSED(of64_DiffHeight)
   // adapt conflict icon
   if (oq_Initial == false)
   {
      const int32_t s32_IconSizeDiff = this->ms32_IconSize - s32_OldIconSize;
      this->mpc_ConflictIcon->moveBy(of64_DiffWidth - static_cast<float64_t>(s32_IconSizeDiff), 0.0);
      this->mpc_ConflictIcon->SetNewSize(QSizeF(static_cast<float64_t>(this->ms32_IconSize),
                                                static_cast<float64_t>(this->ms32_IconSize)));
      this->mpc_ConflictIcon->update();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current scale category

   \return
   mhu32_ScaleCategory0 Default
   mhu32_ScaleCategory1 Category 1
   mhu32_ScaleCategory2 Category 2
   mhu32_ScaleCategory3 Category 3
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_GiSvPc::m_GetScaleCategory(void) const
{
   const QSizeF c_ActSize = this->mpc_SvgGraphicsItem->boundingRect().size();
   uint32_t u32_ScaleCategory = mhu32_SCALE_CATEGORY_0;

   // check first scaling
   if ((c_ActSize.width() >= mhaf64_SCALE_MIN_WIDTH_NODE[mhu32_SCALE_CATEGORY_1]) &&
       (c_ActSize.height() >= mhaf64_SCALE_MIN_HEIGHT_NODE[mhu32_SCALE_CATEGORY_1]))
   {
      // check second scaling
      if ((c_ActSize.width() >= mhaf64_SCALE_MIN_WIDTH_NODE[mhu32_SCALE_CATEGORY_2]) &&
          (c_ActSize.height() >= mhaf64_SCALE_MIN_HEIGHT_NODE[mhu32_SCALE_CATEGORY_2]))
      {
         // check third scaling
         if ((c_ActSize.width() >= mhaf64_SCALE_MIN_WIDTH_NODE[mhu32_SCALE_CATEGORY_3]) &&
             (c_ActSize.height() >= mhaf64_SCALE_MIN_HEIGHT_NODE[mhu32_SCALE_CATEGORY_3]))
         {
            u32_ScaleCategory = mhu32_SCALE_CATEGORY_3;
         }
         else
         {
            u32_ScaleCategory = mhu32_SCALE_CATEGORY_2;
         }
      }
      else
      {
         u32_ScaleCategory = mhu32_SCALE_CATEGORY_1;
      }
   }

   return u32_ScaleCategory;
}
