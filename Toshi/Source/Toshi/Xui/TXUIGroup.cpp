#include "ToshiPCH.h"
#include "TXUIGroup.h"

namespace Toshi
{
	TBOOL XURXUIGroupData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
		return TFALSE;
	}

	TBOOL XURXUIGroupData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
		return TFALSE;
	}

	uint32_t XURXUIGroupData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		return 4;
	}

	TBOOL XURXUIGroupData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		param_2++;
		return XURXUIElementData::TranslateTimelineProp(name, param_2, propType);
	}

	TBOOL XURXUIGroupData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex != 0) return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
		return TFALSE;
	}

	TBOOL XURXUIGroupData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIElementData::Load(resource, a_pData);
		a_pData++;
		return TTRUE;
	}
}