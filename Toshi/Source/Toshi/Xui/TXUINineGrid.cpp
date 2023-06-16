#include "ToshiPCH.h"
#include "TXUINineGrid.h"
#include "XURReader.h"

namespace Toshi
{
	TBOOL XURXUINineGridData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		return TFALSE;
	}

	TBOOL XURXUINineGridData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		return TFALSE;
	}

	uint32_t XURXUINineGridData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		return 0;
	}

	TBOOL XURXUINineGridData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
	{
		return TFALSE;
	}

	TBOOL XURXUINineGridData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		return TFALSE;
	}

	TBOOL XURXUINineGridData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIElementData::Load(resource, a_pData);

		if (*a_pData++ != 0)
		{
			XURReader reader(a_pData);
			if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProperty<XUI_EPT_STRING>(PropType_TextureFileName, m_TextureFileName);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_LeftOffset, m_LeftOffset);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TopOffset, m_TopOffset);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_RightOffset, m_RightOffset);
			reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_BottomOffset, m_BottomOffset);
			reader.ReadProperty<XUI_EPT_BOOL>(PropType_NoCenter, m_NoCenter);
		}

		return TFALSE;
	}
}
