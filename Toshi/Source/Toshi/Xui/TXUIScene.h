#pragma once
#include "Toshi/Xui/TXUIControl.h"

namespace Toshi
{
	class TXUIScene
	{
	};

	class XURXUISceneData : public XURXUIControlData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUISceneData";

		enum PropType_ : PropType
		{
			PropType_DefaultFocus,
			PropType_TransFrom,
			PropType_TransTo,
			PropType_TransBackFrom,
			PropType_TransBackTo,
			PropType_InterruptTransitions,
			PropType_IgnorePresses,
			PropType_NUMOF,
		};

	public:
		XURXUISceneData();

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);

	private:
		/* 0 */ XUIEPTString m_DefaultFocus;
		/* 1 */ XUIEPTString m_TransFrom;
		/* 2 */ XUIEPTString m_TransTo;
		/* 3 */ XUIEPTString m_TransBackFrom;
		/* 4 */ XUIEPTString m_TransBackTo;
		/* 5 */ XUIEPTUnsigned m_InterruptTransitions;
		/* 6 */ XUIEPTBool m_IgnorePresses;
	};
}


