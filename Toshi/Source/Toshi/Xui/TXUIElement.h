#pragma once
#include "Toshi/Xui/TXUIResource.h"
#include "Toshi/Xui/TXUINamedFrame.h"
#include "Toshi/Xui/TXUITimeline.h"

namespace Toshi
{
	enum Flags : uint8_t
	{
		Flags_BlendModeMask = BITFIELD(3),
		Flags_AnchorMask = BITFIELD(6)
	};

	class TXUIElement :
		public TGenericClassDerived<TXUIElement, TObject, "TXUIElement", TMAKEVERSION(1, 0), false>
	{

	};

	class XURXUIObjectData
	{
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

		uint16_t m_index; //0x12 de blob 0x1C NT08
		uint32_t m_index2; // 0x14 de blob 1E NT08

		XURXUINamedFrameData* m_pNamedFrames; // 0x8 both
		uint16_t m_uiNumNamedFrames; // 0x10 de blob 0x14 NT08

		XURXUITimelineData* m_timelinesData; // 0xC NT08
		uint8_t m_NumTimelines;

		uint32_t m_countOfChildren; // 0x16 de blob 0x10 NT08
		XURXUIObjectData** m_children; // 0x4 both

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
		virtual bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) { return false; };
		virtual bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) { return false; };
		virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) { return 0; };
		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3) { return false; };
		virtual bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2) { return false; };

		void LoadChildren(TXUIResource& resource, uint8_t*& a_pData);
		bool LoadNamedFrames(TXUIResource& resource, uint8_t*& a_pData);
		void LoadTimelines(TXUIResource& resource, uint8_t*& a_pData);
		XURXUIObjectData* FindChildElementData(uint32_t index);
	};

	class XURXUIElementData : public XURXUIObjectData
	{
	public:
		XURXUIElementData() : XURXUIObjectData()
		{
			m_width = 60.0f;
			m_height = 30.0f;
			m_opacity = 1.0f;
			m_positon = -1;
			m_scale = -1;
			m_rotation = -1;
			m_pivot = -1;
			m_anchor = 0;
			m_blendMode = 0;
			m_colorWriteFlags = 0;
			m_clipChildren = false;
			m_id = 0;
			m_show = true;
			m_disableTimelineRecursion = false;
		}

		~XURXUIElementData() = default;

		/* 0 */ uint16_t m_id;
		/* 1 */ float m_width;
		/* 2 */ float m_height;
		/* 3 */ int32_t m_positon;
		/* 4 */ int32_t m_scale;
		/* 5 */ int32_t m_rotation;
		/* 6 */ float m_opacity;
		/* 7 */ uint32_t m_anchor;
		/* 8 */ int32_t m_pivot;
		/* 9 */ bool m_show; 
		/* 10 */ uint32_t m_blendMode;
		/* 11 */ bool m_disableTimelineRecursion;
		/* 13 */ uint32_t m_colorWriteFlags;
		/* 14 */ bool m_clipChildren;

		static constexpr const char* sm_sTypeInfo = "XURXUIElementData";

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		bool Load(TXUIResource& resource, uint8_t*& a_pData);

		bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);

		bool ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2);

		bool IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) {
			return propType != 1 && propType != 2 && propType != 6 ? false : true;
		}

		// No it's not
		bool IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) {
			return false;
		}

		uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) {

			if (propType != 11 && propType != 9 && propType != 14)
			{
				if (propType != 0) return 4;
				return 2;
			}
			return 1;

		}
	};
}