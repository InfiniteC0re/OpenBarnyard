#include "ToshiPCH.h"
#include "TXUIElement.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Xui/TXUIListItem.h"

namespace Toshi
{
	bool XURXUIObjectData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		m_index = PARSEWORD_BIG(a_pData);
		a_pData += 2;
		return true;
	}

	void XURXUIObjectData::LoadChildren(TXUIResource& resource, uint8_t*& a_pData)
	{
		TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 8), "Not a Word");
		m_countOfChildren = *(a_pData + 3);
		a_pData += 4;
		m_children = new (TXUI::MemoryBlock()) XURXUIObjectData * [m_countOfChildren];

		for (size_t i = 0; i < m_countOfChildren; i++)
		{
			uint16_t objectIndex = PARSEWORD_BIG(a_pData);
			a_pData += 2;

			m_children[i] = TXUIResource::CreateObjectData(resource, objectIndex);
			m_children[i]->m_index2 = objectIndex;
			uint8_t opcode = *a_pData++;

			m_children[i]->Load(resource, a_pData);

			if ((opcode & 2) != 0)
			{
				m_children[i]->LoadChildren(resource, a_pData);
			}
			if ((opcode & 4) != 0 && m_children[i]->LoadNamedFrames(resource, a_pData) && (opcode & 2) != 0)
			{
				m_children[i]->LoadTimelines(resource, a_pData);
			}
		}
	}

	bool XURXUIObjectData::LoadNamedFrames(TXUIResource& resource, uint8_t*& a_pData)
	{
		TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
		m_uiNumNamedFrames = PARSEWORD_BIG(a_pData + 2);
		TASSERT(m_uiNumNamedFrames < 1000, "Max of named Frames reached");
		a_pData += 4;

		if (m_uiNumNamedFrames != 0)
		{
			m_pNamedFrames = new (TXUI::MemoryBlock()) XURXUINamedFrameData();

			for (size_t i = 0; i < m_uiNumNamedFrames; i++)
			{
				m_pNamedFrames[i].m_unk2 = PARSEWORD_BIG(a_pData);
				a_pData += 2;
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
				m_pNamedFrames[i].m_unk = PARSEWORD_BIG(a_pData + 2);
				a_pData += 4;
				m_pNamedFrames[i].m_unk4 = *a_pData++;
				m_pNamedFrames[i].m_unk3 = PARSEWORD_BIG(a_pData);
				a_pData += 2;
			}
		}
		return true;
	}

	void XURXUIObjectData::LoadTimelines(TXUIResource& resource, uint8_t*& a_pData)
	{
		TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 8));
		m_NumTimelines = *(a_pData + 3);

		if (m_NumTimelines != 0)
		{
			m_timelinesData = new (TXUI::MemoryBlock()) XURXUITimelineData();

			for (size_t i = 0; i < m_NumTimelines; i++)
			{
				//m_timelinesData[i] = *this;
				m_timelinesData[i].Load(resource, a_pData);
			}
		}
	}

	XURXUIObjectData* XURXUIObjectData::FindChildElementData(uint32_t index)
	{
		for (size_t i = 0; i < m_countOfChildren; i++)
		{
			if (((XURXUIElementData*)m_children[m_countOfChildren])->m_Id == index)
			{
				return m_children[m_countOfChildren];
			}
		}
		return TNULL;
	}

	bool XURXUIElementData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIObjectData::Load(resource, a_pData);
		uint8_t smth = *a_pData++;

		if (smth != 0)
		{
			int flag = 0;
			if (m_index != 0) TXUI_READ_WORD(a_pData, flag);

			TXUI_READ_PROP_WORD(a_pData, flag, Id);
			TXUI_READ_PROP_FLOAT(a_pData, flag, Width);
			TXUI_READ_PROP_FLOAT(a_pData, flag, Height);

			if (TXUI_CHECK_READFLAG(flag, PropType_Position))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));

				a_pData += 2;
				TXUI_READ_WORD(a_pData, m_Position);
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_Scale))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
				a_pData += 2;
				TXUI_READ_WORD(a_pData, m_Scale);
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_Rotation))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
				a_pData += 2;
				TXUI_READ_WORD(a_pData, m_Rotation);
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_Opacity))
			{
				TXUI_READ_FLOAT(a_pData, m_Opacity);
				TASSERT(m_Opacity >= 0.0f && m_Opacity <= 1.0f);
				m_Opacity *= 255.0f;
			}

			if (TXUI_CHECK_READFLAG(flag, Flags_AnchorMask))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) <= Flags_AnchorMask);
				m_Anchor |= PARSEWORD_BIG(a_pData + 2) * 8;
				a_pData += 4;
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_Pivot))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) < (1 << 16));
				m_Pivot = PARSEWORD_BIG(a_pData + 2);
				a_pData += 4;
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_Show))
			{
				m_Show &= ~0x1000;
				uint8_t val = *a_pData++;
				if (val != 0) val = 0x1000;
				m_Show |= val;
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_BlendMode))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) <= Flags_BlendModeMask);
				m_BlendMode |= PARSEWORD_BIG(a_pData + 2);
				a_pData += 4;
			}

			if (TXUI_CHECK_READFLAG(flag, PropType_DisableTimelineRecursion))
			{
				m_DisableTimelineRecursion &= ~0x2000;
				uint8_t val = *a_pData++;
				if (val != 0) val = 0x2000;
				m_DisableTimelineRecursion |= val;
			}

			TXUI_READ_PROP_WORD(a_pData, flag, ColorWriteFlags);

			if (TXUI_CHECK_READFLAG(flag, PropType_ClipChildren))
			{
				m_ClipChildren &= ~0x800;
				uint8_t val = *a_pData++;
				if (val != 0) val = 0x800;
				m_ClipChildren |= val;
			}
		}
		return true;
	}


	bool XURXUIElementData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType) // param_3 = position?
	{
		TXUI_TRANSLATE_TIMELINE_PROP(name, Id, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Width, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Height, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Position, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Scale, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Rotation, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Opacity, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Anchor, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Pivot, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, Show, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, BlendMode, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, DisableTimelineRecursion, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, ColorWriteFlags, propType);
		TXUI_TRANSLATE_TIMELINE_PROP(name, ClipChildren, propType);

		return false;
	}

	bool XURXUIElementData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
	{
		TASSERT(a_uiObjectIndex == 0);
		return param_2 < PropType_NUMOF;
	}

	// TXUIElement

	void TXUIElement::Create(TXUIResource& a_rResource, XURXUIObjectData* a_pObjectData, bool a_bool)
	{
		if (GetClass() == TXUICanvas::GetClassStatic())
		{
			//a_rResource.PushID(a_pObjectData->id);
		}
		if (TXUIResource::s_bGenerateUIDs) //&& TStringManager::String16Length(a_pObjectData->id)) != 0
		{
			if (GetClass()->IsA(TXUIListItem::GetClassStatic()))
			{
				m_iUIDCount = TXUIResource::s_iUIDCount;
				TXUIResource::s_iUIDCount++;
			}
		}
	}
}
