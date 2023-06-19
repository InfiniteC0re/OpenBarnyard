#pragma once
#include "TXUIControl.h"

namespace Toshi {

	class TXUISlider
	{
	};

	class XURXUISliderBarData : public XURXUIControlData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUISliderBarData";

		enum PropType_ : PropType
		{
			PropType_RangeMin,
			PropType_RangeMax,
			PropType_Value,
			PropType_Step,
			PropType_Vertical,
			PropType_AccelInc,
			PropType_AccelTime,
			PropType_NUMOF,
		};

	public:
		XURXUISliderBarData();

		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
		virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
		virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
		virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
		virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

	private:
		XUIEPTInteger m_iRangeMin;
		XUIEPTInteger m_iRangeMax;
		XUIEPTInteger m_iValue;
		XUIEPTInteger m_iStep;
		XUIEPTInteger m_iAccelInc;
		XUIEPTInteger m_iAccelTime;
		XUIEPTBool m_bVertical;
	};

}
