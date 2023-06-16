#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "XURReader.h"

namespace Toshi
{
	TBOOL XURXUIButtonData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == PropType_PressKey) return TFALSE;
		TASSERT(a_uiObjectIndex > PropType_PressKey);

		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUIButtonData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == PropType_PressKey) return TFALSE;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	uint32_t XURXUIButtonData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == PropType_PressKey) return 4;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUIButtonData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, PressKey, propType);

		param_2++;
		return XURXUIControlData::TranslateTimelineProp(name, param_2, propType);
	}

	TBOOL XURXUIButtonData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex == PropType_PressKey) return param_2 == 0;
		TASSERT(a_uiObjectIndex > PropType_PressKey);
		return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	}

	TBOOL XURXUIButtonData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIControlData::Load(resource, a_pData);

		if (*a_pData++ != 0)
		{
			XURReader reader(a_pData);
			if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_PressKey, m_PressKey);
		}

		return TTRUE;
	}
}