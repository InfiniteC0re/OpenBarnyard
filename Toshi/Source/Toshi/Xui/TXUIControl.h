#pragma once

#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUIControl
	{
	};

	class XURXUIControlData : public XURXUIElementData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIControlData";

		/* 0 */ uint16_t m_classOverride;
		/* 1 */ uint16_t m_visual;
		/* 2 */ bool m_enabled;
		/* 3 */ bool m_unfocussedInput;
		/* 4 */ uint16_t m_navLeft;
		/* 5 */ uint16_t m_navRight;
		/* 6 */ uint16_t m_navUp;
		/* 7 */ uint16_t m_navDown;
		/* 8 */ uint16_t m_text;
		/* 10 */ uint16_t m_imagePath;

	public:
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};

}


