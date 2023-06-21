#include "ToshiPCH.h"
#include "TXUILabel.h"

namespace Toshi {

	TBOOL XURXUILabelData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
		return TFALSE;
	}

	TBOOL XURXUILabelData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
		return TFALSE;
	}

	uint32_t XURXUILabelData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, propType);
		return 4;
	}

	TBOOL XURXUILabelData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
	{
		a_uiObjectIndex++;
		return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
	}

	TBOOL XURXUILabelData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex != 0) return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
		return TFALSE;
	}

	TBOOL XURXUILabelData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIControlData::Load(resource, a_pData);

		if (*a_pData++ != 0)
		{
			a_pData += 4;
		}

		return TTRUE;
	}

}
