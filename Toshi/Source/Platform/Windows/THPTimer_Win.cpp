#include "ToshiPCH.h"
#include "THPTimer_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

THPTimer::THPTimer()
{
	Reset();
}

THPTimer::~THPTimer()
{
	Reset();
}

TUINT32 THPTimer::GetRaw32()
{
	LARGE_INTEGER raw32;
	QueryPerformanceCounter( &raw32 );
	return static_cast<TUINT32>( raw32.QuadPart );
}

int64_t THPTimer::GetRaw64()
{
	LARGE_INTEGER raw64;
	QueryPerformanceCounter( &raw64 );
	return raw64.QuadPart;
}

void THPTimer::Reset()
{
	QueryPerformanceCounter( &m_iCurrentTime.LargeInteger() );
	QueryPerformanceFrequency( &m_iFrequency.LargeInteger() );
	m_iOldTime = m_iCurrentTime;
	m_fDelta   = 0;
}

void THPTimer::Update()
{
	m_iOldTime = m_iCurrentTime;
	QueryPerformanceCounter( &m_iCurrentTime.LargeInteger() );

	TFLOAT ratio      = 1.0f / m_iFrequency.QuadPart();
	m_fDelta          = ( m_iCurrentTime.QuadPart() - m_iOldTime.QuadPart() ) * ratio;
	m_fCurrentSeconds = m_iCurrentTime.QuadPart() * ratio;
}

TOSHI_NAMESPACE_END
