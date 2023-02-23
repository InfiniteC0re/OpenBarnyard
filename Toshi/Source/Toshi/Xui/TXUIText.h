#pragma once

#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUIText
	{

	};

	class XURXUITextData : public XURXUIElementData
	{
	public:
		static constexpr uint32_t sm_uiDefaultFontSize = 14;
		static constexpr const char* sm_sTypeInfo = "XURXUITextData";

		enum PropType_ : PropType
		{
			PropType_Text,
			PropType_TextColor,
			PropType_DropShadowColor,
			PropType_PointSize,
			PropType_Font,
			PropType_TextStyle,
			PropType_LineSpacingAdjust,
			PropType_NUMOF,
		};

	public:
		XURXUITextData();

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);

	private:
		/* 0 */ XUIEPTString m_Text;
		/* 1 */ XUIEPTColor m_TextColor;
		/* 2 */ XUIEPTColor m_DropShadowColor;
		/* 3 */ XUIEPTFloat m_PointSize;
		/* 4 */ XUIEPTString m_Font;
		/* 5 */ XUIEPTUnsigned m_TextStyle;
		/* 6 */ XUIEPTInteger m_LineSpacingAdjust;
	};
}

