#include "pch.h"
#include "ABarnExploreMinigameState.h"
#include "ABarnHelper.h"
#include "ABarnSeatedHelper.h"
#include "ABarnChatHelper.h"
#include "AGopherShopInterface.h"
#include "AGopherShopItem.h"
#include "ACookingRecipe.h"
#include "ARandom.h"
#include "ALoadScreen.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "Assets/AAssetLoader.h"
#include "Assets/AModelLoader.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/ANamedAnimationSet.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FSM/AMethodPointer.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AHackedQuickSave.h"
#include "GameInterface/ASSeqState.h"
#include "Helpers/ABarnFanHelper.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/AInstanceManager.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/AReplayManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/ASimPersistData.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ASunFlareEffect.h"
#include "Render/AShadowManager.h"
#include "Sound/AMultiplexStream.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainSection.h"
#include "Terrain/ATerrainLocator.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "Units/AAnimalPopulationManager.h"
#include "Units/AKinematicsObstructionUnit.h"
#include "Units/ACharacterUnit.h"
#include "Units/AInteraction.h"
#include "Units/AInteractionSequence.h"
#include "Units/AKinematicsObstructionManager.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AScriptedSequenceActivity.h"
#include "Units/AUnitBehaviour.h"
#include "Units/AUnitFacialComponent.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitType.h"
#include "Units/AUnitNameManager.h"
#include "Physics/ACollisionObjectMesh.h"
#include "SoundBank/nb.h"

// Opcode (pulled in by the Physics headers) #defines PI, which breaks TMath::PI below
#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0049b8e0, CreateObject 004a1bc0, CreateObjectInPlace 004a1c20 (the constructor is inlined in both)
TDEFINE_CLASS_INIT( ABarnExploreMinigameState, 1, 0 );

// DAT_00782d7c
AFunctionTable ABarnExploreMinigameState::ms_oFunctionTable( &TGetClass( ABarnExploreMinigameState ), TNULL );

// The terrain sections of the barn (DAT_0077396c, DAT_00773970, DAT_00773974)
static TINT s_iStairsSection   = -1;
static TINT s_iBasementSection = -1;
static TINT s_iBarnSection     = -1;

// The skin the next swapped animal gets (DAT_00782d78)
static TINT s_iSkin = 0;

// The player's position and direction when the barn is left (DAT_00782dac, DAT_00782dfc), they're put back if the barn is entered from a save
static TVector4 s_vecPlayerPosition;
static TVector4 s_vecPlayerDirection;

// The position of the saloon bar, where the sound of the ground floor plays (DAT_00782dec)
static TVector4 s_vecSaloonBarPosition;

// The dancers of the wurlitzer (DAT_00782e0c)
static const TCHAR* s_aDancers[] = { "nb_generic_goat", "nb_generic_horse", "nb_generic_cow" };

// The "random" animals of the tables and booths (DAT_00782e18)
static const TCHAR* s_aTableAnimals[] = { "nb_generic_goat", "nb_generic_hen", "nb_generic_sheep", "nb_generic_pig" };

// The "random" animals elsewhere (DAT_00773954)
static const TCHAR* s_aRandomAnimals[] = { "nb_generic_goat", "nb_generic_sheep", "nb_generic_horse", "nb_generic_pig", "nb_generic_hen" };

// FSM "GetBenchGoal" and "GetDanceGoal" (off_00773978, off_00773990)
static const TCHAR* s_aBenchGoals[] = { "bench1", "bench2", "bench3", "bench6", "bench4", "bench5" };
static const TCHAR* s_aDanceGoals[] = { "dance1", "dance2", "dance3", "dance6", "dance4", "dance5" };

// The scripted sequences of the ending (index in AScriptedSequenceActivity::ms_aSequences)
static constexpr TINT SEQUENCE_SSEND_CHAMP  = 27; // 0x0077976c
static constexpr TINT SEQUENCE_SSEND_DETAIL = 28; // 0x007797a0

// The player can use the area (the ladder, the rope...)
static void SetAreaSpecial( AInteriorArea* a_pArea, TBOOL a_bSpecial )
{
	if ( a_bSpecial )
		a_pArea->m_eFlags |= AInteriorArea::FLAGS_RUNTIME_SPECIAL;
	else
		a_pArea->m_eFlags &= ~AInteriorArea::FLAGS_RUNTIME_SPECIAL;
}

static ATerrainSection* GetTerrainSection( TINT a_iSection )
{
	// NOTE: inlined ATerrainInterface::GetSection
	return &ATerrainInterface::GetSingleton()->GetVIS()->GetSections()[ a_iSection ];
}

// The natives of the table are called on the state on the stack
// $Barnyard: FUNCTION 0049ba90
static TObject* __stdcall ABarnExploreMinigameStateFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ABarnExploreMinigameState" ) );
}

// $Barnyard: FUNCTION 0049fa80
void ABarnExploreMinigameState::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ABarnExploreMinigameStateFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1Args<ABarnExploreMinigameState, TBOOL>::Create( "ToggleMusic", &ABarnExploreMinigameState::ToggleMusic ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ABarnExploreMinigameState>::Create( "IsMusicPlaying", &ABarnExploreMinigameState::IsMusicPlaying ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ABarnExploreMinigameState>::Create( "DropLadder", &ABarnExploreMinigameState::DropLadder ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, ABarnExploreMinigameState>::Create( "GetBenchGoal", &ABarnExploreMinigameState::GetBenchGoal ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, ABarnExploreMinigameState>::Create( "GetDanceGoal", &ABarnExploreMinigameState::GetDanceGoal ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ABarnExploreMinigameState>::Create( "IncDanceGoal", &ABarnExploreMinigameState::IncDanceGoal ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ABarnExploreMinigameState>::Create( "SSEND_Detail", &ABarnExploreMinigameState::SSEND_Detail ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ABarnExploreMinigameState>::Create( "SSEND_Champ", &ABarnExploreMinigameState::SSEND_Champ ) );
}

// $Barnyard: FUNCTION 0049b8f0
void ABarnExploreMinigameState::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0049fef0
AFunctionTable* ABarnExploreMinigameState::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0049fd30
ABarnExploreMinigameState::ABarnExploreMinigameState()
{
	m_bNight             = TFALSE;
	m_bParty             = TFALSE;
	m_bBandNight         = TFALSE;
	m_bInChampSequence   = TFALSE;
	m_bInDetailSequence  = TFALSE;
	m_iNumItems          = 0;
	m_iNumSkinSwaps      = 0;
	m_iNumSnorers        = 0;
	m_pFanHelper         = TNULL;
	m_pSeatedHelper      = TNULL;
	m_pHelper            = TNULL;
	m_pChatHelper        = TNULL;
	m_iNumCollisions     = 0;
	m_fMusicFade         = 0.0f;
	m_iNumBooths2        = 0;
	m_iNumBooths1        = 0;
	m_iLoftCue           = -1;
	m_iGroundCue         = -1;
	m_iBasementCue       = -1;
	m_bDroppingLadder    = TFALSE;
	m_pRoot              = TNULL;
	m_pRootBarnGoal      = TNULL;
	m_pRootNightBarnGoal = TNULL;
	m_bRootOnGround      = TFALSE;
	m_bBarnyardHero      = TFALSE;
	m_iDanceGoal         = 0;
	m_pStageModel        = TNULL;
	m_iFoosballCue       = -1;
	m_iMechManCue        = -1;

	AGameLoader::SetBarnExploreMiniGame( TTRUE );

	m_iNumAreaMapEntries = 0;
	m_pAreaMap           = TNULL;
	m_iNumFSMMapEntries  = 0;
	m_pFSMMap            = TNULL;

	ms_bWurlitzerOn = TTRUE;
	ms_bRadioOn     = TTRUE;
	s_iSkin         = 0;

	m_iSnoreEffect = AParticleEffectManager::GetSingleton()->FindEffect( "cowzzzz" );
}

// $Barnyard: FUNCTION 0049ff20
// $Barnyard: FUNCTION 0049ff00
ABarnExploreMinigameState::~ABarnExploreMinigameState()
{
}

// $Barnyard: FUNCTION 0049b960
TBOOL ABarnExploreMinigameState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	// NOTE: the original checks the class of the event's device here and ignores the result
	return ASimState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 004a1c60
void ABarnExploreMinigameState::OnInsertion()
{
	AModelLoader::ResolveTextures();

	m_HUDParams.SetFlags( AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 | AHudElement::MASK_MINIMAP | AHudElement::MASK_UNK11 | AHudElement::MASK_UNK15 | AHudElement::MASK_ITEM_COUNT );
	ms_iFloor = FLOOR_LOFT;

	m_bNight = AGameTimeManager::GetSingleton()->IsItNight();

	if ( m_bNight )
	{
		AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvBarnInt, TTRUE, TTRUE, 0, 0, 0, 0 );
		AGameLoader::Terrain_Load();

		AInteriorNetworkManager::GetSingleton()->LoadNetwork( TPS8D( "barn_night" ) );
		AInteriorNetworkManager::GetSingleton()->LoadNetwork( TPS8D( "barnauto_night" ) );
		AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8D( "barn_night" ) );

		// No sky in the barn
		ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
		pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
		pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );
		ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

		// The barn and the basement don't see each other, the stairs see the floor the player is on (see EnterLoft)
		ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

		s_iStairsSection   = pTerrain->GetSectionIndex( "stairs" );
		s_iBasementSection = pTerrain->GetSectionIndex( "basement" );
		s_iBarnSection     = pTerrain->GetSectionIndex( "barn" );

		GetTerrainSection( s_iBarnSection )->SetOtherSectionVisibility( s_iBasementSection, ATerrainLODType_None );
		GetTerrainSection( s_iBasementSection )->SetOtherSectionVisibility( s_iBarnSection, ATerrainLODType_None );

		pTerrain->SetGetCurrentVISGroupCallback( GetCurrentVISGroup );
	}
	else
	{
		AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvBarnIntDay, TTRUE, TTRUE, 0, 0, 0, 0 );
		AGameLoader::Terrain_Load();

		AInteriorNetworkManager::GetSingleton()->LoadNetwork( TPS8D( "barn" ) );
		AInteriorNetworkManager::GetSingleton()->LoadNetwork( TPS8D( "barnauto" ) );
		AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8D( "barn" ) );

		CreateLadder();
	}

	AGameLoader::SpawnSimPlayer();

	if ( m_bNight == TTRUE )
	{
		// The ceiling fans
		m_pFanHelper = new ABarnFanHelper();
		ASimAnimModelHelperManager::GetSingleton()->AddModelHelper( m_pFanHelper );

		// The items of the barn
		AddBaseItems();
		AddShopItems();
		UpdateItemCounts();
		DisableReplacedItems();
		UpdateVases();

		// The dance floor party: Friday and Saturday (the band plays) once the barn is complete
		AGameTimeManager::WEEKDAY eWeekDay = AGameTimeManager::GetSingleton()->GetWeekDay();

		if ( FindItem( 962 ) && FindItem( 964 ) && FindItem( 961 ) && FindItem( 963 ) && FindItem( 929 ) && m_bDanceFloor == TTRUE &&
		     ( eWeekDay == AGameTimeManager::WEEKDAY_FRIDAY || eWeekDay == AGameTimeManager::WEEKDAY_SATURDAY ) )
		{
			m_bParty = TTRUE;

			if ( eWeekDay == AGameTimeManager::WEEKDAY_SATURDAY )
				m_bBandNight = TTRUE;
		}

		// Create the items, the progress of the loading screen goes up to 20 steps
		TFLOAT fStep     = 1.0f;
		TFLOAT fProgress = 0.0f;

		if ( m_iNumItems > 20 )
			fStep = m_iNumItems * 0.05f;

		for ( TINT i = 0; i < m_iNumItems; i++ )
		{
			Item& rItem = m_aItems[ i ];

			if ( rItem.bEnabled == TTRUE )
			{
				rItem.CreateObjects();

				for ( TINT k = 0; k < rItem.GetNumObjects(); k++ )
				{
					AAnimatableObject* pObject = rItem.GetObject( k );
					const TPString8&   rcType  = pObject->GetObjectType()->GetName();

					// The object belongs to the floor of its bone (the one named like its type)
					TINT iBone = pObject->GetModelInstance()->GetBoneID( rcType );

					if ( T2String8::FindString( rcType.GetString(), "lamp" ) || iBone == -1 )
						continue;

					TMatrix44 mBone;
					pObject->GetModelInstance()->GetBoneTransform( iBone, mBone );

					if ( mBone.GetTranslation().y <= 0.1f )
						m_vecGroundObjects.PushBack( pObject );
					else
						m_vecLoftObjects.PushBack( pObject );
				}
			}

			fProgress += 1.0f;

			if ( fProgress >= fStep )
			{
				g_oLoadScreen.Update( 1.0f, TTRUE );
				fProgress -= fStep;
			}
		}

		// The dancing animals need the dance floor
		AAnimalPopulationManager*                                             pPopulationManager = AAnimalPopulationManager::GetSingleton();
		AAnimalPopulationManager::Set<AAnimalPopulationManager::AnimalSpawn>* pSpawnSet          = pPopulationManager->GetAnimalSpawnSet();
		AAnimalPopulationManager::StringTable*                                pStringTables      = pPopulationManager->GetStringMappingTables();

		for ( TINT i = 0; i < pSpawnSet->iNumEntries; i++ )
		{
			AAnimalPopulationManager::AnimalSpawn& rSpawn = pSpawnSet->pEntries[ i ];

			if ( TStringManager::String8Compare( pStringTables[ AAnimalPopulationManager::STRINGTABLE_BEHAVIOUR ].ppStrings[ rSpawn.oSpawn.iBehaviour ], "dance" ) == 0 )
			{
				if ( m_bDanceFloor == TTRUE )
					rSpawn.uiState |= AAnimalPopulationManager::SPAWNSTATE_ENABLED;
				else
					rSpawn.uiState &= ~AAnimalPopulationManager::SPAWNSTATE_ENABLED;
			}
		}
	}

	AAnimalPopulationManager::GetSingleton()->Populate( AAnimalPopulationManager::SPAWNFLAGS( ( m_bNight == TTRUE ) ? ( AAnimalPopulationManager::SPAWNFLAGS_NIGHT | AAnimalPopulationManager::SPAWNFLAGS_UNK3 ) : ( AAnimalPopulationManager::SPAWNFLAGS_DAY | AAnimalPopulationManager::SPAWNFLAGS_UNK3 ) ), TFALSE );

	// The animals of the floors (but the ones of the ending)
	AUnitManager*                     pUnitManager = AUnitManager::GetSingleton();
	Toshi::TNodeList<AUnit>::Iterator itBegin      = pUnitManager->BeginUnits();
	Toshi::TNodeList<AUnit>::Iterator itEnd        = pUnitManager->EndUnits();

	for ( auto it = itBegin; it != itEnd; it++ )
	{
		AUnit*       pUnit  = it;
		const TCHAR* szType = pUnit->GetType()->GetName().GetString();

		if ( TStringManager::String8Compare( szType, "end_otis" ) == 0 || TStringManager::String8Compare( szType, "end_daisy" ) == 0 )
			continue;

		if ( !pUnit->IsA( &TGetClass( ACharacterUnit ) ) )
			continue;

		if ( pUnit->GetSteer()->GetPosition()->y <= 0.1f )
			m_vecGroundCharacters.PushBack( pUnit );
		else
			m_vecLoftCharacters.PushBack( pUnit );
	}

	ASimState::OnInsertion();

	if ( !ShouldPlacePlayerAtStartGoal() )
	{
		// Back from a minigame of the barn, the player is where they were
		ASteer* pSteer = m_pPlayerProxy->GetUnit()->GetSteer();
		pSteer->SetPosition( s_vecPlayerPosition );
		pSteer->SetDirection( s_vecPlayerDirection );
	}

	AUnit* pPlayer = m_pPlayerProxy->GetUnit();

	if ( m_bNight )
	{
		AGameTimeFX oFX = *AGameTimeFXManager::GetSingleton()->FindFX( "BarnNight" );
		AGameTimeFXManager::GetSingleton()->SetForcedFX( &oFX );

		m_pSeatedHelper = new ABarnSeatedHelper( pPlayer );
		m_pHelper       = new ABarnHelper( pPlayer );
		m_pChatHelper   = new ABarnChatHelper( pPlayer );
	}
	else
	{
		// The sleeping cows
		for ( TINT i = 0; i < 3; i++ )
			SpawnQuadCow( i + 1 );

		m_pHelper = new ABarnHelper( pPlayer );
		SpawnWildMikeBox();

		AGameTimeFX oFX = *AGameTimeFXManager::GetSingleton()->FindFX( "BarnDay" );
		AGameTimeFXManager::GetSingleton()->SetForcedFX( &oFX );
	}

	TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetUsePath( TTRUE );
	ARenderer::GetSingleton()->SetNear( 0.4f );
	ARenderer::GetSingleton()->SetFar( 200.0f );

	m_HUDParams.SetFlags( AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK15 );
	AHudManager::GetSingleton()->OnUpdate( 0.0f );

	APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, m_pPlayerProxy->GetUnit()->FindComponent( &TGetClass( APlayerControlComponent ) ) );
	pPlayerControl->SetCanSprint( TTRUE );

	if ( !m_bNight )
		return;

	// Root and the goals he walks to
	m_pRoot = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "root" ) );

	m_pRootBarnGoal      = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8D( "barn_night" ) )->FindGoalOrFirst( "root_barn" );
	m_pRootNightBarnGoal = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8D( "barn_night" ) )->FindGoalOrFirst( "root_nightbarn_pos" );

	if ( !m_bNight || !m_bParty )
		return;

	// A dancer at the wurlitzer
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	TINT              iDancer  = ARandom::GetSingleton()->m_oRandom.GetInt( 3 );

	TPString8      strWurlitzerGoal( "wurlitzer_goal" );
	AInteriorGoal* pWurlitzerGoal = pNetwork->FindGoal( strWurlitzerGoal );

	// NOTE: the goal itself is passed as the direction, its first member is the forward vector
	AUnit* pDancer = AUnitManager::GetSingleton()->SpawnUnit( s_aDancers[ iDancer ], TVector4::VEC_ZERO, pWurlitzerGoal->m_vForward );

	if ( pDancer->ChangeSteer( &TGetClass( AInteriorSteer ) ) )
	{
		AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pDancer->GetSteer() );
		pSteer->ConnectToNetwork( pNetwork );
		pSteer->Command_SnapToGoal( strWurlitzerGoal );
	}

	FSMMapEntry oDanceEntry;
	oDanceEntry.szParent   = TNULL;
	oDanceEntry.szBone     = TNULL;
	oDanceEntry.szActivity = "minidance";
	oDanceEntry.uiFlags    = FSMFLAGS_SHADOW | FSMFLAGS_PARTY;
	oDanceEntry.iMinTier   = -1;
	oDanceEntry.iMaxTier   = -1;
	SetupUnit( pDancer, TNULL, &oDanceEntry, TFALSE );

	TINT iWurlitzerArea = pNetwork->FindAreaIndex( TPS8D( "wurlitzer_area" ) );
	SetAreaSpecial( pNetwork->GetArea( iWurlitzerArea ), TTRUE );

	if ( !m_bBandNight )
		return;

	// Wild Mike dances on the stage
	AModelInstance* pStage = FindItem( 962 )->GetObject( 0 )->GetModelInstance();

	TMatrix44 mBone;
	pStage->GetBoneTransform( pStage->GetBoneID( "wm_start" ), mBone );
	m_fWildMikeHeight = mBone.GetTranslation().y;

	AAnimatableObjectManager::GetSingleton()->LoadAnimObjType( "nb_wildmike", TNULL, TFALSE );
	AAnimatableObjectManager::GetSingleton()->FindType( TPS8D( "nb_wildmike" ) );

	TINT     iStageArea = pNetwork->FindAreaIndex( TPS8D( "stage_area" ) );
	TVector4 vecPosition;
	pNetwork->GetArea( iStageArea )->GetPosition( vecPosition );
	pNetwork->ResolvePointHeight( iStageArea, vecPosition, 0 );
	vecPosition.y = m_fWildMikeHeight;

	AAnimatableObject* pWildMike = CreateObject( TNULL, "nb_wildmike", TNULL, TFALSE, vecPosition, TTRUE, TNULL );
	m_pHelper->AddWildMike( pWildMike );

	m_pHelper->GetWildMikePositions()[ 0 ] = mBone.GetTranslation();

	pStage->GetBoneTransform( pStage->GetBoneID( "wm_left" ), mBone );
	m_pHelper->GetWildMikePositions()[ 1 ] = mBone.GetTranslation();

	pStage->GetBoneTransform( pStage->GetBoneID( "wm_right" ), mBone );
	m_pHelper->GetWildMikePositions()[ 2 ] = mBone.GetTranslation();
}

// $Barnyard: FUNCTION 0049ffa0
void ABarnExploreMinigameState::OnRemoval()
{
	if ( m_bBarnyardHero )
		RemoveEndDaisyObstruction();

	if ( m_pStageModel )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pStageModel );
		m_pStageModel = TNULL;
	}

	ATerrainInterface::GetSingleton()->SetGetCurrentVISGroupCallback( ATerrainInterface::GetSectionPlayerIsIn );

	if ( m_bPartySetUp && m_bNight )
	{
		AInstanceManager::GetSingleton()->SetInteriorMode( TFALSE );
		ARegrowthManager::GetSingleton()->SetInteriorMode( TFALSE );
	}

	// Where to put the player back if the barn is entered again from one of its minigames
	ASteer* pPlayerSteer = m_pPlayerProxy->GetUnit()->GetSteer();
	s_vecPlayerPosition  = *pPlayerSteer->GetPosition();
	s_vecPlayerDirection = *pPlayerSteer->GetDirection();

	for ( TINT i = 0; i < m_iNumCollisions; i++ )
	{
		if ( m_apCollisions[ i ]->IsCollisionObjectLinked() )
			m_apCollisions[ i ]->UnlinkCollisionObject();
	}

	for ( TINT i = 0; i < m_iNumCollisions; i++ )
	{
		if ( m_apCollisions[ i ] )
			delete m_apCollisions[ i ];
	}

	for ( TINT i = 0; i < m_iNumCollisions; i++ )
		m_apCollisions[ i ] = TNULL;

	m_iNumCollisions = 0;

	for ( TINT i = 0; i < m_iNumSkinSwaps; i++ )
		m_aSkinSwaps[ i ].Reset();

	if ( m_bNight == TTRUE )
	{
		ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( m_pFanHelper );
		delete m_pFanHelper;

		m_pHelper->Reset();
		delete m_pHelper;

		m_pSeatedHelper->Reset();
		delete m_pSeatedHelper;

		m_pChatHelper->Reset();
		delete m_pChatHelper;

		// The sky is back
		ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
		pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
		pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );
		ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );
	}
	else
	{
		m_pHelper->Reset();
		delete m_pHelper;
	}

	GetSimHudGroup()->SetVisible( TFALSE );
	GetSimHudGroup()->GetPickUpElement()->SetForceHide( TFALSE );

	APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, m_pPlayerProxy->GetUnit()->FindComponent( &TGetClass( APlayerControlComponent ) ) );
	pPlayerControl->SetCanSprint( TFALSE );

	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetUpdateStamina( TTRUE );
	TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetUsePath( TFALSE );

	// The objects of the items aren't the floors' to destroy
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		Item& rItem = m_aItems[ i ];

		if ( rItem.bEnabled != TTRUE )
			continue;

		for ( TINT k = 0; k < rItem.GetNumObjects(); k++ )
		{
			AAnimatableObject* pObject = rItem.GetObject( k );
			auto               itLoft  = m_vecLoftObjects.Find( pObject );

			if ( itLoft != m_vecLoftObjects.End() )
			{
				m_vecLoftObjects.Erase( itLoft );
				continue;
			}

			auto itGround = m_vecGroundObjects.Find( pObject );

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: the original compares the result of Find with Find instead of End, the objects of the ground floor are
			// never removed and get destroyed twice (by the item and with the ground floor's objects below)
			if ( itGround != m_vecGroundObjects.End() )
#else
			if ( itGround != m_vecGroundObjects.Find( pObject ) )
#endif
				m_vecGroundObjects.Erase( itGround );
		}
	}

	for ( TINT i = 0; i < m_iNumItems; i++ )
		m_aItems[ i ].DestroyObjects();

	DestroyUnits( m_vecAlwaysShownUnits, TTRUE );
	DestroyUnits( m_vecGroundUnits, TTRUE );
	DestroyUnits( m_vecLoftUnits, TTRUE );
	DestroyUnits( m_vecLoftCharacters, TTRUE );
	DestroyUnits( m_vecGroundCharacters, TTRUE );

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < m_vecLoftObjects.Size(); i++ )
		pObjectManager->DestroyAnimatableObject( m_vecLoftObjects[ i ] );

	m_vecLoftObjects.Clear();

	for ( TINT i = 0; i < m_vecGroundObjects.Size(); i++ )
		pObjectManager->DestroyAnimatableObject( m_vecGroundObjects[ i ] );

	m_vecGroundObjects.Clear();

	// NOTE: inlined AAnimatableObjectManager::DestroyType
	pObjectManager->DestroyType( TPS8D( "nb_wildmike" ) );

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	if ( m_bNight == TTRUE )
	{
		if ( AMultiplexStream::GetSingleton()->GetState() != AMultiplexStream::STATE_CLOSED )
			AMultiplexStream::GetSingleton()->Close( TTRUE );

		AInteriorNetworkManager::GetSingleton()->UnloadNetwork( TPS8D( "barn_night" ) );
		AInteriorNetworkManager::GetSingleton()->UnloadNetwork( TPS8D( "barnauto_night" ) );
	}
	else
	{
		AInteriorNetworkManager::GetSingleton()->UnloadNetwork( TPS8D( "barn" ) );
		AInteriorNetworkManager::GetSingleton()->UnloadNetwork( TPS8D( "barnauto" ) );
	}

	// The regrowth of the barn is kept while the player is outside
	AHackedQuickSave::SaveRegrowth( m_bNight );

	ASimState::OnRemoval();

	AGameLoader::SetBarnExploreMiniGame( TFALSE );
}

// $Barnyard: FUNCTION 004a3580
void ABarnExploreMinigameState::OnActivate()
{
	ASimState::OnActivate();

	m_HUDParams.SetFlags( AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 | AHudElement::MASK_MINIMAP | AHudElement::MASK_UNK11 | AHudElement::MASK_UNK15 | AHudElement::MASK_ITEM_COUNT );

	ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 )->SetTarget( m_pPlayerProxy );
	ACameraManager::GetSingleton()->SetSplitscreenCameraHelper( 1 );

	if ( m_bInChampSequence )
	{
		// Back from SSEND_Champ
		AVOStream::GetSingleton()->Close( TTRUE );

		if ( m_pStageModel )
		{
			if ( FindItem( 962 ) )
				FindItem( 962 )->GetObject( 0 )->SetSkeletonUpdating( TTRUE, TTRUE );

			AModelRepos::GetSingleton()->DestroyModelInstance( m_pStageModel );
			m_pStageModel = TNULL;
		}

		SetupParty();
		m_bInChampSequence = TFALSE;
		UpdateFloor();
	}
	else
	{
		if ( !m_bInDetailSequence )
			goto RESUME_CUES;

		// Back from SSEND_Detail
		AVOStream::GetSingleton()->Close( TTRUE );

		Item* pDanceFloor = FindItem( 947 );

		if ( pDanceFloor->bEnabled && pDanceFloor->HasSubModel() && pDanceFloor->iNumBought == 0 )
		{
			AMultiplexStream::GetSingleton()->Open( "NB_Radio" );
			m_bWurlitzer = TFALSE;
		}
		else
		{
			AMultiplexStream::GetSingleton()->Open( "NB_Wurlitzer" );
			m_bWurlitzer = TTRUE;
		}

		m_bInDetailSequence = TFALSE;
		m_pHelper->SetBandVisible( TTRUE );
	}

	m_pHelper->SetWildMikeUpdating( TTRUE );

RESUME_CUES:
	if ( m_pHelper->GetPinballCue() != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_pHelper->GetPinballCue(), TFALSE );

	if ( m_iFoosballCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iFoosballCue, TFALSE );

	if ( m_iMechManCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iMechManCue, TFALSE );

	GetSimHudGroup()->SetVisible( TFALSE );
	GetSimHudGroup()->GetPickUpElement()->SetForceHide( TFALSE );
}

// $Barnyard: FUNCTION 0049bab0
void ABarnExploreMinigameState::OnDeactivate()
{
	if ( m_pHelper->GetPinballCue() != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_pHelper->GetPinballCue(), TTRUE );

	if ( m_iFoosballCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iFoosballCue, TTRUE );

	if ( m_iMechManCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iMechManCue, TTRUE );

	ASimState::OnDeactivate();

	GetSimHudGroup()->SetVisible( TFALSE );
	GetSimHudGroup()->GetPickUpElement()->SetForceHide( TFALSE );
}

// $Barnyard: FUNCTION 0049b9b0
void ABarnExploreMinigameState::UpdateSim( TFLOAT a_fDeltaTime )
{
	GetSimHudGroup()->Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004a3780
void ABarnExploreMinigameState::OnLoaded()
{
	// The ending: Otis and Daisy on the throne once the player is the Barnyard Hero
	TINT iHeroAct;
	TINT iHeroTier;
	AQuestManager::GetSingleton()->FindQuestTier( "Barnyard Hero", &iHeroAct, &iHeroTier );

	if ( !m_bNight && AQuestManager::GetSingleton()->GetAct() >= iHeroAct && AQuestManager::GetSingleton()->GetTier() >= iHeroTier )
		m_bBarnyardHero = TTRUE;

	m_bPartySetUp = TFALSE;

	if ( m_bNight )
	{
		// The player is the champion of the barn: SSEND_Champ plays once
		AHighScoreManager::Record oChampionChart;

		if ( AHighScoreManager::GetChampionChart( &oChampionChart )->aPlayers[ 0 ] < 0 )
		{
			ASimPersistData* pPersistData = ASimPersistData::GetSingleton();

			if ( !pPersistData->HasFlags( ASimPersistData::FLAGS_CHAMP_SEQUENCE ) )
			{
				pPersistData->SetFlags( ASimPersistData::FLAGS_CHAMP_SEQUENCE );
				SSEND_Champ();
				APlayerProgress::GetSingleton()->SetCurrentSlotChampion();
			}
		}
	}

	if ( !m_bInChampSequence )
	{
		SetupParty();
		m_bPartySetUp = TTRUE;
		UpdateFloor();
	}

	AReplayManager::GetSingleton()->UpdateReplays();

	// The regrowth of the barn: regrown on the first visit (of the day or the night), then restored
	ASimPersistData* pPersistData = ASimPersistData::GetSingleton();
	TBOOL            bVisited     = pPersistData->HasFlags( m_bNight ? ASimPersistData::FLAGS_BARN_NIGHT_VISITED : ASimPersistData::FLAGS_BARN_DAY_VISITED );

	if ( bVisited )
	{
		TBOOL bNewDay = pPersistData->HasFlags( m_bNight ? ASimPersistData::FLAGS_REGROWTH_NEWDAY : ASimPersistData::FLAGS_REGROWTH_NEWDAY2 );

		if ( !bNewDay )
		{
			AHackedQuickSave::LoadRegrowth( m_bNight );
			goto DONE;
		}

		ARegrowthManager::GetSingleton()->NewDay( TTRUE, TFALSE );
	}
	else
	{
		ARegrowthManager::GetSingleton()->NewDayFruits( m_bNight );

		if ( m_bNight == TTRUE )
			pPersistData->SetFlags( ASimPersistData::FLAGS_BARN_NIGHT_VISITED );
		else
			pPersistData->SetFlags( ASimPersistData::FLAGS_BARN_DAY_VISITED );
	}

	if ( m_bNight )
		pPersistData->ClearFlags( ASimPersistData::FLAGS_REGROWTH_NEWDAY );
	else
		pPersistData->ClearFlags( ASimPersistData::FLAGS_REGROWTH_NEWDAY2 );

DONE:
	if ( m_bBarnyardHero )
		SetupEnding();

	ASimState::OnLoaded();
}

// $Barnyard: FUNCTION 0049efe0
TBOOL ABarnExploreMinigameState::ShouldPlacePlayerAtStartGoal()
{
	// Entered from the sim, the frontend (a save) or with a start goal
	TPString8 strPrevState = AGameLoader::GetPrevGameStateName();

	if ( TStringManager::String8Compare( strPrevState.GetString(), "ASimState" ) == 0 )
		return TTRUE;

	return T2String8::FindString( AGameLoader::GetPrevGameStateName().GetString(), "AFrontEnd" ) || AGameLoader::ShouldPlacePlayerAtStartGoal();
}

// $Barnyard: FUNCTION 0049d480
TBOOL ABarnExploreMinigameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bPendingAutoSave && !AQuestManager::GetSingleton()->IsTimerPaused() )
		AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );

	m_bPendingAutoSave = TFALSE;

	if ( m_bBarnyardHero )
		UpdateEndOtis();

	UpdateFloor();

	if ( m_bNight == TTRUE )
	{
		UpdateMusic( a_fDeltaTime );
		m_pSeatedHelper->Update( a_fDeltaTime );
		m_pHelper->Update( a_fDeltaTime );
		m_pChatHelper->Update( a_fDeltaTime );
	}
	else
	{
		m_pHelper->Update( a_fDeltaTime );

		// The sleeping cows snore
		for ( TINT i = 0; i < m_iNumSnorers; i++ )
		{
			Snorer& rSnorer = m_aSnorers[ i ];
			rSnorer.fTimer -= a_fDeltaTime;

			if ( rSnorer.fTimer < 0.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iSnoreEffect, rSnorer.vecPosition, TVector4::VEC_NEGY );

				// NOTE: inlined TRandom::GetFloatMinMax( 6.0f, 15.0f )
				rSnorer.fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 6.0f, 15.0f );
			}
		}

		// The ladder of the loft falls and bounces
		if ( m_bDroppingLadder )
		{
			m_fLadderDrop += a_fDeltaTime * m_fLadderSpeed;
			m_fLadderSpeed += a_fDeltaTime * 4.0f;

			if ( m_fLadderDrop > 2.0f )
			{
				m_fLadderDrop  = 2.0f;
				m_fLadderSpeed = m_fLadderSpeed * -0.5f;

				if ( TMath::Abs( m_fLadderSpeed ) < 0.5f )
				{
					// It's down, the player can climb it instead of the rope
					AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
					SetAreaSpecial( pNetwork->GetArea( pNetwork->FindAreaIndex( TPS8D( "ladder_area" ) ) ), TFALSE );
					SetAreaSpecial( pNetwork->GetArea( pNetwork->FindAreaIndex( TPS8D( "rope_area" ) ) ), TTRUE );

					m_bDroppingLadder = TFALSE;
				}
			}

			TMatrix44 mTransform = m_oLadderTransform;
			mTransform.m_f42 += m_fLadderDrop;
			m_pLadder->GetModelInstance()->SetTransform( mTransform );
		}
	}

	UpdateSim( a_fDeltaTime );

	if ( m_pRoot )
	{
		// Root's interaction ends when the player talks to him
		APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, m_pPlayerProxy->GetUnit()->FindComponent( &TGetClass( APlayerControlComponent ) ) );
		AInteractionSequence*    pSequence      = pPlayerControl->FindUnitInteraction( m_pRoot, TTRUE );

		if ( pSequence )
		{
			if ( pSequence->GetInteraction()->GetFlags() & AInteraction::FLAGS_MISSION_OBJECTIVE )
				AAnimalPopulationManager::GetSingleton()->SetMarker( m_pRoot, AAnimalPopulationManager::MARKERTYPE_QUESTION, TTRUE );

			pSequence->SetFlags( pSequence->GetFlags() | AInteractionSequence::FLAGS_ENDED );
			pPlayerControl->ReleaseInteraction( &pSequence );
		}

		m_bRootOnGround = m_pRoot->GetSteer()->GetPosition()->y < 0.5f;
	}

	APlayerProgress::SaveSlot* pSlot = APlayerProgress::GetSingleton()->GetCurrentSlot();

	if ( pSlot && ( pSlot->eFlags & APlayerProgress::SLOTFLAGS_NEWUNLOCK ) )
		CheckEndGame();

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0049c770
void ABarnExploreMinigameState::CheckEndGame()
{
	APlayerProgress* pProgress = APlayerProgress::GetSingleton();
	TBOOL            bChampion = pProgress->IsChampion();
	TBOOL            bDetail   = pProgress->HasCurrentSlotBarnDetail();
	TINT             iTier     = AQuestManager::GetSingleton()->GetCurAbsQuestTier();

	// The champion of the barn with the barn detailed late enough in the story: the end
	if ( bChampion && bDetail && iTier >= AQuestManager::GetSingleton()->GetAbsQuestTier( 3, 17 ) )
		AGameLoader::SetTransitionToGameState( TPS8D( "AEndGameState" ), TTRUE );

	// NOTE: the original doesn't check for a missing slot here
	pProgress->GetCurrentSlot()->eFlags &= ~APlayerProgress::SLOTFLAGS_NEWUNLOCK;
}

// $Barnyard: FUNCTION 0049b930
TINT ABarnExploreMinigameState::GetCurrentVISGroup()
{
	if ( ms_iFloor == FLOOR_LOFT )
		return s_iBasementSection;

	if ( ms_iFloor == FLOOR_GROUND || ms_iFloor == FLOOR_BASEMENT )
		return s_iBarnSection;

	return -1;
}

//-----------------------------------------------------------------------------
// FSM natives
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0049c510
void ABarnExploreMinigameState::ToggleMusic( TBOOL a_bWurlitzer )
{
	if ( a_bWurlitzer == TTRUE )
	{
		ms_bWurlitzerOn = !ms_bWurlitzerOn;
		SetMusicVolume( 1, ms_bWurlitzerOn ? 1.0f : 0.0f );
	}
	else
	{
		ms_bRadioOn = !ms_bRadioOn;
		SetMusicVolume( 0, ms_bRadioOn ? 1.0f : 0.0f );
	}
}

// $Barnyard: FUNCTION 0049b990
TBOOL ABarnExploreMinigameState::IsMusicPlaying()
{
	return ms_bWurlitzerOn;
}

// $Barnyard: FUNCTION 0049b9a0
void ABarnExploreMinigameState::DropLadder()
{
	m_bDroppingLadder = TTRUE;
}

// $Barnyard: FUNCTION 0049d290
TPString8 ABarnExploreMinigameState::GetBenchGoal()
{
	return TPString8( s_aBenchGoals[ m_iDanceGoal ] );
}

// $Barnyard: FUNCTION 0049d2d0
TPString8 ABarnExploreMinigameState::GetDanceGoal()
{
	return TPString8( s_aDanceGoals[ m_iDanceGoal ] );
}

// $Barnyard: FUNCTION 0049b900
void ABarnExploreMinigameState::IncDanceGoal()
{
	m_iDanceGoal++;
}

// $Barnyard: FUNCTION 0049c840
void ABarnExploreMinigameState::SSEND_Detail()
{
	m_pHelper->SetBandVisible( TFALSE );
	m_pHelper->SetWildMikeUpdating( TFALSE );

	const AScriptedSequenceActivity::SequenceInfo& rcSequence = AScriptedSequenceActivity::ms_aSequences[ SEQUENCE_SSEND_DETAIL ];
	TBOOL                                          bForcedFX  = TFALSE;

	if ( AGameTimeFXManager::IsSingletonCreated() && rcSequence.iTimeFX != -1 )
	{
		AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();
		pFXManager->SetForcedFX( pFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE( rcSequence.iTimeFXGroup ), AGameTimeManager::DAYPHASE( rcSequence.iTimeFX ) ) );
		bForcedFX = TTRUE;
	}

	ASSeqState* pState = new ASSeqState( rcSequence.szName, rcSequence.szScript, rcSequence.szCamera, TVector4::VEC_ZERO, rcSequence.iSyncType, TTRUE, TTRUE, TFALSE, TFALSE, bForcedFX, rcSequence.bShowChapterTitle, TTRUE, TFALSE );

	if ( pState )
	{
		AMultiplexStream::GetSingleton()->Stop();
		AMultiplexStream::GetSingleton()->Close( TTRUE );
		AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TTRUE );

		pState->SetCanSkip( rcSequence.bCanSkip );
		AGameStateController::GetSingleton()->PushState( pState );

		m_bInDetailSequence = TTRUE;
	}
}

// $Barnyard: FUNCTION 0049f5b0
void ABarnExploreMinigameState::SSEND_Champ()
{
	m_pHelper->SetWildMikeUpdating( TFALSE );

	const AScriptedSequenceActivity::SequenceInfo& rcSequence = AScriptedSequenceActivity::ms_aSequences[ SEQUENCE_SSEND_CHAMP ];
	TBOOL                                          bForcedFX  = TFALSE;

	if ( AGameTimeFXManager::IsSingletonCreated() && rcSequence.iTimeFX != -1 )
	{
		AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();
		pFXManager->SetForcedFX( pFXManager->GetWeatherSettings( AWeatherManager::WEATHERTYPE( rcSequence.iTimeFXGroup ), AGameTimeManager::DAYPHASE( rcSequence.iTimeFX ) ) );
		bForcedFX = TTRUE;
	}

	ASSeqState* pState = new ASSeqState( rcSequence.szName, rcSequence.szScript, rcSequence.szCamera, TVector4::VEC_ZERO, rcSequence.iSyncType, TTRUE, TTRUE, TFALSE, TFALSE, bForcedFX, rcSequence.bShowChapterTitle, TTRUE, TFALSE );

	if ( !pState )
		return;

	// Pip isn't carried in the sequence
	if ( APlayerProfileManager::GetSingleton()->GetMainProfile() )
		APlayerProfileManager::GetSingleton()->GetMainProfile()->ReleaseItem( TPS8D( "pip" ) );

	// The stage of the sequence
	Item* pStage = FindItem( 962 );

	if ( pStage && pStage->HasSubModel() )
	{
		m_pStageModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPS8D( "bn_stage_add" ), TNULL );

		if ( m_pStageModel )
		{
			pStage->GetObject( 0 )->SetSkeletonUpdating( TFALSE, TTRUE );
			m_pStageModel->SetSkeletonUpdating( TTRUE );
			m_pStageModel->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
		}
	}

	if ( AMultiplexStream::GetSingleton()->GetState() == AMultiplexStream::STATE_READY )
	{
		AMultiplexStream::GetSingleton()->Stop();
		AMultiplexStream::GetSingleton()->Close( TTRUE );
	}

	AVOStream::GetSingleton()->Open( "data/sound/FSB/ss.fsb", TTRUE, TTRUE );

	pState->SetCanSkip( rcSequence.bCanSkip );
	AGameStateController::GetSingleton()->PushState( pState );

	m_bInChampSequence = TTRUE;
}

//-----------------------------------------------------------------------------
// Music
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0049c4b0
void ABarnExploreMinigameState::SetMusicVolume( TINT a_iChannel, TFLOAT a_fVolume )
{
	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( m_bWurlitzer )
	{
		// NB_Wurlitzer has two channels per instrument
		pStream->SetVolume( a_fVolume, a_iChannel * 2, TTRUE );
		pStream->SetVolume( a_fVolume, a_iChannel * 2 + 1, TTRUE );
	}
	else
	{
		pStream->SetVolume( a_fVolume, a_iChannel, TTRUE );
	}
}

// $Barnyard: FUNCTION 0049d0d0
void ABarnExploreMinigameState::UpdateMusic( TFLOAT a_fDeltaTime )
{
	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( pStream->GetState() == AMultiplexStream::STATE_READY )
	{
		// The stream is open, start it with the channel of the floor
		pStream->Play( 0.0f );
		m_fMusicFade = 0.1f;
		a_fDeltaTime = 1.0f;
	}

	if ( m_fMusicFade > 0.0f && pStream->GetState() == AMultiplexStream::STATE_PLAYING )
	{
		m_fMusicFade -= a_fDeltaTime;

		if ( m_fMusicFade < 0.0f )
			m_fMusicFade = 0.0f;

		// m_iMusicChannel fades out, the other one fades in
		TFLOAT fVolume = m_fMusicFade * 0.5f;

		if ( ( m_iMusicChannel == 0 && ms_bRadioOn ) || ( m_iMusicChannel == 1 && ms_bWurlitzerOn ) )
			SetMusicVolume( m_iMusicChannel, fVolume );

		if ( ( m_iMusicChannel == 1 && ms_bRadioOn ) || ( m_iMusicChannel == 0 && ms_bWurlitzerOn ) )
			SetMusicVolume( ( m_iMusicChannel - 1 ) & 1, 1.0f - fVolume );
	}
}

//-----------------------------------------------------------------------------
// Floors
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0049cda0
void ABarnExploreMinigameState::UpdateFloor()
{
	TVector4 vecPosition = *m_pPlayerProxy->GetUnit()->GetSteer()->GetPosition();

	if ( vecPosition.y > 3.246f && ms_iFloor != FLOOR_LOFT )
	{
		EnterLoft( vecPosition );
	}
	else if ( vecPosition.y >= 3.246f || vecPosition.y <= -4.0f || ms_iFloor == FLOOR_GROUND )
	{
		if ( vecPosition.y < -4.0f && ms_iFloor != FLOOR_BASEMENT )
			EnterBasement( vecPosition );
	}
	else
	{
		EnterGround( vecPosition );
	}
}

// $Barnyard: FUNCTION 0049c030
void ABarnExploreMinigameState::EnterLoft( const TVector4& a_rcPosition )
{
	ms_iFloor = FLOOR_LOFT;

	for ( TINT i = 0; i < m_vecLoftObjects.Size(); i++ )
		m_vecLoftObjects[ i ]->SetSkeletonUpdating( TTRUE, TTRUE );

	for ( TINT i = 0; i < m_vecGroundObjects.Size(); i++ )
		m_vecGroundObjects[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < m_vecLoftUnits.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecLoftUnits[ i ] )->SetVisible( TTRUE );

	for ( TINT i = 0; i < m_vecLoftCharacters.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecLoftCharacters[ i ] )->SetVisible( TTRUE );

	for ( TINT i = 0; i < m_vecGroundUnits.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecGroundUnits[ i ] )->SetVisible( TFALSE );

	for ( TINT i = 0; i < m_vecGroundCharacters.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecGroundCharacters[ i ] )->SetVisible( TFALSE );

	if ( !m_bNight )
		return;

	m_fMusicFade    = 2.0f;
	m_iMusicChannel = 1;

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->FadeOutCue( m_iGroundCue, 1.0f, TTRUE, 3.0f );
	m_iGroundCue = -1;

	// The crowd of the loft once the barn is busy
	if ( AGopherShopInterface::GetPercentBought() > 60 )
	{
		m_iLoftCue = pSoundManager->PlayCue( soundbank::NB_CROWD_DOWNSTAIRS );
		pSoundManager->SetCuePosition( m_iLoftCue, TVector4( 13.8985538f, 4.74194384f, 34.9553146f, 1.0f ) );
		pSoundManager->SetCueVolume( m_iLoftCue, 0.0f );
		pSoundManager->FadeCue( m_iLoftCue, 2.0f, 1.0f, 3.0f );
	}

	ATerrainSection* pStairs = GetTerrainSection( s_iStairsSection );
	pStairs->SetOtherSectionVisibility( s_iBarnSection, ATerrainLODType_None );
	pStairs->SetOtherSectionVisibility( s_iBasementSection, ATerrainLODType_High );
}

// $Barnyard: FUNCTION 0049c270
void ABarnExploreMinigameState::EnterGround( const TVector4& a_rcPosition )
{
	FLOOR ePrevFloor = ms_iFloor;
	ms_iFloor        = FLOOR_GROUND;

	for ( TINT i = 0; i < m_vecLoftObjects.Size(); i++ )
		m_vecLoftObjects[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < m_vecGroundObjects.Size(); i++ )
		m_vecGroundObjects[ i ]->SetSkeletonUpdating( TTRUE, TTRUE );

	for ( TINT i = 0; i < m_vecLoftUnits.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecLoftUnits[ i ] )->SetVisible( TFALSE );

	for ( TINT i = 0; i < m_vecLoftCharacters.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecLoftCharacters[ i ] )->SetVisible( TFALSE );

	for ( TINT i = 0; i < m_vecGroundUnits.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecGroundUnits[ i ] )->SetVisible( TTRUE );

	for ( TINT i = 0; i < m_vecGroundCharacters.Size(); i++ )
		TSTATICCAST( AVisibleUnit, m_vecGroundCharacters[ i ] )->SetVisible( TTRUE );

	if ( !m_bNight )
		return;

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( ePrevFloor != FLOOR_LOFT )
	{
		pSoundManager->FadeOutCue( m_iBasementCue, 1.0f, TTRUE, 3.0f );
		m_iBasementCue = -1;
	}
	else
	{
		m_fMusicFade    = 2.0f;
		m_iMusicChannel = 0;
		pSoundManager->FadeOutCue( m_iLoftCue, 1.0f, TTRUE, 3.0f );
		m_iLoftCue = -1;
	}

	// The crowd at the saloon bar
	m_iGroundCue = pSoundManager->PlayCue( soundbank::NB_CROWD_BAR );
	pSoundManager->SetCuePosition( m_iGroundCue, s_vecSaloonBarPosition );
	pSoundManager->SetCueVolume( m_iGroundCue, 0.0f );
	pSoundManager->FadeCue( m_iGroundCue, 2.0f, 1.0f, 3.0f );

	ATerrainSection* pStairs = GetTerrainSection( s_iStairsSection );
	pStairs->SetOtherSectionVisibility( s_iBarnSection, ATerrainLODType_High );
	pStairs->SetOtherSectionVisibility( s_iBasementSection, ATerrainLODType_None );
}

// $Barnyard: FUNCTION 0049bed0
void ABarnExploreMinigameState::EnterBasement( const TVector4& a_rcPosition )
{
	ms_iFloor = FLOOR_BASEMENT;

	if ( !m_bNight )
		return;

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->FadeOutCue( m_iGroundCue, 1.0f, TTRUE, 3.0f );
	m_iGroundCue   = -1;
	m_iBasementCue = -1;

	// The crowd of the booths
	if ( a_rcPosition.x > 20.0f && m_iNumBooths2 > 0 )
	{
		m_iBasementCue = pSoundManager->PlayCue( soundbank::NB_CROWD_BALCONY );
		pSoundManager->SetCuePosition( m_iBasementCue, TVector4( 29.4827843f, -8.22789288f, 14.9314737f, 1.0f ) );
	}
	else if ( m_iNumBooths1 > 0 )
	{
		m_iBasementCue = pSoundManager->PlayCue( soundbank::NB_CROWD_BALCONY );
		pSoundManager->SetCuePosition( m_iBasementCue, TVector4( 0.387652010f, -7.78934002f, 22.6086998f, 1.0f ) );
	}

	if ( m_iBasementCue != -1 )
	{
		pSoundManager->SetCueVolume( m_iBasementCue, 0.0f );
		pSoundManager->FadeCue( m_iBasementCue, 2.0f, 1.0f, 3.0f );
	}

	ATerrainSection* pStairs = GetTerrainSection( s_iStairsSection );
	pStairs->SetOtherSectionVisibility( s_iBarnSection, ATerrainLODType_High );
	pStairs->SetOtherSectionVisibility( s_iBasementSection, ATerrainLODType_None );
}

//-----------------------------------------------------------------------------
// Items
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004a0c60
void ABarnExploreMinigameState::AddBaseItems()
{
	// NOTE: the same item is filled for each entry, the fields of the previous one are kept
	Item oItem;

	// The saloon bar
	oItem.SetAddModel( TPS8D( "bn_saloon_bar_add" ), 928 );
	oItem.iNumBought = 1;
	oItem.SetSubModel( TPS8D( "bn_saloon_bar_sub" ) );
	oItem.SetName( TPS8D( "in_saloon_bar" ) );
	m_aItems[ m_iNumItems++ ] = oItem;

	// The mirror
	oItem.SetAddModel( TPS8D( "bn_mirror_add" ), 933 );
	oItem.iNumBought = 1;
	oItem.SetSubModel( TPS8D( "bn_mirror_sub" ) );
	oItem.SetName( TPS8D( "in_mirror" ) );
	m_aItems[ m_iNumItems++ ] = oItem;

	oItem.SetAddModel( TPS8D( "bn_cows_milk" ), 924 );
	oItem.iNumBought = 1;
	oItem.DestroySubObjects();
	oItem.SetName( TPS8D( "in_cows_milk" ) );
	m_aItems[ m_iNumItems++ ] = oItem;

	oItem.SetAddModel( TPS8D( "bn_drop_lights" ), 941 );
	oItem.iNumBought = 1;
	oItem.DestroySubObjects();
	oItem.SetName( TPS8D( "in_drop_lights" ) );
	m_aItems[ m_iNumItems++ ] = oItem;

	oItem.SetAddModel( TPS8D( "bn_crocodile" ), 944 );
	oItem.iNumBought = 1;
	oItem.DestroySubObjects();
	oItem.SetName( TPS8D( "in_crocodile" ) );
	m_aItems[ m_iNumItems++ ] = oItem;

	// The stage
	oItem.SetAddModel( TPS8D( "bn_stage_add" ), 962 );
	oItem.iNumBought = 0;
	oItem.SetSubModel( TPS8D( "bn_stage_sub" ) );
	oItem.SetName( TPS8D( "in_stage" ) );
	m_aItems[ m_iNumItems++ ] = oItem;

	// The dance floor (with the jukebox or the stage lights), the stage comes with it
	m_bDanceFloor = TFALSE;

	if ( AGopherShopInterface::HasBought( 947 ) || AGopherShopInterface::HasBought( 929 ) )
	{
		oItem.SetAddModel( TPS8D( "bn_dance_1" ), 959 );
		oItem.iNumBought = 1;
		oItem.DestroySubObjects();
		oItem.SetName( TPS8D( "in_dance_1" ) );
		m_aItems[ m_iNumItems++ ] = oItem;

		FindItem( 962 )->iNumBought = 1;
		m_bDanceFloor               = TTRUE;
	}

	// The three tables
	oItem.SetAddModel( TPS8D( "bn_table_1_add" ), -1 );
	oItem.iNumBought = 3;
	oItem.iNumModels = 3;
	oItem.SetSubModel( TPS8D( "bn_table_1_sub" ) );
	oItem.SetName( TPS8D( "in_table" ) );
	m_aItems[ m_iNumItems++ ] = oItem;
}

// $Barnyard: FUNCTION 0049d8b0
void ABarnExploreMinigameState::SetupBooth( Item& a_rItem, const TCHAR* a_szName, TINT a_iItemId )
{
	TString8 strName;

	strName.Format( "%s_add", a_szName );
	a_rItem.SetAddModel( TPString8( strName.GetString() ), a_iItemId );
	a_rItem.iNumModels = 1;

	strName.Format( "%s_sub", a_szName );
	a_rItem.SetSubModel( TPString8( strName.GetString() ) );
}

// $Barnyard: FUNCTION 004a06c0
void ABarnExploreMinigameState::AddShopItems()
{
	TINT*                   pNumPrices = TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "gopherpricelist", "NumberOfCostItems", AAssetType_AssetPack ) );
	const AGopherShopPrice* pPrices    = TSTATICCAST( const AGopherShopPrice, AAssetLoader::GetSymbolAddress( "gopherpricelist", "CostMapping", AAssetType_AssetPack ) );

	TString8 strName;

	for ( TINT i = 0; i < *pNumPrices; i++ )
	{
		const AGopherShopPrice& rcPrice = pPrices[ i ];

		// The items of the barn
		if ( rcPrice.iCategory != 0 )
			continue;

		Item oItem;

		if ( rcPrice.iTextId == 966 )
		{
			// The booths of one side of the basement
			SetupBooth( oItem, "bn_booth_1", 966 );
			m_aItems[ m_iNumItems++ ] = oItem;
			SetupBooth( oItem, "bn_booth_2", 966 );
			m_aItems[ m_iNumItems++ ] = oItem;
			SetupBooth( oItem, "bn_booth_5", 966 );
			m_aItems[ m_iNumItems++ ] = oItem;
			SetupBooth( oItem, "bn_booth_7", 966 );
			m_aItems[ m_iNumItems++ ] = oItem;
		}
		else if ( rcPrice.iTextId == 965 )
		{
			// The booths of the other side
			SetupBooth( oItem, "bn_booth_8", 965 );
			m_aItems[ m_iNumItems++ ] = oItem;
			SetupBooth( oItem, "bn_booth_3", 965 );
			m_aItems[ m_iNumItems++ ] = oItem;
			SetupBooth( oItem, "bn_booth_4", 965 );
			m_aItems[ m_iNumItems++ ] = oItem;
			SetupBooth( oItem, "bn_booth_6", 965 );
			m_aItems[ m_iNumItems++ ] = oItem;
		}
		else
		{
			strName.Format( "%s", rcPrice.szInventoryType );
			oItem.SetAddModel( TPString8( strName.GetString() ), rcPrice.iTextId );

			strName.Format( "%s", rcPrice.szModel );
			oItem.SetName( TPString8( strName.GetString() ) );

			// Added once
			if ( FindItem( rcPrice.iTextId ) )
				continue;

			if ( rcPrice.szName3 )
			{
				strName.Format( "%s", rcPrice.szName3 );
				oItem.SetReplaces( TPString8( strName.GetString() ) );
			}

			if ( rcPrice.szName2 )
			{
				strName.Format( "%s", rcPrice.szName2 );
				oItem.SetSubModel( TPString8( strName.GetString() ) );
			}

			oItem.iNumModels          = rcPrice.iMaxCount;
			m_aItems[ m_iNumItems++ ] = oItem;
		}
	}
}

// $Barnyard: FUNCTION 0049bb30
void ABarnExploreMinigameState::UpdateItemCounts()
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		Item& rItem = m_aItems[ i ];

		rItem.iNumBought = AGopherShopInterface::GetNumBought( rItem.iItemId ) + rItem.iNumBought;

#ifdef BARNYARD_COMMUNITY_PATCH
		// Development: "-devbarnall" places every item of the barn as if it had been bought once (the ceiling fans, the games...)
		if ( rItem.iNumBought == 0 && TStringManager::String8FindString( GetCommandLineA(), "-devbarnall" ) )
			rItem.iNumBought = 1;
#endif
		rItem.bEnabled = rItem.iNumBought > 0 || ( rItem.iNumBought == 0 && rItem.HasSubModel() );

		// The vases are placed at the "vase_loc" bones instead (SpawnAtBones)
		if ( rItem.iItemId == 958 )
			rItem.bEnabled = TFALSE;
	}
}

// $Barnyard: FUNCTION 004a1170
void ABarnExploreMinigameState::DisableReplacedItems()
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		Item& rItem = m_aItems[ i ];

		if ( rItem.iNumBought == 0 )
			continue;

		TPString8 strReplaces = rItem.GetReplaces();

		if ( !strReplaces.IsEmpty() )
		{
			Item* pReplaced = FindItem( strReplaces );

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: the original doesn't check that the replaced item exists
			if ( pReplaced )
#endif
				pReplaced->bEnabled = TFALSE;
		}
	}
}

// $Barnyard: FUNCTION 0049ce60
void ABarnExploreMinigameState::UpdateVases()
{
	m_bHasVases = TFALSE;

	if ( FindItem( 958 )->iNumBought == 1 )
		m_bHasVases = TTRUE;

	// The booth lights: all of them
	Item* pBoothLights = FindItem( 967 );

	if ( pBoothLights->iNumBought == 1 )
		pBoothLights->iNumBought = 8;
}

// $Barnyard: FUNCTION 0049bbc0
ABarnExploreMinigameState::Item* ABarnExploreMinigameState::FindItem( TINT a_iItemId )
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( m_aItems[ i ].iItemId == a_iItemId )
			return &m_aItems[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0049fba0
ABarnExploreMinigameState::Item* ABarnExploreMinigameState::FindItem( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( m_aItems[ i ].GetName() == a_rcName )
			return &m_aItems[ i ];
	}

	return TNULL;
}

//-----------------------------------------------------------------------------
// barndata
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0049bc20
const TCHAR* ABarnExploreMinigameState::FindArea( const TPString8& a_rcParent, const TCHAR* a_szBone )
{
	for ( TINT i = 0; i < m_iNumAreaMapEntries; i++ )
	{
		const AreaMapEntry& rcEntry = m_pAreaMap[ i ];

		if ( TStringManager::String8CompareNoCase( a_rcParent.GetString(), rcEntry.szParent ) == 0 && TStringManager::String8CompareNoCase( a_szBone, rcEntry.szBone ) == 0 )
			return rcEntry.szArea;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0049bcb0
const ABarnExploreMinigameState::FSMMapEntry* ABarnExploreMinigameState::FindFSMMapEntry( const TPString8& a_rcParent, const TCHAR* a_szBone )
{
	for ( TINT i = 0; i < m_iNumFSMMapEntries; i++ )
	{
		const FSMMapEntry& rcEntry = m_pFSMMap[ i ];

		if ( ( !rcEntry.szParent || TStringManager::String8CompareNoCase( a_rcParent.GetString(), rcEntry.szParent ) == 0 ) &&
		     TStringManager::String8CompareNoCase( a_szBone, rcEntry.szBone ) == 0 )
			return &rcEntry;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004a2bb0
void ABarnExploreMinigameState::SpawnAtBones()
{
	m_iNumFSMMapEntries  = *TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "barndata", "NumFSMMap", AAssetType_AssetPack ) );
	m_iNumAreaMapEntries = *TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "barndata", "NumAreaMap", AAssetType_AssetPack ) );
	m_pFSMMap            = TSTATICCAST( const FSMMapEntry, AAssetLoader::GetSymbolAddress( "barndata", "FSMMap", AAssetType_AssetPack ) );
	m_pAreaMap           = TSTATICCAST( const AreaMapEntry, AAssetLoader::GetSymbolAddress( "barndata", "AreaMap", AAssetType_AssetPack ) );

	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		Item& rItem = m_aItems[ i ];

		for ( TINT k = 0; k < rItem.GetNumObjects(); k++ )
		{
			// The dance floor has its own dancers (the "dance" spawns of AAnimalPopulationManager)
			TPString8 strName = rItem.GetName();

			if ( strName.GetString8().Find( "dance", 0 ) == -1 )
				SpawnAtBones( rItem.GetObject( k ) );
		}
	}
}

// $Barnyard: FUNCTION 004a1240
void ABarnExploreMinigameState::SpawnAtBones( AAnimatableObject* a_pObject )
{
	AModelInstance*  pModel   = a_pObject->GetModelInstance();
	const TPString8& rcParent = a_pObject->GetObjectType()->GetName();

	for ( TINT i = 0; i < pModel->GetNumBones(); i++ )
	{
		const TCHAR* szBone = pModel->GetBoneName( i );

		if ( T2String8::FindString( szBone, "empty" ) )
			continue;

		// The vases are there once bought
		if ( !m_bHasVases && TStringManager::String8CompareNoCase( szBone, "vase_loc" ) == 0 )
			continue;

		// "<type>_loc"
		TSIZE uiBoneLength = T2String8::Length( szBone );

		if ( TStringManager::String8CompareNoCase( szBone + uiBoneLength - 4, "_loc" ) != 0 )
			continue;

		const TCHAR*       szArea = FindArea( rcParent, szBone );
		const FSMMapEntry* pEntry = FindFSMMapEntry( rcParent, szBone );
		TINT               iTier  = AQuestManager::GetSingleton()->GetAbsQuestTier( AQuestManager::GetSingleton()->GetAct(), AQuestManager::GetSingleton()->GetTier() );

		// Only at the quest tiers of the entry
		if ( pEntry && ( iTier < pEntry->iMinTier || ( pEntry->iMaxTier != 0 && iTier > pEntry->iMaxTier ) ) )
			continue;

		TCHAR szName[ 128 ];
		T2String8::Copy( szName, szBone );
		szName[ uiBoneLength - 4 ] = '\0';
		T2String8::ToLowerCase( szName );

		if ( TStringManager::String8Compare( szName, "random", 6 ) == 0 )
			szName[ 6 ] = '\0';

		if ( !pEntry )
			pEntry = FindFSMMapEntry( rcParent, szName );

		if ( !ResolveUnitType( a_pObject, szName, szName, sizeof( szName ) ) )
			continue;

		TBOOL bGeneric = T2String8::FindString( szName, "generic" ) != TNULL;

		TMatrix44 mBone;
		pModel->GetBoneTransform( i, mBone );

		TVector4 vecPosition = mBone.GetTranslation();
		TVector4 vecDirection( -mBone.m_f21, 0.0f, -mBone.m_f23, 1.0f );
		vecDirection.Normalise();

		AUnitType* pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( TPString8( szName ) );

		if ( pEntry )
		{
			// Not on a party night, only on a party night, the band entries aren't there on band nights (the band is on the stage)
			if ( ( pEntry->uiFlags & FSMFLAGS_NOT_PARTY ) && m_bParty )
				continue;

			if ( ( pEntry->uiFlags & FSMFLAGS_PARTY ) && !m_bParty )
				continue;

			if ( ( pEntry->uiFlags & FSMFLAGS_PARTY ) && TStringManager::String8Compare( pEntry->szActivity, "band" ) == 0 && m_bParty && m_bBandNight )
				continue;
		}

		// Duke is elsewhere during his quest
		if ( TStringManager::String8Compare( "nb_duke", szName ) == 0 )
		{
			TINT iDukeAct;
			TINT iDukeTier;
			TINT iAct  = AQuestManager::GetSingleton()->GetAct();
			TINT iTier = AQuestManager::GetSingleton()->GetTier();
			AQuestManager::GetSingleton()->FindQuestTier( "Duke (Night) Barn Pool", &iDukeAct, &iDukeTier );

			if ( iAct == iDukeAct && iTier == iDukeTier )
				continue;
		}

		// And Otis at the end of act 3
		if ( TStringManager::String8Compare( "nb_otis", szName ) == 0 && AQuestManager::GetSingleton()->GetAct() == 3 &&
		     ( AQuestManager::GetSingleton()->GetTier() == 14 || AQuestManager::GetSingleton()->GetTier() == 15 ) )
			continue;

		if ( pUnitType )
		{
			// An animal (the named ones might exist already)
			AUnit* pExisting = TNULL;
			AUnit* pUnit;

			if ( !bGeneric && ( pExisting = AUnitManager::GetSingleton()->FindUnitByTypeName( TPString8( szName ) ) ) )
			{
				pUnit = pExisting;
				pUnit->SetSteerPosition( vecPosition );
				pUnit->SetSteerDirection( vecDirection );
			}
			else
			{
				pUnit = AUnitManager::GetSingleton()->SpawnUnit( szName, vecPosition, vecDirection );

				TPString8 strName( szName );

				if ( AUnitNameManager::GetSingleton()->GetUnitId( strName ) == -1 )
					AUnitNameManager::GetSingleton()->SetUnitName( strName, pUnit->GetId() );
			}

			if ( szArea )
			{
				// Stands still in its area
				if ( pUnit->ChangeSteer( &TGetClass( AInteriorSteer ) ) )
				{
					AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pUnit->GetSteer() );
					pSteer->SetInteracting( 1 );
					pSteer->SetMode( AInteriorSteer::MODE_STILL );

					TSTATICCAST( AUnitBehaviour, pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) )->SetCommand( TPS8D( "command_forcestandstill" ), TNULL, TTRUE );

					pSteer->ConnectToNetwork( AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork() );
					pSteer->Command_SnapToNamedArea( TPString8( szArea ) );
				}
			}
			else if ( pUnit->ChangeSteer( &TGetClass( AStationarySteer ) ) )
			{
				pUnit->GetSteer()->SetInteracting( 1 );
				TSTATICCAST( AUnitBehaviour, pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) )->SetCommand( TPS8D( "command_forcestandstill" ), TNULL, TTRUE );
				AKinematicsObstructionManager::GetSingleton()->AddObstruction( TSTATICCAST( AKinematicsObstruction, pUnit->GetObstruction() ) );
			}

			AShadowManager::GetSingleton()->RemoveShadow( pUnit );
			SetupUnit( pUnit, a_pObject, pEntry, pExisting != TNULL );
		}
		else
		{
			// An object
			AAnimatableObjectType* pObjectType = AAnimatableObjectManager::GetSingleton()->FindType( TPString8( szName ) );

			if ( !pObjectType )
				continue;

			AAnimatableObject* pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pObjectType, TNULL, 0 );

			if ( m_bHasVases && TStringManager::String8CompareNoCase( szName, "vase" ) == 0 )
				pObject->GetModelInstance()->SetUnlit();

			pObject->GetModelInstance()->SetTransform( vecPosition, vecDirection );
			pObject->SetSkeletonUpdating( TTRUE, TTRUE );
			pObject->SetVisible( TTRUE );
			pObject->GetModelInstance()->SetTranslation( vecPosition.AsVector3() );
			pObject->GetModelInstance()->SetDirection( vecDirection );

			AddCollision( pObject );

			// Objects have bones with more things
			if ( pObject->GetModelInstance()->GetNumBones() > 0 )
				SpawnAtBones( pObject );

			if ( pObject->HasAnimation( TPS8D( "idle_lp" ) ) )
				pObject->PlayAnimation( TPS8D( "idle_lp" ) );

			// The ceiling fans spin
			if ( T2String8::FindString( szName, "fan" ) )
				m_pFanHelper->AddFan( pObject, mBone );

			AddFloorObject( pObject, vecPosition );

			if ( TStringManager::String8CompareNoCase( pObject->GetObjectType()->GetName().GetString(), "bn_foosball_players" ) == 0 )
				m_iFoosballCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::NB_FOOZBALL, vecPosition );
		}
	}
}

// $Barnyard: FUNCTION 0049c570
TBOOL ABarnExploreMinigameState::ResolveUnitType( AAnimatableObject* a_pParent, const TCHAR* a_szName, TCHAR* a_szOutName, TSIZE a_uiSize )
{
	// NOTE: a_szName and a_szOutName are the same buffer when called by SpawnAtBones, the steps below see the result of the previous ones

	// "nb_gen*" is "gen*"
	if ( TStringManager::String8Compare( a_szName, "nb_gen", 6 ) == 0 )
	{
		TString8 strName( a_szName + 3 );
		T2String8::Copy( a_szOutName, strName.GetString() );
	}

	if ( TStringManager::String8Compare( a_szName, "random", 6 ) != 0 )
	{
		// The numbered bones ("nb_duke1") are the unit without the number
		TSIZE uiLength = T2String8::Length( a_szName );

		if ( a_szName[ uiLength - 1 ] >= '0' && a_szName[ uiLength - 1 ] <= '9' )
		{
			T2String8::Copy( a_szOutName, a_szName, a_uiSize );
			a_szOutName[ T2String8::Length( a_szOutName ) - 1 ] = '\0';
		}
	}
	else
	{
		TRandom&     rRandom  = ARandom::GetSingleton()->m_oRandom;
		const TCHAR* szParent = a_pParent->GetObjectType()->GetName().GetString();

		if ( TStringManager::String8Compare( szParent, "nb_generic_table" ) == 0 || T2String8::FindString( szParent, "booth" ) )
		{
			// The seats of the tables and booths can stay empty
			TINT iAnimal = rRandom.GetInt( 5 );

			if ( iAnimal == 4 )
				return TFALSE;

			T2String8::Copy( a_szOutName, s_aTableAnimals[ iAnimal ] );
		}
		else
		{
			T2String8::Copy( a_szOutName, s_aRandomAnimals[ rRandom.GetInt( 5 ) ] );
		}
	}

	// "gen_*" is "nb_generic_*"
	if ( TStringManager::String8Compare( a_szName, "gen_", 4 ) == 0 )
	{
		TString8 strName( a_szName + 4 );
		TString8 strGeneric = "nb_generic_" + strName;
		strName             = strGeneric;

		T2String8::Copy( a_szOutName, strName.GetString(), a_uiSize );
	}

	T2String8::ToLowerCase( a_szOutName );
	return TTRUE;
}

// $Barnyard: FUNCTION 0049fc60
void ABarnExploreMinigameState::SetupUnit( AUnit* a_pUnit, AAnimatableObject* a_pParent, const FSMMapEntry* a_pEntry, TBOOL a_bExisting )
{
	if ( a_pEntry )
	{
		SetupActivity( a_pUnit, a_pParent, a_pEntry );
		SetupProps( a_pUnit, a_pEntry->uiFlags );
	}

	if ( !a_bExisting )
	{
		if ( !a_pEntry || !( a_pEntry->uiFlags & FSMFLAGS_ALWAYS_SHOWN ) )
		{
			if ( a_pUnit->GetSteer()->GetPosition()->y <= 0.1f )
				m_vecGroundCharacters.PushBack( a_pUnit );
			else
				m_vecLoftCharacters.PushBack( a_pUnit );
		}
		else
		{
			m_vecAlwaysShownUnits.PushBack( a_pUnit );
		}
	}

	AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, a_pUnit->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

	if ( pFacial )
		pFacial->SetBlink( TFALSE );

	AddCollision( TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject() );
}

// $Barnyard: FUNCTION 0049e5a0
void ABarnExploreMinigameState::SetupActivity( AUnit* a_pUnit, AAnimatableObject* a_pParent, const FSMMapEntry* a_pEntry )
{
	const TCHAR* szActivity = a_pEntry->szActivity;

	if ( !szActivity )
		return;

	if ( TStringManager::String8CompareNoCase( szActivity, "seated" ) == 0 )
	{
		m_pSeatedHelper->AddUnit( a_pUnit );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "foosball" ) == 0 )
	{
		TPString8          strFoosball( "foosball" );
		AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
		pObject->PlayAnimation( strFoosball );

		// Each one starts at another time of the anim
		TFLOAT fDuration = pObject->GetAnimationDuration( strFoosball );
		pObject->SetAnimationTime( strFoosball, ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, fDuration ) );

		a_pUnit->SetObstructionFlags( AKinematicsObstructionUnit::FLAGS_USE_BONE );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "pool" ) == 0 )
	{
		// Duke plays pool elsewhere during his quest
		TINT iAct  = AQuestManager::GetSingleton()->GetAct();
		TINT iTier = AQuestManager::GetSingleton()->GetTier();
		TINT iDukeAct;
		TINT iDukeTier;
		AQuestManager::GetSingleton()->FindQuestTier( "Duke (Night) Barn Pool", &iDukeAct, &iDukeTier );

		if ( iAct != iDukeAct || iTier != iDukeTier )
		{
			m_pHelper->AddPool( a_pUnit );
			CreateObject( TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject(), "nb_poolcue", "cb_llocator", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
		}
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "pinball" ) == 0 )
	{
		m_pHelper->AddPinball( a_pUnit );
		a_pUnit->SetObstructionFlags( AKinematicsObstructionUnit::FLAGS_USE_BONE );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "freddy" ) == 0 )
	{
		m_pHelper->AddFreddy( a_pUnit );
		a_pUnit->SetObstructionFlags( AKinematicsObstructionUnit::FLAGS_USE_BONE );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "otis" ) == 0 )
	{
		m_pHelper->AddOtis( a_pUnit );
		a_pUnit->SetObstructionFlags( AKinematicsObstructionUnit::FLAGS_USE_BONE );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "darts" ) == 0 )
	{
		m_pHelper->AddDarts( a_pUnit );
		a_pUnit->SetObstructionFlags( AKinematicsObstructionUnit::FLAGS_USE_BONE );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "cheer" ) == 0 )
	{
		m_pHelper->AddCheer( a_pUnit );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "mechman" ) == 0 )
	{
		// Rides the mechanical bull
		AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
		TINT               iBone   = a_pParent->GetModelInstance()->GetBoneID( "nb_gen_cow_loc" );

		AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, a_pParent, -1, TINT8( iBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		pObject->PlayAnimation( TPS8D( "mechman_bucking" ) );

		TMatrix44 mTransform;
		pObject->GetModelInstance()->GetLocalTransform( mTransform );
		m_vecMechManPosition = mTransform.GetTranslation();

		m_iMechManCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::NB_MMAN, m_vecMechManPosition );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "band" ) == 0 )
	{
		m_pHelper->AddBandMember( a_pUnit );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "cards" ) == 0 )
	{
		m_pHelper->AddCards( a_pUnit );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "jersey" ) == 0 )
	{
		m_pChatHelper->AddUnit( a_pUnit );
	}
	else if ( TStringManager::String8CompareNoCase( szActivity, "daisybessy" ) == 0 )
	{
		m_pHelper->AddDaisyBessy( a_pUnit );
	}
	else
	{
		// The activity is an FSM command
		TSTATICCAST( AUnitBehaviour, a_pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) )->SetCommand( TPString8( szActivity ), TNULL, TTRUE );

		if ( a_pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, a_pUnit->GetSteer() );
			pSteer->SetInteracting( 0 );
			pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
		}
	}
}

// $Barnyard: FUNCTION 0049eb60
void ABarnExploreMinigameState::SetupProps( AUnit* a_pUnit, FSMFLAGS a_uiFlags )
{
	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();

	if ( a_uiFlags & FSMFLAGS_SKINSWAP )
		AddSkinSwap( pObject, &s_iSkin, TSTATICCAST( AAnimatedUnit, a_pUnit )->GetNumSkins() );

	if ( a_uiFlags & FSMFLAGS_DRINK )
	{
		const TCHAR* szBone = ( TStringManager::String8CompareNoCase( "nb_otis", a_pUnit->GetType()->GetName().GetString() ) != 0 ) ? "drink_loc" : "cb_llocator";
		CreateObject( pObject, "nb_shake_full", szBone, TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
	}

	if ( a_uiFlags & FSMFLAGS_HENCARDS )
		CreateObject( pObject, "hencards", "drink_loc", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );

	if ( a_uiFlags & FSMFLAGS_SHADOW )
	{
		TINT8 iBone = TINT8( pObject->GetModelInstance()->GetSkeletonInstance()->GetSkeleton()->GetBoneID( "C_Pelvis", 0 ) );
		AShadowManager::GetSingleton()->AddUnitShadow( a_pUnit, 1.25f, iBone, -0.1f, ( a_uiFlags & FSMFLAGS_ALWAYS_SHOWN ) ? 0x28 : 0x20, 0 );
	}

	if ( a_uiFlags & FSMFLAGS_FIDDLE )
	{
		CreateObject( pObject, "fiddlebow", "cb_rlocator", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
		CreateObject( pObject, "fiddle", "fiddle_loc", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
		return;
	}

	if ( a_uiFlags & FSMFLAGS_DRUMS )
	{
		CreateObject( pObject, "drumstick_l", "c_l_hand", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
		CreateObject( pObject, "drumstick_r", "c_r_hand", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
		CreateObject( pObject, "drums", TNULL, TFALSE, *a_pUnit->GetSteer()->GetPosition(), TTRUE, TNULL );
		return;
	}

	if ( a_uiFlags & FSMFLAGS_BASS )
		CreateObject( pObject, "bass", "bass_loc", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );
}

// $Barnyard: FUNCTION 0049bde0
void ABarnExploreMinigameState::AddSkinSwap( AAnimatableObject* a_pObject, TINT* a_pSkin, TINT a_iNumSkins )
{
	ASkinSwap& rSkinSwap = m_aSkinSwaps[ m_iNumSkinSwaps++ ];

	// The next skin that isn't the one of the player
	TINT         iPlayerSkin = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetCostume();
	const TCHAR* szSkin;
	const TCHAR* szPlayerSkin;

	do
	{
		*a_pSkin = ( *a_pSkin + 1 ) % a_iNumSkins;

		szSkin       = a_pObject->GetObjectType()->GetSkinTexture( *a_pSkin );
		szPlayerSkin = TSTATICCAST( AAnimatedUnit, m_pPlayerProxy->GetUnit() )->GetAnimatableObject()->GetObjectType()->GetSkinTexture( iPlayerSkin );
	} while ( TStringManager::String8CompareNoCase( szSkin, szPlayerSkin ) == 0 );

	rSkinSwap.SetObject( a_pObject, *a_pSkin );
}

// $Barnyard: FUNCTION 0049dc40
AAnimatableObject* ABarnExploreMinigameState::CreateObject( AAnimatableObject* a_pParent, const TCHAR* a_szType, const TCHAR* a_szBone, TBOOL a_bAttach, TVector4 a_vecPosition, TBOOL a_bAddToFloor, const TMatrix44* a_pTransform )
{
	TINT      iBone = -1;
	TMatrix44 mTransform;

	if ( a_bAttach )
	{
		iBone = a_pParent->GetModelInstance()->GetBoneID( TPString8( a_szBone ) );

		if ( iBone == -1 )
			return TNULL;

		a_pParent->GetModelInstance()->GetBoneTransform( iBone, mTransform );
	}

	AAnimatableObjectType* pType   = AAnimatableObjectManager::GetSingleton()->FindType( TPString8( a_szType ) );
	AAnimatableObject*     pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pType, TNULL, 0 );

	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pObject->SetOpacity( 1.0f, TTRUE );
	pObject->SetVisible( TTRUE );

	if ( a_bAttach )
	{
		AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, a_pParent, -1, TINT8( iBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		a_pParent->GetModelInstance()->GetLocalTransform( mTransform );

		if ( a_bAddToFloor )
			AddFloorObject( pObject, mTransform.GetTranslation() );
	}
	else
	{
		if ( a_pTransform )
			pObject->GetModelInstance()->SetTransform( *a_pTransform );
		else
			pObject->GetModelInstance()->SetTransform( a_vecPosition, TVector4::VEC_NEGZ );

		if ( a_bAddToFloor )
			AddFloorObject( pObject, a_pTransform ? a_pTransform->GetTranslation() : a_vecPosition );
	}

	return pObject;
}

// $Barnyard: FUNCTION 0049be80
void ABarnExploreMinigameState::AddFloorObject( AAnimatableObject* a_pObject, const TVector4& a_rcPosition )
{
	if ( a_rcPosition.y <= 0.1f )
		m_vecGroundObjects.PushBack( a_pObject );
	else
		m_vecLoftObjects.PushBack( a_pObject );
}

// $Barnyard: FUNCTION 0049df50
void ABarnExploreMinigameState::AddCollision( AAnimatableObject* a_pObject )
{
	if ( a_pObject->GetModelInstance()->GetSceneObject()->GetModelRef()->GetModel()->GetNumCollisionMeshes() <= 0 )
		return;

	ACollisionObjectMesh* pCollision = new ACollisionObjectMesh();
	pCollision->Create( a_pObject->GetModelInstance()->GetSceneObject() );

	m_apCollisions[ m_iNumCollisions++ ] = pCollision;
}

// $Barnyard: FUNCTION 0049bd50
void ABarnExploreMinigameState::DestroyUnits( UnitList& a_rUnits, TBOOL a_bDestroy )
{
	for ( TINT i = 0; i < a_rUnits.Size(); i++ )
	{
		AUnit* pUnit = a_rUnits[ i ];

		if ( pUnit->GetSteer()->IsA( &TGetClass( AStationarySteer ) ) == TTRUE )
			AKinematicsObstructionManager::GetSingleton()->RemoveObstruction( TSTATICCAST( AKinematicsObstruction, pUnit->GetObstruction() ) );

		if ( a_bDestroy )
			AUnitManager::GetSingleton()->DestroyUnit( pUnit );
	}

	for ( TINT i = 0; i < a_rUnits.Size(); i++ )
		a_rUnits[ i ] = TNULL;

	a_rUnits.Clear();
}

//-----------------------------------------------------------------------------
// Night
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004a2d60
void ABarnExploreMinigameState::SetupParty()
{
	AGameTimeManager::GetSingleton()->PushUpdateState();
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetUpdateStamina( TFALSE );

	// The camera behind the player
	AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
	ASteer*                pSteer     = m_pPlayerProxy->GetUnit()->GetSteer();

	TVector4 vecPosition;
	TVector4 vecDirection;
	pSteer->CopyPosition( vecPosition );
	pSteer->CopyDirection( vecDirection );

	TVector4 vecTarget( vecPosition.x, vecPosition.y - pGTAHelper->GetHeightOffset(), vecPosition.z, 1.0f );

	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, -5.0f );

	TVector4 vecCamera( vecOffset.x + vecTarget.x, vecOffset.y + vecTarget.y, vecOffset.z + vecTarget.z, 1.0f );
	pGTAHelper->SetPosition( vecCamera, vecTarget, TTRUE );
	ACameraManager::GetSingleton()->StopPositionLerp( 0.0f );

	if ( m_bNight )
	{
		// The animals and the food
		SpawnAtBones();
		FindFoodSpots();
		SpawnCookedFood();

		// The booth lights are lit at the bought booths
		Item* pBoothLights = FindItem( 967 );

		for ( TINT i = 0; i < pBoothLights->GetNumObjects(); i++ )
		{
			AAnimatableObject* pLight = pBoothLights->GetObject( i );
			pLight->SetSkeletonUpdating( TFALSE, TTRUE );

			TCHAR szBooth[ 64 ];
			TStringManager::String8Format( szBooth, 64, "bn_booth_%i_add", pLight->GetObjectType()->GetName().GetString()[ 9 ] - '0' );

			for ( TINT k = 0; k < m_vecGroundObjects.Size(); k++ )
			{
				if ( TStringManager::String8Compare( szBooth, m_vecGroundObjects[ k ]->GetObjectType()->GetName().GetString() ) == 0 )
				{
					pLight->SetSkeletonUpdating( TTRUE, TTRUE );
					break;
				}
			}
		}

		// The radio, or the wurlitzer once bought
		Item* pDanceFloor = FindItem( 947 );

		if ( pDanceFloor->bEnabled && pDanceFloor->HasSubModel() && pDanceFloor->iNumBought == 0 )
		{
			AMultiplexStream::GetSingleton()->Open( "NB_Radio" );
			m_bWurlitzer = TFALSE;
		}
		else
		{
			AMultiplexStream::GetSingleton()->Open( "NB_Wurlitzer" );
			m_bWurlitzer = TTRUE;
		}

		// The player's shadow
		AUnit* pPlayer = m_pPlayerProxy->GetUnit();
		AShadowManager::GetSingleton()->RemoveShadow( pPlayer );

		TINT8 iPelvis = TINT8( TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance()->GetSkeleton()->GetBoneID( "C_Pelvis", 0 ) );
		AShadowManager::GetSingleton()->AddUnitShadow( pPlayer, 1.25f, iPelvis, -0.1f, 0x20, 0 );

		// The pig's empty shake and the waiter's tray
		// NOTE: the original doesn't check that they exist
		AUnit* pPig = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "nb_pig" ) );

#ifdef BARNYARD_COMMUNITY_PATCH
		if ( pPig )
#endif
			CreateObject( TSTATICCAST( AAnimatedUnit, pPig )->GetAnimatableObject(), "nb_shake_empty", "cb_rlocator", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );

		AUnit* pWaiter = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "waiter" ) );

#ifdef BARNYARD_COMMUNITY_PATCH
		if ( pWaiter )
#endif
			CreateObject( TSTATICCAST( AAnimatedUnit, pWaiter )->GetAnimatableObject(), "bar_tray", "cb_rlocator", TTRUE, TVector4::VEC_ZERO, TTRUE, TNULL );

		// The tapper machine once the replay is unlocked
		if ( AReplayManager::GetSingleton()->IsReplayUnlocked( TPS8D( "barntender_replay" ), TFALSE ) )
		{
			AAnimatableObject* pTapper = CreateObject( TNULL, "bn_tapper_add", TNULL, TFALSE, TVector4::VEC_ZERO, TFALSE, TNULL );
			AddCollision( pTapper );
			m_vecLoftObjects.PushBack( pTapper );

			AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
			SetAreaSpecial( pNetwork->GetArea( pNetwork->FindAreaIndex( TPS8D( "saltlick_area_bm" ) ) ), TTRUE );
		}
		else
		{
			AAnimatableObject* pTapper = CreateObject( TNULL, "bn_tapper_sub", TNULL, TFALSE, TVector4::VEC_ZERO, TFALSE, TNULL );
			AddCollision( pTapper );
			m_vecLoftObjects.PushBack( pTapper );
		}

		// A goat walks between the games of the loft
		if ( ABarnHelper::HasGoatSpots() )
		{
			AUnit* pGoat = AUnitManager::GetSingleton()->SpawnUnit( "nb_generic_goat", TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( pGoat->ChangeSteer( &TGetClass( AInteriorSteer ) ) )
			{
				AInteriorSteer* pGoatSteer = TSTATICCAST( AInteriorSteer, pGoat->GetSteer() );
				pGoatSteer->ConnectToNetwork( AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork() );
				pGoatSteer->Command_SnapToNamedArea( TPS8D( "goat_spawn_area" ) );
			}

			m_pHelper->AddGoat( pGoat );
			m_vecLoftUnits.PushBack( pGoat );
		}

		// The sound of the ground floor plays at the saloon bar
		AAnimatableObject* pSaloonBar = FindItem( 928 )->GetObject( 0 );
		TINT               iBone      = pSaloonBar->GetModelInstance()->GetBoneID( "bn_saloon_bar_add" );

		TMatrix44 mBone;
		pSaloonBar->GetModelInstance()->GetBoneTransform( iBone, mBone );
		s_vecSaloonBarPosition = mBone.GetTranslation();

		if ( FindItem( 965 )->iNumBought == 1 )
			m_iNumBooths1 = 4;

		if ( FindItem( 966 )->iNumBought == 1 )
			m_iNumBooths2 = 4;
	}

	if ( m_bNight )
	{
		// Only the floor the player is on is rendered
		AInstanceManager::GetSingleton()->SetInteriorMode( TTRUE );
		ARegrowthManager::GetSingleton()->SetInteriorMode( TTRUE );

		// Item 940 isn't lit
		Item* pLights = FindItem( 940 );

		if ( pLights && pLights->GetNumObjects() > 0 )
			pLights->GetObject( 0 )->GetModelInstance()->SetUnlit();
	}
}

// $Barnyard: FUNCTION 0049da50
void ABarnExploreMinigameState::FindFoodSpots()
{
	// The "fd_" locators of the barn
	ATerrainLocatorList* pLocators = ATerrainInterface::GetSingleton()->GetLocatorList();

	for ( TINT i = 0; i < pLocators->GetNumLocators(); i++ )
	{
		if ( TStringManager::String8Compare( pLocators->GetLocatorName( i ), "fd_", 3 ) != 0 )
			continue;

		TMatrix44 mList;
		TMatrix44 mLocator;
		pLocators->GetWorldTransform().GetLocalMatrixImp( mList );
		pLocators->GetLocator( i )->GetMatrix( mLocator );

		TMatrix44 mListCopy = mList;
		mList.Multiply( mListCopy, mLocator );

		FoodSpots::Spot& rSpot = m_oFoodSpots.aSpots[ m_oFoodSpots.iNumSpots++ ];
		rSpot.vecPosition      = mList.GetTranslation();
		rSpot.bFree            = TTRUE;
		rSpot.iUnk14           = 0;
	}

	// The "ct_" bones of the saloon bar take the cocktails
	AModelInstance* pSaloonBar = FindItem( 928 )->GetObject( 0 )->GetModelInstance();

	for ( TINT i = 0; i < pSaloonBar->GetNumBones(); i++ )
	{
		if ( TStringManager::String8Compare( pSaloonBar->GetBoneName( i ), "ct_", 3 ) != 0 )
			continue;

		TMatrix44 mBone;
		pSaloonBar->GetBoneTransform( i, mBone );

		FoodSpots::Spot& rSpot = m_oBarFoodSpots.aSpots[ m_oBarFoodSpots.iNumSpots++ ];
		rSpot.vecPosition      = mBone.GetTranslation();
		rSpot.bFree            = TTRUE;
		rSpot.iUnk14           = 0;
	}
}

// $Barnyard: FUNCTION 0049cec0
void ABarnExploreMinigameState::SpawnCookedFood()
{
	TINT*                  pNumRecipes = TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cooking", "NumberOfRecipes", AAssetType_AssetPack ) );
	const RecipeEntryData* pRecipes    = TSTATICCAST( const RecipeEntryData, AAssetLoader::GetSymbolAddress( "cooking", "AllRecipes", AAssetType_AssetPack ) );

	// The dishes cooked in the cooking minigame
	AHackedQuickSave::Buffer* pBuffer = AHackedQuickSave::GetSingleton()->GetBuffer( AHackedQuickSave::BUFFER_COOKING );

	if ( !pBuffer->IsSaved() )
		return;

	pBuffer->SetCursor( 0 );

	TBOOL bUnk;
	TINT  iNumDishes;
	pBuffer->ReadBool( bUnk );
	pBuffer->ReadInt( iNumDishes );

	const RecipeEntryData* pRecipe = TNULL;

	for ( TINT i = 0; i < iNumDishes; i++ )
	{
		TUINT  uiNameString;
		TUINT8 uiType;
		TUINT8 uiUnk;
		pBuffer->ReadUInt( uiNameString );
		pBuffer->ReadUInt8( uiType );
		pBuffer->ReadUInt8( uiUnk );

		// NOTE: the recipe of the previous dish is kept if there's none
		for ( TINT k = 0; k < *pNumRecipes; k++ )
		{
			if ( pRecipes[ k ].iNameString == TINT( uiNameString ) )
			{
				pRecipe = &pRecipes[ k ];
				break;
			}
		}

		// The cocktails go on the bar
		FoodSpots& rSpots = ( uiType == ACookingRecipe::TYPE_COCKTAIL ) ? m_oBarFoodSpots : m_oFoodSpots;

		for ( TINT k = 0; k < rSpots.iNumSpots; k++ )
		{
			FoodSpots::Spot& rSpot = rSpots.aSpots[ k ];

			if ( rSpot.bFree != TTRUE )
				continue;

			TVector4 vecPosition = rSpot.vecPosition;
			rSpot.bFree          = TFALSE;

			AUnit* pDish = AUnitManager::GetSingleton()->SpawnUnit( pRecipe->szName, vecPosition, TVector4::VEC_NEGX );

			if ( vecPosition.y <= 0.1f )
				m_vecGroundCharacters.PushBack( pDish );
			else
				m_vecLoftCharacters.PushBack( pDish );

			break;
		}
	}
}

// $Barnyard: FUNCTION 0049b9c0
ABarnExploreMinigameState::FoodSpots::FoodSpots()
{
	iNumSpots = 0;

	for ( TINT i = 0; i < MAX_NUM_SPOTS; i++ )
	{
		aSpots[ i ].vecPosition = TVector4::VEC_ZERO;
		aSpots[ i ].bFree       = TTRUE;
		aSpots[ i ].iUnk14      = 0;
	}
}

//-----------------------------------------------------------------------------
// Day
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0049ee90
void ABarnExploreMinigameState::CreateLadder()
{
	ATerrainLocator* pLocator = ATerrainInterface::GetSingleton()->GetLocatorList()->FindLocator( "ladder_barn_loc" );

	// The ladder lies above the loft until the FSM drops it (DropLadder)
	m_oLadderTransform.Identity();
	m_oLadderTransform.RotateX( 1.5707964f );
	TMatrix44::TransformVector( m_oLadderTransform.GetTranslation3(), m_oLadderTransform, pLocator->vecPosition );

	m_fLadderDrop = 0.0f;
	m_oLadderTransform.m_f42 -= 2.0f;
	m_fLadderSpeed = 0.0f;

	m_pLadder = CreateObject( TNULL, "ladder_barn", TNULL, TFALSE, TVector4::VEC_ZERO, TTRUE, &m_oLadderTransform );

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	SetAreaSpecial( pNetwork->GetArea( pNetwork->FindAreaIndex( TPS8D( "ladder_area" ) ) ), TTRUE );

	AddCollision( m_pLadder );
}

// $Barnyard: FUNCTION 0049e050
void ABarnExploreMinigameState::SpawnWildMikeBox()
{
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	AInteriorGoal*    pGoal    = pNetwork->FindGoalOrFirst( "wildmike_box" );
	AUnit*            pBox     = AUnitManager::GetSingleton()->SpawnUnitAtGoal( TPS8D( "wildmike_box" ), pNetwork, pGoal, TNULL );

	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, pBox )->GetAnimatableObject();

	TVector4 vecPosition;
	pGoal->GetPosition( vecPosition );
	pObject->GetModelInstance()->SetTranslation( vecPosition.AsVector3() );
	pObject->GetModelInstance()->SetDirection( pGoal->m_vForward );

	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pObject->SetVisible( TTRUE );
	pObject->SetAnimControllersDisabled( TTRUE );

	m_pHelper->AddWildMikeBox( pObject );
	m_vecAlwaysShownUnits.PushBack( pBox );

	AKinematicsObstructionManager::GetSingleton()->AddObstruction( TSTATICCAST( AKinematicsObstruction, pBox->GetObstruction() ) );
}

// $Barnyard: FUNCTION 0049e1f0
void ABarnExploreMinigameState::SpawnQuadCow( TINT a_iIndex )
{
	TCHAR szGoal[ 8 ];
	TStringManager::String8Format( szGoal, sizeof( szGoal ), "quad%i", a_iIndex );

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	AInteriorGoal*    pGoal    = pNetwork->FindGoalOrFirst( szGoal );
	AUnit*            pCow     = AUnitManager::GetSingleton()->SpawnUnitAtGoal( TPS8D( "quad_cow" ), pNetwork, pGoal, TNULL );

	AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, pCow )->GetAnimatableObject();

	TVector4 vecPosition;
	pGoal->GetPosition( vecPosition );
	pObject->GetModelInstance()->SetTranslation( vecPosition.AsVector3() );
	pObject->GetModelInstance()->SetDirection( pGoal->m_vForward );

	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pObject->SetVisible( TTRUE );
	pObject->PlayAnimation( TPS8D( "idle_lp" ) );

	m_vecAlwaysShownUnits.PushBack( pCow );

	// It snores (above its head)
	TINT iHead = pObject->GetModelInstance()->GetBoneIndex( TPS8D( "c_head" ) );

	TMatrix44 mHead;
	pObject->GetModelInstance()->GetBoneTransform( iHead, mHead );

	Snorer& rSnorer     = m_aSnorers[ m_iNumSnorers++ ];
	rSnorer.vecPosition = mHead.GetTranslation();

	// NOTE: inlined TRandom::GetFloatMinMax( 6.0f, 15.0f )
	rSnorer.fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 6.0f, 15.0f );
	rSnorer.vecPosition.y -= 0.5f;

	AKinematicsObstructionManager::GetSingleton()->AddObstruction( TSTATICCAST( AKinematicsObstruction, pCow->GetObstruction() ) );

	AddSkinSwap( pObject, &s_iSkin, TSTATICCAST( AAnimatedUnit, pCow )->GetNumSkins() );
}

//-----------------------------------------------------------------------------
// The ending
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0049f180
void ABarnExploreMinigameState::SetupEnding()
{
	// The throne with Otis and Daisy
	AAnimatableObject* pThrone = CreateObject( TNULL, "throneofmaternity", TNULL, TFALSE, TVector4::VEC_ZERO, TTRUE, TNULL );
	AddCollision( pThrone );

	TMatrix44 mBone;
	pThrone->GetModelInstance()->GetBoneTransform( pThrone->GetModelInstance()->GetBoneID( "otis_loc" ), mBone );

	TVector4 vecPosition = mBone.GetTranslation();
	TVector4 vecDirection( -mBone.m_f21, 0.0f, -mBone.m_f23, 1.0f );
	vecDirection.Normalise();

	m_pEndOtis = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "end_otis" ) );
	m_pEndOtis->GetSteer()->SetInteracting( 1 );
	m_pEndOtis->SetSteerPosition( vecPosition );
	m_pEndOtis->SetSteerDirection( vecDirection );

	// Otis is kept there (UpdateEndOtis)
	m_vecEndOtisPosition  = vecPosition;
	m_vecEndOtisDirection = vecDirection;

	pThrone->GetModelInstance()->GetBoneTransform( pThrone->GetModelInstance()->GetBoneID( "daisy_loc" ), mBone );

	TVector4 vecDaisyPosition = mBone.GetTranslation();
	TVector4 vecDaisyDirection( -mBone.m_f21, 0.0f, -mBone.m_f23, 1.0f );
	vecDaisyDirection.Normalise();

	AUnit* pDaisy = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "end_daisy" ) );
	pDaisy->ChangeSteer( &TGetClass( AStationarySteer ) );
	pDaisy->GetSteer()->SetInteracting( 1 );
	pDaisy->SetSteerPosition( vecDaisyPosition );
	pDaisy->SetSteerDirection( vecDaisyDirection );

	AKinematicsObstructionManager::GetSingleton()->AddObstruction( TSTATICCAST( AKinematicsObstruction, pDaisy->GetObstruction() ) );
}

// $Barnyard: FUNCTION 0049d1d0
void ABarnExploreMinigameState::UpdateEndOtis()
{
	ASteer* pSteer = m_pEndOtis->GetSteer();

	// Unless he's talking to the player
	if ( !pSteer || !pSteer->IsA( &TGetClass( AInteriorSteer ) ) || !TSTATICCAST( AInteriorSteer, pSteer )->IsSpeechDialogMode() )
	{
		m_pEndOtis->SetSteerPosition( m_vecEndOtisPosition );
		m_pEndOtis->SetSteerDirection( m_vecEndOtisDirection );
	}
}

// $Barnyard: FUNCTION 0049f0f0
void ABarnExploreMinigameState::RemoveEndDaisyObstruction()
{
	AUnit* pDaisy = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "end_daisy" ) );
	AKinematicsObstructionManager::GetSingleton()->RemoveObstruction( TSTATICCAST( AKinematicsObstruction, pDaisy->GetObstruction() ) );
}

//-----------------------------------------------------------------------------
// Item
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004a3950
ABarnExploreMinigameState::Item::Item()
{
	iNumModels   = 1;
	bHasSubModel = TFALSE;
	iNumBought   = 0;
	bEnabled     = TTRUE;
}

// $Barnyard: FUNCTION 0049d310
ABarnExploreMinigameState::Item::~Item()
{
}

// $Barnyard: FUNCTION 0049f7e0
ABarnExploreMinigameState::Item& ABarnExploreMinigameState::Item::operator=( const Item& a_rcOther )
{
	vecCollisions = a_rcOther.vecCollisions;
	iNumModels    = a_rcOther.iNumModels;
	strName       = a_rcOther.strName;
	strAddModel   = a_rcOther.strAddModel;
	strSubModel   = a_rcOther.strSubModel;
	bHasSubModel  = a_rcOther.bHasSubModel;
	iItemId       = a_rcOther.iItemId;
	iNumBought    = a_rcOther.iNumBought;
	vecObjects    = a_rcOther.vecObjects;
	strReplaces   = a_rcOther.strReplaces;
	bEnabled      = a_rcOther.bEnabled;

	return *this;
}

// $Barnyard: FUNCTION 004a3980
void ABarnExploreMinigameState::Item::SetAddModel( TPString8 a_strModel, TINT a_iItemId )
{
	strAddModel = a_strModel;
	iItemId     = a_iItemId;
}

// $Barnyard: FUNCTION 004a3a00
void ABarnExploreMinigameState::Item::SetSubModel( TPString8 a_strModel )
{
	strSubModel  = a_strModel;
	bHasSubModel = TTRUE;
}

// $Barnyard: FUNCTION 0049f9d0
void ABarnExploreMinigameState::Item::SetName( TPString8 a_strName )
{
	strName = a_strName;
}

// $Barnyard: FUNCTION 0049d230
void ABarnExploreMinigameState::Item::SetReplaces( const TPString8& a_rcName )
{
	strReplaces = a_rcName;
}

// $Barnyard: FUNCTION 0049fa50
TPString8 ABarnExploreMinigameState::Item::GetName() const
{
	return strName;
}

// $Barnyard: FUNCTION 0049f9a0
TPString8 ABarnExploreMinigameState::Item::GetReplaces() const
{
	return strReplaces;
}

// Replaces the first digit of a_rcModel with a_iNumber + 1 ("bn_table_1_add" -> "bn_table_3_add")
static TPString8 GetNumberedModel( const TPString8& a_rcModel, TINT a_iNumber )
{
	const TString8& rcModel = a_rcModel.GetString8();
	TINT            iDigit  = 0;

	for ( TINT i = 0; i < rcModel.Length(); i++ )
	{
		if ( rcModel[ i ] >= '0' && rcModel[ i ] <= '9' )
		{
			iDigit = i;
			break;
		}
	}

	TCHAR szModel[ 128 ];
	T2String8::Copy( szModel, rcModel.GetString() );
	szModel[ iDigit ] = TCHAR( '1' + a_iNumber );

	TString8 strModel;
	strModel.Format( "%s", szModel );

	return TPString8( strModel.GetString() );
}

// $Barnyard: FUNCTION 004a3e00
void ABarnExploreMinigameState::Item::CreateObjects()
{
	if ( iNumBought == 1 )
	{
		CreateObject( strAddModel, TTRUE );
		return;
	}

	if ( iNumBought == 0 && iNumModels == 1 )
	{
		CreateObject( strSubModel, TTRUE );
		return;
	}

	// Numbered models: the bought ones...
	for ( TINT i = 0; i < iNumBought; i++ )
		CreateObject( GetNumberedModel( strAddModel, i ), TTRUE );

	// ...and the ones that are still to be bought
	if ( bHasSubModel )
	{
		for ( TINT i = iNumBought; i < iNumModels; i++ )
		{
			TPString8 strModel = GetNumberedModel( strSubModel, i );

			if ( TStringManager::String8Compare( "bn_table_6_sub", strModel.GetString() ) != 0 )
				CreateObject( strModel, TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 004a3a80
void ABarnExploreMinigameState::Item::CreateObject( TPString8 a_strType, TBOOL a_bCollision )
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pType    = pManager->FindType( a_strType );

	if ( !pType )
	{
		pManager->LoadAnimObjType( a_strType, TNULL, TFALSE );
		pType = pManager->FindType( a_strType );
	}

	AAnimatableObject* pObject = pManager->CreateAnimatableObject( pType, TNULL, 0 );
	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pObject->SetVisible( TTRUE );
	pObject->GetModelInstance()->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

	TBOOL bAnimated = pType->GetNumAnimationSets() > 0 && pType->GetAnimationSet( 0 )->GetNumNonOverlayAnimations() > 0;

	if ( !bAnimated )
		pObject->AddFlags( AAnimatableObject::FLAGS_NO_ANIMATION );

	vecObjects.PushBack( pObject );

	if ( a_bCollision == TTRUE && pObject->GetModelInstance()->GetSceneObject()->GetModelRef()->GetModel()->GetNumCollisionMeshes() > 0 )
	{
		ACollisionObjectMesh* pCollision = new ACollisionObjectMesh();
		pCollision->Create( pObject->GetModelInstance()->GetSceneObject() );

		vecCollisions.PushBack( pCollision );
	}

	if ( bAnimated )
	{
		if ( pObject->HasAnimation( TPS8D( "idle" ) ) )
			pObject->PlayAnimation( TPS8D( "idle" ) );
		else if ( pObject->HasAnimation( TPS8D( "idle_lp" ) ) )
			pObject->PlayAnimation( TPS8D( "idle_lp" ) );
	}
}

// $Barnyard: FUNCTION 004a4270
void ABarnExploreMinigameState::Item::DestroyObjects()
{
	for ( TINT i = 0; i < vecCollisions.Size(); i++ )
	{
		if ( vecCollisions[ i ]->IsCollisionObjectLinked() )
			vecCollisions[ i ]->UnlinkCollisionObject();
	}

	for ( TINT i = 0; i < vecCollisions.Size(); i++ )
	{
		if ( vecCollisions[ i ] )
			delete vecCollisions[ i ];
	}

	for ( TINT i = 0; i < vecCollisions.Size(); i++ )
		vecCollisions[ i ] = TNULL;

	vecCollisions.Clear();

	// The types of the items are loaded by them (CreateObject)
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < vecObjects.Size(); i++ )
	{
		TPString8 strType = vecObjects[ i ]->GetObjectType()->GetName();

		pManager->DestroyAnimatableObject( vecObjects[ i ] );
		pManager->DestroyType( strType );
	}

	for ( TINT i = 0; i < vecObjects.Size(); i++ )
		vecObjects[ i ] = TNULL;

	vecObjects.Clear();
}

// $Barnyard: FUNCTION 004a43b0
void ABarnExploreMinigameState::Item::DestroySubObjects()
{
	if ( bHasSubModel )
		DestroyObjects();

	bHasSubModel = TFALSE;
}
