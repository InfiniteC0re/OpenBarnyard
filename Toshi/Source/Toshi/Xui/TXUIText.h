#pragma once

#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUIText
	{

	};

	class XURXUITextData : public XURXUIElementData
	{
		static constexpr uint32_t sm_uiDefaultFontSize = 14;
		static constexpr const char* sm_sTypeInfo = "XURXUITextData";

		uint16_t m_text;
		uint32_t m_textColor;
		uint32_t m_dropShadowColor;
		uint32_t m_pointSize;
		uint16_t m_font;
		uint32_t m_textStyle;
		uint32_t m_lineSpacingAdjust;

	public:
		XURXUITextData();

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}

