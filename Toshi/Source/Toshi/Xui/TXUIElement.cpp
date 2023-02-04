#include "ToshiPCH.h"
#include "TXUIElement.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Xui/TXUIListItem.h"
#include "TXUIReader.h"

namespace Toshi
{
	bool XURXUIObjectData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		TXUIReader reader(a_pData);
		m_index = reader.ReadUInt16();
		return true;
	}

	void XURXUIObjectData::LoadChildren(TXUIResource& resource, uint8_t*& a_pData)
	{
		TXUIReader reader(a_pData);
		m_countOfChildren = reader.ReadUInt8From32();
		m_children = new (TXUI::MemoryBlock()) XURXUIObjectData * [m_countOfChildren];
		TASSERT(m_children != TNULL);

		for (size_t i = 0; i < m_countOfChildren; i++)
		{
			uint16_t objectIndex = reader.ReadUInt16();

			m_children[i] = TXUIResource::CreateObjectData(resource, objectIndex);
			m_children[i]->m_index2 = objectIndex;
			uint8_t opcode = reader.ReadUInt8();

			m_children[i]->Load(resource, a_pData);

			if (opcode & 2)
			{
				m_children[i]->LoadChildren(resource, a_pData);
			}
			if (opcode & 2 && opcode & 4 && m_children[i]->LoadNamedFrames(resource, a_pData))
			{
				m_children[i]->LoadTimelines(resource, a_pData);
			}
		}
	}

	bool XURXUIObjectData::LoadNamedFrames(TXUIResource& resource, uint8_t*& a_pData)
	{
		TXUIReader reader(a_pData);
		m_uiNumNamedFrames = reader.ReadUInt16From32();
		TASSERT(m_uiNumNamedFrames < 1000, "Max of named Frames reached");

		if (m_uiNumNamedFrames != 0)
		{
			m_pNamedFrames = new (TXUI::MemoryBlock()) XURXUINamedFrameData();
			TASSERT(m_pNamedFrames != TNULL);

			for (size_t i = 0; i < m_uiNumNamedFrames; i++)
			{
				m_pNamedFrames[i].m_unk2 = reader.ReadUInt16();
				m_pNamedFrames[i].m_unk = reader.ReadUInt16From32();
				m_pNamedFrames[i].m_unk4 = reader.ReadUInt8();
				m_pNamedFrames[i].m_unk3 = reader.ReadUInt16();
			}
		}
		return true;
	}

	void XURXUIObjectData::LoadTimelines(TXUIResource& resource, uint8_t*& a_pData)
	{
		TXUIReader reader(a_pData);
		m_NumTimelines = reader.ReadUInt8From32();

		if (m_NumTimelines != 0)
		{
			m_timelinesData = new (TXUI::MemoryBlock()) XURXUITimelineData();
			TASSERT(m_timelinesData != TNULL);

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
	
		if (*a_pData++ != 0)
		{
			TXUIReader reader(a_pData);
			if (m_index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

			reader.ReadProp16(PropType_Id, m_Id);
			reader.ReadPropFloat(PropType_Width, m_Width);
			reader.ReadPropFloat(PropType_Height, m_Height);
			reader.ReadProp16From32(PropType_Position, m_Position);
			reader.ReadProp16From32(PropType_Scale, m_Scale);
			reader.ReadProp16From32(PropType_Rotation, m_Rotation);
			
			if (reader.ReadPropFloat(PropType_Opacity, m_Opacity))
			{
				TASSERT(m_Opacity >= 0.0f && m_Opacity <= 1.0f);
				m_Opacity *= 255.0f;
			}

			if (reader.ShouldReadThisProp(PropType_Anchor))
			{
				TASSERT(PARSEDWORD_BIG(a_pData) <= Flags_AnchorMask);
				m_Anchor |= reader.ReadUInt16From32() * 8;
			}

			reader.ReadProp16From32(PropType_Pivot, m_Pivot);

			if (reader.ShouldReadThisProp(PropType_Show))
			{
				m_Show &= ~0x1000;
				uint8_t val = reader.ReadUInt8();
				if (val != 0) val = 0x1000;
				m_Show |= val;
			}

			uint32_t blend;
			if (reader.ReadProp16From32(PropType_BlendMode, blend))
			{
				m_BlendMode |= blend;
			}

			if (reader.ShouldReadThisProp(PropType_DisableTimelineRecursion))
			{
				m_DisableTimelineRecursion &= ~0x2000;
				uint8_t val = reader.ReadUInt8();
				if (val != 0) val = 0x2000;
				m_DisableTimelineRecursion |= val;
			}

			reader.ReadProp32(PropType_ColorWriteFlags, m_ColorWriteFlags);
			
			if (reader.ShouldReadThisProp(PropType_ClipChildren))
			{
				m_ClipChildren &= ~0x800;
				uint8_t val = reader.ReadUInt8();
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

	TXUIElement::TXUIElement()
	{
		m_pObjectData = TNULL;
		m_Flags1 |= FLAGS_XUIELEMENT;
		m_vRotation = T2GUITransform::Rotation(0,0);
		m_vPosition = T2GUITransform::Rotation(0, 0);
		m_vScale = T2GUITransform::Rotation(0, 0);
	}

	bool TXUIElement::SkipRender()
	{
		TIMPLEMENT();
		return false;
	}

	void TXUIElement::SetHeight(float height)
	{
		TVector2 currentDimension;
		GetDimensions(currentDimension.x, currentDimension.y);

		T2GUIElement::SetHeight(height);

		TVector2 newDimension;
		GetDimensions(newDimension.x, newDimension.y);

		if (!currentDimension.IsEqual(newDimension))
		{
			TVector2 combined = currentDimension - newDimension;
			UpdateAnchoring(combined);
		}
	}

	void TXUIElement::SetWidth(float width)
	{
		TVector2 currentDimension;
		GetDimensions(currentDimension.x, currentDimension.y);

		T2GUIElement::SetWidth(width);

		TVector2 newDimension;
		GetDimensions(newDimension.x, newDimension.y);

		if (!currentDimension.IsEqual(newDimension))
		{
			TVector2 combined = currentDimension - newDimension;
			UpdateAnchoring(combined);
		}
	}

	void TXUIElement::UpdateAnchoring(const TVector2& vec)
	{
		TIMPLEMENT();
	}

	bool TXUIElement::IsVisible()
	{
		if (HASFLAG(m_Flags1 & FLAGS_VISIBLE) && HASFLAG(m_Flags1 & (uint8_t)T2GUIElement::s_uiGlobalVisMask))
		{
			return TFALSE;
		}

		return TTRUE;
	}

	bool TXUIElement::Create(TXUIResource& a_rResource, XURXUIObjectData* a_pObjectData, bool hasChildren)
	{
		m_pObjectData = a_pObjectData;
		if (GetClass() == TGetClass(TXUICanvas))
		{
			a_rResource.PushID(a_rResource.GetString(a_pObjectData->m_id));
		}
		m_objectID = (wchar_t*)a_rResource.GetString(a_pObjectData->m_id);
		if (TXUIResource::s_bGenerateUIDs && TStringManager::String16Length(m_objectID) != 0)
		{
			if (GetClass()->IsA(TGetClass(TXUIListItem)))
			{
				m_iUIDCount = TXUIResource::s_iUIDCount;
				TXUIResource::s_iUIDCount++;
			}
		}
		m_Width = a_pObjectData->m_Width;
		m_Height = a_pObjectData->m_Height;

		if (a_pObjectData->m_Position != -1)
		{
			TVector4* pos = a_rResource.GetVector(a_pObjectData->m_Position);
			
			m_vPosition.SetX(pos->x);
			m_vPosition.SetY(pos->x);
		}
		if (a_pObjectData->m_Rotation != -1)
		{
			//m_Rotation = a_rResource.GetQuat(a_pObjectData->m_Scale);
		}
		if (a_pObjectData->m_Scale != -1)
		{
			TVector4* scale = a_rResource.GetVector(a_pObjectData->m_Scale);
			m_vScale.SetX(scale->x);
			m_vScale.SetY(scale->x);
		}

		SetVisible(a_pObjectData->m_Show);

		if (hasChildren)
		{
			CreateChildren(a_rResource, a_pObjectData);
		}

		if (GetClass() == TGetClass(TXUICanvas))
		{
			a_rResource.PopID();
		}
		return true;
	}

	void TXUIElement::CreateChildren(TXUIResource& a_rResource, XURXUIObjectData* a_pObjectData)
	{
		for (size_t i = 0; i < a_pObjectData->m_countOfChildren; i++)
		{
			auto child = a_pObjectData->m_children[i];
		}
		TIMPLEMENT();
	}
	
}
