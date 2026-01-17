//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief      Wrapper class for QtXml (QDomDocument)

   cf. .h file header for details

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include <QFile>
#include <QTextStream>
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscXmlParser.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParserBase::C_OscXmlParserBase(void)
{
   m_Init();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init of common xml structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::m_Init(void)
{
   //empty file ?
   if (mc_Document.documentElement().isNull())
   {
      //create header:
      mc_Document.appendChild(mc_Document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParserBase::~C_OscXmlParserBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParser::C_OscXmlParser(void) :
   C_OscXmlParserBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParser::~C_OscXmlParser(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Open XML data from file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParser::LoadFromFile(const C_SclString & orc_FileName)
{
   int32_t s32_Return = C_NO_ERR;
   mc_Document.clear();

   QFile c_File(QString::fromStdString(*orc_FileName.AsStdString()));
   if (!c_File.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      s32_Return = C_NOACT;
      m_Init();
   }
   else
   {
      QString c_ErrorMsg;
      int s32_ErrorLine;
      int s32_ErrorCol;
      //setContent handles the rest
      if (!mc_Document.setContent(&c_File, &c_ErrorMsg, &s32_ErrorLine, &s32_ErrorCol))
      {
         s32_Return = C_NOACT;
         m_Init();
      }
      c_File.close();
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write XML data to file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParser::SaveToFile(const C_SclString & orc_FileName)
{
   int32_t s32_Return = C_NO_ERR;
   QFile c_File(QString::fromStdString(*orc_FileName.AsStdString()));
   if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      s32_Return = C_NOACT;
   }
   else
   {
      QTextStream c_Stream(&c_File);
      mc_Document.save(c_Stream, 3); //indent 3 spaces
      c_File.close();
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select root node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::SelectRoot(void)
{
   C_SclString c_RootName;

   mc_CurrentElement = mc_Document.documentElement();
   if (!mc_CurrentElement.isNull())
   {
      c_RootName = mc_CurrentElement.tagName().toStdString();
   }
   return c_RootName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select root node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::SelectRootError(const C_SclString & orc_Name)
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->SelectRoot() != orc_Name)
   {
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select next node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::SelectNodeNext(const C_SclString & orc_Name)
{
   C_SclString c_Name;
   QDomElement c_Save = mc_CurrentElement;

   if (!mc_CurrentElement.isNull())
   {
      if (orc_Name != "")
      {
         mc_CurrentElement = mc_CurrentElement.nextSiblingElement(QString::fromStdString(*orc_Name.AsStdString()));
      }
      else
      {
         mc_CurrentElement = mc_CurrentElement.nextSiblingElement();
      }
   }

   if (!mc_CurrentElement.isNull())
   {
      c_Name = mc_CurrentElement.tagName().toStdString();
   }
   else
   {
      mc_CurrentElement = c_Save;
   }
   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select child node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::SelectNodeChild(const C_SclString & orc_Name)
{
   C_SclString c_Name;
   QDomElement c_Element;

   if (!mc_CurrentElement.isNull())
   {
      c_Element = mc_CurrentElement;
   }
   else
   {
      c_Element = mc_Document.documentElement();
   }

   if (!c_Element.isNull())
   {
      if (orc_Name == "")
      {
         c_Element = c_Element.firstChildElement();
      }
      else
      {
         c_Element = c_Element.firstChildElement(QString::fromStdString(*orc_Name.AsStdString()));
      }
   }

   if (!c_Element.isNull())
   {
      mc_CurrentElement = c_Element;
      c_Name = mc_CurrentElement.tagName().toStdString();
   }
   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select child node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::SelectNodeChildError(const C_SclString & orc_Name)
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->SelectNodeChild(orc_Name) != orc_Name)
   {
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select parent of active node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::SelectNodeParent(void)
{
   C_SclString c_Name;

   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement = mc_CurrentElement.parentNode().toElement();
   }
   if (!mc_CurrentElement.isNull())
   {
      c_Name = mc_CurrentElement.tagName().toStdString();
   }
   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get content of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::GetNodeContent(void) const
{
   C_SclString c_Content;

   if (!mc_CurrentElement.isNull())
   {
      c_Content = mc_CurrentElement.text().toStdString();
   }

   return c_Content;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check whether specified attribute exists
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscXmlParserBase::AttributeExists(const C_SclString & orc_Name) const
{
   bool q_Return = false;

   if (!mc_CurrentElement.isNull())
   {
      q_Return = mc_CurrentElement.hasAttribute(QString::fromStdString(*orc_Name.AsStdString()));
   }
   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get current node name
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::GetCurrentNodeName(void) const
{
   return (mc_CurrentElement.isNull()) ? "" : mc_CurrentElement.tagName().toStdString();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get file line for current node
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscXmlParserBase::GetFileLineForCurrentNode(void) const
{
   // QDomDocument does not store line numbers for elements.
   return 0U;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::GetAttributeString(const C_SclString & orc_Name, const C_SclString & orc_Default) const
{
   C_SclString c_Value = orc_Default;

   if (!mc_CurrentElement.isNull())
   {
      QString c_AttrName = QString::fromStdString(*orc_Name.AsStdString());
      if (mc_CurrentElement.hasAttribute(c_AttrName))
      {
         c_Value = mc_CurrentElement.attribute(c_AttrName).toStdString();
      }
   }
   return c_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeSint32(const C_SclString & orc_Name, const int32_t os32_Default) const
{
   int32_t s32_Value = os32_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      try
      {
         s32_Value = c_Text.ToInt();
      }
      catch (...)
      {
      }
   }
   return s32_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscXmlParserBase::GetAttributeUint32(const C_SclString & orc_Name, const uint32_t ou32_Default) const
{
   uint32_t u32_Value = ou32_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      try
      {
         u32_Value = static_cast<uint32_t>(c_Text.ToInt());
      }
      catch (...)
      {
      }
   }
   return u32_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int64_t C_OscXmlParserBase::GetAttributeSint64(const C_SclString & orc_Name, const int64_t os64_Default) const
{
   int64_t s64_Value = os64_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      try
      {
         s64_Value = c_Text.ToInt64();
      }
      catch (...)
      {
      }
   }
   return s64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
uint64_t C_OscXmlParserBase::GetAttributeUint64(const C_SclString & orc_Name, const uint64_t ou64_Default) const
{
   uint64_t u64_Value = ou64_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      if (QString::fromStdString(*c_Text.AsStdString()).startsWith("0x"))
      {
         u64_Value = QString::fromStdString(*c_Text.AsStdString()).toULongLong(&q_Ok, 16);
      }
      else
      {
         u64_Value = QString::fromStdString(*c_Text.AsStdString()).toULongLong(&q_Ok, 10);
      }
      if (!q_Ok)
      {
         u64_Value = ou64_Default;
      }
   }
   return u64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscXmlParserBase::GetAttributeBool(const C_SclString & orc_Name, const bool oq_Default) const
{
   bool q_Value = oq_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      QString c_Qs = QString::fromStdString(*c_Text.AsStdString()).toLower();
      if (c_Qs == "true" || c_Qs == "1")
      {
         q_Value = true;
      }
      else if (c_Qs == "false" || c_Qs == "0")
      {
         q_Value = false;
      }
   }
   return q_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
float32_t C_OscXmlParserBase::GetAttributeFloat32(const C_SclString & orc_Name, const float32_t of32_Default) const
{
   float32_t f32_Value = of32_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      f32_Value = QString::fromStdString(*c_Text.AsStdString()).toFloat(&q_Ok);
      if (!q_Ok)
      {
         f32_Value = of32_Default;
      }
   }
   return f32_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
float64_t C_OscXmlParserBase::GetAttributeFloat64(const C_SclString & orc_Name, const float64_t of64_Default) const
{
   float64_t f64_Value = of64_Default;
   C_SclString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      f64_Value = QString::fromStdString(*c_Text.AsStdString()).toDouble(&q_Ok);
      if (!q_Ok)
      {
         f64_Value = of64_Default;
      }
   }
   return f64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeStringError(const C_SclString & orc_Name, C_SclString & orc_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orc_Value = this->GetAttributeString(orc_Name);
   }
   else
   {
      orc_Value = "";
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeSint32Error(const C_SclString & orc_Name, int32_t & ors32_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      ors32_Value = this->GetAttributeSint32(orc_Name);
   }
   else
   {
      ors32_Value = 0L;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeUint32Error(const C_SclString & orc_Name, uint32_t & oru32_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      oru32_Value = this->GetAttributeUint32(orc_Name);
   }
   else
   {
      oru32_Value = 0UL;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeSint64Error(const C_SclString & orc_Name, int64_t & ors64_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      ors64_Value = this->GetAttributeSint64(orc_Name);
   }
   else
   {
      ors64_Value = 0LL;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeUint64Error(const C_SclString & orc_Name, uint64_t & oru64_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      oru64_Value = this->GetAttributeUint64(orc_Name);
   }
   else
   {
      oru64_Value = 0ULL;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeBoolError(const C_SclString & orc_Name, bool & orq_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orq_Value = this->GetAttributeBool(orc_Name);
   }
   else
   {
      orq_Value = false;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeFloat32Error(const C_SclString & orc_Name, float32_t & orf32_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orf32_Value = this->GetAttributeFloat32(orc_Name);
   }
   else
   {
      orf32_Value = 0.0F;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeFloat64Error(const C_SclString & orc_Name, float64_t & orf64_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orf64_Value = this->GetAttributeFloat64(orc_Name);
   }
   else
   {
      orf64_Value = 0.0;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for node content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForNodeContentAppendXmlContext(const C_SclString & orc_ErrorMessage) const
{
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for attribute content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForAttributeContentAppendXmlContext(const C_SclString & orc_Attribute,
                                                                        const C_SclString & orc_ErrorMessage) const
{
   (void)orc_Attribute;
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for node content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForNodeContentStartingWithXmlContext(const C_SclString & orc_ErrorMessage) const
{
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for attribute content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForAttributeContentStartingWithXmlContext(const C_SclString & orc_Attribute,
                                                                              const C_SclString & orc_ErrorMessage) const
{
   (void)orc_Attribute;
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for node missing
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForNodeMissing(const C_SclString & orc_MissingNodeName) const
{
   (void)orc_MissingNodeName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get all attribute values of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<C_OscXmlAttribute> C_OscXmlParserBase::GetAttributes(void) const
{
   std::vector<C_OscXmlAttribute> c_AttributeList;
   if (!mc_CurrentElement.isNull())
   {
      QDomNamedNodeMap c_Attrs = mc_CurrentElement.attributes();
      for (int i = 0; i < c_Attrs.length(); ++i)
      {
         QDomNode c_Node = c_Attrs.item(i);
         if (c_Node.isAttr())
         {
            QDomAttr c_Attr = c_Node.toAttr();
            C_OscXmlAttribute c_Data;
            c_Data.c_Name = c_Attr.name().toStdString();
            c_Data.c_Value = c_Attr.value().toStdString();
            c_AttributeList.push_back(c_Data);
         }
      }
   }

   return c_AttributeList;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create a node under the currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::CreateNodeChild(const C_SclString & orc_Name, const C_SclString & orc_Content)
{
   QDomElement c_NewNode = mc_Document.createElement(QString::fromStdString(*orc_Name.AsStdString()));
   if (orc_Content != "")
   {
      c_NewNode.appendChild(mc_Document.createTextNode(QString::fromStdString(*orc_Content.AsStdString())));
   }
   
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.appendChild(c_NewNode);
   }
   else
   {
      mc_Document.appendChild(c_NewNode);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create a node under the currently selected node and select it.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::CreateAndSelectNodeChild(const C_SclString & orc_Name)
{
   C_SclString c_ResultName;

   this->CreateNodeChild(orc_Name);
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement = mc_CurrentElement.lastChildElement();
   }
   else
   {
      mc_CurrentElement = mc_Document.documentElement();
   }

   if (!mc_CurrentElement.isNull())
   {
      c_ResultName = mc_CurrentElement.tagName().toStdString();
   }
   return c_ResultName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Delete the current node.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscXmlParserBase::DeleteNode(void)
{
   C_SclString c_Name;

   if (!mc_CurrentElement.isNull())
   {
      c_Name = mc_CurrentElement.tagName().toStdString();
      QDomNode c_Parent = mc_CurrentElement.parentNode();
      c_Parent.removeChild(mc_CurrentElement);
      mc_CurrentElement = QDomElement();
   }

   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set content of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetNodeContent(const C_SclString & orc_Content)
{
   if (!mc_CurrentElement.isNull())
   {
      // Remove existing text nodes
      QDomNodeList c_Children = mc_CurrentElement.childNodes();
      for (int i = 0; i < c_Children.count(); ++i)
      {
         if (c_Children.at(i).isText())
         {
            mc_CurrentElement.removeChild(c_Children.at(i));
            --i;
         }
      }
      mc_CurrentElement.appendChild(mc_Document.createTextNode(QString::fromStdString(*orc_Content.AsStdString())));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set string content of one attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeString(const C_SclString & orc_Name, const C_SclString & orc_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), 
                                     QString::fromStdString(*orc_Value.AsStdString()));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set sint32 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeSint32(const C_SclString & orc_Name, const int32_t os32_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), os32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set uint32 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeUint32(const C_SclString & orc_Name, const uint32_t ou32_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), ou32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set sint64 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeSint64(const C_SclString & orc_Name, const int64_t os64_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), static_cast<long long>(os64_Value));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set uint64 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeUint64(const C_SclString & orc_Name, const uint64_t ou64_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), static_cast<unsigned long long>(ou64_Value));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set bool content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeBool(const C_SclString & orc_Name, const bool oq_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), oq_Value ? "true" : "false");
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set float32 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeFloat32(const C_SclString & orc_Name, const float32_t of32_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), static_cast<double>(of32_Value));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set float64 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeFloat64(const C_SclString & orc_Name, const float64_t of64_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(QString::fromStdString(*orc_Name.AsStdString()), of64_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Open XML data from string
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParser::LoadFromString(const C_SclString & orc_String)
{
   int32_t s32_Return = C_NO_ERR;
   mc_Document.clear();

   QString c_ErrorMsg;
   int s32_ErrorLine;
   int s32_ErrorCol;
   if (!mc_Document.setContent(QString::fromStdString(*orc_String.AsStdString()), &c_ErrorMsg, &s32_ErrorLine, &s32_ErrorCol))
   {
      s32_Return = C_NOACT;
      m_Init();
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write XML data to string
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParser::SaveToString(C_SclString & orc_String) const
{
   orc_String = mc_Document.toString(3).toStdString();
}
