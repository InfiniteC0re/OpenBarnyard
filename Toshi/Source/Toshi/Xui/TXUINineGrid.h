#pragma once
#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class XURXUINineGridData : public XURXUIElementData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUINineGridData";

		enum PropType_ : PropType
		{
			PropType_TextureFileName,
			PropType_LeftOffset,
			PropType_TopOffset,
			PropType_RightOffset,
			PropType_BottomOffset,
			PropType_NoCenter,
			PropType_NUMOF
		};

	public:
		virtual const char* GetTypeInfo() const
		{
			return sm_sTypeInfo;
		}

		virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);
		virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);
		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);
		virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType);
		virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);

	protected:
		XUIEPTString m_TextureFileName;
		XUIEPTUnsigned m_LeftOffset;
		XUIEPTUnsigned m_TopOffset;
		XUIEPTUnsigned m_RightOffset;
		XUIEPTUnsigned m_BottomOffset;
		XUIEPTBool m_NoCenter;
	};
}