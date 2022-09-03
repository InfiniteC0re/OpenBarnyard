#pragma once

namespace Toshi
{
	class THPTimer
	{
	public:
		THPTimer();

		uint32_t GetRaw32();
		int64_t GetRaw64();

		void Reset();
		void Update();

	public:
		inline float GetDelta() const { return m_fDelta; }
		inline float GetCurrentSeconds() const { return m_fCurrentSeconds; }

	private:
		LARGE_INTEGER m_iFrequency;        // 0x0
		float m_fCurrentSeconds;           // 0x8
		LARGE_INTEGER m_iOldTime;          // 0x10
		LARGE_INTEGER m_iCurrentTime;      // 0x18
		float m_fDelta;                    // 0x20
	};
}

