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

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}
