#include "ToshiPCH.h"
#include "TXUITabScene.h"
#include "XURReader.h"

namespace Toshi {

	TBOOL XURXUITabSceneData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUISceneData::Load(resource, a_pData);

		if (*a_pData++ != 0)
		{
			XURReader reader(a_pData);
			if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TabCount, m_uiTabCount);
			reader.ReadProperty<XUI_EPT_BOOL>(PropType_Wrap, m_bWrap);
			reader.ReadProperty<XUI_EPT_BOOL>(PropType_UserInterrupt, m_bUserInterrupt);
			reader.ReadProperty<XUI_EPT_BOOL>(PropType_VerticalTabs, m_bVerticalTabs);
			reader.ReadProperty<XUI_EPT_BOOL>(PropType_NoAutoHide, m_bNoAutoHide);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_DefaultTab, m_uiDefaultTab);
		}

		return TTRUE;
	}

	TBOOL XURXUITabSceneData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex == 0)
			return a_uiPropIndex < PropType_NUMOF;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUISceneData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
	}

	TBOOL XURXUITabSceneData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, TabCount, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Wrap, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, UserInterrupt, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, VerticalTabs, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, NoAutoHide, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, DefaultTab, propType);

		a_uiObjectIndex++;
		return XURXUISceneData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
	}

	uint32_t XURXUITabSceneData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
		{
			if (propType == PropType_Wrap || propType == PropType_UserInterrupt || propType == PropType_VerticalTabs || propType == PropType_NoAutoHide)
				return 1;
			else
				return 4;
		}
		else
		{
			TASSERT(a_uiObjectIndex > 0);
			return XURXUISceneData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		}
	}

	TBOOL XURXUITabSceneData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUISceneData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUITabSceneData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0)
			return TFALSE;

		TASSERT(a_uiObjectIndex > 0);
		return XURXUISceneData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}
	
}
