#pragma once
#include <Toshi/Xui/TXUIResource.h>
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
		
		uint32_t m_uiNumNamedFrames; // 0x10
		uint32_t m_uiNumChildren; // 0x16
		void* m_child;
	};

	class XURXUIObjectData
	{
	public:
		XURXUIObjectData()
		{

		}

		virtual ~XURXUIObjectData() = default;

		uint16_t unk; //0x12 de blob 0x1C NT08

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
		virtual bool IsFloatPropType(uint32_t unused, uint32_t propType) = 0;
		virtual bool IsColourPropType(uint32_t unused, uint32_t propType) = 0;
		virtual uint32_t GetTimelinePropSize(uint32_t unused, uint32_t propType) = 0;
		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3) = 0;
		virtual bool ValidateTimelineProp(uint32_t unused, uint32_t param_2) = 0;

		void LoadChildren(uint8_t* a_pData);
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

		uint16_t m_id;
		float m_width; //0x24
		float m_height; //0x28
		float m_positon; //0x2C
		float m_scale; //0x30
		float m_rotation;
		uint32_t m_opacity;
		uint32_t m_anchor; 
		uint32_t m_pivot; 
		uint32_t m_show; 
		uint32_t m_blendMode;
		uint32_t m_disableTimelineRecursion;
		bool m_colorWriteFlags;
		bool m_clipChildren;

		static const char* sm_sTypeInfo;
		
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		bool Load(TXUIResource& resource, uint8_t*& a_pData);

		bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);

		bool ValidateTimelineProp(uint32_t unused, uint32_t param_2) { return true; }

		bool IsFloatPropType(uint32_t unused, uint32_t propType) {
			return propType != 1 && propType != 2 && propType != 6 ? false : true;
		}

		// No it's not
		bool IsColourPropType(uint32_t unused, uint32_t propType) {
			return false;
		}

		uint32_t GetTimelinePropSize(uint32_t unused, uint32_t propType) {

			if (propType != 11 && propType != 9 && propType != 14)
			{
				if (propType != 0) return 4;
				return 2;
			}
			return 1;

		}
	};
}

