#pragma once
#include "Toshi/Xui/XUI.h"
#include "Toshi/Xui/TXUIResource.h"
#include "Toshi/Xui/TXUITimeline.h"
#include "Toshi2/T2GUI/T2GUIElement.h"

#define TXUI_TRANSLATE_TIMELINE_PROP(propName1, propName2, typeOut) \
if (TStringManager::String8Compare(propName1, #propName2, -1) == 0) \
{ \
	typeOut = PropType_##propName2; \
	return TTRUE; \
}

#define TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(propName1, propName2, typeOut, flag) \
if (TStringManager::String8Compare(propName1, propName2, -1) == 0) \
{ \
	typeOut = flag; \
	return TTRUE; \
}

#define TXUI_PROPTYPE_TO_READFLAG(propType) (1 << propType)
#define TXUI_CHECK_READFLAG(flag, propType) (flag & TXUI_PROPTYPE_TO_READFLAG(propType))

namespace Toshi
{
	enum Flags : uint8_t
	{
		Flags_BlendModeMask = BITFIELD(3),
		Flags_AnchorMask = BITFIELD(6)
	};

	class TXUIElement;

	class XURXUIObjectData
	{
	public:
		typedef uint32_t PropType;
		enum PropType_ : PropType
		{
			PropType_Id,
			PropType_Width,
			PropType_Height,
			PropType_Position,
			PropType_Scale,
			PropType_Rotation,
			PropType_Opacity,
			PropType_Anchor,
			PropType_Pivot,
			PropType_Show,
			PropType_BlendMode,
			PropType_DisableTimelineRecursion,
			PropType_DesignTime,
			PropType_ColorWriteFlags,
			PropType_ClipChildren,
			PropType_NUMOF,
		};

	public:
		XURXUIObjectData()
		{
			m_Index = 0;
			m_Index2 = 0;
			m_NumNamedFrames = 0;
			m_pNamedFrames = TNULL;
			m_TimelinesData = TNULL;
			m_Children = TNULL;
			m_NumChildren = 0;
		}

		virtual ~XURXUIObjectData() = default;

		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);
		virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, PropType propType) { return TFALSE; };
		virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, PropType propType) { return TFALSE; };
		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, PropType propType) { return 0; };
		virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType) { return TFALSE; };
		virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, PropType propType) { return TFALSE; };

		void LoadChildren(TXUIResource& resource, uint8_t*& a_pData);
		TBOOL LoadNamedFrames(TXUIResource& resource, uint8_t*& a_pData);
		void LoadTimelines(TXUIResource& resource, uint8_t*& a_pData);
		class XURXUIElementData* FindChildElementData(uint32_t index);

	public:
		XURXUIObjectData** m_Children;        // 0x4 both
		XURXUINamedFrameData* m_pNamedFrames; // 0x8 both
		XURXUITimelineData* m_TimelinesData;  // 
		XUIEPTUShort32 m_NumNamedFrames;      // 0x10 de blob 0x14 NT08
		uint16_t m_Index;                     // 0x12 de blob 0x1C NT08
		uint32_t m_Index2;                    // 0x14 de blob 1E NT08
		uint32_t m_NumChildren;               // 0x16 de blob 0x10 NT08
		uint8_t m_NumTimelines;               // 
	};

	class XURXUIElementData : public XURXUIObjectData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUIElementData";

		friend XURXUIObjectData;

	public:
		XURXUIElementData()
		{
			m_Width = T2GUIElement::PackFloat(60.0f);
			m_Height = T2GUIElement::PackFloat(30.0f);
			m_Opacity = 255;
			m_Position = -1;
			m_Scale = -1;
			m_Rotation = -1;
			m_Pivot = -1;
			m_Flags = 0x1000;
			m_Id = 0;
		}

		~XURXUIElementData() = default;

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);

		TBOOL TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType);

		TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
		{
			return propType != 1 && propType != 2 && propType != 6 ? TFALSE : TTRUE;
		}

		// No it's not
		TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
		{
			return TFALSE;
		}

		uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
		{
			if (propType != 11 && propType != 9 && propType != 14)
			{
				if (propType != 0) return 4;
				return 2;
			}
			return 1;
		}

		XUIEPTString GetId() const
		{
			return m_Id;
		}

		T2GUIElement::Float GetWidth() const
		{
			return m_Width;
		}

		T2GUIElement::Float GetHeight() const
		{
			return m_Height;
		}

		XUIEPTVector GetPosition() const
		{
			return m_Position;
		}

		XUIEPTVector GetScale() const
		{
			return m_Scale;
		}

		XUIEPTQuaternion GetRotation() const
		{
			return m_Rotation;
		}

		TBOOL IsVisible() const
		{
			return m_Flags & 0x1000;
		}

	protected:
		T2GUIElement::Float m_Width;
		T2GUIElement::Float m_Height;
		XUIEPTVector m_Position;
		XUIEPTVector m_Scale;
		XUIEPTQuaternion m_Rotation;
		XUIEPTVector m_Pivot;
		XUIEPTString m_Id;
		uint32_t m_Flags;
		uint8_t m_Opacity;
	};

	class TXUIElement :
		public TGenericClassDerived<TXUIElement, TObject, "TXUIElement", TMAKEVERSION(1, 0), TFALSE>,
		public T2GUIElement
	{
	public:
		TXUIElement();

		virtual TBOOL SkipRender();
		virtual void SetHeight(float height);
		virtual void SetWidth(float width);

		TBOOL Create(TXUIResource& a_rResource, XURXUIElementData* a_pElementData, TBOOL hasChildren);
		void CreateChildren(TXUIResource& a_rResource, XURXUIElementData* a_pElementData);
		void UpdateAnchoring(const TVector2& vec);
		TBOOL IsVisible();
	
	private:

		XURXUIObjectData* m_pObjectData; // 0x40 de blob
		const wchar_t* m_objectID; // 0x44

		T2GUITransform::Rotation m_vPosition;
		T2GUITransform::Rotation m_vRotation;
		T2GUITransform::Rotation m_vScale;

		int m_iUIDCount; // 0xB4 globs
	};
}