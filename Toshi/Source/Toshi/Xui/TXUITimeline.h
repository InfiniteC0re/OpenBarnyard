#pragma once

#include "Toshi/Xui/TXUIResource.h"
#include "Toshi/Xui/TXUIKeyframe.h"

namespace Toshi
{
	class TXUITimeline
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

		void Load(Toshi::TXUIResource& a_rXur, unsigned char*& a_pData);
	};

}