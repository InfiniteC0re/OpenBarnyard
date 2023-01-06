#include "ToshiPCH.h"
#include "TXUIGroup.h"

bool Toshi::XURXUIGroupData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	return false;
}

bool Toshi::XURXUIGroupData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	return false;
}

uint32_t Toshi::XURXUIGroupData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	return 4;
}

bool Toshi::XURXUIGroupData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	param_1++;
	return XURXUIElementData::TranslateTimelineProp(param_1, param_2, param_3);
}

bool Toshi::XURXUIGroupData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	return false;
}

bool Toshi::XURXUIGroupData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
    a_pData++;
    return true;
}
