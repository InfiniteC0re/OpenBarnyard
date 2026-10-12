#include "pch.h"
#include "AMultiplexStream.h"
#include "ASoundManager.h"

#include <File/TFile.h>
#include <Plugins/PPropertyParser/PBProperties.h>
#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( Name );
TPSTRING8_DECLARE( FSB );
TPSTRING8_DECLARE( Loop );
TPSTRING8_DECLARE( StreamBufferSize );
TPSTRING8_DECLARE( Ingame );

TINT AMultiplexStream::ms_iPauseCount = 0;

// $Barnyard: FUNCTION 005d4070
AMultiplexStream::Stream::Stream()
    : aCategories( TNULL )
    , afMaxVolumes( TNULL )
    , afVolumes( TNULL )
    , aiPans( TNULL )
    , iNumChannels( 0 )
    , bLoop( TFALSE )
    , fVolume( 1.0f )
    , iStreamBufferSize( 0 )
{
}

// $Barnyard: FUNCTION 005d40a0
AMultiplexStream::Stream::~Stream()
{
	delete[] aCategories;
	TFree( afMaxVolumes );
	TFree( afVolumes );
	TFree( aiPans );
}

// $Barnyard: FUNCTION 005d4160
AMultiplexStream::AMultiplexStream()
    : m_pStream( TNULL )
    , m_eState( STATE_CLOSED )
    , m_iChannel( -1 )
    , m_iNumSubChannels( -1 )
    , m_iNumStreams( 0 )
    , m_iCurrentStream( -1 )
{
	m_oTRB.Load( "data/sound/multiplexstreams.trb", 0 );

	const PBProperties* pMain = TSTATICCAST( const PBProperties, m_oTRB.GetSymbolAddress( "Main" ) );
	m_iNumStreams             = pMain->GetPropertyCount();

	for ( TINT i = 0; i < m_iNumStreams; i++ )
	{
		const PBProperties::PBProperty* pProperty = pMain->GetPropertyByIndex( i );
		Stream&                         rStream   = m_aStreams[ i ];

		if ( T2String8::Compare( pProperty->GetName().GetString(), "MultiplexStream" ) != 0 )
			continue;

		const PBProperties* pStreamProperties = pProperty->GetValue()->GetProperties();

		pStreamProperties->GetOptionalPropertyValue( rStream.strName, TPS8( Name ) );
		pStreamProperties->GetOptionalPropertyValue( rStream.strFSB, TPS8( FSB ) );

		// The channels
		const PBPropertyValueArray* pCategories  = pStreamProperties->GetProperty( "ChannelCategories" )->GetArray();
		const TINT                  iNumChannels = pCategories->GetSize();

		rStream.iNumChannels = iNumChannels;
		rStream.aCategories  = new TPString8[ iNumChannels ];
		rStream.afVolumes    = TSTATICCAST( TFLOAT, TMalloc( sizeof( TFLOAT ) * iNumChannels ) );
		rStream.afMaxVolumes = TSTATICCAST( TFLOAT, TMalloc( sizeof( TFLOAT ) * iNumChannels ) );
		rStream.aiPans       = TSTATICCAST( TINT, TMalloc( sizeof( TINT ) * iNumChannels ) );

		for ( TINT k = 0; k < iNumChannels; k++ )
		{
			rStream.aCategories[ k ] = pCategories->GetValue( k )->GetTPString8();
			rStream.afVolumes[ k ]   = 1.0f;
			rStream.aiPans[ k ]      = 128;
		}

		if ( const PBPropertyValue* pPans = pStreamProperties->GetProperty( "ChannelPan" ) )
		{
			const PBPropertyValueArray* pArray = pPans->GetArray();

			for ( TUINT k = 0; k < pArray->GetSize(); k++ )
				rStream.aiPans[ k ] = pArray->GetValue( k )->GetInteger();
		}

		if ( const PBPropertyValue* pMaxVolumes = pStreamProperties->GetProperty( "ChannelMaxVolumes" ) )
		{
			const PBPropertyValueArray* pArray = pMaxVolumes->GetArray();

			for ( TUINT k = 0; k < pArray->GetSize(); k++ )
			{
				const PBPropertyValue* pValue = pArray->GetValue( k );

				if ( pValue->GetType() == PBPropertyValue::Type::Float )
					rStream.afMaxVolumes[ k ] = pValue->GetFloat();
				else
					rStream.afMaxVolumes[ k ] = TFLOAT( pValue->GetInteger() );

				TMath::Clip( rStream.afMaxVolumes[ k ], 0.0f, 1.0f );
			}
		}
		else
		{
			for ( TINT k = 0; k < iNumChannels; k++ )
				rStream.afMaxVolumes[ k ] = 1.0f;
		}

		pStreamProperties->GetOptionalPropertyValue( rStream.bLoop, TPS8( Loop ) );

		rStream.iStreamBufferSize = 2000;
		pStreamProperties->GetOptionalPropertyValue( rStream.iStreamBufferSize, TPS8( StreamBufferSize ) );
	}

	m_oPauseListener.Connect( g_oSystemManager.GetPauseEmitter(), this, OnSystemPause, 0 );
}

// $Barnyard: FUNCTION 005d4530
AMultiplexStream::~AMultiplexStream()
{
	m_oTRB.Close();
	m_oPauseListener.Disconnect();
}

// $Barnyard: FUNCTION 005d3a30
void AMultiplexStream::Update( TFLOAT a_fDeltaTime )
{
	TINT iOpenState = -6;

	if ( m_pStream )
		iOpenState = FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_pStream );

	switch ( m_eState )
	{
		case STATE_OPENING:
			if ( iOpenState == ASoundManager::OPENSTATE_READY )
				m_eState = STATE_READY;

			break;
		case STATE_STOP_REQUESTED:
			if ( iOpenState == ASoundManager::OPENSTATE_READY )
			{
				FSOUND_Stream_Stop( (FSOUND_STREAM*)m_pStream );
				m_eState = STATE_READY;
			}

			break;
		case STATE_CLOSE_REQUESTED:
			if ( iOpenState != ASoundManager::OPENSTATE_OPENING )
			{
				FSOUND_Stream_Close( (FSOUND_STREAM*)m_pStream );
				m_pStream = TNULL;
				m_eState  = STATE_CLOSED;
			}

			break;
	}
}

// $Barnyard: FUNCTION 005d3ac0
void AMultiplexStream::Close( TBOOL a_bImmediately )
{
	if ( m_iChannel != -1 )
	{
		FSOUND_SetReserved( m_iChannel, FALSE );

		for ( TINT i = 0; i < m_iNumSubChannels; i++ )
			FSOUND_SetReserved( FSOUND_GetSubChannel( m_iChannel, i ), FALSE );
	}

	if ( a_bImmediately )
	{
		while ( FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_pStream ) == ASoundManager::OPENSTATE_OPENING )
			Sleep( 5 );

		FSOUND_Stream_Close( (FSOUND_STREAM*)m_pStream );
		m_pStream        = TNULL;
		m_eState         = STATE_CLOSED;
		m_iCurrentStream = -1;
	}
	else
	{
		m_eState         = STATE_CLOSE_REQUESTED;
		m_iCurrentStream = -1;
	}
}

// $Barnyard: FUNCTION 005d3b60
void AMultiplexStream::Stop()
{
	if ( m_iChannel != -1 )
	{
		FSOUND_SetReserved( m_iChannel, FALSE );

		for ( TINT i = 0; i < m_iNumSubChannels; i++ )
			FSOUND_SetReserved( FSOUND_GetSubChannel( m_iChannel, i ), FALSE );
	}

	m_eState   = STATE_STOP_REQUESTED;
	m_iChannel = -1;
}

// $Barnyard: FUNCTION 005d3bc0
TFLOAT AMultiplexStream::GetVolume( TINT a_iChannel )
{
	if ( m_iCurrentStream < 0 || m_iCurrentStream >= m_iNumStreams )
		return 0.0f;

	if ( a_iChannel == -1 )
		return m_aStreams[ m_iCurrentStream ].fVolume;

	return m_aStreams[ m_iCurrentStream ].afVolumes[ a_iChannel ];
}

// $Barnyard: FUNCTION 005d3c00
void AMultiplexStream::Pause( TBOOL a_bPause )
{
	if ( a_bPause )
	{
		if ( ms_iPauseCount == 0 && m_iChannel != -1 )
			FSOUND_SetPaused( m_iChannel, TRUE );

		ms_iPauseCount++;
	}
	else if ( ms_iPauseCount > 0 && --ms_iPauseCount == 0 )
	{
		if ( m_iChannel != -1 )
			FSOUND_SetPaused( m_iChannel, FALSE );
	}
}

// $Barnyard: FUNCTION 005d3c60
TBOOL __stdcall AMultiplexStream::OnSystemPause( AMultiplexStream* a_pStream, TSystemManager* a_pSystemManager, TBOOL* a_pPaused )
{
	a_pStream->Pause( *a_pPaused );
	return TTRUE;
}

// $Barnyard: FUNCTION 005d3cd0
void AMultiplexStream::Open( const TCHAR* a_szName )
{
	TINT iStream = -1;

	for ( TINT i = 0; i < m_iNumStreams; i++ )
	{
		if ( T2String8::CompareNoCase( a_szName, m_aStreams[ i ].strName.GetString() ) == 0 )
		{
			iStream = i;
			break;
		}
	}

	if ( m_pStream )
	{
		// Already open
		if ( iStream == m_iCurrentStream )
			return;

		Close( TTRUE );
	}

	if ( iStream == -1 )
		return;

	m_iCurrentStream = iStream;
	Stream& rStream  = m_aStreams[ m_iCurrentStream ];

	TNativeFileInfo oFileInfo;
	oFileInfo.Offset = 0;
	oFileInfo.Length = 0;
	TFileManager::GetSingleton()->GetFileInfo( TString8( rStream.strFSB.GetString() ), oFileInfo );

	// A software stream. Must not be FSOUND_HW2D/HW3D: the FSBs are multichannel, and FMOD 3.75 leaks its DirectSound
	// lock when it allocates a hardware buffer for a multichannel sample, which then deadlocks every other FMOD call
	TUINT uiMode = FSOUND_2D | FSOUND_NONBLOCKING;

	if ( rStream.bLoop == TTRUE )
		uiMode |= FSOUND_LOOP_NORMAL;

	FSOUND_Stream_SetBufferSize( rStream.iStreamBufferSize );
	m_pStream = FSOUND_Stream_Open( oFileInfo.InternalPath, uiMode, oFileInfo.Offset, oFileInfo.Length );
	m_eState  = STATE_OPENING;
}

// $Barnyard: FUNCTION 005d3e70
void AMultiplexStream::SetVolume( TFLOAT a_fVolume, TINT a_iChannel, TBOOL a_bStore )
{
	if ( m_iChannel == -1 || m_iCurrentStream < 0 || m_iCurrentStream >= m_iNumStreams )
		return;

	TMath::Clip( a_fVolume, 0.0f, 1.0f );
	Stream& rStream = m_aStreams[ m_iCurrentStream ];

	if ( a_iChannel == -1 )
	{
		TFLOAT fCategoryVolume = ASoundManager::GetSingleton()->GetCategoryVolume( TPS8( Ingame ) );
		FSOUND_SetVolume( m_iChannel, TINT( fCategoryVolume * a_fVolume * 255.0f ) );

		if ( a_bStore )
			rStream.fVolume = a_fVolume;
	}
	else if ( a_iChannel >= 0 && a_iChannel < m_iNumSubChannels )
	{
		TFLOAT fCategoryVolume = ASoundManager::GetSingleton()->GetCategoryVolume( rStream.aCategories[ a_iChannel ] );
		TINT   iVolume         = TINT( fCategoryVolume * rStream.afMaxVolumes[ a_iChannel ] * a_fVolume * 255.0f );
		TINT   iSubChannel     = FSOUND_GetSubChannel( m_iChannel, a_iChannel );

		if ( iSubChannel != -1 )
		{
			FSOUND_SetVolume( iSubChannel, iVolume );

			if ( a_bStore )
				rStream.afVolumes[ a_iChannel ] = a_fVolume;
		}
	}
}

// $Barnyard: FUNCTION 005d3fa0
TBOOL AMultiplexStream::Play( TFLOAT a_fVolume )
{
	if ( m_eState != STATE_READY )
		return TFALSE;

	if ( m_iCurrentStream < 0 || m_iCurrentStream >= m_iNumStreams )
		return TFALSE;

	m_iChannel = FSOUND_Stream_PlayEx( FSOUND_FREE, (FSOUND_STREAM*)m_pStream, TNULL, TRUE );
	FSOUND_SetReserved( m_iChannel, TRUE );

	m_eState          = STATE_PLAYING;
	m_iNumSubChannels = FSOUND_GetNumSubChannels( m_iChannel );

	for ( TINT i = 0; i < m_iNumSubChannels; i++ )
	{
		TINT iSubChannel = FSOUND_GetSubChannel( m_iChannel, i );
		FSOUND_SetPan( iSubChannel, m_aStreams[ m_iCurrentStream ].aiPans[ i ] );
		FSOUND_SetReserved( iSubChannel, TRUE );
		SetVolume( a_fVolume, i, TTRUE );
	}

	if ( ms_iPauseCount <= 0 )
		FSOUND_SetPaused( m_iChannel, FALSE );

	return TTRUE;
}
