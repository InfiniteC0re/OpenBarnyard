#include "ToshiPCH.h"
#include "TXUIButton.h"

namespace Toshi
{
	bool XURXUIButtonData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return false;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

	bool XURXUIButtonData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return false;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	uint32_t XURXUIButtonData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return 4;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}

	bool XURXUIButtonData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, PressKey, propType);

		param_2++;
		return XURXUIControlData::TranslateTimelineProp(name, param_2, propType);
	}

	bool XURXUIButtonData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex == 0) return param_2 == 0;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	}

	bool XURXUIButtonData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIControlData::Load(resource, a_pData);
		uint8_t smth = *a_pData++;

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_BYTE(a_pData, flag);

			TXUI_READ_PROP_DWORD(a_pData, flag, PressKey);
		}
		return true;
	}

}