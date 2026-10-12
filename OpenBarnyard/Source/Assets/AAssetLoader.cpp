#include "pch.h"
#include "AAssetLoader.h"
#include "GUI/ASubtitleManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Helpers/ASimPersistData.h"
#include "GameInterface/AHackedQuickSave.h"
#include "Sound/AVOStream.h"
#include "AAssetStreaming.h"
#include "AMaterialLibraryManager.h"
#include "AKeyFrameLibraryManager.h"
#include "ATRBLoaderJob.h"
#include "ALoadScreen.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Player/APlayerProfileManager.h"
#include "Helpers/ABikeManager.h"
#include "Physics/AVehicleSkidManager.h"
#include "Sound/ASoundManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleEffectLocatorManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

#include <Toshi/TString8.h>
#include <File/TTRB.h>
#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Names of the player's costumes, used to build the name of the player's material library (0x007727d8)
static const TCHAR* s_aCostumeNames[] = {
	"american",
	"angus",
	"beefmstr",
	"spaniard",
	"longhorn",
	"brahman",
	"holsten",
};

// Returns the day phase stored by the quick save (DAT_007810fc), 4 means night
static TINT GetSavedDayPhase()
{
	return AHackedQuickSave::GetSavedDayPhase();
}

// Returns the current act of the story (+0x64 of the singleton at DAT_00783e64)
static TINT GetCurrentMainGameAct()
{
	return ASimPersistData::GetSingleton()->GetAct();
}

// NOTE: retail inlines this into LoadAssetPackOfLibrary, DestroyAssetPackOfLibrary and CreateAssetsFromLibrary
void AAssetLoader::GetLibraryFileName( TString8& a_rOutLibName, TString8& a_rOutFileName, const TCHAR* a_szLibraryName, TINT a_iMainGameAct )
{
	TVALIDPTR( a_szLibraryName );

	a_rOutLibName = a_szLibraryName;

	if ( TStringManager::String8CompareNoCase( a_szLibraryName, "ABarnExploreMinigameState" ) == 0 && GetSavedDayPhase() == AGameTimeManager::DAYPHASE_NIGHT )
	{
		// Night version of the barn
		a_rOutLibName.Format( "%s-n", a_szLibraryName );
	}
	else if ( TStringManager::String8CompareNoCase( a_szLibraryName, "lib_maingame" ) == 0 && a_iMainGameAct > 1 )
	{
		// Library of the current act
		a_rOutLibName.Format( "%s_a%d", a_szLibraryName, a_iMainGameAct );
	}

	a_rOutFileName.Format( "Data/%s.trb", a_rOutLibName.GetString() );
}

// $Barnyard: FUNCTION 00425600
TBOOL AAssetLoader::LoadAssetPackOfLibrary( const TCHAR* a_szLibraryName, TBOOL a_bStream )
{
	TPROFILER_SCOPE();

	TString8 strLibName;
	TString8 strLibFileName;
	GetLibraryFileName( strLibName, strLibFileName, a_szLibraryName, GetCurrentMainGameAct() );

	TTRB libTrb;

	if ( libTrb.Load( strLibFileName ) == TTRB::ERROR_OK )
	{
		const PBProperties* pProperties = PBProperties::LoadFromTRB( libTrb );

		// Load asset pack
		if ( const PBPropertyValue* pModelLib = pProperties->GetProperty( "modellib" ) )
			Load( pModelLib->GetString(), AAssetType_AssetPack, a_bStream );

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00424cf0
// $BarnyardWiiJP: FUNCTION 8012ce08
TBOOL AAssetLoader::DestroyAssetPackOfLibrary( const TCHAR* a_szLibraryName )
{
	TPROFILER_SCOPE();

	if ( AVehicleSkidManager::IsSingletonCreated() )
		AVehicleSkidManager::GetSingleton()->ReleaseTyreTrackTexture();

	if ( ABikeManager::IsSingletonCreated() )
	{
		if ( ABikeManager::GetSingleton()->GetLoadedBikeType() != -1 )
			ABikeManager::GetSingleton()->DestroyBikeAssets();

		ABikeManager::GetSingleton()->Disable();
	}

	TString8 strLibName;
	TString8 strLibFileName;

#ifdef BARNYARD_LATEST
	// Wii: the time of the day was changed by the story while the barn of the night was loaded (AQuestManager::SetQuestTier)
	if ( TStringManager::String8CompareNoCase( a_szLibraryName, "ABarnExploreMinigameState" ) == 0 && GetSavedDayPhase() == AHackedQuickSave::SAVEDDAYPHASE_NIGHT_BARN )
	{
		strLibName.Format( "%s-n", a_szLibraryName );
		strLibFileName.Format( "Data/%s.trb", strLibName.GetString() );
		AHackedQuickSave::SetSavedDayPhase( 6 );
	}
	else
#endif // BARNYARD_LATEST
		GetLibraryFileName( strLibName, strLibFileName, a_szLibraryName, ms_iMainGameAct );

	// The variant of lib_maingame is no longer loaded
	if ( TStringManager::String8CompareNoCase( a_szLibraryName, "lib_maingame" ) == 0 && TStringManager::String8CompareNoCase( strLibName.GetString(), a_szLibraryName ) != 0 )
		ms_iMainGameAct = -1;

	TTRB libTrb;

	if ( libTrb.Load( strLibFileName ) == TTRB::ERROR_OK )
	{
		const PBProperties* pProperties = PBProperties::LoadFromTRB( libTrb );

		// Unload animatable object types
		const PBPropertyValue* pAnimObjTypes = pProperties->GetProperty( "AnimObjTypes" );
		if ( pAnimObjTypes && AAnimatableObjectManager::IsSingletonCreated() )
		{
			AAnimatableObjectManager::GetSingleton()->UnloadTypesFromProperties( pAnimObjTypes );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Unload keyframe libraries
		const PBPropertyValue* pKeylibs = pProperties->GetProperty( "keylib" );
		if ( pKeylibs && AKeyFrameLibraryManager::IsSingletonCreated() )
		{
			AKeyFrameLibraryManager::GetSingleton()->UnloadLibrariesFromProperties( pKeylibs );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Unload material libraries
		const PBPropertyValue* pMatlibs = pProperties->GetProperty( "matlib" );
		if ( pMatlibs && AMaterialLibraryManager::IsSingletonCreated() )
		{
			AMaterialLibraryManager::GetSingleton()->UnloadLibrariesFromProperties( pMatlibs, TTRUE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Unload sound banks
		const PBPropertyValue* pSoundBanks = pProperties->GetProperty( "soundbanks" );
		if ( pSoundBanks && ASoundManager::IsSingletonCreated() )
		{
			ASoundManager::GetSingleton()->UnloadSoundBanksFromProperty( pSoundBanks );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Unload ambient sounds
		const PBPropertyValue* pAmbientSounds = pProperties->GetProperty( "ambientsoundbanks" );
		if ( pAmbientSounds && APositionalSoundSet::IsSingletonCreated() )
		{
			// NOTE: the original passes TNULL (the parameter isn't used)
			APositionalSoundSet::GetSingleton()->UnloadFromProperty( TNULL );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Unload assets of the player character
		if ( const PBPropertyValue* pPlayerCharacter = pProperties->GetProperty( "playercharacter" ) )
			UnloadPlayerCharacter( pPlayerCharacter );

		// Unload particle library
		if ( pProperties->GetProperty( "ParticleLibrary" ) )
		{
			AParticleEffectManager::UnloadParticleLibrary();
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		if ( pProperties->GetProperty( "VOStream" ) && AVOStream::IsSingletonCreated() )
		{
			AVOStream::GetSingleton()->Close( TTRUE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		if ( pProperties->GetProperty( "Subs" ) && ASubtitleManager::IsSingletonCreated() )
		{
			ASubtitleManager::GetSingleton()->UnloadSubtitles();
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		Close( AAssetType_AssetPack );
		g_oLoadScreen.Update( 1.0f, TTRUE );

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00425060
TBOOL AAssetLoader::CreateAssetsFromLibrary( const TCHAR* a_szLibraryName )
{
	TPROFILER_SCOPE();

	TINT iMainGameAct = GetCurrentMainGameAct();

	TString8 strLibName;
	TString8 strLibFileName;
	GetLibraryFileName( strLibName, strLibFileName, a_szLibraryName, iMainGameAct );

	// Remember the variant of lib_maingame, DestroyAssetPackOfLibrary needs it
	if ( TStringManager::String8CompareNoCase( a_szLibraryName, "lib_maingame" ) == 0 && TStringManager::String8CompareNoCase( strLibName.GetString(), a_szLibraryName ) != 0 )
		ms_iMainGameAct = iMainGameAct;

	TTRB libTrb;

	if ( libTrb.Load( strLibFileName ) == TTRB::ERROR_OK )
	{
		const PBProperties* pProperties = PBProperties::LoadFromTRB( libTrb );

		// Load particle library
		if ( const PBPropertyValue* pParticleLibrary = pProperties->GetProperty( "ParticleLibrary" ) )
		{
			AParticleEffectManager::UnloadParticleLibrary();

			AParticleEffectManager::GetSingleton()->GetParticleSystem()->LoadLibrary( pParticleLibrary->GetString() );

			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load material libraries
		if ( const PBPropertyValue* pMatlibs = pProperties->GetProperty( "matlib" ) )
		{
			AMaterialLibraryManager::GetSingleton()->LoadLibrariesFromProperties( pMatlibs, GetAssetTRB( AAssetType_AssetPack ), TTRUE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load keyframe libraries
		if ( const PBPropertyValue* pKeylibs = pProperties->GetProperty( "keylib" ) )
		{
			AKeyFrameLibraryManager::GetSingleton()->LoadLibrariesFromProperties( pKeylibs, GetAssetTRB( AAssetType_AssetPack ) );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load animatable object types (Data/ModelTypes/Libs/<library>.trb)
		if ( pProperties->GetProperty( "AnimObjTypes" ) )
		{
			AAnimatableObjectManager::GetSingleton()->LoadTypesFromLibrary( strLibName.GetString() );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load pathfinding networks
		if ( const PBPropertyValue* pNetworks = pProperties->GetProperty( "AInteriorNetwork" ) )
		{
			AInteriorNetworkManager::GetSingleton()->LoadNetworksFromProperty( pNetworks, TFALSE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load sound banks
		if ( const PBPropertyValue* pSoundBanks = pProperties->GetProperty( "soundbanks" ) )
		{
			ASoundManager::GetSingleton()->LoadSoundBanksFromProperty( pSoundBanks );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load ambient sounds
		if ( const PBPropertyValue* pAmbientSounds = pProperties->GetProperty( "ambientsoundbanks" ) )
		{
			APositionalSoundSet::GetSingleton()->LoadFromProperty( pAmbientSounds );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		// Load assets of the player character (gender and costume dependent)
		if ( const PBPropertyValue* pPlayerCharacter = pProperties->GetProperty( "playercharacter" ) )
		{
			LoadPlayerCharacter( pPlayerCharacter );
		}

		if ( const PBPropertyValue* pVOStream = pProperties->GetProperty( "VOStream" ) )
		{
			AVOStream::GetSingleton()->Open( pVOStream->GetString(), TTRUE, TFALSE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		if ( const PBPropertyValue* pLocatorLists = pProperties->GetProperty( "ParticleEffectLocators" ) )
		{
			AParticleEffectLocatorManager::GetSingleton()->LoadLocatorLists( pLocatorLists );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: the original doesn't check whether the singleton exists
		if ( AVehicleSkidManager::IsSingletonCreated() )
#endif
			AVehicleSkidManager::GetSingleton()->LoadTyreTrackTexture();

		if ( const PBPropertyValue* pSubs = pProperties->GetProperty( "Subs" ) )
		{
			ASubtitleManager::GetSingleton()->LoadSubtitles( pSubs->GetString() );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00424a20
void AAssetLoader::LoadPlayerCharacter( const PBPropertyValue* a_pProperty )
{
	if ( !a_pProperty )
		return;

	const PBProperties*    pProperties         = a_pProperty->GetProperties();
	const PBPropertyValue* pDisableBreakpoints = pProperties->GetProperty( "disablebreakpoints" );
	TBOOL                  bDisableBreakpoints = pDisableBreakpoints && pDisableBreakpoints->GetRaw() != TNULL;
	APlayerProfileManager* pProfileManager     = APlayerProfileManager::GetSingleton();

	const PBProperties* pCharacterProperties;
	TCHAR               chGender;

	switch ( pProfileManager->GetMainGender() )
	{
		case APlayerProfile::GENDER_MALE:
			pCharacterProperties = pProperties->FindProperties( "male" );
			chGender             = 'm';
			break;
		case APlayerProfile::GENDER_FEMALE:
			pCharacterProperties = pProperties->FindProperties( "female" );
			chGender             = 'f';
			break;
		default:
			return;
	}

	ms_iPlayerCostume = pProfileManager->GetMainCostume();

	// Load material library of the player's costume (pmamerican, pfangus...)
	TCHAR szMatlibName[ 16 ];
	TStringManager::String8Format( szMatlibName, sizeof( szMatlibName ), "p%c%s", chGender, s_aCostumeNames[ ms_iPlayerCostume ] );

	AMaterialLibraryManager::GetSingleton()->LoadLibrary( TPString8( szMatlibName ), GetAssetTRB( AAssetType_AssetPack ), TTRUE );
	g_oLoadScreen.Update( 1.0f, TTRUE );

	// Load keyframe libraries
	if ( const PBPropertyValue* pKeylibs = pCharacterProperties->GetProperty( "keylib" ) )
	{
		AKeyFrameLibraryManager::GetSingleton()->LoadLibrariesFromProperties( pKeylibs, GetAssetTRB( AAssetType_AssetPack ) );
		g_oLoadScreen.Update( 1.0f, TTRUE );
	}

	// Load animatable object types, each from Data/ModelTypes/<name>.trb
	if ( const PBPropertyValue* pAnimObjTypes = pCharacterProperties->GetProperty( "AnimObjTypes" ) )
	{
		const PBPropertyValueArray* pTypes = pAnimObjTypes->GetArray();

		for ( TUINT i = 0; i < pTypes->GetSize(); i++ )
		{
			AAnimatableObjectManager::GetSingleton()->LoadAnimObjType( pTypes->GetValue( i )->GetTPString8(), TNULL, bDisableBreakpoints );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 004248a0
void AAssetLoader::UnloadPlayerCharacter( const PBPropertyValue* a_pProperty )
{
	if ( !a_pProperty )
		return;

	const PBProperties* pCharacterProperties;
	TCHAR               chGender;

	switch ( APlayerProfileManager::GetSingleton()->GetMainGender() )
	{
		case APlayerProfile::GENDER_MALE:
			pCharacterProperties = a_pProperty->GetProperties()->FindProperties( "male" );
			chGender             = 'm';
			break;
		case APlayerProfile::GENDER_FEMALE:
			pCharacterProperties = a_pProperty->GetProperties()->FindProperties( "female" );
			chGender             = 'f';
			break;
		default:
			return;
	}

	// Unload animatable object types
	if ( const PBPropertyValue* pAnimObjTypes = pCharacterProperties->GetProperty( "AnimObjTypes" ) )
	{
		AAnimatableObjectManager::GetSingleton()->UnloadTypesFromProperties( pAnimObjTypes );
		g_oLoadScreen.Update( 1.0f, TTRUE );
	}

	// Unload keyframe libraries
	if ( const PBPropertyValue* pKeylibs = pCharacterProperties->GetProperty( "keylib" ) )
	{
		AKeyFrameLibraryManager::GetSingleton()->UnloadLibrariesFromProperties( pKeylibs );
		g_oLoadScreen.Update( 1.0f, TTRUE );
	}

	// Unload material library of the costume that was loaded
	TASSERT( ms_iPlayerCostume >= 0 && ms_iPlayerCostume < TINT( TARRAYSIZE( s_aCostumeNames ) ) );

	TCHAR szMatlibName[ 16 ];
	TStringManager::String8Format( szMatlibName, sizeof( szMatlibName ), "p%c%s", chGender, s_aCostumeNames[ ms_iPlayerCostume ] );

	AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPString8( szMatlibName ), TTRUE );
	ms_iPlayerCostume = -1;

	g_oLoadScreen.Update( 1.0f, TTRUE );
}

// $Barnyard: FUNCTION 004012d0
TBOOL AAssetLoader::Load( const TCHAR* a_szFileName, AAssetType a_eAssetType, TBOOL a_bStream )
{
	TPROFILER_SCOPE();

	if ( !ms_pTRBFiles[ a_eAssetType ] )
	{
		// Create TTRB if it doesn't exist
		ms_pTRBFiles[ a_eAssetType ] = new TTRB;
	}
	else
	{
		ms_pTRBFiles[ a_eAssetType ]->Close();
	}

	if ( a_bStream && AAssetStreaming::IsSingletonCreated() )
	{
		ATRBLoaderJob trbLoaderJob;
		trbLoaderJob.InitJob( ms_pTRBFiles[ a_eAssetType ], a_szFileName );

		AAssetStreaming::GetSingleton()->AddMainThreadJob( &trbLoaderJob );

		while ( AAssetStreaming::GetSingleton()->HasActiveJobs() )
		{
			g_oLoadScreen.Update( 1.0f, TTRUE );
#ifndef BARNYARD_COMMUNITY_PATCH
			ThreadSleep( 5 );
#endif // BARNYARD_COMMUNITY_PATCH
			AAssetStreaming::GetSingleton()->Update();
		}
	}
	else
	{
		auto eError = ms_pTRBFiles[ a_eAssetType ]->Load( a_szFileName );

		if ( eError != TTRB::ERROR_OK )
		{
			delete ms_pTRBFiles[ a_eAssetType ];
			ms_pTRBFiles[ a_eAssetType ] = TNULL;

			return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00401180
void AAssetLoader::Close( AAssetType a_eAssetType )
{
	if ( ms_pTRBFiles[ a_eAssetType ] )
	{
		ms_pTRBFiles[ a_eAssetType ]->Close();
		delete ms_pTRBFiles[ a_eAssetType ];
		ms_pTRBFiles[ a_eAssetType ] = TNULL;
	}
}

// $Barnyard: FUNCTION 004011c0
// $Barnyard: FUNCTION 004012c0
void AAssetLoader::CloseAll()
{
	for ( TINT i = 0; i < AAssetType_NUMOF; i++ )
	{
		if ( ms_pTRBFiles[ i ] )
		{
			ms_pTRBFiles[ i ]->Close();
			delete ms_pTRBFiles[ i ];
			ms_pTRBFiles[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 00401200
void* AAssetLoader::GetSymbolAddress( const TCHAR* a_szFileName, const TCHAR* a_szSymbolName, AAssetType a_eAssetType )
{
	if ( ms_pTRBFiles[ a_eAssetType ] )
	{
		TCHAR szName[ 256 ];
		Toshi::TStringManager::String8Format( szName, sizeof( szName ), "%s_%s", a_szFileName, a_szSymbolName );
		return ms_pTRBFiles[ a_eAssetType ]->GetSymbolAddress( szName );
	}

	return TNULL;
}
