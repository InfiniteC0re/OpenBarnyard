#include "pch.h"
#include "AWaveBankFMODFSBStream.h"
#include "ASoundManager.h"
#include "ALoadScreen.h"

#include "Locale/ALocaleManager.h"

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

AWaveBankFMODFSBStream::AWaveBankFMODFSBStream( const TPString8& a_strBank, const TPString8& a_strPath ) :
	AWaveBank( a_strBank, a_strPath, TTRUE )
{

}

TBOOL AWaveBankFMODFSBStream::IsReady() const
{
	return FSOUND_Stream_GetOpenState( (FSOUND_STREAM*)m_FileHandle ) == 0;
}

AWaveBank::LOADRESULT AWaveBankFMODFSBStream::Load( LOADFLAGS a_uiFlags, TINT a_iBufferSize )
{
	if ( m_iNumLoads > 0 || m_FileHandle != TNULL )
	{
		m_iNumLoads++;
		return LOADRESULT_OK;
	}

	TString8 strFileName;
	strFileName += ASoundManager::SOUNDS_BASE_DIRECTORY;
	strFileName += "FSB/";

	strFileName += m_strPath.GetString8();
	strFileName += ".";
	strFileName += "fsb";

	TFileManager* pFileManager = TFileManager::GetSingleton();

	TNativeFileInfo oFileInfo;
	TBOOL bGotFileInfo = pFileManager->GetFileInfo( strFileName, oFileInfo );

	TASSERT( TTRUE == bGotFileInfo );

	TINT iBufferSize = ( a_iBufferSize <= 0 ) ? 2000 : a_iBufferSize;
	FSOUND_Stream_SetBufferSize( TINT( iBufferSize ) );

	// Open the FSB stream
	TUINT uiStreamOpenMode =
		( FSOUND_NONBLOCKING | FSOUND_SIGNED | FSOUND_MONO | FSOUND_16BITS ) |
		( ( m_pWaves->uiFlags & 2 ) ? FSOUND_HW3D : FSOUND_HW2D );

	FSOUND_STREAM* pStream = FSOUND_Stream_Open( oFileInfo.InternalPath, uiStreamOpenMode, oFileInfo.Offset, oFileInfo.Length );
	TVALIDPTR( pStream );

	if ( pStream != TNULL )
	{
		// Update ALoadScreen if neccessary
		if ( ASoundManager::ms_bShouldUpdateLoadingScreen )
		{
			TINT iOpenState = FSOUND_Stream_GetOpenState( pStream );
			while ( iOpenState == -2 )
			{
				g_oLoadScreen.Update();
				ThreadSleep( 5 );
				iOpenState = FSOUND_Stream_GetOpenState( pStream );
			}
		}

		// Store info about the samples

		m_iNumLoads += 1;
		m_FileHandle = pStream;

		for ( TINT i = 0; i < m_iNumWaves; i++ )
		{
			AWave* pWave = &m_pWaves[ i ];
			pWave->iIndex = i;
			pWave->pSampleHandle = pStream;
			pWave->uiFlags |= ( 4 | 8 );
		}

		return LOADRESULT_OK;
	}

	return LOADRESULT_ERR;
}

AWaveBank::UNLOADRESULT AWaveBankFMODFSBStream::Unload()
{
	if ( m_iNumLoads != 0 && --m_iNumLoads < 1 && m_FileHandle )
	{
		m_iNumLoads = 0;

		FSOUND_STREAM* pStream = (FSOUND_STREAM*)m_FileHandle;

		TINT iOpenState = FSOUND_Stream_GetOpenState( pStream );
		while ( iOpenState == -2 )
		{
			ThreadSleep( 5 );
			iOpenState = FSOUND_Stream_GetOpenState( pStream );
		}

		FSOUND_Stream_Close( pStream );

		// Reset the waves
		for ( TINT i = 0; i < m_iNumWaves; i++ )
		{
			AWave* pWave = &m_pWaves[ i ];
			pWave->pSampleHandle = TNULL;
			pWave->iIndex = -1;
			pWave->uiFlags &= ~( 4 | 8 );
		}

		m_FileHandle = TNULL;
	}

	return UNLOADRESULT_OK;
}

