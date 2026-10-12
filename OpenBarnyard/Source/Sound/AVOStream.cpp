#include "pch.h"
#include "AVOStream.h"
#include "ASoundManager.h"
#include "ALoadScreen.h"

#include <File/TFile.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/T2String8.h>
#include <Toshi/Platform.h>

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( VO );

// The quest voice overs of the languages (0x007753dc)
static const TCHAR* s_aszQuestVOFiles[] = {
	"data/sound/FSB/QuestVO.fsb",
	"data/sound/FSB/QuestVO.fsb",
	"data/sound/FSB/QuestVO.fsb",
	"data/sound/FSB/GER/QuestVO.fsb",
	"data/sound/FSB/QuestVO.fsb",
	"data/sound/FSB/SPA/QuestVO.fsb",
	"data/sound/FSB/FRE/QuestVO.fsb",
	"data/sound/FSB/QuestVO.fsb",
};

// The cutscene voice overs of the languages (0x007753fc)
static const TCHAR* s_aszCutsceneVOFiles[] = {
	"data/sound/FSB/SS.fsb",
	"data/sound/FSB/SS.fsb",
	"data/sound/FSB/SS.fsb",
	"data/sound/FSB/GER/SS.fsb",
	"data/sound/FSB/SS.fsb",
	"data/sound/FSB/SPA/SS.fsb",
	"data/sound/FSB/FRE/SS.fsb",
	"data/sound/FSB/SS.fsb",
};

// The language the stream was opened with (0x007753d8)
static TINT s_iLanguage = -1;

// $Barnyard: FUNCTION 005d6020
AVOStream::AVOStream()
{
	m_oCurrent.pStream     = TNULL;
	m_oCurrent.eState      = STATE_CLOSED;
	m_oCurrent.iChannel    = -1;
	m_oCurrent.szName[ 0 ] = '\0';
	m_oPushed.eState       = STATE_CLOSED;
	m_oPushed.pStream      = TNULL;
	m_oPushed.iChannel     = -1;
	m_oPushed.szName[ 0 ]  = '\0';
	m_pSets                = TNULL;
	m_iNumSets             = 0;
	m_pPlayingSet          = TNULL;
	m_b3D                  = TFALSE;

	m_oPauseListener.Connect( g_oSystemManager.GetPauseEmitter(), this, OnPause, 0 );

	m_vPosition.z = 0.0f;
	m_vPosition.y = 0.0f;
	m_vPosition.x = 0.0f;
}

// $Barnyard: FUNCTION 005d59d0
// $Barnyard: FUNCTION 005d59c0
AVOStream::~AVOStream()
{
	m_oPauseListener.Disconnect();
}

// $Barnyard: FUNCTION 005d6000
TBOOL __stdcall AVOStream::OnPause( AVOStream* a_pVOStream, TSystemManager* a_pSystemManager, TBOOL* a_pPaused )
{
	if ( a_pVOStream->m_oCurrent.iChannel >= 0 )
		FSOUND_SetPaused( a_pVOStream->m_oCurrent.iChannel, *a_pPaused );

	return TTRUE;
}

// $Barnyard: FUNCTION 005d5f30
void AVOStream::Update( TFLOAT a_fDeltaTime )
{
	TINT iOpenState = -6;

	if ( m_oCurrent.pStream )
		iOpenState = FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_oCurrent.pStream );

	switch ( m_oCurrent.eState )
	{
		case STATE_OPENING:
			if ( iOpenState == ASoundManager::OPENSTATE_READY )
				m_oCurrent.eState = STATE_READY;

			break;
		case STATE_PLAY_REQUESTED:
			if ( iOpenState == ASoundManager::OPENSTATE_READY )
			{
				StartPlaying();
				m_oCurrent.eState = STATE_PLAYING;
			}

			break;
		case STATE_STOP_REQUESTED:
			if ( iOpenState == ASoundManager::OPENSTATE_READY )
			{
				FSOUND_Stream_Stop( (FSOUND_STREAM*)m_oCurrent.pStream );
				m_oCurrent.eState = STATE_READY;
			}

			break;
		case STATE_CLOSE_REQUESTED:
			if ( iOpenState != ASoundManager::OPENSTATE_OPENING )
			{
				FSOUND_Stream_Close( (FSOUND_STREAM*)m_oCurrent.pStream );
				m_oCurrent.pStream = TNULL;
				m_oCurrent.eState  = STATE_CLOSED;

				if ( m_oPushed.pStream )
				{
					// Back to the pushed stream
					m_oCurrent            = m_oPushed;
					m_oPushed.pStream     = TNULL;
					m_oPushed.eState      = STATE_CLOSED;
					m_oPushed.iChannel    = -1;
					m_oPushed.szName[ 0 ] = '\0';
				}
			}

			break;
	}
}

// $Barnyard: FUNCTION 005d5a40
void AVOStream::Open( const TCHAR* a_szName, TBOOL a_bWait, TBOOL a_bPush )
{
	TINT iLanguage = T2Locale::GetSingleton() ? T2Locale::GetSingleton()->GetLanguage() : -1;

	if ( iLanguage == -1 )
		iLanguage = 0;

	TBOOL bQuestVO = TFALSE;

	TCHAR szName[ 128 ];
	T2String8::Copy( szName, a_szName, sizeof( szName ) );
	T2String8::ToLowerCase( szName );

	// The file of the language
	TCHAR szPath[ 128 ];

	if ( T2String8::FindString( szName, "questvo" ) )
	{
		T2String8::Copy( szPath, s_aszQuestVOFiles[ iLanguage ], sizeof( szPath ) );
		bQuestVO = TTRUE;
	}
	else if ( T2String8::FindString( szName, "data/sound/fsb/ss.fsb" ) )
	{
		T2String8::Copy( szPath, s_aszCutsceneVOFiles[ iLanguage ], sizeof( szPath ) );
	}
	else
	{
		T2String8::Copy( szPath, a_szName, sizeof( szPath ) );
	}

	if ( iLanguage == s_iLanguage && T2String8::Compare( m_oCurrent.szName, szName ) == 0 && m_oCurrent.pStream )
		return;

	s_iLanguage = iLanguage;

	if ( a_bPush )
	{
		if ( m_oPushed.pStream || bQuestVO )
			return;

		// Keep the current stream
		while ( FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_oCurrent.pStream ) != ASoundManager::OPENSTATE_READY )
			ThreadSleep( 5 );

		m_oCurrent.eState = STATE_READY;
		FSOUND_Stream_Stop( (FSOUND_STREAM*)m_oCurrent.pStream );

		m_oPushed = m_oCurrent;

		m_oCurrent.pStream     = TNULL;
		m_oCurrent.eState      = STATE_CLOSED;
		m_oCurrent.iChannel    = -1;
		m_oCurrent.szName[ 0 ] = '\0';
	}

	T2String8::Copy( m_oCurrent.szName, szName, sizeof( m_oCurrent.szName ) );

	TNativeFileInfo oFileInfo;
	oFileInfo.Offset = 0;
	oFileInfo.Length = 0;
	TFileManager::GetSingleton()->GetFileInfo( TString8( szPath ), oFileInfo );

	const TUINT uiMode = ( bQuestVO ? FSOUND_HW3D : FSOUND_HW2D ) | FSOUND_NONBLOCKING;

	if ( a_bWait )
	{
		FSOUND_Stream_SetBufferSize( 2000 );
		m_oCurrent.pStream = FSOUND_Stream_Open( oFileInfo.InternalPath, uiMode, oFileInfo.Offset, oFileInfo.Length );

		while ( FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_oCurrent.pStream ) == ASoundManager::OPENSTATE_OPENING )
		{
			g_oLoadScreen.Update( 1.0f, TTRUE );
			ThreadSleep( 5 );
		}

		m_oCurrent.eState = STATE_READY;
	}
	else
	{
		FSOUND_Stream_SetBufferSize( 2000 );
		m_oCurrent.pStream = FSOUND_Stream_Open( oFileInfo.InternalPath, uiMode, oFileInfo.Offset, oFileInfo.Length );
		m_oCurrent.eState  = STATE_OPENING;
	}

	if ( bQuestVO )
	{
		// The sets of lines of the quests
		m_oQuestVOTRB.Load( "data/sound/QuestVO.trb", 0 );
		m_pSets    = TSTATICCAST( VOSet, m_oQuestVOTRB.GetSymbolAddress( "QuestVOSets" ) );
		m_iNumSets = *TSTATICCAST( TINT, m_oQuestVOTRB.GetSymbolAddress( "NumQuestVOSets" ) );
		m_oQuestVOTRB.DeleteSymbolTable();
	}
}

// $Barnyard: FUNCTION 005d5d80
void AVOStream::Close( TBOOL a_bImmediately )
{
	TBOOL bQuestVO = T2String8::FindString( m_oCurrent.szName, "questvo" ) != TNULL;

	if ( a_bImmediately )
	{
		while ( FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_oCurrent.pStream ) == ASoundManager::OPENSTATE_OPENING )
			ThreadSleep( 5 );

		FSOUND_Stream_Close( (FSOUND_STREAM*)m_oCurrent.pStream );
		m_oCurrent.pStream = TNULL;
		m_oCurrent.eState  = STATE_CLOSED;

		if ( m_oPushed.pStream )
		{
			// Back to the pushed stream
			m_oCurrent.pStream  = m_oPushed.pStream;
			m_oCurrent.eState   = m_oPushed.eState;
			m_oCurrent.iChannel = m_oPushed.iChannel;
			T2String8::Copy( m_oCurrent.szName, m_oPushed.szName, sizeof( m_oCurrent.szName ) );

			m_oPushed.pStream     = TNULL;
			m_oPushed.eState      = STATE_CLOSED;
			m_oPushed.iChannel    = -1;
			m_oPushed.szName[ 0 ] = '\0';
		}
	}
	else
	{
		m_oCurrent.eState = STATE_CLOSE_REQUESTED;
	}

	if ( bQuestVO )
		m_oQuestVOTRB.Close();
}

// $Barnyard: FUNCTION 005d5e60
void AVOStream::StartPlaying()
{
	m_oCurrent.iChannel = FSOUND_Stream_PlayEx( FSOUND_FREE, (FSOUND_STREAM*)m_oCurrent.pStream, TNULL, TTRUE );

	if ( m_b3D )
	{
		TFLOAT fMinDistance = 1.2f;

		if ( m_pPlayingSet )
			fMinDistance = m_pPlayingSet->fMinDistance;

		FSOUND_Stream_SetMode( (FSOUND_STREAM*)m_oCurrent.pStream, FSOUND_HW3D | FSOUND_LOOP_OFF );
		FSOUND_3D_SetMinMaxDistance( m_oCurrent.iChannel, fMinDistance, 10000.0f );
		FSOUND_3D_SetAttributes( m_oCurrent.iChannel, &m_vPosition.x, TNULL );
	}
	else
	{
		FSOUND_Stream_SetMode( (FSOUND_STREAM*)m_oCurrent.pStream, FSOUND_HW2D | FSOUND_LOOP_OFF );
		FSOUND_SetPan( m_oCurrent.iChannel, 128 );
	}

	FSOUND_SetVolume( m_oCurrent.iChannel, TINT( ASoundManager::GetSingleton()->GetCategoryVolume( TPS8( VO ) ) * 255.0f ) );
	FSOUND_SetPaused( m_oCurrent.iChannel, TFALSE );

	m_pPlayingSet = TNULL;
}

// $Barnyard: FUNCTION 005d56f0
void AVOStream::Play( TINT a_iSubStream, TBOOL a_b3D )
{
	m_b3D = a_b3D;

	if ( m_oCurrent.pStream )
	{
		FSOUND_Stream_SetSubStream( (FSOUND_STREAM*)m_oCurrent.pStream, a_iSubStream );
		m_oCurrent.eState = STATE_PLAY_REQUESTED;
	}
}

// $Barnyard: FUNCTION 005d58a0
void AVOStream::PlayAtPosition( TINT a_iSubStream, const TVector4& a_rcPosition )
{
	if ( m_oPushed.pStream )
		return;

	m_vPosition.x = a_rcPosition.x;
	m_vPosition.y = a_rcPosition.y;
	m_vPosition.z = -a_rcPosition.z;
	m_pPlayingSet = TNULL;
	m_b3D         = TTRUE;

	if ( m_oCurrent.pStream )
	{
		FSOUND_Stream_SetSubStream( (FSOUND_STREAM*)m_oCurrent.pStream, a_iSubStream );
		m_oCurrent.eState = STATE_PLAY_REQUESTED;
	}
}

// $Barnyard: FUNCTION 005d57e0
AVOStream::VOSet* AVOStream::FindSet( const TCHAR* a_szName, TINT a_iUnused )
{
	for ( TINT i = 0; i < m_iNumSets; i++ )
	{
		if ( T2String8::Compare( m_pSets[ i ].szName, a_szName ) == 0 )
			return &m_pSets[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005d5840
void AVOStream::PlayFromSet( VOSet* a_pSet )
{
	if ( m_oPushed.pStream || a_pSet->uiNumIds == 0 )
		return;

	TUINT16 uiId = a_pSet->auiIds[ a_pSet->uiCurrentId ];
	TBOOL   b3D  = ( a_pSet->uiFlags & VOSet::FLAGS_3D ) == VOSet::FLAGS_3D;

	// The next time the next line
	a_pSet->uiCurrentId = TUINT16( ( a_pSet->uiCurrentId + 1 ) % a_pSet->uiNumIds );
	m_b3D               = b3D;

	if ( m_oCurrent.pStream )
	{
		FSOUND_Stream_SetSubStream( (FSOUND_STREAM*)m_oCurrent.pStream, uiId );
		m_oCurrent.eState = STATE_PLAY_REQUESTED;
	}
}

// $Barnyard: FUNCTION 005d5900
void AVOStream::PlaySet( const TCHAR* a_szName, const TVector4& a_rcPosition )
{
	if ( m_oPushed.pStream )
		return;

	VOSet* pSet = FindSet( a_szName, 0 );

	if ( pSet )
	{
		m_vPosition.x = a_rcPosition.x;
		m_vPosition.y = a_rcPosition.y;
		m_vPosition.z = -a_rcPosition.z;
		m_pPlayingSet = pSet;

		PlayFromSet( pSet );
	}
}

// $Barnyard: FUNCTION 005d5950
void AVOStream::PlaySet( const TCHAR* a_szName, const TCHAR* a_szSuffix, const TVector4& a_rcPosition )
{
	// NOTE: retail formats the name into a global buffer (0x0078c438)
	static TCHAR s_szSetName[ 64 ];

	if ( m_oPushed.pStream )
		return;

	TStringManager::String8Format( s_szSetName, sizeof( s_szSetName ), "%s_%s", a_szName, a_szSuffix );
	PlaySet( s_szSetName, a_rcPosition );
}

// $Barnyard: FUNCTION 005d5720
void AVOStream::Stop()
{
	m_oCurrent.eState   = STATE_STOP_REQUESTED;
	m_oCurrent.iChannel = -1;
}

// $Barnyard: FUNCTION 005d5730
TBOOL AVOStream::IsPlaying()
{
	return FSOUND_IsPlaying( m_oCurrent.iChannel ) != 0;
}

// $Barnyard: FUNCTION 005d5740
TBOOL AVOStream::IsPlayRequested()
{
	return m_oCurrent.eState == STATE_PLAY_REQUESTED;
}

// $Barnyard: FUNCTION 005d5750
TINT AVOStream::GetLengthMs()
{
	if ( m_oCurrent.iChannel == -1 || !FSOUND_IsPlaying( m_oCurrent.iChannel ) )
		return 0;

	return FSOUND_Stream_GetLengthMs( (FSOUND_STREAM*)m_oCurrent.pStream );
}

// $Barnyard: FUNCTION 005d5780
const TCHAR* AVOStream::GetLineName()
{
	if ( m_oCurrent.iChannel != -1 && FSOUND_IsPlaying( m_oCurrent.iChannel ) )
	{
		FSOUND_SAMPLE* pSample = FSOUND_GetCurrentSample( m_oCurrent.iChannel );

		if ( pSample )
			return FSOUND_Sample_GetName( pSample );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005d57b0
TINT AVOStream::GetTime()
{
	if ( m_oCurrent.iChannel == -1 || !FSOUND_IsPlaying( m_oCurrent.iChannel ) )
		return 0;

	return FSOUND_Stream_GetTime( (FSOUND_STREAM*)m_oCurrent.pStream );
}
