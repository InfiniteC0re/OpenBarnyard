#include "ToshiPCH.h"
#include "THPTimer_Win.h"

#include <windows.h>

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
	QueryPerformanceCounter( (LARGE_INTEGER*)&m_iCurrentTime );
	QueryPerformanceFrequency( (LARGE_INTEGER*)&m_iFrequency );
	m_iOldTime = m_iCurrentTime;
	m_fDelta   = 0;
}

void THPTimer::Update()
{
	m_iOldTime = m_iCurrentTime;
	QueryPerformanceCounter( (LARGE_INTEGER*)&m_iCurrentTime );

	TFLOAT ratio      = 1.0f / m_iFrequency;
	m_fDelta          = ( m_iCurrentTime - m_iOldTime ) * ratio;
	m_fCurrentSeconds = m_iCurrentTime * ratio;
}

TOSHI_NAMESPACE_END
