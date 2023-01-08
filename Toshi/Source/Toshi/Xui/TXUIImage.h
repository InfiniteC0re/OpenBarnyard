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
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUIImageData";

		enum PropType_ : PropType
		{
			PropType_SizeMode,
			PropType_ImagePath,
			PropType_BrushFlags,
			PropType_NUMOF,
		};

	public:

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);

	private:
		/* 0 */ uint32_t m_SizeMode;
		/* 1 */ uint16_t m_ImagePath;
		/* 2 */ uint32_t m_BrushFlags;
	};
}