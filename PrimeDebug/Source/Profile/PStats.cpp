#include "PrimeDebugPCH.h"
#include "PStats.h"
#include "Draw/PDebugText.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV
#  include <float.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TSTATICASSERT( sizeof( PStats ) == 0x30 );

PStats PStats::ms_oCpuStats( 2.0f );
PStats PStats::ms_oFrameStats( 2.0f );
TUINT  PStats::ms_uiUnknown = 1;

// DOES_NOTHING
// $deBlob2: FUNCTION 0080a9ca
void PStats::Shutdown()
{
}

// DOES_NOTHING
// $deBlob2: FUNCTION 0080a9cb
void PStats::Startup()
{
}

// NOTE: the first interval's minimum starts at 0, so it stays 0
// $deBlob2: FUNCTION 0080aa51
PStats::PStats( TFLOAT a_fInterval )
{
	m_fTime       = 0.0f;
	m_iCount      = 0;
	m_fMin        = 0.0f;
	m_fMax        = 0.0f;
	m_fSum        = 0.0f;
	m_fSumSquares = 0.0f;
	m_fLastMin    = 0.0f;
	m_fLastMax    = 0.0f;
	m_fAverage    = 0.0f;
	m_fDeviation  = 0.0f;
	m_fInterval   = a_fInterval;
	m_fRate       = 1.0f;
}

// Adds a value; once a_fTime adds up to the interval, the results are updated and it starts again (returns TTRUE)
// $deBlob2: FUNCTION 0080aa92
TBOOL PStats::Add( TFLOAT a_fValue, TFLOAT a_fTime )
{
	if ( a_fValue < 0.0f )
		a_fValue = 0.0f;

	m_iCount++;

	if ( a_fValue <= m_fMin )
		m_fMin = a_fValue;

	if ( m_fMax <= a_fValue )
		m_fMax = a_fValue;

	m_fSum += a_fValue;
	m_fSumSquares += a_fValue * a_fValue;
	m_fTime += a_fTime;

	if ( m_fTime < m_fInterval )
		return TFALSE;

	m_fLastMin = m_fMin;
	m_fLastMax = m_fMax;

	TFLOAT fScale = 1.0f / TFLOAT( m_iCount );
	m_fAverage    = m_fSum * fScale;

	TFLOAT fVariance = m_fSumSquares * fScale - m_fAverage * m_fAverage;
	m_fDeviation     = TFLOAT( sqrt( double( fVariance < 0.0f ? 0.0f : fVariance ) ) );
	m_fRate          = 1.0f / ( m_fAverage < 0.0001f ? 0.0001f : m_fAverage );

	m_fMax        = 0.0f;
	m_fSum        = 0.0f;
	m_fSumSquares = 0.0f;
	m_iCount      = 0;
	m_fTime       = 0.0f;
	m_fMin        = FLT_MAX;

	return TTRUE;
}

// NOTE: Prime reads the times from gcKernel itself
// $deBlob2: FUNCTION 0080a9d1
void PStats::Update( TFLOAT a_fCpuTime, TFLOAT a_fFrameTime )
{
	ms_oCpuStats.Add( a_fCpuTime, a_fFrameTime );
	ms_oFrameStats.Add( a_fFrameTime, a_fFrameTime );
}

// Green at 60 frames a second, yellow below, red below 30
// $deBlob2: FUNCTION 0080a826
TUINT32 PStats::GetColour( TFLOAT a_fRate )
{
	TUINT32 uiColour = PDebugText::COLOUR_GREEN;

	if ( 59.9f > a_fRate )
		uiColour = PDebugText::COLOUR_YELLOW;

	if ( 29.9f > a_fRate )
		uiColour = PDebugText::COLOUR_RED;

	return uiColour;
}

// $deBlob2: FUNCTION 0080a875
void PStats::Print()
{
	TCHAR szText[ 52 ];

	PString::Format( szText, 50, " TOTAL  FPS %2.3f   Avg %2.1fms   Max %2.1fms", double( ms_oFrameStats.m_fRate ), double( ms_oFrameStats.m_fAverage * 1000.0f ), double( ms_oFrameStats.m_fLastMax * 1000.0f ) );
	PDebugText::Print( szText, GetColour( ms_oFrameStats.m_fRate ), 1.0f );

	PString::Format( szText, 50, " CPU    FPS %2.3f   Avg %2.1fms   Max %2.1fms", double( ms_oCpuStats.m_fRate ), double( ms_oCpuStats.m_fAverage * 1000.0f ), double( ms_oCpuStats.m_fLastMax * 1000.0f ) );
	PDebugText::Print( szText, GetColour( ms_oCpuStats.m_fRate ), 1.0f );
}

// $deBlob2: FUNCTION 0080a9bd
void PStats::SetUnknown( TUINT a_uiValue )
{
	ms_uiUnknown = a_uiValue;
}

#endif // PRIMEDEBUG_DEV
