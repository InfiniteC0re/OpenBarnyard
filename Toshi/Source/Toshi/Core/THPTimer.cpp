#include "pch.h"
#include "THPTimer.h"

namespace Toshi
{
	THPTimer::THPTimer()
	{
		Reset();
	}

	uint32_t THPTimer::GetRaw32()
	{
		LARGE_INTEGER raw32;
		QueryPerformanceCounter(&raw32);
		return static_cast<uint32_t>(raw32.QuadPart);
	}

	int64_t THPTimer::GetRaw64()
	{
		LARGE_INTEGER raw64;
		QueryPerformanceCounter(&raw64);
		return raw64.QuadPart;
	}

	void THPTimer::Reset()
	{
		QueryPerformanceCounter(&m_iCurrentTime);
		QueryPerformanceFrequency(&m_iFrequency);
		m_iOldTime = m_iCurrentTime;
		m_fDelta = 0;
	}

	void THPTimer::Update()
	{
		m_iOldTime = m_iCurrentTime;
		QueryPerformanceCounter(&m_iCurrentTime);

		float ratio = 1.0f / m_iFrequency.QuadPart;
		m_fDelta = (m_iCurrentTime.QuadPart - m_iOldTime.QuadPart) * ratio;
		m_fCurrentSeconds = m_iCurrentTime.QuadPart * ratio;
	}
}
