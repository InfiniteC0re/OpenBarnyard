#include "pch.h"
#include "AWaveBankFMODFSB.h"

#include "Locale/ALocaleManager.h"

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

static const TCHAR* s_pLangSoundDirectories[] = {
	"",
	"",
	"",
	"GER/",
	"",
	"SPA/",
	"FRE/",
	""
};

TSTATICASSERT( ALocaleManager::NUM_LOCALES == TARRAYSIZE( s_pLangSoundDirectories ) );

TBOOL g_bIsLoadingFSBFile = TFALSE;

AWaveBankFMODFSB::AWaveBankFMODFSB( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strPath ) : 
	AWaveBank( a_strBank, a_strPath, TFALSE )
{

}

TBOOL AWaveBankFMODFSB::IsReady() const
{
	return FMUSIC_GetOpenState( (FMUSIC_MODULE*)m_FileHandle ) == 0;
}

AWaveBank::LOADRESULT AWaveBankFMODFSB::Load( TBOOL a_bLocalise )
{
	if ( m_iNumLoads > 0 || m_FileHandle != TNULL )
	{
		m_iNumLoads++;
		return LOADRESULT_OK;
	}

	TString8 strFileName;
	strFileName += SOUNDS_BASE_DIRECTORY;
	strFileName += "FSB/";

	if ( a_bLocalise )
	{
		ALocaleManager::Lang iLangId = ALocaleManager::Lang_UNKNOWN;
		T2Locale* pLocale = T2Locale::GetSingleton();

		if ( pLocale )
			iLangId = pLocale->GetLanguage();

		if ( iLangId == ALocaleManager::Lang_UNKNOWN )
			iLangId = ALocaleManager::Lang_English;

		strFileName += s_pLangSoundDirectories[ iLangId ];
	}

	strFileName += m_strPath.GetString8();
	TFileManager* pFileManager = TFileManager::GetSingleton();

	TNativeFileInfo oFileInfo;
	TBOOL bGotFileInfo = pFileManager->GetFileInfo( strFileName, oFileInfo );

	TASSERT( TTRUE == bGotFileInfo );

	if ( bGotFileInfo )
	{
		// Load the FSB file
		g_bIsLoadingFSBFile = TTRUE;

		FMUSIC_MODULE* pModule = FMUSIC_LoadSongEx( oFileInfo.InternalPath, oFileInfo.Offset, oFileInfo.Length, FSOUND_NONBLOCKING, 0, 0 );
		TVALIDPTR( pModule );

		TINT iOpenState = FMUSIC_GetOpenState( pModule );
		while ( iOpenState == -2 )
		{
			ThreadSleep( 5 );
			iOpenState = FMUSIC_GetOpenState( pModule );
		}

		g_bIsLoadingFSBFile = TFALSE;

		// The file is loaded now, get some info about the samples

		if ( pModule != TNULL )
		{
			m_iNumLoads += 1;
			m_FileHandle = pModule;

			TINT iNumSamples = FMUSIC_GetNumSamples( pModule );

			for ( TINT i = 0; i < iNumSamples; i++ )
			{
				FSOUND_SAMPLE* pSample = FMUSIC_GetSample( pModule, i );

				AWave* pWave = &m_pWaves[ i ];
				pWave->uiFlags |= ( 4 | 8 );
				pWave->pSampleHandle = pSample;
				pWave->iIndex = i;
				
				TINT iDeffreq;
				FSOUND_Sample_GetDefaults( pSample, &iDeffreq, TNULL, TNULL, TNULL );

				pWave->uiFrequency = TUINT( iDeffreq );
			}

			return LOADRESULT_OK;
		}
	}

	return LOADRESULT_ERR;
}

AWaveBank::UNLOADRESULT AWaveBankFMODFSB::Unload()
{
	if ( m_iNumLoads != 0 && --m_iNumLoads < 1 && m_FileHandle )
	{
		m_iNumLoads = 0;

		// Reset the waves
		for ( TINT i = 0; i < m_iNumWaves; i++ )
		{
			AWave* pWave = &m_pWaves[ i ];
			pWave->pSampleHandle = TNULL;
			pWave->iIndex = -1;
			pWave->uiFlags &= ~( 4 | 8 );
		}

		FMUSIC_MODULE* pModule = (FMUSIC_MODULE*)m_FileHandle;

		TINT iOpenState = FMUSIC_GetOpenState( pModule );
		while ( iOpenState == -2 )
		{
			ThreadSleep( 5 );
			iOpenState = FMUSIC_GetOpenState( pModule );
		}

		FMUSIC_FreeSong( pModule );
		m_FileHandle = TNULL;
	}

	return UNLOADRESULT_OK;
}
