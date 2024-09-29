#include "pch.h"
#include "AGameSystemManager.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Render/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Sound/AMusicManager.h"

#include <Toshi/TScheduler.h>
#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS( AGameSystemManager );

TOSHI_NAMESPACE_USING

TBOOL AGameSystemManager::OnCreate()
{
	TIMPLEMENT();

	// Load keylibs from the startup library
	TString8 startupLibFileName;
	startupLibFileName.Format( "Data/%s.trb", "lib_startup" );

	TTRB startupLibTrb;
	startupLibTrb.Load( startupLibFileName );
	auto pProperties     = PBProperties::LoadFromTRB( startupLibTrb );
	auto pKeylibProperty = pProperties->GetProperty( "keylib" );

	if ( pKeylibProperty )
	{
		AKeyFrameLibraryManager::GetSingleton()->LoadLibrariesFromProperties(
		    pKeylibProperty,
		    &startupLibTrb
		);
	}

	TGetClass( AModelRepos ).CreateObject();

	auto pScheduler = g_oSystemManager.GetScheduler();
	pScheduler->CreateTask<ACameraManager>( this )->Create();
	pScheduler->CreateTask<AMusicManager>( this )->Create();

	AAssetStreaming::CreateSingleton()->SetFlag( TFALSE );

	return TTRUE;
}

TBOOL AGameSystemManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();

	AAssetStreaming::GetSingleton()->Update();

	if ( ATerrainInterface::IsSingletonCreated() )
	{
		ATerrainInterface::GetSingleton()->Update();
	}

	return TTRUE;
}
