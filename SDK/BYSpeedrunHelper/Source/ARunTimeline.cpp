#include "pch.h"
#include "ARunTimeline.h"

#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TUINT64 s_uiPerformanceFrequency = 0;
static TBOOL   s_bGotPerformanceFrequency = TFALSE;

ARunTimeline::ARunTimeline()
    : m_vecEvents( GetGlobalAllocator(), 512 )
    , m_vecEventsData( GetGlobalAllocator(), 512 * 1024 )
    , m_uiStartTime( 0 )
    , m_uiLastEventTime( 0 )
    , m_uiDuration( 0 )
    , m_uiDurationNoLoads( 0 )
    , m_bIsLive( TFALSE )
{
	// Get performance frequency only once
	if ( !s_bGotPerformanceFrequency )
	{
		QueryPerformanceFrequency( (LARGE_INTEGER*)&s_uiPerformanceFrequency );
		s_bGotPerformanceFrequency = TTRUE;
	}
}

ARunTimeline::~ARunTimeline()
{
}

void ARunTimeline::Reset()
{
	// Clear data
	m_vecEvents.Clear();
	m_vecEventsData.Clear();
	m_uiDuration        = 0;
	m_uiDurationNoLoads = 0;
	m_uiStartTime       = 0;
	m_uiLastEventTime   = 0;
	m_bIsLive           = TFALSE;
}

void ARunTimeline::Start()
{
	Reset();

	// Update start time
	QueryPerformanceCounter( (LARGE_INTEGER*)&m_uiStartTime );
	m_uiLastEventTime = m_uiStartTime;

	m_bIsLive = TTRUE;
}

void ARunTimeline::AddEvent( EventType a_eEventType, void* a_pData /*= TNULL*/, TUINT16 a_uiDataSize /*= 0 */ )
{
	if ( !m_bIsLive )
		return;

	const TBOOL   bWasLoading         = m_vecEvents.Size() > 0 && m_vecEvents.Back()->eType == EventType::LoadStart;
	const TUINT64 uiPreviousEventTime = m_uiLastEventTime;
	QueryPerformanceCounter( (LARGE_INTEGER*)&m_uiLastEventTime );

	Event evt;
	evt.uiTime     = m_uiLastEventTime - uiPreviousEventTime;
	evt.bData      = a_pData && a_uiDataSize != 0;
	evt.eType      = a_eEventType;
	evt.uiDataSize = ( evt.bData ) ? a_uiDataSize : 0;
	m_vecEvents.PushBack( evt );

	// Copy data to the buffer
	if ( evt.bData )
	{
		TINT iDataCursor = m_vecEventsData.Size();
		m_vecEventsData.SetSize( iDataCursor + a_uiDataSize );
		TUtil::MemCopy( &m_vecEventsData[ iDataCursor ], a_pData, a_uiDataSize );
	}

	// Update duration
	m_uiDuration += evt.uiTime;
	TASSERT( !bWasLoading || ( bWasLoading && a_eEventType == EventType::LoadEnd ) );

	if ( !bWasLoading && a_eEventType != EventType::LoadEnd )
		m_uiDurationNoLoads += evt.uiTime;
}

void ARunTimeline::Stop()
{
	AddEvent( EventType::Stop );
	m_bIsLive = TFALSE;
}

void ARunTimeline::SaveToFile( const TCHAR* a_pchBaseName )
{
	if ( !a_pchBaseName )
		a_pchBaseName = "Run";

	CreateDirectoryA( "Timelines", NULL );

	time_t seconds;
	time( &seconds );
	tm* curTime = gmtime( &seconds );

	// Make path to the file
	T2FormatString256 filename;
	filename.Format(
	    "Timelines\\%s_%d%02d%02d_%02d_%02d_%02d.by",
	    a_pchBaseName,
	    curTime->tm_year + 1900,
	    curTime->tm_mon + 1,
	    curTime->tm_mday,
	    curTime->tm_hour,
	    curTime->tm_min,
	    curTime->tm_sec
	);

	TFile* pFile = TFile::Create( filename.Get(), TFILEMODE_CREATENEW | TFILEMODE_WRITE );

	if ( !pFile )
	{
		TERROR( "Unable to save timeline!" );
		TASSERT( TFALSE );
		return;
	}

	// The file is surely open...

	// Write header
	TVersion uiVersion = TVERSION( 1, 0 );
	pFile->Write( "BYTL", 4 );
	pFile->Write( &uiVersion, sizeof( uiVersion ) );
	pFile->Write( &s_uiPerformanceFrequency, sizeof( s_uiPerformanceFrequency ) );
	pFile->Write( &m_uiStartTime, sizeof( m_uiStartTime ) );
	pFile->Write( &m_uiDuration, sizeof( m_uiDuration ) );
	pFile->Write( &m_uiDurationNoLoads, sizeof( m_uiDurationNoLoads ) );
	
	// Write the events
	TINT iNumEvents = m_vecEvents.Size();
	pFile->Write( &iNumEvents, sizeof(iNumEvents) );

	TINT iDataCursor = 0;

	T2_FOREACH( m_vecEvents, it )
	{
		// Write base event info
		pFile->Write( it.Get(), sizeof( TUINT64 ) );

		// If stores some data, write it too
		if ( it->bData )
		{
			pFile->Write( &it->uiDataSize, sizeof( it->uiDataSize ) );
			pFile->Write( &m_vecEventsData[ iDataCursor ], it->uiDataSize );
			iDataCursor += it->uiDataSize;
		}
	}

	// Close the file
	pFile->Destroy();
}

void ARunTimeline::GetDuration( TDOUBLE& a_rDuration, TDOUBLE& a_rDurationNoLoads, TBOOL a_bForceLiveTime /*= TFALSE */ )
{
	TUINT64 uiDuration        = m_uiDuration;
	TUINT64 uiDurationNoLoads = m_uiDurationNoLoads;

	if ( ( a_bForceLiveTime || m_bIsLive ) )
	{
		TUINT64 uiCurrentTime;
		QueryPerformanceCounter( (LARGE_INTEGER*)&uiCurrentTime );

		TUINT64 uiDelta = uiCurrentTime - m_uiLastEventTime;
		uiDuration += uiDelta;

		// Don't add live time if load started
		if ( m_vecEvents.Size() == 0 || m_vecEvents.Back()->eType != EventType::LoadStart )
			uiDurationNoLoads += uiDelta;
	}

	a_rDuration        = TDOUBLE( uiDuration ) / TDOUBLE( s_uiPerformanceFrequency );
	a_rDurationNoLoads = TDOUBLE( uiDurationNoLoads ) / TDOUBLE( s_uiPerformanceFrequency );
}
