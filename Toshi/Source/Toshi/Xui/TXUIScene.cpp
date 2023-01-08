#include "ToshiPCH.h"
#include "TXUIScene.h"

namespace Toshi
{
	XURXUISceneData::XURXUISceneData()
	{
		m_DefaultFocus = 0;
		m_TransFrom = 0;
		m_TransTo = 0;
		m_TransBackFrom = 0;
		m_TransBackTo = 0;
		m_InterruptTransitions = 0;
		m_IgnorePresses = false;
	}

	bool XURXUISceneData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return false;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

	bool XURXUISceneData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return false;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	uint32_t XURXUISceneData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0)
		{
			TASSERT(a_uiObjectIndex > 0);
			return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		}

		if (propType == PropType_IgnorePresses)
		{
			return 1;
		}

		if (propType != PropType_DefaultFocus && propType != PropType_TransFrom && propType != PropType_TransTo && propType != PropType_TransBackFrom && propType != PropType_TransBackTo)
		{
			return 4;
		}

		return 2;
	}

	bool XURXUISceneData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, DefaultFocus, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, TransFrom, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, TransTo, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, TransBackFrom, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, TransBackTo, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, InterruptTransitions, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, IgnorePresses, propType);

		param_2++;
		return XURXUIControlData::TranslateTimelineProp(name, param_2, propType);
	}

	bool XURXUISceneData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex == 0) return param_2 < PropType_NUMOF;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	}

	bool XURXUISceneData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIControlData::Load(resource, a_pData);
		uint8_t smth = *a_pData++;

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_BYTE(a_pData, flag);

			TXUI_READ_PROP_WORD(a_pData, flag, DefaultFocus);
			TXUI_READ_PROP_WORD(a_pData, flag, TransFrom);
			TXUI_READ_PROP_WORD(a_pData, flag, TransTo);
			TXUI_READ_PROP_WORD(a_pData, flag, TransBackFrom);
			TXUI_READ_PROP_WORD(a_pData, flag, TransBackTo);
			TXUI_READ_PROP_DWORD(a_pData, flag, InterruptTransitions);
			TXUI_READ_PROP_BYTE(a_pData, flag, IgnorePresses);
		}

		return true;
	}

}