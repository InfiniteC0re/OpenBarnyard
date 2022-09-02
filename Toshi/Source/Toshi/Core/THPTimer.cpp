#include "pch.h"
#include "THPTimer.h"

Toshi::THPTimer::THPTimer()
{
	Reset();
}

unsigned int Toshi::THPTimer::GetRaw32()
{
	LARGE_INTEGER raw32;
	QueryPerformanceCounter(&raw32);
	return static_cast<int>(raw32.QuadPart);
}

int64_t Toshi::THPTimer::GetRaw64()
{
	LARGE_INTEGER raw64;
	QueryPerformanceCounter(&raw64);
	return raw64.QuadPart;
}

void Toshi::THPTimer::Reset()
{
	QueryPerformanceCounter(&m_iPerformanceCount);
	QueryPerformanceFrequency(&m_iFrequency);
	m_iUnk = m_iPerformanceCount;
	m_iUnk2 = m_iUnk3;
	m_fDelta = 0;
}
