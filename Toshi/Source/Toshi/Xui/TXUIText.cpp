#include "ToshiPCH.h"
#include "TXUIText.h"
#include "XURReader.h"

Toshi::XURXUITextData::XURXUITextData() : Toshi::XURXUIElementData()
{
	m_Text = 0;
	m_TextColor = 0xFF000000;
	m_DropShadowColor = 0x80000000;
	m_PointSize = sm_uiDefaultFontSize;
	m_Font = 0;
	m_TextStyle = 0x110;
	m_LineSpacingAdjust = 0;
	// this + 0x20 = TXUITEXT
	// TClass::Find("TXUIText",(TClass *)&TXUIElement::m_sClass);
}

bool Toshi::XURXUITextData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}
	if (propType == 1) return true; // propType 1 = TextColor
	return propType == 2;
}

bool Toshi::XURXUITextData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUITextData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType != 0 && propType != 4) return 4;
	return 2;
}

bool Toshi::XURXUITextData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, Text, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextColor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DropShadowColor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, PointSize, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Font, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextStyle, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, LineSpacingAdjust, propType);
	
	param_2++;
	return XURXUIElementData::TranslateTimelineProp(name, param_2, propType);
}

bool Toshi::XURXUITextData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUITextData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
	
	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_Text, m_Text);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_TextColor, m_TextColor);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_DropShadowColor, m_DropShadowColor);
		reader.ReadProperty<XUI_EPT_FLOAT>(PropType_PointSize, m_PointSize);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_Font, m_Font);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TextStyle, m_TextStyle);
		reader.ReadProperty<XUI_EPT_INTEGER>(PropType_LineSpacingAdjust, m_LineSpacingAdjust);
	}

    return true;
}
