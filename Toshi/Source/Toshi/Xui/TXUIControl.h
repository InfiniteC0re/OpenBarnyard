#pragma once
#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUIControl :
		public TGenericClassDerived<TXUIControl, TXUIElement, "TXUIControl", TMAKEVERSION(1, 0), TFALSE>
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
		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
		virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
		virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
		virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
		virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

	protected:
		/* 0 */ XUIEPTString m_ClassOverride;
		/* 1 */ XUIEPTString m_Visual;
		/* 4 */ XUIEPTString m_NavLeft;
		/* 5 */ XUIEPTString m_NavRight;
		/* 6 */ XUIEPTString m_NavUp;
		/* 7 */ XUIEPTString m_NavDown;
		/* 8 */ XUIEPTString m_Text;
		/* 10 */ XUIEPTString m_ImagePath;
		/* 2 */ XUIEPTBool m_Enabled;
		/* 3 */ XUIEPTBool m_UnfocussedInput;
	};

}


