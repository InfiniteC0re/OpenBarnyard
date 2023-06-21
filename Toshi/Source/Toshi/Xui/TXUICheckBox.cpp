#include "ToshiPCH.h"
#include "TXUICheckBox.h"
#include "XURReader.h"

namespace Toshi {

	TBOOL XURXUICheckBoxData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIControlData::Load(resource, a_pData);

		if (*a_pData++ != 0)
		{
			XURReader reader(a_pData);
			if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_PressKey, m_uiPressKey);
		}

		return TTRUE;
	}

	TBOOL XURXUICheckBoxData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex == 0)
			return a_uiPropIndex < PropType_NUMOF;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
	}

	TBOOL XURXUICheckBoxData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, PressKey, propType);

		a_uiObjectIndex++;
		return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
	}

	uint32_t XURXUICheckBoxData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
		{
			return 4;
		}
		else
		{
			TASSERT(a_uiObjectIndex > 0);
			return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		}
	}

	TBOOL XURXUICheckBoxData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUICheckBoxData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

}
