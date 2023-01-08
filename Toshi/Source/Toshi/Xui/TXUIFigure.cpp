#include "ToshiPCH.h"
#include "TXUIFigure.h"

namespace Toshi
{
	bool XURXUIFillData::IsColourPropType(uint32_t propType)
	{
		return propType == PropType_FillType;
	}

	bool XURXUIFillData::IsFloatPropType(uint32_t propType)
	{
		return propType == PropType_FillRotation;
	}

	uint32_t XURXUIFillData::GetTimelinePropSize(uint32_t propType)
	{
		return propType != 2 ? 4 : 2;
	}

	bool XURXUIFillData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.FillType", propType, PropType_FillType);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.FillColor", propType, PropType_FillColor);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.TextureFileName", propType, PropType_FillTextureFileName);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Translation", propType, PropType_FillTranslation);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Scale", propType, PropType_FillScale);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Rotation", propType, PropType_FillRotation);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.WrapX", propType, PropType_FillWrapX);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.WrapY", propType, PropType_FillWrapY);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.BrushFlags", propType, PropType_FillBrushFlags);

		return false;
	}

	bool XURXUIFillData::ValidateTimelineProp(uint32_t param_2)
	{
		return param_2 < PropType_NUMOF;
	}

	bool XURXUIFillData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIObjectData::Load(resource, a_pData);
		uint8_t smth = *a_pData++;

		// TODO:!!!!! NOT FINISHED !!!!!!

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_WORD(a_pData, flag);

			if (TXUI_CHECK_READFLAG(flag, PropType_FillType))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
				m_FillType = PARSEWORD_BIG(a_pData + 2);
				a_pData += 4;
			}

			TXUI_READ_PROP_DWORD(a_pData, flag, FillColor);
			TXUI_READ_PROP_WORD(a_pData, flag, FillTextureFileName);

			if (TXUI_CHECK_READFLAG(flag, PropType_Unknown))
			{
				TTODO("XURXUIGradientData");
				// TODO XURXUIGradientData
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_FillTranslation))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 15)); // 15 is probably a typo they did?
				m_FillTranslation = PARSEDWORD_BIG(a_pData + 2);
				a_pData += 4;
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_FillScale))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 15)); // 15 is probably a typo they did?
				m_FillScale = PARSEDWORD_BIG(a_pData + 2);
				a_pData += 4;
			}

			TXUI_READ_PROP_FLOAT(a_pData, flag, FillRotation);
			TXUI_READ_PROP_DWORD(a_pData, flag, FillWrapX);
			TXUI_READ_PROP_DWORD(a_pData, flag, FillWrapY);

			if (TXUI_CHECK_READFLAG(flag, PropType_FillBrushFlags))
			{
				// there's nothing here in globs and de blob
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_Unknown2))
			{
				// there's nothing here in globs and de blob
				// m_FillRotation = *a_pData++;
			}
		}
		return true;
	}

	// StrokeData

	bool XURXUIStrokeData::IsColourPropType(uint32_t propType)
	{
		return propType == 1;
	}

	bool XURXUIStrokeData::IsFloatPropType(uint32_t propType)
	{
		return propType == 0;
	}

	uint32_t XURXUIStrokeData::GetTimelinePropSize(uint32_t propType)
	{
		return 4;
	}

	bool XURXUIStrokeData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Stroke.StrokeWidth", propType, PropType_StrokeWidth);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Stroke.StrokeColor", propType, PropType_StrokeColor);

		return false;
	}

	bool XURXUIStrokeData::ValidateTimelineProp(uint32_t param_2)
	{
		return param_2 < 2;
	}

	bool XURXUIStrokeData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIObjectData::Load(resource, a_pData);

		uint8_t smth = *a_pData++;

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_BYTE(a_pData, flag);

			TXUI_READ_PROP_FLOAT(a_pData, flag, StrokeWidth);
			TXUI_READ_PROP_DWORD(a_pData, flag, StrokeColor);
		}
		return true;
	}

	// GradientData

	bool XURXUIGradientData::IsColourPropType(uint32_t propType)
	{
		return propType == 2;
	}

	bool XURXUIGradientData::IsFloatPropType(uint32_t propType)
	{
		return propType == 3;
	}

	uint32_t XURXUIGradientData::GetTimelinePropSize(uint32_t propType)
	{
		return propType == 0 ? 1 : 4;
	}

	bool XURXUIGradientData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.Radial", propType, PropType_FillGradientRadial);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.NumStops", propType, PropType_FillGradientNumStops);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.StopPos", propType, PropType_FillGradientStopPos);
		TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.StopColor", propType, PropType_FillGradientStopColor);

		return false;
	}

	bool XURXUIGradientData::ValidateTimelineProp(uint32_t param_2)
	{
		return param_2 < PropType_NUMOF;
	}

	bool XURXUIGradientData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIObjectData::Load(resource, a_pData);

		uint8_t smth = *a_pData++;

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_BYTE(a_pData, flag);

			TXUI_READ_PROP_BYTE_MANUAL(a_pData, flag, PropType_FillGradientRadial, m_Radial);
			
			if (TXUI_CHECK_READFLAG(flag, PropType_FillGradientNumStops))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
				m_NumStops = PARSEWORD_BIG(a_pData + 2);
				a_pData += 4;
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_FillGradientStopPos))
			{
				uint32_t count = 0;
				TXUI_READ_BYTE(a_pData, count);
				m_Stops = new uint32_t[count];

				for (size_t i = 0; i < count; i++)
				{
					TXUI_READ_DWORD(a_pData, m_Stops[i]);
				}

			}
			if (TXUI_CHECK_READFLAG(flag, PropType_FillGradientStopColor))
			{
				uint32_t count = 0;
				TXUI_READ_BYTE(a_pData, count);
				m_Stops2 = new uint32_t[count];

				for (size_t i = 0; i < count; i++)
				{
					TXUI_READ_DWORD(a_pData, m_Stops2[i]);
				}
			}
		}
		return true;
	}

	bool XURXUIFigureData::IsColourPropType(uint32_t propType)
	{
		return false;
	}

	bool XURXUIFigureData::IsFloatPropType(uint32_t propType)
	{
		return false;
	}

	uint32_t XURXUIFigureData::GetTimelinePropSize(uint32_t propType)
	{
		return uint32_t();
	}

	bool XURXUIFigureData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		return false;
	}

	bool XURXUIFigureData::ValidateTimelineProp(uint32_t param_2)
	{
		return false;
	}

	bool XURXUIFigureData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		return false;
	}

}