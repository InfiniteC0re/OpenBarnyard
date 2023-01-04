#pragma once
#include <cstdint>

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
}