#pragma once
#include "TXUIElement.h"
#include "Toshi2/T2GUI/T2GUIElement.h"

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

		virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType);
		virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);

	private:
		/* 0 */ XUIEPTUnsigned m_SizeMode;
		/* 1 */ XUIEPTString m_ImagePath;
		/* 2 */ XUIEPTUnsigned m_BrushFlags;
	};
}