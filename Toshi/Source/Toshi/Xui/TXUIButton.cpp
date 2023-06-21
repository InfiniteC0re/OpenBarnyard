#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "XURReader.h"

namespace Toshi
{
	TBOOL XURXUIButtonData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return TFALSE;
		TASSERT(a_uiObjectIndex > 0);

		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUIButtonData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return TFALSE;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	uint32_t XURXUIButtonData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return 4;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUIButtonData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, PressKey, propType);

		a_uiObjectIndex++;
		return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
	}

	TBOOL XURXUIButtonData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex == 0) return a_uiPropIndex == 0;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
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