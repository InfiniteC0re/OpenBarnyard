#include "ToshiPCH.h"
#include "TXUIFill.h"

bool Toshi::XURXUIFillData::IsColourPropType(uint32_t propType)
{
    return propType == 1;
}

bool Toshi::XURXUIFillData::IsFloatPropType(uint32_t propType)
{
    return propType == 6;
}

uint32_t Toshi::XURXUIFillData::GetTimelinePropSize(uint32_t propType)
{
    return propType != 2 ? 4 : 2;
}

bool Toshi::XURXUIFillData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
    return false;
}

bool Toshi::XURXUIFillData::ValidateTimelineProp(uint32_t param_2)
{
    return param_2 < 11;
}

bool Toshi::XURXUIFillData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	// TODO:!!!!! NOT FINISHED !!!!!!

	if (smth != 0)
	{
		int smth2 = 0;
		if (unk != 0)
		{
			smth2 = PARSEWORD_BIG(a_pData);
		}
		if ((smth2 & 1) != 0)
		{
			m_fillType = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 2) != 0)
		{
			m_fillColor = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 4) != 0)
		{
			m_fillTextureFileName = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x8) != 0)
		{
			m_fillTranslation = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x10) != 0)
		{
			m_fillScale = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 0x20) != 0)
		{
			m_fillRotation = *a_pData++;
		}
		if ((smth2 & 0x40) != 0)
		{
			m_fillRotation = *a_pData++;
		}
		if ((smth2 & 0x80) != 0)
		{
			m_fillRotation = *a_pData++;
		}
		if ((smth2 & 0x100) != 0)
		{
			m_fillRotation = *a_pData++;
		}
		if ((smth2 & 0x200) != 0)
		{
			m_fillRotation = *a_pData++;
		}
		if ((smth2 & 0x400) != 0)
		{
			m_fillRotation = *a_pData++;
		}
	}
	return true;
}
