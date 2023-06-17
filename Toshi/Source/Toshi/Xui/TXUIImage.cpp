#include "ToshiPCH.h"
#include "TXUIImage.h"
#include "XURReader.h"

namespace Toshi
{
	TBOOL XURXUIImageData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return TFALSE;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUIImageData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return TFALSE;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	}

	uint32_t XURXUIImageData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex == 0) return (propType != 1) * 2 + 2;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}

	TBOOL XURXUIImageData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, SizeMode, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, ImagePath, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, BrushFlags, propType);
	
		param_2++;
		return XURXUIElementData::TranslateTimelineProp(name, param_2, propType);
	}

	TBOOL XURXUIImageData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		if (a_uiObjectIndex == 0) return param_2 < PropType_NUMOF;
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	}

	TBOOL XURXUIImageData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIElementData::Load(resource, a_pData);
		
		if (*a_pData++ != 0)
		{
			XURReader reader(a_pData);
			if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_SizeMode, m_SizeMode);
			reader.ReadProperty<XUI_EPT_STRING>(PropType_ImagePath, m_ImagePath);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_BrushFlags, m_BrushFlags);
		}

		return TTRUE;
	}
}
