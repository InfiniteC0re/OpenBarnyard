#pragma once

#include "TXUIElement.h"
#include "Toshi/T2gui/T2GUIElement.h"

namespace Toshi
{
	class TXUIImage : TXUIElement
	{
	};

	class XURXUIImageData : XURXUIElementData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIImageData";

		/* 0 */ uint32_t m_sizeMode;
		/* 1 */ uint16_t m_imagePath;
		/* 2 */ uint32_t m_brushFlags;


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