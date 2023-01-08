#include "ToshiPCH.h"
#include "TXUIImage.h"

bool Toshi::XURXUIImageData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
    return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIImageData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUIImageData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return (propType != 1) * 2 + 2;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUIImageData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, SizeMode, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ImagePath, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, BrushFlags, propType);
	
	param_2++;
	return XURXUIElementData::TranslateTimelineProp(name, param_2, propType);
}

bool Toshi::XURXUIImageData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUIImageData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int flag = 0;
		if (m_index != 0) TXUI_READ_BYTE(a_pData, flag);

		TXUI_READ_PROP_DWORD(a_pData, flag, SizeMode);
		TXUI_READ_PROP_WORD(a_pData, flag, ImagePath);
		TXUI_READ_PROP_DWORD(a_pData, flag, BrushFlags);
	}

    return true;
}
