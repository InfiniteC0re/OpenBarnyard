#include "ToshiPCH.h"
#include "TXUIGroup.h"

namespace Toshi
{
	bool XURXUIGroupData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
		return false;
	}

	bool XURXUIGroupData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
		return false;
	}

	uint32_t XURXUIGroupData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		return 4;
	}

	bool XURXUIGroupData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		param_2++;
		return XURXUIElementData::TranslateTimelineProp(name, param_2, propType);
	}

	bool XURXUIGroupData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
		return false;
	}

	bool XURXUIGroupData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIElementData::Load(resource, a_pData);
		a_pData++;
		return true;
	}
}