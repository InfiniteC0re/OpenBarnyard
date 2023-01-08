#include "ToshiPCH.h"
#include "TXUIButton.h"

bool Toshi::XURXUIButtonData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIButtonData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUIButtonData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return 4;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIButtonData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "PressKey", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	param_3++;
	return XURXUIControlData::TranslateTimelineProp(param_1, param_2, param_3);
}

bool Toshi::XURXUIButtonData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 == 0;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUIButtonData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIControlData::Load(resource, a_pData);
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
			m_pressKey = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
	}
	return true;
}
