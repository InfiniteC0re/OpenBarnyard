#pragma once
#include <cstdint>

namespace Toshi
{
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
}
