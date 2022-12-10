#pragma once

namespace Toshi
{
	class THPTimer
	{
	public:
		THPTimer()
		{
			Reset();
		}

		uint32_t GetRaw32()
		{
			LARGE_INTEGER raw32;
			QueryPerformanceCounter(&raw32);
			return static_cast<uint32_t>(raw32.QuadPart);
		}

		int64_t GetRaw64()
		{
			LARGE_INTEGER raw64;
			QueryPerformanceCounter(&raw64);
			return raw64.QuadPart;
		}

		void Reset()
		{
			QueryPerformanceCounter(&m_iCurrentTime);
			QueryPerformanceFrequency(&m_iFrequency);
			m_iOldTime = m_iCurrentTime;
			m_fDelta = 0;
		}

		void Update()
		{
			m_iOldTime = m_iCurrentTime;
			QueryPerformanceCounter(&m_iCurrentTime);

			float ratio = 1.0f / m_iFrequency.QuadPart;
			m_fDelta = (m_iCurrentTime.QuadPart - m_iOldTime.QuadPart) * ratio;
			m_fCurrentSeconds = m_iCurrentTime.QuadPart * ratio;
		}

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

