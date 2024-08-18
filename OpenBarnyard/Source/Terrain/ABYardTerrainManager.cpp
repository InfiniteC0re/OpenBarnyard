#include "pch.h"
#include "ABYardTerrainManager.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetStreaming.h"
#include "ALoadScreen.h"

#include <Toshi/T2String.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void ABYardTerrainManager::SetTerrain( TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup )
{
	TIMPLEMENT();

	if ( a_eTerrain == ms_eCurrentLevel ) return;

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );

	if ( ms_pCurrentTerrain )
	{
		delete ms_pCurrentTerrain;
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( a_eTerrain == Terrain_FrontEnd )
	{
		a_iStartVISGroup = 0;
	}

	ms_pCurrentTerrain = new ATerrainInterface(
	    a_iUnused1,
	    a_iUnused2,
	    a_iPreloadTerrainBlockSize < 1 ? 0x2200000 : a_iPreloadTerrainBlockSize,
	    a_iStartVISGroup );

	T2FormatString128 terrainFile;
	terrainFile.Format( "Data\\Terrain\\%s\\%s.trb", GetTerrainName( a_eTerrain ), GetTerrainName( a_eTerrain ) );

	ms_pCurrentTerrain->LoadFromFile( terrainFile.Get(), TTRUE, a_bPersistantCollision );
	ms_eCurrentLevel = a_eTerrain;

	if ( !a_bLoadLater )
	{
		StartLoading();
	}

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
}

void ABYardTerrainManager::StartLoading()
{
	TPROFILER_SCOPE();

	auto pTerrain         = ATerrainInterface::GetSingleton();
	pTerrain->m_bIsLoaded = TFALSE;

	if ( ATerrainInterface::ms_bAutoVIS )
	{
		pTerrain->m_fnGetCurrentVISGroup = ATerrainInterface::GetPersistantSectionID;
	}

	pTerrain->Update();

	while ( !pTerrain->IsLoaded() )
	{
		TRenderInterface::GetSingleton()->Update( 1.0f / 1000.0f );
		AAssetStreaming::GetSingleton()->Update();
		ThreadSleep( 20 );

		pTerrain->Update();
		g_oLoadScreen.Update();
	}

	pTerrain->m_bIsLoaded = TTRUE;

	if ( ATerrainInterface::ms_bAutoVIS )
	{
		pTerrain->m_fnGetCurrentVISGroup = ATerrainInterface::GetCurrentSectionID;
	}
}
