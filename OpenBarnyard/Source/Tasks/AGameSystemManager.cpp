#include "pch.h"
#include "AGameSystemManager.h"
#include "GUI/ASubtitleManager.h"
#include "Sound/APositionalSoundSet.h"
#include "GameInterface/AHackedQuickSave.h"
#include "Helpers/ASimPersistData.h"
#include "Sound/AVOStream.h"
#include "FSM/AFSMManager.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Cameras/ACameraManager.h"
#include "Units/AUnitFlagSetsManager.h"
#include "Units/ASSeqController.h"
#include "Units/AUnitManager.h"
#include "Units/AInteractionManager.h"
#include "Units/AAnimalPopulationManager.h"
#include "Units/ANPCBehaviourManager.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/Steering/ASteerPool.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/Steering/AKinematicsPool.h"
#include "Units/AKinematicsObstructionManager.h"
#include "Player/APlayerKnowledgeManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleEffectLocatorManager.h"
#include "Helpers/AGameLoader.h"
#include "Render/AShadowManager.h"
#include "Render/ARenderer.h"
#include "FX/AFXManager.h"
#include "Helpers/ATarget.h"
#include "Physics/ABarnyardCollision.h"
#include "Units/AUnitNameManager.h"
#include "Render/ARainRenderer.h"
#include "Helpers/AWeatherManager.h"
#include "FX/AGameTimeFXManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Sound/AMusicManager.h"
#include "Sound/AMultiplexStream.h"
#include "Helpers/AHintManager.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/AConversationManager.h"
#include "Helpers/ASpeechDialogManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/ASkinLightingManager.h"
#include "Helpers/AInstanceLightingHelper.h"
#include "Helpers/ATreeManager.h"
#include "Helpers/AInstanceManager.h"
#include "Helpers/AGameTimeManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "HUD/AHudManager.h"
#include "Tasks/ARootTask.h"
#include "ACheatActivator.h"
#include "Helpers/AGateManager.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/ATreasureManager.h"

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Physics/ACollisionManager.h"
#include "Physics/ABarnyardCollision.h"
#include "Physics/AVehicleSkidManager.h"
#include "Quests/AQuestManager.h"
#include "FX/AAniEffectManager.h"

#include <Toshi/TScheduler.h>
#include "Helpers/AReplayManager.h"
#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// NOTE: GetClass 00401480, CreateObject 004018a0, CreateObjectInPlace 00401920 (the constructor is inlined in both)
TDEFINE_CLASS( AGameSystemManager );

TOSHI_NAMESPACE_USING

AGameSystemManager::AGameSystemManager()
    : m_iNumTasks( 0 )
    , m_iNumManagers( 0 )
    , m_iUnk104( 0 )
{
}

// $Barnyard: FUNCTION 00401570
// $Barnyard: FUNCTION 004019a0
AGameSystemManager::~AGameSystemManager()
{
}

TTask* AGameSystemManager::CreateSystemTask( TClass* a_pClass )
{
	TTask* pTask = g_oSystemManager.GetScheduler()->CreateTask( a_pClass, this );
	pTask->Create();

	TASSERT( m_iNumTasks < MAX_NUM_TASKS );
	m_apTasks[ m_iNumTasks++ ] = pTask;

	return pTask;
}

TObject* AGameSystemManager::CreateManager( TClass* a_pClass )
{
	TObject* pManager = a_pClass->CreateObject();

	TASSERT( m_iNumManagers < MAX_NUM_MANAGERS );
	m_apManagers[ m_iNumManagers++ ] = pManager;

	return pManager;
}

// $Barnyard: FUNCTION 00401820
void AGameSystemManager::ActivateSystemTasks( TBOOL a_bActivate )
{
	for ( TINT i = 0; i < m_iNumTasks; i++ )
	{
		TTask* pTask = m_apTasks[ i ];

		if ( !pTask->GetClass()->IsA( &TGetClass( AMiniGameManager ) ) &&
		     !pTask->GetClass()->IsA( &TGetClass( AMusicManager ) ) &&
		     !pTask->GetClass()->IsA( &TGetClass( AHudManager ) ) )
		{
			pTask->Activate( a_bActivate );
		}
	}
}

// $Barnyard: FUNCTION 00401760
TBOOL AGameSystemManager::Reset()
{
	AShadowManager::GetSingleton()->Reset();

	for ( TINT i = 0; i < m_iNumTasks; i++ )
	{
		if ( !m_apTasks[ i ]->Reset() )
			return TFALSE;
	}

	ACheatActivator::GetSingleton()->Reset();

	AUnitManager::GetSingleton()->ClearUIDMap();
	AUnitNameManager::GetSingleton()->Clear();
	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	AHighScoreManager::GetSingleton()->Reset();

	AHintManager::GetSingleton()->Reset();

	AParticleEffectLocatorManager::GetSingleton()->Reset();

	AFSMManager::GetSingleton()->ResetStateMachines();

	AParticleEffectManager::GetSingleton()->KillAll();

	ACollisionManager::GetSingleton()->DestroyObjectHashMain();
	ARenderer::GetSingleton()->Reset();

	return TTRUE;
}

// $Barnyard: FUNCTION 00401a20
TBOOL AGameSystemManager::OnCreate()
{

	// Load keylibs from the startup library
	TString8 startupLibFileName;

	startupLibFileName.Format( "Data/%s.trb", "lib_startup" );

	TTRB startupLibTrb;

	startupLibTrb.Load( startupLibFileName );

	auto pProperties = PBProperties::LoadFromTRB( startupLibTrb );

	auto pKeylibProperty = pProperties->GetProperty( "keylib" );

	if ( pKeylibProperty )
	{
		AKeyFrameLibraryManager::GetSingleton()->LoadLibrariesFromProperties(
		    pKeylibProperty,
		    &startupLibTrb
		);
	}

	CreateManager( &TGetClass( AHackedQuickSave ) );

	CreateManager( &TGetClass( AKinematicsPool ) );
	CreateManager( &TGetClass( AKinematicsObstructionManager ) );
	CreateManager( &TGetClass( ASteerPool ) );
	CreateManager( &TGetClass( APlayerKnowledgeManager ) );

	CreateManager( &TGetClass( AModelRepos ) );

	CreateManager( &TGetClass( AUnitFlagSetsManager ) );

	CreateSystemTask( &TGetClass( AInteriorNetworkManager ) );
	AInteriorNetworkManager::GetSingleton()->LoadNetworksFromProperty( pProperties->GetProperty( "AInteriorNetwork" ), TTRUE );

	CreateSystemTask( &TGetClass( AParticleEffectManager ) );

	CreateSystemTask( &TGetClass( ASubtitleManager ) );

	CreateSystemTask( &TGetClass( AAnimatableObjectManager ) );

	AAnimatableObjectManager::GetSingleton()->LoadAnimSoundBreakpoints( "Data/Sound/AnimSoundBP.trb" );

	// Lighting managers for skinned meshes and instances
	ASkinLightingManager::CreateSingleton();

	AInstanceLightingHelper::CreateSingleton( 0x400 );

	CreateSystemTask( &TGetClass( ACameraManager ) );

	CreateSystemTask( &TGetClass( AHighScoreManager ) );

	CreateSystemTask( &TGetClass( AHintManager ) );

	CreateSystemTask( &TGetClass( AParticleEffectLocatorManager ) );

	// Load models from the startup library
	AAnimatableObjectManager::GetSingleton()->LoadTypesFromLibrary( "lib_startup" );

	CreateSystemTask( &TGetClass( AFXManager ) );

	CreateSystemTask( &TGetClass( AHudManager ) );

	CreateSystemTask( &TGetClass( AMusicManager ) );

	CreateSystemTask( &TGetClass( ASSeqController ) );

	CreateSystemTask( &TGetClass( AUnitManager ) );

	CreateSystemTask( &TGetClass( AGameTimeManager ) );

	( new ARegrowthManager() )->ResetGrowthObject( TTRUE );

	CreateSystemTask( &TGetClass( AAnimalPopulationManager ) );

	CreateSystemTask( &TGetClass( AQuestManager ) );

	CreateManager( &TGetClass( ACheatActivator ) );

	CreateSystemTask( &TGetClass( AInteractionManager ) );

	CreateSystemTask( &TGetClass( ACollisionManager ) );

	new ABarnyardCollision();
	CreateSystemTask( &TGetClass( ATarget ) );

	CreateManager( &TGetClass( AUnitNameManager ) );

	CreateManager( &TGetClass( APlayerManager ) );

	CreateSystemTask( &TGetClass( AConversationManager ) );
	CreateSystemTask( &TGetClass( ASpeechDialogManager ) );

	CreateSystemTask( &TGetClass( ANPCBehaviourManager ) );

	CreateSystemTask( &TGetClass( AMiniGameManager ) );

	CreateSystemTask( &TGetClass( ASimAnimModelHelperManager ) );

	CreateManager( &TGetClass( AShadowManager ) );

	CreateManager( &TGetClass( AReplayManager ) );

	new AVOStream();

	new AMultiplexStream();

	CreateManager( &TGetClass( APlayerProfileManager ) );

	CreateSystemTask( &TGetClass( ABikeManager ) );
	( new ATreasureManager() )->Initialise();

	AAssetStreaming::CreateSingleton()->SetFlag( TFALSE );

	// Instances, trees and other things...
	ATreeManager::CreateSingleton()->Initialise();

	AInstanceManager::CreateSingleton();

	// NOTE: the original creates the rain renderer in ARenderer::OnCreate (it's at ARenderer+0x80)
	ARainRenderer::CreateSingleton();

	AWeatherManager::CreateSingleton();

	CreateSystemTask( &TGetClass( AGameTimeFXManager ) );

	CreateManager( &TGetClass( ASimPersistData ) );
	ASimPersistData::GetSingleton()->CreateLadderBeans();

	ARenderer::GetSingleton()->GetSkyDome()->LoadStars();

	CreateSystemTask( &TGetClass( ASimpleProjectileManager ) );

	CreateSystemTask( &TGetClass( AVehicleSkidManager ) );
	AVehicleSkidManager::GetSingleton()->Create( 200 );

	startupLibTrb.Close();

	APositionalSoundSet::Create();

	return TTask::OnCreate();
}

// $Barnyard: FUNCTION 004016a0
TBOOL AGameSystemManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	AAssetStreaming::GetSingleton()->Update();

	if ( ATerrainInterface::IsSingletonCreated() )
		ATerrainInterface::GetSingleton()->Update();

	APositionalSoundSet::GetSingleton()->Update( a_fDeltaTime );

	AInstanceLightingHelper::GetSingleton()->Update();

	if ( !ARootTask::GetSingleton()->IsPaused() )
	{
		if ( AAniEffectManager::IsSingletonCreated() )
			AAniEffectManager::GetSingleton()->Update( a_fDeltaTime );

		if ( ARegrowthManager::IsSingletonCreated() )
			ARegrowthManager::GetSingleton()->Update( a_fDeltaTime );

		if ( ATreasureManager::IsSingletonCreated() )
			ATreasureManager::GetSingleton()->Update( a_fDeltaTime );

		if ( APlayerProfileManager::IsSingletonCreated() )
			APlayerProfileManager::GetSingleton()->UpdateMainProfile( a_fDeltaTime );

		AVOStream::GetSingleton()->Update( a_fDeltaTime );

		AMultiplexStream::GetSingleton()->Update( a_fDeltaTime );

		if ( AGateManager::IsSingletonCreated() )
			AGateManager::GetSingleton()->Update( a_fDeltaTime );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00401590
void AGameSystemManager::OnDestroy()
{

	APositionalSoundSet::Destroy();
	AGameLoader::DestroyAll();
	ASimPersistData::GetSingleton()->DestroyLadderBeans();

	// Delete the managers in the reverse order of creation
	while ( m_iNumManagers != 0 )
	{
		TObject* pManager              = m_apManagers[ --m_iNumManagers ];
		m_apManagers[ m_iNumManagers ] = TNULL;

		delete pManager;
	}

	if ( ASkinLightingManager::IsSingletonCreated() )
		delete ASkinLightingManager::GetSingleton();

	if ( AInstanceLightingHelper::IsSingletonCreated() )
		delete AInstanceLightingHelper::GetSingleton();

	if ( AAssetStreaming::IsSingletonCreated() )
		delete AAssetStreaming::GetSingleton();

	if ( AWeatherManager::IsSingletonCreated() )
		delete AWeatherManager::GetSingleton();

	// NOTE: the original deletes the rain renderer in the destructor of ARenderer
	if ( ARainRenderer::IsSingletonCreated() )
		delete ARainRenderer::GetSingleton();

	if ( ABarnyardCollision::IsSingletonCreated() )
		delete ABarnyardCollision::GetSingleton();

	if ( AMultiplexStream::IsSingletonCreated() )
		delete AMultiplexStream::GetSingleton();

	if ( AVOStream::IsSingletonCreated() )
		delete AVOStream::GetSingleton();
}
