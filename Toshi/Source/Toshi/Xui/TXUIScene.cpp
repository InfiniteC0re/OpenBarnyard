#include "ToshiPCH.h"
#include "TXUIScene.h"

Toshi::XURXUISceneData::XURXUISceneData()
{
	m_defaultFocus = 0;
	m_transFrom = 0;
	m_transTo = 0;
	m_transBackFrom = 0;
	m_transBackTo = 0;
	m_interruptTransitions = 0;
	m_ignorePresses = false;
}

bool Toshi::XURXUISceneData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

bool Toshi::XURXUISceneData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return false;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

uint32_t Toshi::XURXUISceneData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
    if (a_uiObjectIndex != 0) 
    {
        TASSERT(a_uiObjectIndex > 0);
        return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
    }
    if (propType == 6) {
        return 1;
    }
    if ((propType != 0) && (propType != 1) && (propType != 2) && (propType != 3) && (propType != 4))
    {
        return 4;
    }
    return 2;
}

bool Toshi::XURXUISceneData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "DefaultFocus", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "TransFrom", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "TransTo", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "TransBackFrom", -1) == 0)
	{
		param_3 = 3;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "TransBackTo", -1) == 0)
	{
		param_3 = 4;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "InterruptTransitions", -1) == 0)
	{
		param_3 = 5;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "IgnorePresses", -1) == 0)
	{
		param_3 = 6;
		return true;
	}
	else
	{
		param_2++;
		return XURXUIControlData::TranslateTimelineProp(param_1, param_2, param_3);
	}
}

bool Toshi::XURXUISceneData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex == 0) return param_2 < 7;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
}

bool Toshi::XURXUISceneData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIControlData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int smth2 = 0;
		if (m_index != 0)
		{
			smth2 = *a_pData++;
		}
		if ((smth2 & 1) != 0)
		{
			m_defaultFocus = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 2) != 0)
		{
			m_transFrom = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 4) != 0)
		{
			m_transTo = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 8) != 0)
		{
			m_transBackFrom = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x10) != 0)
		{
			m_transBackTo = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x20) != 0)
		{
			m_interruptTransitions = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 0x40) != 0)
		{
			m_ignorePresses = *a_pData++;
		}
	}
	return true;
}
