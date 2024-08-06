#include "pch.h"
#include "ASoundManager.h"
#include "Assets/AAssetLoader.h"
#include "AWaveBankFMODFSBStream.h"

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

AWaveBank* ASoundManager::FindWaveBank( const TPString8& a_rName )
{
	auto pFoundNode = ms_WaveBanks.FindNode( a_rName );

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
		return new AWaveBankFMODFSBStream( a_strBank, a_strPath );
	}
	else if ( a_strType == TPString8( "Stream" ) )
	{
		// Load from a stream
		TASSERT(!"Not supported yet")
	}

	return TNULL;
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

TBOOL ASoundManager::LoadWaveBanks(const TCHAR* a_szFileName)
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
