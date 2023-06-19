#pragma once
#include <Toshi/Xui/TXUIResource.h>
#include "TXUIElement.h"

namespace Toshi
{
	class TXUIGroup
	{
	};

	class XURXUIGroupData : public XURXUIElementData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIGroupData";

	public:
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType);
		virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}
