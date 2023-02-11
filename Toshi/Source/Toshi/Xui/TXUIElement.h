#pragma once
#include "Toshi/Xui/TXUIResource.h"
#include "Toshi/Xui/TXUITimeline.h"
#include "Toshi2/T2GUI/T2GUIElement.h"

#define TXUI_TRANSLATE_TIMELINE_PROP(propName1, propName2, typeOut) \
if (TStringManager::String8Compare(propName1, #propName2, -1) == 0) \
{ \
	typeOut = PropType_##propName2; \
	return true; \
}

#define TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(propName1, propName2, typeOut, flag) \
if (TStringManager::String8Compare(propName1, propName2, -1) == 0) \
{ \
	typeOut = flag; \
	return true; \
}

#define TXUI_PROPTYPE_TO_READFLAG(propType) (1 << propType)
#define TXUI_CHECK_READFLAG(flag, propType) (flag & TXUI_PROPTYPE_TO_READFLAG(propType))

#define TXUI_READ_BYTE(buffer, out) out = *buffer; buffer += 1;
#define TXUI_READ_WORD(buffer, out) out = PARSEWORD_BIG(buffer); buffer += 2;
#define TXUI_READ_DWORD(buffer, out) out = PARSEDWORD_BIG(buffer); buffer += 4;
#define TXUI_READ_FLOAT(buffer, out) out = PARSEFLOAT_BIG(buffer); buffer += 4;
#define TXUI_READ_COLOUR(buffer, out) TASSERT(PARSEDWORD_BIG(buffer) < (1 << 16)); out = PARSEWORD_BIG(buffer + 2); buffer += 4;

#define TXUI_READ_PROP_BYTE(buffer, flag, propType) if (TXUI_CHECK_READFLAG(flag, PropType_##propType)) { TXUI_READ_BYTE(buffer, m_##propType); }
#define TXUI_READ_PROP_WORD(buffer, flag, propType) if (TXUI_CHECK_READFLAG(flag, PropType_##propType)) { TXUI_READ_WORD(buffer, m_##propType); }
#define TXUI_READ_PROP_DWORD(buffer, flag, propType) if (TXUI_CHECK_READFLAG(flag, PropType_##propType)) { TXUI_READ_DWORD(buffer, m_##propType); }
#define TXUI_READ_PROP_FLOAT(buffer, flag, propType) if (TXUI_CHECK_READFLAG(flag, PropType_##propType)) { TXUI_READ_FLOAT(buffer, m_##propType); }

#define TXUI_READ_PROP_BYTE_MANUAL(buffer, flag, propType, out) if (TXUI_CHECK_READFLAG(flag, propType)) { TXUI_READ_BYTE(buffer, out); }
#define TXUI_READ_PROP_WORD_MANUAL(buffer, flag, propType, out) if (TXUI_CHECK_READFLAG(flag, propType)) { TXUI_READ_WORD(buffer, out); }
#define TXUI_READ_PROP_DWORD_MANUAL(buffer, flag, propType, out) if (TXUI_CHECK_READFLAG(flag, propType)) { TXUI_READ_DWORD(buffer, out); }
#define TXUI_READ_PROP_FLOAT_MANUAL(buffer, flag, propType, out) if (TXUI_CHECK_READFLAG(flag, propType)) { TXUI_READ_FLOAT(buffer, out); }

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

	public:
		XURXUIObjectData()
		{
			m_index = 0;
			m_index2 = 0;
			m_uiNumNamedFrames = 0;
			m_pNamedFrames = TNULL;
			m_timelinesData = TNULL;
			m_children = TNULL;
			m_countOfChildren = 0;
		}

		virtual ~XURXUIObjectData() = default;

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, PropType propType) { return false; };
		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, PropType propType) { return false; };
		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, PropType propType) { return 0; };
		virtual bool TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType) { return false; };
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, PropType propType) { return false; };

		void LoadChildren(TXUIResource& resource, uint8_t*& a_pData);
		bool LoadNamedFrames(TXUIResource& resource, uint8_t*& a_pData);
		void LoadTimelines(TXUIResource& resource, uint8_t*& a_pData);
		XURXUIObjectData* FindChildElementData(uint32_t index);

	public:

		uint16_t m_id; // 0x28 de blob

		uint16_t m_index; //0x12 de blob 0x1C NT08
		uint32_t m_index2; // 0x14 de blob 1E NT08

		XURXUINamedFrameData* m_pNamedFrames; // 0x8 both
		uint16_t m_uiNumNamedFrames; // 0x10 de blob 0x14 NT08

		XURXUITimelineData* m_timelinesData; // 0xC NT08
		uint8_t m_NumTimelines; // 0x20?? NT08

		uint32_t m_countOfChildren; // 0x16 de blob 0x10 NT08
		XURXUIObjectData** m_children; // 0x4 both

		/* 0 */ uint16_t m_Id;
		/* 1 */ uint16_t m_Width;
		/* 2 */ uint16_t m_Height;
		/* 3 */ int16_t m_Position;
		/* 4 */ int16_t m_Scale;
		/* 5 */ int16_t m_Rotation;
		/* 6 */ float m_Opacity;
		/* 7 */ uint32_t m_Anchor;
		/* 8 */ int32_t m_Pivot;
		/* 9 */ bool m_Show;
		/* 10 */ uint32_t m_BlendMode;
		/* 11 */ bool m_DisableTimelineRecursion;
		/* 13 */ uint32_t m_ColorWriteFlags;
		/* 14 */ bool m_ClipChildren;
	};

	class XURXUIElementData : public XURXUIObjectData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUIElementData";

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
			PropType_Unknown,
			PropType_ColorWriteFlags,
			PropType_ClipChildren,
			PropType_NUMOF,
		};

		friend XURXUIObjectData;

	public:
		XURXUIElementData()
		{
			m_Width = T2GUIElement::PackFloat(60.0f);
			m_Height = T2GUIElement::PackFloat(30.0f);
			m_Opacity = 1.0f;
			m_Position = -1;
			m_Scale = -1;
			m_Rotation = -1;
			m_Pivot = -1;
			m_Anchor = 0;
			m_BlendMode = 0;
			m_ColorWriteFlags = 0;
			m_ClipChildren = false;
			m_Id = 0;
			m_Show = true;
			m_DisableTimelineRecursion = false;
		}

		~XURXUIElementData() = default;

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		bool Load(TXUIResource& resource, uint8_t*& a_pData);

		bool TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType);

		bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
		{
			return propType != 1 && propType != 2 && propType != 6 ? false : true;
		}

		// No it's not
		bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
		{
			return false;
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

	
	};

	class TXUIElement :
		public TGenericClassDerived<TXUIElement, TObject, "TXUIElement", TMAKEVERSION(1, 0), false>,
		public T2GUIElement
	{
	public:
		TXUIElement();

		virtual bool SkipRender();
		virtual void SetHeight(float height);
		virtual void SetWidth(float width);

		bool Create(TXUIResource& a_rResource, XURXUIObjectData* a_pObjectData, bool hasChildren);
		void CreateChildren(TXUIResource& a_rResource, XURXUIObjectData* a_pObjectData);
		void UpdateAnchoring(const TVector2& vec);
		bool IsVisible();
	
	private:

		XURXUIObjectData* m_pObjectData; // 0x40 de blob
		wchar_t* m_objectID; // 0x44

		T2GUITransform::Rotation m_vPosition;
		T2GUITransform::Rotation m_vRotation;
		T2GUITransform::Rotation m_vScale;

		int m_iUIDCount; // 0xB4 globs
	};
}