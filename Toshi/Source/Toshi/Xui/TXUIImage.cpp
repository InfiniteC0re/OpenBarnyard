#include "ToshiPCH.h"
#include "TXUIImage.h"

bool Toshi::XURXUIImageData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
    return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIImageData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUIImageData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return (propType != 1) * 2 + 2;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIImageData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "SizeMode", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "ImagePath", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "BrushFlags", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else
	{
		param_2++;
		return XURXUIElementData::TranslateTimelineProp(param_1, param_2, param_3);
	}
}

bool Toshi::XURXUIImageData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 < 3;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUIImageData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int smth2 = 0;
		if (m_index != 0)
		{
			smth2 = *a_pData++;
		}
		if ((smth2 & 1) != 0)
		{
			m_sizeMode = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 2) != 0)
		{
			m_imagePath = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 4) != 0)
		{
			m_imagePath = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
	}
    return true;
}
