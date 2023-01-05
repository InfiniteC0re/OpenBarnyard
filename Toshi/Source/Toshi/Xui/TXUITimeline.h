#pragma once

#include "Toshi/Xui/TXUIResource.h"

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

		XURXUIObjectData* m_pControlledChild; // 0x4 de blob
		XURXUIKeyframeData* m_aKeyframeData; // 0x8 de blob
		TimelineProp* m_aTimelineProp; // 0xC de blob
		
		uint16_t m_uiNumTimelineProp; // 0x10 de blob
		uint16_t m_uiNumKeyframes; // 0x12 de blob
		uint16_t m_uiNumUnk; // 0x14 de blob

	public:

		void Load(Toshi::TXUIResource& a_rXur, unsigned char*& a_pData);
	};

}