#include "ToshiPCH.h"
#include "TXUISliderBar.h"
#include "XURReader.h"

namespace Toshi {

	XURXUISliderBarData::XURXUISliderBarData()
	{
		m_iRangeMin = 0;
		m_iRangeMax = 100;
		m_iValue = 0;
		m_iStep = 1;
		m_iAccelInc = 0;
		m_iAccelTime = 0;
		m_bVertical = TFALSE;
		m_pClass = TClass::Find("TXUISliderBar", TGetClass(TXUIControl));
	}

	TBOOL XURXUISliderBarData::Load(TXUIResource& resource, uint8_t*& a_pData)
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

			TASSERT(m_iRangeMax > m_iRangeMin);
			TASSERT(m_iValue >= m_iRangeMin && m_iValue <= m_iRangeMax);
		}

		return TTRUE;
	}

	TBOOL XURXUISliderBarData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex == 0)
			return a_uiPropIndex < PropType_NUMOF;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
	}

	TBOOL XURXUISliderBarData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
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

	uint32_t XURXUISliderBarData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
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

	TBOOL XURXUISliderBarData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUISliderBarData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

}
