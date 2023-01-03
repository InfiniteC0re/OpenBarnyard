#include "ToshiPCH.h"
#include "TXUIElement.h"

const char* Toshi::XURXUIElementData::sm_sTypeInfo = "XURXUIElementData";

bool Toshi::XURXUIObjectData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	unk = PARSEWORD_BIG(a_pData);
	a_pData += 2;
	return true;
}

void Toshi::XURXUIObjectData::LoadChildren(uint8_t* a_pData)
{
	TASSERT(PARSEDWORD(a_pData) < (1 << 8), "Not a Word");
}

bool Toshi::XURXUIElementData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	Toshi::XURXUIObjectData::Load(resource, a_pData);
	uint8_t smth = *a_pData++;

	if (smth != 0)
	{
		int smth2 = 0;
		if (unk != 0)
		{
			smth2 = PARSEWORD_BIG(a_pData + 1);
			a_pData += 3;
		}
		if ((smth2 & 1) != 0)
		{
			m_id = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 2) != 0)
		{
			m_width = PARSEFLOAT_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 4) != 0)
		{
			m_height = PARSEFLOAT_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 8) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16), "");
			m_positon = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 0x10) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16), "");
			m_rotation = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 0x20) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16), "");
			m_scale = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 0x40) != 0)
		{
			float opacity = (float)PARSEDWORD_BIG(a_pData);
			TASSERT(opacity >= 0.0f && opacity <= 1.0f, "");
			m_opacity = opacity * 255.0f;
			a_pData += 4;
		}
		if ((smth2 & 0x80) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) <= Flags_AnchorMask, "");
			m_anchor |= PARSEWORD_BIG(a_pData + 2) * 8;
			a_pData += 4;
		}
		if ((smth2 & 0x100) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16), "");
			m_pivot = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 0x200) != 0)
		{
			m_show &= ~0x1000;
			uint8_t val = *a_pData++;
			if (val != 0) val = 0x1000;
			m_show |= val;
		}
		if ((smth2 & 0x400) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) <= Flags_BlendModeMask, "");
			m_blendMode |= PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 0x800) != 0)
		{
			m_disableTimelineRecursion &= ~0x2000;
			uint8_t val = *a_pData++;
			if (val != 0) val = 0x2000;
			m_disableTimelineRecursion |= val;
		}
		// m_colorWriteFlags
		if ((smth2 & 0x2000) != 0)
		{
			a_pData += 4;
		}
		if ((smth2 & 0x4000) != 0)
		{
			m_clipChildren &= ~0x800;
			uint8_t val = *a_pData++;
			if (val != 0) val = 0x800;
			m_clipChildren |= val;
		}
	}
	return true;
}


bool Toshi::XURXUIElementData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3) // param_3 = position?
{
	if (TStringManager::String8Compare(param_1, "Id", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Width", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Height", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Position", -1) == 0)
	{
		param_3 = 3;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Scale", -1) == 0)
	{
		param_3 = 4;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Rotation", -1) == 0)
	{
		param_3 = 5;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Opacity", -1) == 0)
	{
		param_3 = 6;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Anchor", -1) == 0)
	{
		param_3 = 7;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Pivot", -1) == 0)
	{
		param_3 = 8;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Show", -1) == 0)
	{
		param_3 = 9;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "BlendMode", -1) == 0)
	{
		param_3 = 10;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "DisableTimelineRecursion", -1) == 0)
	{
		param_3 = 11;
		return true;
	}
	// This is correct for some reason they skipped 12
	else if (TStringManager::String8Compare(param_1, "ColorWriteFlags", -1) == 0)
	{
		param_3 = 13;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "ClipChidren", -1) == 0)
	{
		param_3 = 14;
		return true;
	}
	return false;
}
