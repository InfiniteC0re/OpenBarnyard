#include "pch.h"
#include "ASoundManager.h"
#include "ASound.h"
#include "Assets/AAssetLoader.h"
#include "AWaveBankFMODFSB.h"
#include "AWaveBankFMODFSBStream.h"
#include "ALoadScreen.h"

#include <Plugins/PPropertyParser/PBProperties.h>

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ASoundManager);

ASoundManager::ASoundManager()
{
	TIMPLEMENT();
	m_bUseMinHardwareChannels = TTRUE;
	m_iMinHWChannels = 32;
	m_iNumChannels = 32;

	ms_pFileSystem = TFileManager::GetSingleton()->FindFileSystem("local");
}

ASoundManager::~ASoundManager()
{
	TIMPLEMENT();

	m_FreeListS2.Clear();

	FSOUND_Close();
	m_PauseListener.Disconnect();
	Reset();
}

TBOOL ASoundManager::Reset()
{
	TIMPLEMENT();
	return TTRUE;
}

TBOOL ASoundManager::OnCreate()
{
	Initialise();
	m_PauseListener.Connect(
		g_oSystemManager.GetPauseEmitter(),
		this,
		[](ASoundManager* a_pSndMngr, TSystemManager* a_pSysMngr, TBOOL* a_pPaused) {
			a_pSndMngr->PauseAllSound(*a_pPaused);
			return TTRUE;
		}
	, 0);

	m_pS4 = new S4[32];

	for (TINT i = 0; i < 32; i++)
	{
		m_FreeListS4.PushBack(&m_pS4[i]);
	}

	for (TINT i = 0; i < TARRAYSIZE(m_aS2); i++)
	{
		m_FreeListS2.PushBack(&m_aS2[i]);
	}

	return TTRUE;
}

TBOOL ASoundManager::OnUpdate(TFLOAT a_fDeltaTime)
{
	TIMPLEMENT();
	m_fTotalTime += a_fDeltaTime;

	FSOUND_Update();
	return TTRUE;
}

void ASoundManager::OnDestroy()
{
	m_UnkList1.Clear();
	m_FreeListS4.Clear();
	delete[] m_pS4;
	m_PauseListener.Disconnect();
}

AWaveBank* ASoundManager::FindWaveBank( const TPString8& a_rcName )
{
	auto pFoundNode = ms_WaveBanks.FindNode( a_rcName );

	return ( pFoundNode != ms_WaveBanks.End() ) ? pFoundNode->GetValue()->GetSecond() : TNULL;
}

AWaveBank* ASoundManager::LoadWaveBankFromAsset( const Toshi::TString8& a_strName, TUINT a_uiForcedFlags )
{
	// Generate bank file name
	TString8 strBankFileName = "Bank_";
	strBankFileName += a_strName;
	strBankFileName.MakeLower();

	TPString8 pStrBank( "Bank" );

	// Get PProperties symbol of the currently loaded sound asset file
	const PBProperties* pProperties =
		AAssetLoader::CastSymbol<const PBProperties>( strBankFileName, PBProperties::TRB_SECTION_NAME, AAssetType_WaveBank );
	TVALIDPTR( pProperties );

	const PBProperties* pBankProperties = pProperties->Begin()->GetValue()->GetProperties();

	// Get the bank name
	TPString8 strWaveWankBankName;
	pBankProperties->GetOptionalStringProperty( strWaveWankBankName, "name" );

	TINT iNameCmpRslt = strWaveWankBankName.GetString8().Compare( a_strName, -1 );
	auto pExistingWaveBank = ms_WaveBanks.FindNode( strWaveWankBankName );
	auto pWaveBankVersionVal = pBankProperties->GetOptionalProperty( "version" );

	// Store wavebank library
	auto pWaveBankLibraryVal = pBankProperties->GetOptionalProperty( "library" );
	TPString8 strWaveBankLibrary = ( pWaveBankLibraryVal ) ? pWaveBankLibraryVal->GetTPString8() : TNULL;

	// Store wavebank type
	auto pWaveBankTypeVal = pBankProperties->GetOptionalProperty( "type" );
	TPString8 strWaveBankType = ( pWaveBankTypeVal ) ? pWaveBankTypeVal->GetTPString8() : TNULL;

	// Store wavebank path
	auto pWaveBankPathVal = pBankProperties->GetOptionalProperty( "path" );
	TPString8 strWaveBankPath = ( pWaveBankPathVal ) ? pWaveBankPathVal->GetTPString8() : TNULL;

	// Store wavebank extension
	auto pWaveBankExtensionVal = pBankProperties->GetOptionalProperty( "extension" );
	TPString8 strWaveBankExtension = ( pWaveBankExtensionVal ) ? pWaveBankExtensionVal->GetTPString8() : TNULL;

	// Create the actual wavebank from the parameters
	AWaveBank* pWaveBank = AllocateWaveBank( strWaveWankBankName, strWaveBankLibrary, strWaveBankType, strWaveBankPath );
	
	pWaveBank->ParseWavesData( pBankProperties, a_uiForcedFlags );

	return pWaveBank;
}

AWaveBank* ASoundManager::AllocateWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strLibrary, const Toshi::TPString8& a_strType, const Toshi::TPString8& a_strPath )
{
	// Check if we are loading the supported format
	if ( a_strLibrary != TPString8( "FMOD" ) || a_strType == TPString8( "Dir" ) )
	{
		TASSERT( !"Trying to load an unknown non-FSB WaveBank format!" );
		return TNULL;
	}

	if ( a_strType == TPString8( "File" ) )
	{
		// Load from a file
		return new AWaveBankFMODFSB( a_strBank, a_strPath );
	}
	else if ( a_strType == TPString8( "Stream" ) )
	{
		// Load from a stream
		return new AWaveBankFMODFSBStream( a_strBank, a_strPath );
	}

	return TNULL;
}

TBOOL ASoundManager::LoadSoundBankImpl( const TCHAR* a_szName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately )
{
	// TEMPORARY HACK: disable advanced sound since it's not implemented
	a_bSimpleSound = TTRUE;

	g_oLoadScreen.Update();

	TString8 strFileName = SOUNDS_BASE_DIRECTORY;
	strFileName += "soundbank_";
	strFileName += a_szName;
	strFileName += ".trb";

	TTRB trb;
	TTRB::ERROR eTrbLoadResult = trb.Load( strFileName );

	if ( eTrbLoadResult != TTRB::ERROR_OK )
	{
		TERROR( "Unable to load soundbank: %s\n", strFileName.GetString() );
		return TFALSE;
	}
	
	auto& rcProperties = *PBProperties::LoadFromTRB( trb );

	// Delete symbol table for some small optimisation
	trb.DeleteSymbolTable();

	TBOOL bNoErrors = TTRUE;

	T2_FOREACH( rcProperties, soundbank )
	{
		const PBProperties* pSoundsProperties = soundbank->GetValue()->GetProperties();

		TINT iNumSounds;
		TBOOL bHasNumSounds = pSoundsProperties->GetOptionalPropertyValue( iNumSounds, "numsounds" );
	
		if ( !bHasNumSounds )
		{
			TASSERT( !"Invalid format of the soundbank property" );
			return TFALSE;
		}

		if ( iNumSounds == 0 )
		{
			return TTRUE;
		}

		// Allocate soundbank
		ASoundBank* pSoundBank = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASoundBank( iNumSounds );
		m_SoundBanks.PushBack( pSoundBank );

		if ( a_bSimpleSound )
		{
			pSoundBank->m_pSounds = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASound[ iNumSounds ];
		}
		else
		{
			TASSERT( !"ASoundEx is not supported" );
		}

		pSoundsProperties->GetOptionalStringProperty( pSoundBank->m_strName, "name" );

		TINT iLocalised;
		pSoundsProperties->GetOptionalPropertyValue( iLocalised, "localised" );

		TINT iSoundIndex = 0;
		T2_FOREACH( *pSoundsProperties, it )
		{
			// Skip anything that doesn't even look like 'Sound' property
			if ( it->GetName().GetString()[ 0 ] != 'S' )
				continue;

			auto pSoundProperties = it->GetValue()->GetProperties();

			TINT iSoundId;
			pSoundProperties->GetOptionalPropertyValue( iSoundId, "id" );

			if ( a_bSimpleSound )
			{
				ASound* pSound = &pSoundBank->m_pSounds[ iSoundIndex ];

				pSound->m_iId = iSoundId;
				m_SoundIdToSound.Insert( iSoundId, pSound );

				if ( !pSoundProperties->GetOptionalPropertyValue( pSound->m_iFlags, "flags" ) )
				{
					bNoErrors = TFALSE;
					TASSERT( !"No flags property specified in the soundbank" );
				}

				PBPropertyValueArray* pBanks = TNULL;
				if ( !pSoundProperties->GetOptionalPropertyValue( pBanks, "banks" ) )
				{
					bNoErrors = TFALSE;
					TASSERT( !"No banks property specified in the soundbank" );
					continue;
				}

				PBPropertyValueArray* pWaves;
				if ( bNoErrors && pSoundProperties->GetOptionalPropertyValue( pWaves, "waves" ) )
				{
					TASSERT( pBanks->GetSize() >= 1 );
					TASSERT( pWaves->GetSize() >= 1 );

					TPString8 strSoundBankName = pBanks->GetValue( 0 )->GetTPString8();
					AWaveBank* pWaveBank = FindWaveBank( strSoundBankName );

					if ( a_bLoadImmediately )
					{
						LoadWaveBankSamples( strSoundBankName, iLocalised != 0 ? AWaveBank::LOADFLAGS_LOCALISE : AWaveBank::LOADFLAGS_NONE, -1 );
					}

					pSound->m_pWaveBank = pWaveBank;
					pSound->m_iWaveId = pWaves->GetValue( 0 )->GetInteger();
				}
			}
			else
			{
				TASSERT( !"Not supported" );
			}

			iSoundIndex++;
		}

		if ( bNoErrors )
		{
			if ( a_bLoadImmediately )
				pSoundBank->Load();
		}
		else
		{
			pSoundBank->Remove();
			delete pSoundBank;
		}
	}

	TUINT uiCurrentAlloced;
	TUINT uiMaxAlloced;
	FSOUND_GetMemoryStats( &uiCurrentAlloced, &uiMaxAlloced );
	TINFO( "LoadSoundBank result: name=%s; success=%d; currentalloced=%u; maxalloced=%u\n", a_szName, bNoErrors, uiCurrentAlloced, uiMaxAlloced );

	return bNoErrors;
}

TBOOL ASoundManager::Initialise()
{
	FSOUND_Init(44100, m_iMinHWChannels + m_iNumChannels, 0);

	TINT num2DC, num3DC, numDC;
	FSOUND_GetNumHWChannels(&num2DC, &num3DC, &numDC);
	FSOUND_GetMaxChannels();
	FSOUND_Close();

	TBOOL bRes = FSOUND_SetMinHardwareChannels(m_bUseMinHardwareChannels ? m_iMinHWChannels : 0);
	FSOUND_Init(44100, m_iNumChannels, 0);
	FSOUND_GetNumHWChannels(&num2DC, &num3DC, &numDC);
	FSOUND_GetMaxChannels();
	FSOUND_Stream_SetBufferSize(2000);
	FSOUND_SetBufferSize(100);
	FSOUND_3D_SetDistanceFactor(1.0f);
	FSOUND_3D_SetRolloffFactor(1.0f);
	FSOUND_3D_SetDopplerFactor(1.0f);

	return bRes;
}

void ASoundManager::PauseAllSound(TBOOL a_bPaused)
{
	TIMPLEMENT();
}

TBOOL ASoundManager::LoadWaveBanksInfo(const TCHAR* a_szFileName)
{
	TBOOL bOpened = AAssetLoader::Load(
		"Data/Assets/lib_wavebank.trb",
		AAssetType_WaveBank,
		TTRUE
	);

	if (!bOpened) return TFALSE;

	const PBProperties* pProperties = 
		AAssetLoader::CastSymbol<const PBProperties>( a_szFileName, PBProperties::TRB_SECTION_NAME, AAssetType_WaveBank );
	TVALIDPTR( pProperties );

	auto pWaveBanksVal = pProperties->GetOptionalProperty( "Wavebanks" );
	TVALIDPTR( pWaveBanksVal );

	auto pWaveBanks = pWaveBanksVal->GetArray();

	for ( TUINT i = 0; i < pWaveBanks->GetSize(); i++ )
	{
		TPString8 strWaveBankName = pWaveBanks->GetValue( i )->GetTPString8();

		if ( FindWaveBank( strWaveBankName ) == TNULL )
		{
			LoadWaveBankFromAsset( strWaveBankName.GetString(), 0 );
		}
	}

	auto pCategoriesVal = pProperties->GetOptionalProperty( "Categories" );
	TVALIDPTR( pCategoriesVal );

	auto pCategories = pCategoriesVal->GetArray();

	for ( TUINT i = 0; i < pCategories->GetSize(); i++ )
	{
		TPString8 strCategoryName = pCategories->GetValue( i )->GetTPString8();

		m_CategoryIndices.Insert( strCategoryName, m_CategoryIndices.Size() );
	}

	AAssetLoader::Close( AAssetType_WaveBank );

	return TTRUE;
}

TBOOL ASoundManager::LoadWaveBankSamples( const Toshi::TPString8& a_rcName, AWaveBank::LOADFLAGS a_eLoadFlags, TINT a_iBufferSize )
{
	if ( a_rcName.GetPooledString() && !a_rcName.GetPooledString()->GetString8().IsEmpty() )
	{
		AWaveBank* pWaveBank = FindWaveBank( a_rcName );

		if ( pWaveBank )
		{
			return pWaveBank->Load( a_eLoadFlags, a_iBufferSize ) == AWaveBank::LOADRESULT_OK;
		}
	}

	return TFALSE;
}

ASoundBank* ASoundManager::FindSoundBank( const Toshi::TPString8& a_rcName )
{
	T2_FOREACH( m_SoundBanks, it )
	{
		if ( it->m_strName == a_rcName )
			return it;
	}

	return TNULL;
}

TBOOL ASoundManager::LoadSoundBank( const Toshi::TPString8& a_rcName, TBOOL a_bSoundEx, TBOOL a_bLoadImmediately )
{
	if ( a_rcName.GetPooledString() && !a_rcName.GetPooledString()->GetString8().IsEmpty() )
	{
		if ( FindSoundBank( a_rcName ) == TNULL )
		{
			return LoadSoundBankImpl( a_rcName, a_bSoundEx, a_bLoadImmediately );
		}
	}

	return TFALSE;
}

void ASoundManager::LoadSoundBankSamples( const Toshi::TPString8& a_rcName )
{
	if ( ASoundBank* pSoundBank = FindSoundBank( a_rcName ) )
	{
		pSoundBank->Load();
	}
}
