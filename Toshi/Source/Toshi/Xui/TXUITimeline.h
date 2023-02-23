#pragma once
#include "Toshi/Xui/TXUIResource.h"
#include "XUI.h"

namespace Toshi
{

	class XURXUIKeyframeData
	{
		uint32_t m_uiKeyframeTime;
		uint16_t m_unk2;
		uint8_t m_unk3;
		uint8_t m_unk4;
		uint8_t m_unk5;
		uint8_t m_unk6;


		XURXUIKeyframeData()
		{
			m_uiKeyframeTime = 0;
			m_unk2 = 0;
			m_unk3 = 0;
			m_unk4 = 0;
			m_unk5 = 0;
			m_unk6 = 50;
		}
	};

	class XURXUINamedFrameData
	{
	public:
		uint32_t m_unk;
		uint16_t m_unk2;
		uint16_t m_unk3;
		uint8_t m_unk4;


		XURXUINamedFrameData()
		{
			m_unk = 0;
			m_unk2 = 0;
			m_unk3 = 0;
			m_unk4 = 0;
		}
	};

	class XURXUITimelineData
	{
	public:
		class TimelineProp
		{
		public:
			uint32_t m_unk;
			int32_t m_unk2;

			TimelineProp()
			{
				m_unk = 0;
				m_unk2 = -1;
			}
		};

	public:
		void Load(Toshi::TXUIResource& a_rXur, uint8_t*& a_pData);

	private:
		XURXUIObjectData* m_pControlledChild; // 0x4 de blob
		XURXUIKeyframeData* m_pKeyframeData; // 0x8 de blob
		TimelineProp* m_TimelineProps;
		XUIEPTShort32 m_NumTimelineProps;
		XUIEPTShort32 m_NumKeyframes;
		XUIEPTString m_StringId;
	};
}