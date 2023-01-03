#pragma once
#include "Toshi/Xui/TXUIControl.h"

namespace Toshi
{
	class TXUIScene
	{
	};

	class XURXUISceneData : public XURXUIControlData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUISceneData";

		/* 0 */ uint16_t m_defaultFocus;
		/* 1 */ uint16_t m_transFrom;
		/* 2 */ uint16_t m_transTo;
		/* 3 */ uint16_t m_transBackFrom;
		/* 4 */ uint16_t m_transBackTo;
		/* 5 */ uint32_t m_interruptTransitions;
		/* 6 */ bool m_ignorePresses;

	public:
		XURXUISceneData();

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}


