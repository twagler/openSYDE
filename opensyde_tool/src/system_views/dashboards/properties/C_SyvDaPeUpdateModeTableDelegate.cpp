//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view dashboard data element update mode table delegate (implementation)

   System view dashboard data element update mode table delegate

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "constants.hpp"
#include "C_GtGetText.hpp"
#include "C_OgeCbxTable.hpp"
#include "C_PuiSvHandler.hpp"
#include "C_SdNdeDpUtil.hpp"
#include "C_OgeSpxFactorTable.hpp"
#include "C_TblTreDelegateUtil.hpp"
#include "C_OgeSpxInt64FactorTable.hpp"
#include "C_SyvDaPeUpdateModeTableModel.hpp"
#include "C_SyvDaPeUpdateModeTableDelegate.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   \param[in]  ou32_ViewIndex   View index
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaPeUpdateModeTableDelegate::C_SyvDaPeUpdateModeTableDelegate(const uint32_t ou32_ViewIndex) :
   QStyledItemDelegate(),
   mu32_ViewIndex(ou32_ViewIndex)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten create editor event slot

   Here: Create appropriate editor widget

   \param[in,out] opc_Parent Parent widget
   \param[in]     orc_Option Style options
   \param[in]     orc_Index  Correlating index

   \return
   Editor widget
*/
//----------------------------------------------------------------------------------------------------------------------
QWidget * C_SyvDaPeUpdateModeTableDelegate::createEditor(QWidget * const opc_Parent,
                                                         const QStyleOptionViewItem & orc_Option,
                                                         const QModelIndex & orc_Index) const
{
   QWidget * pc_Retval = NULL;

   Q_UNUSED(orc_Option)
   if (orc_Index.isValid() == true)
   {
      C_OgeCbxTable * pc_ComboBox;
      const C_PuiSvData * pc_View;
      const C_SyvDaPeUpdateModeTableModel::E_Columns e_Col = C_SyvDaPeUpdateModeTableModel::h_ColumnToEnum(
         orc_Index.column());
      switch (e_Col) //lint !e788 //not all columns need an editor
      {
      case C_SyvDaPeUpdateModeTableModel::eTRANSMISSION_MODE:
         pc_ComboBox = new C_OgeCbxTable(opc_Parent);
         //Init
         pc_ComboBox->addItem(C_GtGetText::h_GetText("Cyclic"));
         pc_ComboBox->addItem(C_GtGetText::h_GetText("On Change"));
         pc_ComboBox->addItem(C_GtGetText::h_GetText("On Trigger"));
         pc_ComboBox->ResizeViewToContents();
         pc_Retval = pc_ComboBox;
         break;
      case C_SyvDaPeUpdateModeTableModel::eCYCLIC_INTERVAL:
         pc_View = C_PuiSvHandler::h_GetInstance()->GetView(this->mu32_ViewIndex);
         if (pc_View != NULL)
         {
            pc_ComboBox = new C_OgeCbxTable(opc_Parent);
            //Init
            pc_ComboBox->addItem(static_cast<QString>(C_GtGetText::h_GetText("Fast (%1 ms)")).arg(pc_View->
                                                                                                  GetUpdateRateFast()));
            pc_ComboBox->addItem(static_cast<QString>(C_GtGetText::h_GetText("Medium (%1 ms)")).arg(pc_View->
                                                                                                    GetUpdateRateMedium()));
            pc_ComboBox->addItem(static_cast<QString>(C_GtGetText::h_GetText("Slow (%1 ms)")).arg(pc_View->
                                                                                                  GetUpdateRateSlow()));
            pc_ComboBox->ResizeViewToContents();
            pc_Retval = pc_ComboBox;
         }
         break;
      case C_SyvDaPeUpdateModeTableModel::eTHRESHOLD:
         {
            const QVariant c_Data = orc_Index.data(static_cast<int32_t>(Qt::EditRole));
            const QVariant c_Max = orc_Index.data(ms32_USER_ROLE_INTERACTION_MAXIMUM_VALUE);
            if (c_Data.typeId() == QMetaType::Double)
            {
               C_OgeSpxFactorTable * const pc_SpinBox = new C_OgeSpxFactorTable(opc_Parent);
               if (c_Max.typeId() == QMetaType::Double)
               {
                  //Factor needs to be above 0
                  pc_SpinBox->SetMinimumCustom(C_OgeSpxFactor::mhf64_FACTOR_MIN);
                  pc_SpinBox->SetMaximumCustom(c_Max.toDouble());
               }
               pc_Retval = pc_SpinBox;
            }
            else if ((c_Data.typeId() == QMetaType::Int) || (c_Data.typeId() == QMetaType::LongLong))
            {
               C_OgeSpxInt64FactorTable * const pc_SpinBox = new C_OgeSpxInt64FactorTable(opc_Parent, false);
               if (c_Max.typeId() == QMetaType::LongLong)
               {
                  const QVariant c_Min(1LL);
                  pc_SpinBox->SetMinimum(c_Min);
                  pc_SpinBox->SetMaximum(c_Max);
               }
               pc_Retval = pc_SpinBox;
            }
            else if ((c_Data.typeId() == QMetaType::UInt) || (c_Data.typeId() == QMetaType::ULongLong))
            {
               C_OgeSpxInt64FactorTable * const pc_SpinBox = new C_OgeSpxInt64FactorTable(opc_Parent, true);
               if (c_Max.typeId() == QMetaType::ULongLong)
               {
                  const QVariant c_Min(1ULL);
                  pc_SpinBox->SetMinimum(c_Min);
                  pc_SpinBox->SetMaximum(c_Max);
               }
               pc_Retval = pc_SpinBox;
            }
            else
            {
               //Unknown
            }
            break;
         }
      default:
         break;
      }
   }
   return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten set editor data event slot

   Here: Pass relevant data

   \param[in,out] opc_Editor Editor widget
   \param[in]     orc_Index  Correlating index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaPeUpdateModeTableDelegate::setEditorData(QWidget * const opc_Editor, const QModelIndex & orc_Index) const
{
   if ((opc_Editor != NULL) && (orc_Index.isValid() == true))
   {
      QDoubleSpinBox * const pc_DoubleSpinBox = dynamic_cast<QDoubleSpinBox * const>(opc_Editor);

      C_OgeSpxInt64 * const pc_Int64SpinBox = dynamic_cast<C_OgeSpxInt64 * const>(opc_Editor);

      QComboBox * const pc_ComboBox = dynamic_cast<QComboBox * const>(opc_Editor);
      const C_SyvDaPeUpdateModeTableModel::E_Columns e_Col = C_SyvDaPeUpdateModeTableModel::h_ColumnToEnum(
         orc_Index.column());
      switch (e_Col) //lint !e788 //not all columns handled on purpose
      {
      case C_SyvDaPeUpdateModeTableModel::eTRANSMISSION_MODE:
      case C_SyvDaPeUpdateModeTableModel::eCYCLIC_INTERVAL:
         if (pc_ComboBox != NULL)
         {
            pc_ComboBox->setCurrentIndex(orc_Index.data(static_cast<int32_t>(Qt::EditRole)).toInt());
         }
         break;
      case C_SyvDaPeUpdateModeTableModel::eTHRESHOLD:
         if (pc_DoubleSpinBox != NULL)
         {
            const QVariant c_Data = orc_Index.data(static_cast<int32_t>(Qt::EditRole));
            switch (c_Data.typeId()) //lint !e788 //not all types required
            {
            case QMetaType::Double:
               pc_DoubleSpinBox->setValue(c_Data.toDouble());
               break;
            case QMetaType::Int:
               pc_DoubleSpinBox->setValue(static_cast<float64_t>(c_Data.toInt()));
               break;
            case QMetaType::UInt:
               pc_DoubleSpinBox->setValue(static_cast<float64_t>(c_Data.toUInt()));
               break;
            case QMetaType::LongLong:
               pc_DoubleSpinBox->setValue(static_cast<float64_t>(c_Data.toLongLong()));
               break;
            case QMetaType::ULongLong:
               pc_DoubleSpinBox->setValue(static_cast<float64_t>(c_Data.toULongLong()));
               break;
            default:
               //No handling possible
               break;
            }
         }
         if (pc_Int64SpinBox != NULL)
         {
            pc_Int64SpinBox->SetValue(orc_Index.data(static_cast<int32_t>(Qt::EditRole)), false);
         }
         break;
      default:
         break;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten set model data event slot

   Here: Pass relevant data

   \param[in,out] opc_Editor Editor widget
   \param[in,out] opc_Model  Model object
   \param[in]     orc_Index  Correlating index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaPeUpdateModeTableDelegate::setModelData(QWidget * const opc_Editor, QAbstractItemModel * const opc_Model,
                                                    const QModelIndex & orc_Index) const
{
   if (((opc_Editor != NULL) && (opc_Model != NULL)) && (orc_Index.isValid() == true))
   {
      QDoubleSpinBox * const pc_DoubleSpinBox = dynamic_cast<QDoubleSpinBox * const>(opc_Editor);

      C_OgeSpxInt64Factor * const pc_Int64SpinBox = dynamic_cast<C_OgeSpxInt64Factor * const>(opc_Editor);

      QComboBox * const pc_ComboBox = dynamic_cast<QComboBox * const>(opc_Editor);
      const C_SyvDaPeUpdateModeTableModel::E_Columns e_Col = C_SyvDaPeUpdateModeTableModel::h_ColumnToEnum(
         orc_Index.column());
      switch (e_Col) //lint !e788 //not all columns handled on purpose
      {
      case C_SyvDaPeUpdateModeTableModel::eTRANSMISSION_MODE:
      case C_SyvDaPeUpdateModeTableModel::eCYCLIC_INTERVAL:
         if (pc_ComboBox != NULL)
         {
            const int32_t s32_NewValue = pc_ComboBox->currentIndex();
            opc_Model->setData(orc_Index, s32_NewValue);
         }
         break;
      case C_SyvDaPeUpdateModeTableModel::eTHRESHOLD:
         if (pc_DoubleSpinBox != NULL)
         {
            opc_Model->setData(orc_Index, pc_DoubleSpinBox->value());
         }
         if (pc_Int64SpinBox != NULL)
         {
            //Trigger manual interpretation to avoid accepting intermediate values
            pc_Int64SpinBox->InterpretValue();
            opc_Model->setData(orc_Index, pc_Int64SpinBox->GetValue());
         }
         break;
      default:
         break;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint item

   Here: handle icons

   \param[in,out] opc_Painter Painter
   \param[in]     orc_Option  Option
   \param[in]     orc_Index   Index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaPeUpdateModeTableDelegate::paint(QPainter * const opc_Painter, const QStyleOptionViewItem & orc_Option,
                                             const QModelIndex & orc_Index) const
{
   QStyledItemDelegate::paint(opc_Painter, orc_Option, orc_Index);
   C_TblTreDelegateUtil::h_PaintIcon(opc_Painter, orc_Option, orc_Index);
}
