#include "ToshiPCH.h"
#include "TXUISlider.h"
#include "XURReader.h"

namespace Toshi {

	TBOOL XURXUISliderData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIControlData::Load(resource, a_pData);

		if (*a_pData++ != 0)
		{
			XURReader reader(a_pData);
			if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_RangeMin, m_iRangeMin);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_RangeMax, m_iRangeMax);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_Value, m_iValue);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_Step, m_iStep);
			reader.ReadProperty<XUI_EPT_BOOL>(PropType_Vertical, m_bVertical);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_AccelInc, m_iAccelInc);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_AccelTime, m_iAccelTime);
		}

		return TTRUE;
	}

	TBOOL XURXUISliderData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex == 0)
			return a_uiPropIndex < PropType_NUMOF;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, a_uiPropIndex);
	}

	TBOOL XURXUISliderData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, RangeMin, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, RangeMax, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Value, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Step, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Vertical, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, AccelInc, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, AccelTime, propType);

		a_uiObjectIndex++;
		return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
	}

	uint32_t XURXUISliderData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
		{
			if (propType == PropType_Vertical)
				return 1;
			else
				return 4;
		}
		else
		{
			TASSERT(a_uiObjectIndex > 0);
			return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		}
	}

	TBOOL XURXUISliderData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUISliderData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

}
