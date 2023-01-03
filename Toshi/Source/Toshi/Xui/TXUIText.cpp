#include "ToshiPCH.h"
#include "TXUIText.h"

Toshi::XURXUITextData::XURXUITextData() : Toshi::XURXUIElementData()
{
	m_text = 0;
	m_textColor = 0xFF000000;
	m_dropShadowColor = 0x80000000;
	m_pointSize = sm_uiDefaultFontSize;
	m_font = 0;
	m_textStyle = 0x110;
	m_lineSpacingAdjust = 0;
	// this + 0x20 = TXUITEXT
	// TClass::Find("TXUIText",(TClass *)&TXUIElement::m_sClass);
}

bool Toshi::XURXUITextData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0, "");
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}
	if (propType == 1) return true; // propType 1 = TextColor
	return propType == 2;
}

bool Toshi::XURXUITextData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0, "");
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUITextData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0, "");
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType != 0 && propType != 4) return 4;
	return 2;
}

bool Toshi::XURXUITextData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "Text", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "TextColor", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "DropShadowColor", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "PointSize", -1) == 0)
	{
		param_3 = 3;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Font", -1) == 0)
	{
		param_3 = 4;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "TextStyle", -1) == 0)
	{
		param_3 = 5;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "LineSpacingAdjust", -1) == 0)
	{
		param_3 = 6;
		return true;
	}
	else
	{
		param_2++;
		return XURXUIElementData::TranslateTimelineProp(param_1, param_2, param_3);
	}
}

bool Toshi::XURXUITextData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 < 7;
	TASSERT(a_uiObjectIndex > 0, "");
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUITextData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int smth2 = 0;
		if (unk != 0)
		{
			smth2 = *(a_pData + 1);
			a_pData += 2;
		}
		if ((smth2 & 1) != 0)
		{
			m_text = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 2) != 0)
		{
			m_textColor = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 4) != 0)
		{
			m_dropShadowColor = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 8) != 0)
		{
			// This might be an half float but assembly looks weird so -.-
			m_pointSize = PARSEFLOAT_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 0x10) != 0)
		{
			m_font = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x20) != 0)
		{
			m_font = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 0x40) != 0)
		{
			m_font = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
	}
    return false;
}
