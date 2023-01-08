#include "ToshiPCH.h"
#include "TXUIControl.h"

namespace Toshi
{
	bool Toshi::XURXUIControlData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return false;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

	bool Toshi::XURXUIControlData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return false;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	uint32_t Toshi::XURXUIControlData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0)
		{
			TASSERT(a_uiObjectIndex > 0);
			return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		}
		if (propType != 2 && propType != 3)
		{
			if (propType != 0 && propType != 1 && propType != 4 &&
				propType != 5 && propType != 6 &&
				propType != 7 && propType != 8 && propType != 10)
			{
				return 4;
			}
			return 2;
		}
		return 1;
	}

	bool Toshi::XURXUIControlData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, ClassOverride, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Visual, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Enabled, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, UnfocussedInput, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, NavLeft, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, NavRight, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, NavUp, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, NavDown, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Text, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, ImagePath, propType);

		param_2++;
		return XURXUIElementData::TranslateTimelineProp(name, param_2, propType);
	}

	bool Toshi::XURXUIControlData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex == 0) return param_2 < 11;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	}

	bool Toshi::XURXUIControlData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIElementData::Load(resource, a_pData);
		uint8_t smth = *a_pData++;

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_WORD(a_pData, flag);

			TXUI_READ_PROP_WORD(a_pData, flag, ClassOverride);
			TXUI_READ_PROP_WORD(a_pData, flag, Visual);
			TXUI_READ_PROP_BYTE(a_pData, flag, Enabled);
			TXUI_READ_PROP_BYTE(a_pData, flag, UnfocussedInput);
			TXUI_READ_PROP_WORD(a_pData, flag, NavLeft);
			TXUI_READ_PROP_WORD(a_pData, flag, NavRight);
			TXUI_READ_PROP_WORD(a_pData, flag, NavUp);
			TXUI_READ_PROP_WORD(a_pData, flag, NavDown);
			TXUI_READ_PROP_WORD(a_pData, flag, Text);
			TXUI_READ_PROP_WORD(a_pData, flag, ImagePath);
		}
		return true;
	}

}