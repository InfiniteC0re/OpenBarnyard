#include "ToshiPCH.h"
#include "TXUIFigure.h"


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
	if (TStringManager::String8Compare(param_1, "Fill.FillType", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.FillColor", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.TextureFileName", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.Translation", -1) == 0)
	{
		param_3 = 4;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.Scale", -1) == 0)
	{
		param_3 = 5;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.Rotation", -1) == 0)
	{
		param_3 = 6;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.WrapX", -1) == 0)
	{
		param_3 = 7;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.WrapY", -1) == 0)
	{
		param_3 = 8;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.BrushFlags", -1) == 0)
	{
		param_3 = 9;
		return true;
	}
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
		if (m_index != 0)
		{
			smth2 = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 1) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
			m_fillType = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 2) != 0)
		{
			m_fillColor = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 4) != 0)
		{
			m_fillTextureFileName = PARSEWORD_BIG(a_pData);
			a_pData += 2;
		}
		if ((smth2 & 0x8) != 0)
		{
			// TODO XURXUIGradientData
		}
		if ((smth2 & 0x10) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 15)); // 15 is probably a typo they did?
			m_fillTranslation = PARSEDWORD_BIG(a_pData);
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

// StrokeData

bool Toshi::XURXUIStrokeData::IsColourPropType(uint32_t propType)
{
	return propType == 1;
}

bool Toshi::XURXUIStrokeData::IsFloatPropType(uint32_t propType)
{
	return propType == 0;
}

uint32_t Toshi::XURXUIStrokeData::GetTimelinePropSize(uint32_t propType)
{
	return 4;
}

bool Toshi::XURXUIStrokeData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "Stroke.StrokeWidth", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Stroke.StrokeColor", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	return false;
}

bool Toshi::XURXUIStrokeData::ValidateTimelineProp(uint32_t param_2)
{
	return param_2 < 2;
}

bool Toshi::XURXUIStrokeData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);

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
			m_strokeWidth = PARSEFLOAT_BIG(a_pData);
			a_pData += 4;
		}
		if ((smth2 & 2) != 0)
		{
			m_strokeColor = PARSEDWORD_BIG(a_pData);
			a_pData += 4;
		}
	}
	return true;
}

// GradientData

bool Toshi::XURXUIGradientData::IsColourPropType(uint32_t propType)
{
	return propType == 2;
}

bool Toshi::XURXUIGradientData::IsFloatPropType(uint32_t propType)
{
	return propType == 3;
}

uint32_t Toshi::XURXUIGradientData::GetTimelinePropSize(uint32_t propType)
{
	return propType == 0 ? 1 : 4;
}

bool Toshi::XURXUIGradientData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	if (TStringManager::String8Compare(param_1, "Fill.Gradient.Radial", -1) == 0)
	{
		param_3 = 0;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.Gradient.NumStops", -1) == 0)
	{
		param_3 = 1;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.Gradient.StopPos", -1) == 0)
	{
		param_3 = 2;
		return true;
	}
	else if (TStringManager::String8Compare(param_1, "Fill.Gradient.StopColor", -1) == 0)
	{
		param_3 = 3;
		return true;
	}
	return false;
}

bool Toshi::XURXUIGradientData::ValidateTimelineProp(uint32_t param_2)
{
	return param_2 < 4;
}

bool Toshi::XURXUIGradientData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);

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
			m_radial = *a_pData++;
		}
		if ((smth2 & 2) != 0)
		{
			TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
			m_numStops = PARSEWORD_BIG(a_pData + 2);
			a_pData += 4;
		}
		if ((smth2 & 4) != 0)
		{
			uint32_t count = *a_pData++;
			m_stops = new uint32_t[count];

			for (size_t i = 0; i < count; i++)
			{
				m_stops[i] = PARSEDWORD_BIG(a_pData);
				a_pData += 4;
			}

		}
		if ((smth2 & 8) != 0)
		{
			uint32_t count = *a_pData++;
			m_stops2 = new uint32_t[count];

			for (size_t i = 0; i < count; i++)
			{
				m_stops2[i] = PARSEDWORD_BIG(a_pData);
				a_pData += 4;
			}
		}
	}
	return true;
}

bool Toshi::XURXUIFigureData::IsColourPropType(uint32_t propType)
{
	return false;
}

bool Toshi::XURXUIFigureData::IsFloatPropType(uint32_t propType)
{
	return false;
}

uint32_t Toshi::XURXUIFigureData::GetTimelinePropSize(uint32_t propType)
{
	return uint32_t();
}

bool Toshi::XURXUIFigureData::TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3)
{
	return false;
}

bool Toshi::XURXUIFigureData::ValidateTimelineProp(uint32_t param_2)
{
	return false;
}

bool Toshi::XURXUIFigureData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	return false;
}
