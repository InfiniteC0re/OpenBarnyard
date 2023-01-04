#include "ToshiPCH.h"
#include "TXUIStroke.h"

bool Toshi::XURXUIStrokeData::IsColourPropType(uint32_t propType)
{
	return propType == 1;
}

bool Toshi::XURXUIStrokeData::IsFloatPropType(uint32_t propType)
{
	return propType == 0;
}

uint32_t Toshi::XURXUIStrokeData::GetTimelinePropSize(uint32_t propType)
{
	return 4;
}

bool Toshi::XURXUIStrokeData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "Stroke.StrokeWidth", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Stroke.StrokeColor", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	return false;
}

bool Toshi::XURXUIStrokeData::ValidateTimelineProp(uint32_t param_2)
{
	return param_2 < 2;
}

bool Toshi::XURXUIStrokeData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);

	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int smth2 = 0;
		if (unk != 0)
		{
			smth2 = *a_pData++;
		}
		if ((smth2 & 1) != 0)
		{
			m_strokeWidth = PARSEFLOAT_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 2) != 0)
		{
			m_strokeColor = PARSEFLOAT_BIG(a_pData);
			a_pData += 4;
		}
	}
	return true;
}
