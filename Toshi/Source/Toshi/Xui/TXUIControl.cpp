#include "ToshiPCH.h"
#include "TXUIControl.h"

bool Toshi::XURXUIControlData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0, "");
	return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIControlData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0, "");
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUIControlData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0, "");
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType != 2 && propType != 3) 
	{
		if (propType != 0 && propType != 1 && propType != 4 &&
			propType != 5 && propType != 6 &&
			propType != 7 && propType != 8 && propType != 10) 
		{
			return 4;
		}
		return 2;
	}
	return 1;
}

bool Toshi::XURXUIControlData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "ClassOverride", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Visual", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Enabled", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "UnfocussedInput", -1) == 0)
	{
		param_3 = 3;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "NavLeft", -1) == 0)
	{
		param_3 = 4;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "NavRight", -1) == 0)
	{
		param_3 = 5;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "NavUp", -1) == 0)
	{
		param_3 = 6;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "NavDown", -1) == 0)
	{
		param_3 = 7;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Text", -1) == 0)
	{
		param_3 = 8;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "ImagePath", -1) == 0)
	{
		param_3 = 10;
		return true;
	}
	else
	{
		param_2++;
		return XURXUIElementData::TranslateTimelineProp(param_1, param_2, param_3);
	}
}

bool Toshi::XURXUIControlData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 < 11;
	TASSERT(a_uiObjectIndex > 0, "");
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUIControlData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int smth2 = 0;
		if (unk != 0)
		{
			smth2 = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 1) != 0)
		{
			m_classOverride = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 2) != 0)
		{
			m_visual = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 4) != 0)
		{
			m_enabled = *a_pData++;
		}
		if ((smth2 & 8) != 0)
		{
			m_unfocussedInput = *a_pData++;
		}
		if ((smth2 & 0x10) != 0)
		{
			m_navLeft = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x20) != 0)
		{
			m_navRight = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x40) != 0)
		{
			m_navUp = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x80) != 0)
		{
			m_navDown = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x100) != 0)
		{
			m_text = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x400) != 0)
		{
			m_imagePath = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
	}
	return true;
}
