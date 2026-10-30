#include "PrimeDebugPCH.h"
#include "PProfileCounter.h"
#include "Console/PConsole.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

PListNode         PProfileCounter::ms_oCounters;
PListNode         PProfileCounter::ms_oActiveCounters;
TUINT             PProfileCounter::ms_uiFrame;
TINT              PWarning::ms_iNumWarnings;
PWarning::Warning PWarning::ms_aWarnings[ MAX_WARNINGS ];

// $deBlob2: FUNCTION 007e48fb
PProfileCounter::PProfileCounter( const TCHAR* a_szName, void* a_pUserData )
{
	Create( a_szName, a_pUserData );
}

PProfileCounter::~PProfileCounter()
{
	m_oNode.Remove();
}

// $deBlob2: FUNCTION 0080e757
// $deBlob2: FUNCTION 007e4920
void PProfileCounter::Add( TFLOAT a_fValue )
{
	m_iFrameCount++;
	m_iTotalCount++;
	m_fFrameTotal = a_fValue + m_fFrameTotal;
	m_fTotal      = a_fValue + m_fTotal;
	m_oNode.Remove();
	ms_oActiveCounters.PushBack( &m_oNode );
}

// $deBlob2: FUNCTION 0080e797
void PProfileCounter::Create( const TCHAR* a_szName, void* a_pUserData )
{
	if ( !ms_oCounters.IsLinked() )
		ms_oCounters.Init();

	if ( !ms_oActiveCounters.IsLinked() )
		ms_oActiveCounters.Init();

	m_oNode.Init();
	ms_oCounters.PushBack( &m_oNode );
	m_szName    = a_szName;
	m_pUserData = a_pUserData;
	Reset();
}

// $deBlob2: FUNCTION 0080e805
TFLOAT PProfileCounter::GetFrameTotal()
{
	return m_fFrameTotal;
}

// $deBlob2: FUNCTION 0080e810
TFLOAT PProfileCounter::GetAverage()
{
	TINT iCount = m_iTotalCount;

	if ( iCount < 1 )
		iCount = 1;

	return m_fTotal / TFLOAT( iCount );
}

// $deBlob2: FUNCTION 0080e83b
TFLOAT PProfileCounter::GetTotalPerFrame()
{
	return m_fTotal / TFLOAT( GetNumFrames() );
}

// $deBlob2: FUNCTION 0080e864
TFLOAT PProfileCounter::GetCountPerFrame()
{
	return TFLOAT( m_iTotalCount ) / TFLOAT( GetNumFrames() );
}

// $deBlob2: FUNCTION 0080e898
TINT PProfileCounter::GetFrameCount()
{
	return m_iFrameCount;
}

// $deBlob2: FUNCTION 0080e8a3
TINT PProfileCounter::GetNumFrames()
{
	TINT iNumFrames = TINT( ms_uiFrame - m_uiStartFrame );

	if ( iNumFrames == 0 )
		iNumFrames = 1;

	return iNumFrames;
}

// $deBlob2: FUNCTION 0080e8bb
TFLOAT PProfileCounter::GetFrameMax()
{
	return m_fFrameMax;
}

// $deBlob2: FUNCTION 0080e8c6
TINT PProfileCounter::GetFrameMaxCount()
{
	return m_iFrameMaxCount;
}

// $deBlob2: FUNCTION 0080e8d1
TFLOAT PProfileCounter::GetTotal()
{
	return m_fTotal;
}

// $deBlob2: FUNCTION 0080e8dc
TINT PProfileCounter::GetTotalCount()
{
	return m_iTotalCount;
}

// The active counters first
// $deBlob2: FUNCTION 0080e8e7
void PProfileCounter::ForEach( Callback a_fnCallback, void* a_pUserData )
{
	if ( !ms_oCounters.IsLinked() || !ms_oActiveCounters.IsLinked() )
		return;

	for ( PListNode* pNode = ms_oActiveCounters.GetFirst(); pNode != TNULL; pNode = ms_oActiveCounters.GetNext( pNode ) )
		a_fnCallback( a_pUserData, (PProfileCounter*)pNode );

	for ( PListNode* pNode = ms_oCounters.GetFirst(); pNode != TNULL; pNode = ms_oCounters.GetNext( pNode ) )
		a_fnCallback( a_pUserData, (PProfileCounter*)pNode );
}

// $deBlob2: FUNCTION 0080e946
void PProfileCounter::Reset()
{
	TUINT uiFrame    = ms_uiFrame;
	m_fFrameTotal    = 0.0f;
	m_uiStartFrame   = uiFrame;
	m_fTotal         = 0.0f;
	m_fFrameMax      = 0.0f;
	m_iFrameCount    = 0;
	m_iTotalCount    = 0;
	m_iFrameMaxCount = 0;
}

// $deBlob2: FUNCTION 0080e9c2
void PProfileCounter::EndFrame()
{
	if ( !ms_oActiveCounters.IsLinked() )
		return;

	while ( ms_oActiveCounters.pNext != &ms_oActiveCounters )
	{
		PProfileCounter* pCounter = (PProfileCounter*)ms_oActiveCounters.pNext;
		pCounter->m_oNode.Remove();

		TFLOAT fMax = pCounter->m_fFrameMax;
		if ( fMax <= pCounter->m_fFrameTotal )
			fMax = pCounter->m_fFrameTotal;

		pCounter->m_fFrameMax = fMax;

		TINT iMaxCount = pCounter->m_iFrameCount;
		if ( pCounter->m_iFrameCount < pCounter->m_iFrameMaxCount )
			iMaxCount = pCounter->m_iFrameMaxCount;

		pCounter->m_fFrameTotal    = 0.0f;
		pCounter->m_iFrameCount    = 0;
		pCounter->m_iFrameMaxCount = iMaxCount;
		ms_oCounters.PushBack( &pCounter->m_oNode );
	}
}

// $deBlob2: FUNCTION 0080ea4c
void PWarning::Add( const TCHAR* a_szFormat, ... )
{
	TUINT uiFrame = PProfileCounter::GetFrame();

	if ( ms_iNumWarnings >= MAX_WARNINGS )
		return;

	va_list args;
	va_start( args, a_szFormat );

	TCHAR szText[ TEXT_SIZE ];
	PString::FormatV( szText, sizeof( szText ), a_szFormat, args );
	va_end( args );

	TUINT32 uiHash = PConsole::HashString( szText );
	TUINT   uiSlot = uiHash & 0xFF;

	while ( ms_aWarnings[ uiSlot ].uiHash != 0 && ms_aWarnings[ uiSlot ].uiHash != uiHash )
	{
		uiSlot++;

		if ( TINT( uiSlot ) > 0xFF )
			uiSlot = 0;
	}

	ms_aWarnings[ uiSlot ].uiLastFrame = uiFrame;

	if ( ms_aWarnings[ uiSlot ].uiHash == 0 )
	{
		ms_aWarnings[ uiSlot ].uiHash = uiHash;
		ms_aWarnings[ uiSlot ].iCount = 1;
		PString::Copy( ms_aWarnings[ uiSlot ].szText, TEXT_SIZE, szText );
		ms_iNumWarnings++;

		if ( ms_iNumWarnings == MAX_WARNINGS - 1 )
			Add( "Too many warnings" );
	}
	else
	{
		ms_aWarnings[ uiSlot ].iCount++;
	}
}

// $deBlob2: FUNCTION 0080eb4d
void PWarning::ForEach( Callback a_fnCallback, void* a_pUserData )
{
	for ( TINT i = 0; i < MAX_WARNINGS; i++ )
	{
		if ( ms_aWarnings[ i ].uiHash != 0 )
			a_fnCallback( a_pUserData, ms_aWarnings[ i ].szText, ms_aWarnings[ i ].iCount, ms_aWarnings[ i ].uiLastFrame );
	}
}

#endif // PRIMEDEBUG_DEV
