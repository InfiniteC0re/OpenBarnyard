#include "ToshiPCH.h"
#include "TXUIText.h"

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
	if (a_uiObjectIndex == 0) return param_2 < 7;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUITextData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int flag = 0;
		if (m_index != 0) TXUI_READ_BYTE(a_pData, flag);

		TXUI_READ_PROP_WORD(a_pData, flag, Text);
		TXUI_READ_PROP_DWORD(a_pData, flag, TextColor);
		TXUI_READ_PROP_DWORD(a_pData, flag, DropShadowColor);
		TXUI_READ_PROP_FLOAT(a_pData, flag, PointSize);
		TXUI_READ_PROP_WORD(a_pData, flag, Font);
		TXUI_READ_PROP_DWORD(a_pData, flag, TextStyle);
		TXUI_READ_PROP_DWORD(a_pData, flag, LineSpacingAdjust);
	}
    return true;
}
