#pragma once

#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUIControl
	{
	};

	class XURXUIControlData : public XURXUIElementData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUIControlData";

		enum PropType_ : PropType
		{
			PropType_ClassOverride,
			PropType_Visual,
			PropType_Enabled,
			PropType_UnfocussedInput,
			PropType_NavLeft,
			PropType_NavRight,
			PropType_NavUp,
			PropType_NavDown,
			PropType_Text,
			PropType_Unknown,
			PropType_ImagePath,
			PropType_NUMOF
		};

	public:
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);

	protected:
		/* 0 */ XUIEPTString m_ClassOverride;
		/* 1 */ XUIEPTString m_Visual;
		/* 2 */ XUIEPTBool m_Enabled;
		/* 3 */ XUIEPTBool m_UnfocussedInput;
		/* 4 */ XUIEPTString m_NavLeft;
		/* 5 */ XUIEPTString m_NavRight;
		/* 6 */ XUIEPTString m_NavUp;
		/* 7 */ XUIEPTString m_NavDown;
		/* 8 */ XUIEPTString m_Text;
		/* 10 */ XUIEPTString m_ImagePath;
	};

}


