#include "pch.h"
#include "AGameSystemManager.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Sound/AMusicManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/ASkinLightingManager.h"
#include "Helpers/ATreeManager.h"

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
	pScheduler->CreateTask<AAnimatableObjectManager>( this )->Create();

	AAnimatableObjectManager::GetSingleton()->LoadAnimSoundBreakpoints( "Data/Sound/AnimSoundBP.trb" );

	// Initialise lighting manages for skinned meshes
	ASkinLightingManager::CreateSingleton();

	// Initialise some basic things like cameras, music and more
	pScheduler->CreateTask<ACameraManager>( this )->Create();
	pScheduler->CreateTask<AMusicManager>( this )->Create();

	// Load models from the startup library
	AAnimatableObjectManager::GetSingleton()->LoadTypesFromLibrary( "lib_startup" );

	pScheduler->CreateTask<ASimAnimModelHelperManager>( this )->Create();

	AAssetStreaming::CreateSingleton()->SetFlag( TFALSE );

	// Instances, trees and other things...
	ATreeManager::CreateSingleton()->LoadModels();

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
