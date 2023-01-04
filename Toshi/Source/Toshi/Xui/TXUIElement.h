#pragma once
#include "Toshi/Xui/TXUIResource.h"
#include "Toshi/Xui/TXUINamedFrame.h"

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

		//uint32_t m_uiNumNamedFrames; // 0x10
		//uint32_t m_uiNumChildren; // 0x16
		//void* m_child;
	};

	class XURXUIObjectData
	{
	public:
		XURXUIObjectData()
		{

		}

		virtual ~XURXUIObjectData() = default;

		uint32_t m_index;
		uint16_t unk; //0x12 de blob 0x1C NT08

		XURXUINamedFrameData* m_pNamedFrames; // 0x8 both
		uint16_t m_uiNumNamedFrames; // 0x10 de blob 0x14 NT08

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
			m_positon = 1.0f;
			//m_scale = { -1.0f, -1.0f, -1.0f };
			//m_rotation = { -1.0f, 0.0f, 0.0f };
			m_opacity = 0;
			m_pivot = 0;
			m_show = 0;
			m_disableTimelineRecursion = true;
			m_clipChildren = true;
		}

		~XURXUIElementData() = default;

		/* 0 */ uint16_t m_id;
		/* 1 */ float m_width;
		/* 2 */ float m_height;
		/* 3 */ uint32_t m_positon;
		/* 4 */ uint32_t m_scale;
		/* 5 */ uint32_t m_rotation;
		/* 6 */ float m_opacity;
		/* 7 */ uint32_t m_anchor;
		/* 8 */ uint32_t m_pivot;
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