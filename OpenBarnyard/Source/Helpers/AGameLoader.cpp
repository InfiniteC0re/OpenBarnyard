#include "pch.h"
#include "AGameLoader.h"
#include "ALoadScreen.h"
#include "Tasks/ARootTask.h"
#include "Cameras/ACameraManager.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

//-----------------------------------------------------------------------------
// GameState Management
//-----------------------------------------------------------------------------
static TINT          s_iCurrMiniGame       = -1;
static TINT          s_iNextMiniGame       = -1;
static TBOOL         s_bLoadingMiniGame    = TFALSE;
static const TClass* s_pPrevGameStateClass = TNULL;

//-----------------------------------------------------------------------------
// Terrain Management
//-----------------------------------------------------------------------------
static TINT                 s_iTerrainVISIndex = -1;
static TBOOL                s_bChangingTerrain = TFALSE;
static ATerrainInterface*   s_pCurrentTerrain  = TNULL;
static AGameLoader::Terrain s_eCurrentLevel    = AGameLoader::Terrain_EnvBeadyFarm;

static TBOOL s_bLastTransitionUpdateResult;


// $Barnyard: FUNCTION 00426370
static TBOOL LoadMiniGameImpl()
{
	TIMPLEMENT();

	AGameStateController::GetSingleton()->ResetStack();
	g_oLoadScreen.Update();

	ASoundManager::GetSingleton()->Reset();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	// TODO: Unload assets...

	// TODO: Need to track whether lib_frontend is even loaded or not
	AAssetLoader::DestroyAssetPackOfLibrary( "lib_frontend" );
	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	return TFALSE;
}

// $Barnyard: FUNCTION 00427e70
TBOOL AGameLoader::UpdateTransitionTask()
{
	TIMPLEMENT();

	if ( s_bLoadingMiniGame )
	{
		s_bLastTransitionUpdateResult = LoadMiniGameImpl();

		if ( s_bLastTransitionUpdateResult )
			s_bLoadingMiniGame = TFALSE;
	}

	return s_bLastTransitionUpdateResult;
}

// $Barnyard: FUNCTION 004239d0
void AGameLoader::SetTransitionToMiniGame( TINT a_iMiniGame, TBOOL a_bStartLoading )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( a_bStartLoading )
	{
		g_oLoadScreen.StartLoading( 100, TTRUE );

		s_pPrevGameStateClass = pController->GetCurrentState()->GetClass();
		ACameraManager::GetSingleton()->DetachCameraHelpers();
		ACameraManager::GetSingleton()->FUN_0045c290();

		ARootTask::GetSingleton()->SetTransitioning( TTRUE );

		s_bLoadingMiniGame = TTRUE;
		s_iCurrMiniGame    = a_iMiniGame;

		AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
		APlayerManager::GetSingleton()->ResetPlayerIterator();
	}
	else
	{
		pController->SetIgnoreInput( TTRUE );
		pController->SetTransitionToMiniGame( TTRUE );
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );

		s_iNextMiniGame = a_iMiniGame;
	}
}

TINT AGameLoader::GetCurrMiniGame()
{
	return s_iCurrMiniGame;
}

TINT AGameLoader::GetNextMiniGame()
{
	return s_iNextMiniGame;
}

TBOOL AGameLoader::IsChangingTerrain()
{
	return s_bChangingTerrain;
}

// $Barnyard: FUNCTION 004232d0
TINT AGameLoader::GetTerrainVIS()
{
	return s_iTerrainVISIndex;
}

// $Barnyard: FUNCTION 006c1e80
void AGameLoader::Debug_VerifyModelsInAssetPack( Toshi::TTRB* a_pTRB )
{
	// This function does nothing, but it was doing something in debug build, I guess
	if ( !a_pTRB ) return;

	TCHAR      szBuffer[ 256 ];
	const TINT iSkeletonStrLength = TStringManager::String8Length( "_Skeleton" );

	for ( TINT i = 0; i < a_pTRB->GetNumSymbols(); i++ )
	{
		TTRB::TTRBSymbol* pSymbol           = a_pTRB->GetSymbol( i );
		const TCHAR*      pchSymbolName     = a_pTRB->GetSymbolName( pSymbol );
		const TINT        iSymbolNameLength = TStringManager::String8Length( a_pTRB->GetSymbolName( pSymbol ) );

		// Check string postfix is _Skeleton
		if ( iSymbolNameLength > iSkeletonStrLength )
		{
			const TINT iPostFixPosition = ( iSymbolNameLength - iSkeletonStrLength );

			if ( 0 == Toshi::TStringManager::String8Compare( pchSymbolName + iPostFixPosition, "_Skeleton" ) )
			{
				const TINT iModelNameSize = iPostFixPosition + 1;
				TStringManager::String8Copy( szBuffer, pchSymbolName, iModelNameSize );
				szBuffer[ iModelNameSize ] = '\0';
				
				T2_FOREACH( TModelManager::ms_oUsedList, it )
				{
					TModel* pModel = it->GetModel();

					if ( pModel->m_bIsAssetFile &&
					     pModel->m_szSymbolPrefixLength == iModelNameSize &&
					     TStringManager::String8Compare( pModel->m_szSymbolPrefix, szBuffer, iModelNameSize ) == 0 )
					{
						break;
					}
				}

				// AND YES IT DOES NOTHING AFTER ALL
			}
		}
	}

	// This whole method is genius omg
	T2_FOREACH( TModelManager::ms_oUsedList, it )
	{
#ifdef TOSHI_DEBUG
		// something goes here...?
#endif
	}
}

// $Barnyard: FUNCTION 00423a80
void AGameLoader::Terrain_Set( TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup )
{
	TIMPLEMENT();

	if ( a_eTerrain == s_eCurrentLevel ) return;

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );

	if ( s_pCurrentTerrain )
	{
		delete s_pCurrentTerrain;
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( a_eTerrain == Terrain_FrontEnd )
	{
		a_iStartVISGroup = 0;
	}

	s_pCurrentTerrain = new ATerrainInterface(
	    a_iUnused1,
	    a_iUnused2,
	    a_iPreloadTerrainBlockSize < 1 ? 0x2200000 : a_iPreloadTerrainBlockSize,
	    a_iStartVISGroup
	);

	T2FormatString128 terrainFile;
	terrainFile.Format( "Data\\Terrain\\%s\\%s.trb", GetTerrainName( a_eTerrain ), GetTerrainName( a_eTerrain ) );

	s_pCurrentTerrain->LoadFromFile( terrainFile.Get(), TTRUE, a_bPersistantCollision );
	s_eCurrentLevel = a_eTerrain;

	if ( !a_bLoadLater ) Terrain_Load();

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
}

// $Barnyard: FUNCTION 00423850
void AGameLoader::Terrain_Load()
{
	TPROFILER_SCOPE();

	auto pTerrain         = ATerrainInterface::GetSingleton();
	pTerrain->m_bIsLoaded = TFALSE;

	if ( AGameLoader::IsChangingTerrain() )
		pTerrain->m_fnGetCurrentVISGroup = AGameLoader::GetTerrainVIS;

	pTerrain->Update();
	while ( !pTerrain->IsLoaded() )
	{
		TRenderInterface::GetSingleton()->Update( 1.0f / 1000.0f );
		AAssetStreaming::GetSingleton()->Update();

#ifndef BARNYARD_COMMUNITY_PATCH
		ThreadSleep( 20 );
#endif // BARNYARD_COMMUNITY_PATCH

		pTerrain->Update();
		g_oLoadScreen.Update();
	}

	pTerrain->m_bIsLoaded = TTRUE;

	if ( AGameLoader::IsChangingTerrain() )
		pTerrain->m_fnGetCurrentVISGroup = ATerrainInterface::GetSectionPlayerIsIn;
}
