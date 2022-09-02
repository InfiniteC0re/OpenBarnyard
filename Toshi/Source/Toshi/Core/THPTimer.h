#pragma once
namespace Toshi
{
	class THPTimer
	{
		LARGE_INTEGER m_iFrequency; // 0x0
		float m_fCurrent; // 0x8
		LARGE_INTEGER m_iUnk; // 0x10
		LARGE_INTEGER m_iUnk2; // 0x14
		LARGE_INTEGER m_iPerformanceCount; // 0x18
		LARGE_INTEGER m_iUnk3; // 0x1C
		float m_fDelta; // 0x20
	public:
		THPTimer();
		unsigned int GetRaw32();
		int64_t GetRaw64();
		void Reset();
	};

}

