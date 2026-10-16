#include "pch.h"
#include "AAnimalPopulationManager.h"
#include "ABarnOpenRequirement.h"
#include "AInventoryGroupComponent.h"
#include "Player/APlayerManager.h"
#include "AUnit.h"
#include "AVisibleUnit.h"
#include "AAnimatedUnit.h"
#include "AUnitType.h"
#include "AUnitManager.h"
#include "AUnitNameManager.h"
#include "AUnitTransform.h"
#include "AInteractionManager.h"
#include "AAnimalAttributes.h"
#include "ANPCBehaviourManager.h"
#include "AUnitBehaviour.h"
#include "AUnitProfileComponent.h"
#include "AUnitGameEventComponent.h"
#include "AInteractionComponent.h"
#include "ANPCBehaviourComponent.h"
#include "AInventoryComponent.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/ACharacterAnimController.h"
#include "Animation/AAlienAnimController.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AGameTimeManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"
#include "GUI/AGUI2ModelRenderer.h"
#include "GUI/AGUI2.h"

#include <Render/TCameraObject.h>
#include "Quests/AQuestManager.h"
#include "Quests/AQuest.h"
#include "Quests/AQuestInstance.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "Physics/ACollisionObject.h"

#include <Render/TRenderInterface.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TSceneObject.h>
#include <Toshi/T2String8.h>
#include <Toshi/T2Vector.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ben );
TPSTRING8_DECLARE( bessy );
TPSTRING8_DECLARE( bud );
TPSTRING8_DECLARE( dag );
TPSTRING8_DECLARE( daisy );
TPSTRING8_DECLARE( duke );
TPSTRING8_DECLARE( eddie );
TPSTRING8_DECLARE( everet );
TPSTRING8_DECLARE( freddy );
TPSTRING8_DECLARE( gopher );
TPSTRING8_DECLARE( hanna );
TPSTRING8_DECLARE( igg );
TPSTRING8_DECLARE( miles );
TPSTRING8_DECLARE( nb_bull );
TPSTRING8_DECLARE( otis );
TPSTRING8_DECLARE( peck );
TPSTRING8_DECLARE( pig );
TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( root );
TPSTRING8_DECLARE( twink );
TPSTRING8_DECLARE( joey );
TPSTRING8_DECLARE( idle_lp );

// Components saved with the units (0x00774174)
static TClass* const s_apSavedUnitComponents[] = {
	&TGetClass( AUnitProfileComponent ),
	&TGetClass( AUnitGameEventComponent ),
	&TGetClass( AInteractionComponent ),
	&TGetClass( AUnitBehaviour ),
	&TGetClass( ANPCBehaviourComponent ),
	&TGetClass( AInventoryComponent ),
};

// NOTE: GetClass 00539ba0, CreateObject 0053aa50, CreateObjectInPlace 0053aab0
TDEFINE_CLASS( AAnimalPopulationManager, 1, 0 );

static TBOOL  s_bRenderMarkersFullBright = TFALSE; // 0x00783c1c, never set
static TFLOAT s_fViewCameraX             = 0.0f;   // 0x00783c20, never set
static TFLOAT s_fViewCameraY             = 0.0f;   // 0x00783c24, never set
static TBOOL  s_bBlink                   = TFALSE; // 0x00783c28
static TFLOAT s_fBlinkTimer              = 0.0f;   // 0x00783c2c


// The main characters, they have a skin for every tier (0x00774120)
static const TPString8* s_apCharacterTypes[] = {
	&TPS8( ben ),
	&TPS8( bessy ),
	&TPS8( bud ),
	&TPS8( dag ),
	&TPS8( daisy ),
	&TPS8( duke ),
	&TPS8( eddie ),
	&TPS8( everet ),
	&TPS8( freddy ),
	&TPS8( gopher ),
	&TPS8( hanna ),
	&TPS8( igg ),
	&TPS8( miles ),
	&TPS8( nb_bull ),
	&TPS8( otis ),
	&TPS8( peck ),
	&TPS8( pig ),
	&TPS8( pip ),
	&TPS8( root ),
	&TPS8( twink ),
	&TPS8( joey ),
};

// NOTE: inlined in the original, sets the state of every entry of a_szType that is called a_szName
static void SetSpawnState( AAnimalPopulationManager::Set<AAnimalPopulationManager::AnimalSpawn>* a_pSet, AAnimalPopulationManager::StringTable* a_pStrings, const TCHAR* a_szType, const TCHAR* a_szName, AAnimalPopulationManager::SPAWNSTATE a_uiState )
{
	for ( TINT i = 0; i < a_pSet->iNumEntries; i++ )
	{
		AAnimalPopulationManager::AnimalSpawn* pEntry = &a_pSet->pEntries[ i ];

		const TCHAR* szName = ( pEntry->oSpawn.iName != 0 ) ? a_pStrings[ AAnimalPopulationManager::STRINGTABLE_UNITNAME ].ppStrings[ pEntry->oSpawn.iName ] : TNULL;
		const TCHAR* szType = ( pEntry->oSpawn.iType != 0 ) ? a_pStrings[ AAnimalPopulationManager::STRINGTABLE_UNITTYPE ].ppStrings[ pEntry->oSpawn.iType ] : TNULL;

		if ( TStringManager::String8Compare( szType, a_szType ) == 0 && TStringManager::String8Compare( szName, a_szName ) == 0 )
			pEntry->uiState = a_uiState;
	}
}

// $Barnyard: FUNCTION 0053a080
AAnimalPopulationManager::AAnimalPopulationManager()
{
	m_eFlags          = FLAGS_NONE;
	m_iUnk24          = 0;
	m_uiSpawnFlags    = 0;
	m_uiStoryFlags    = STORYFLAGS_NONE;
	m_iQuestGiverIcon = -1;

	m_pAnimalSpawnSet      = TNULL;
	m_pQuestTierSpawnSet   = TNULL;
	m_pSaveableUnitSet     = TNULL;
	m_pStringMappingTables = TNULL;
	m_pTRBMemory           = TNULL;
	m_pTRBMemoryCursor     = TNULL;

	for ( TINT i = 0; i < MAX_NUM_MARKERS; i++ )
	{
		m_aMarkers[ i ].pUnit   = TNULL;
		m_aMarkers[ i ].uiFlags = 0;
		m_aMarkers[ i ].pPrev   = TNULL;
		m_aMarkers[ i ].pNext   = TNULL;
	}

	m_pFreeMarkers       = &m_aMarkers[ 0 ];
	m_pUsedMarkers       = TNULL;
	m_pExclamationModel  = TNULL;
	m_pQuestionModel     = TNULL;
	m_pArrowModel        = TNULL;
	m_pExclamationView   = TNULL;
	m_pExclamationObject = TNULL;

	m_pTRBMemory = TSTATICCAST( TUINT8, TMalloc( TRB_MEMORY_SIZE ) );
	m_oTRB.SetMemoryFunctions( TRBAllocator, TRBDeallocator, this );

	// Link all the markers into the free list
	for ( TINT i = 0; i < MAX_NUM_MARKERS; i++ )
	{
		m_aMarkers[ i ].pPrev = &m_aMarkers[ i ] - 1;
		m_aMarkers[ i ].pNext = &m_aMarkers[ i ] + 1;
	}

	m_aMarkers[ 0 ].pPrev                   = TNULL;
	m_aMarkers[ MAX_NUM_MARKERS - 1 ].pNext = TNULL;

	// The "!" on the HUD
	m_pExclamationView = new AGUI2ModelRenderer();
	m_pExclamationView->Create( 100.0f, 100.0f, TFALSE, TTRUE );
	AGUI2::GetRootElement()->AddChildHead( *m_pExclamationView );
}

// $Barnyard: FUNCTION 0053a210
// $Barnyard: FUNCTION 0053aaf0
AAnimalPopulationManager::~AAnimalPopulationManager()
{
	m_oTRB.Close();
	TFree( m_pTRBMemory );

	if ( m_pExclamationView )
	{
		m_pExclamationView->RemoveSelf();
		delete m_pExclamationView;
	}

	AModelRepos* pModelRepos = AModelRepos::GetSingleton();

	if ( m_pExclamationModel )
	{
		pModelRepos->DestroyModelInstance( m_pExclamationModel );
		m_pExclamationModel = TNULL;
	}

	if ( m_pQuestionModel )
	{
		pModelRepos->DestroyModelInstance( m_pQuestionModel );
		m_pQuestionModel = TNULL;
	}

	if ( m_pArrowModel )
	{
		pModelRepos->DestroyModelInstance( m_pArrowModel );
		m_pArrowModel = TNULL;
	}
}

// $Barnyard: FUNCTION 0053cb60
TBOOL AAnimalPopulationManager::Reset()
{
	m_uiSpawnFlags = 0;
	SetMarkersEnabled( TFALSE );

	m_vecDeferredSpawns.Clear();

	// Put all the markers back into the free list
	for ( TINT i = 0; i < MAX_NUM_MARKERS; i++ )
	{
		m_aMarkers[ i ].pPrev   = &m_aMarkers[ i ] - 1;
		m_aMarkers[ i ].pNext   = &m_aMarkers[ i ] + 1;
		m_aMarkers[ i ].pUnit   = TNULL;
		m_aMarkers[ i ].uiFlags = 0;
	}

	m_aMarkers[ MAX_NUM_MARKERS - 1 ].pNext = TNULL;
	m_pUsedMarkers                          = TNULL;
	m_pFreeMarkers                          = &m_aMarkers[ 0 ];
	m_iQuestGiverIcon                       = -1;
	m_aMarkers[ 0 ].pPrev                   = TNULL;

	return TTask::Reset();
}

// $Barnyard: FUNCTION 0053a310
TBOOL AAnimalPopulationManager::OnCreate()
{
	LoadSpawnData();
	return TTask::OnCreate();
}

// $Barnyard: FUNCTION 0053d740
TBOOL AAnimalPopulationManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eFlags & FLAGS_MARKERS )
	{
		UpdateDeferredSpawns( TFALSE );

		ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

		AQuestManager*  pQuestManager  = AQuestManager::GetSingleton();
		AQuestInstance* pQuestInstance = TNULL;

		if ( pQuestManager->GetNumAssignedQuests() > 0 )
			pQuestInstance = pQuestManager->GetAssignedQuest( 0 );

		s_fBlinkTimer += a_fDeltaTime * 3.0f;

		if ( s_fBlinkTimer > 1.0f )
		{
			s_bBlink      = !s_bBlink;
			s_fBlinkTimer = 0.0f;
		}

		for ( Marker* pMarker = m_pUsedMarkers; pMarker != TNULL; pMarker = pMarker->pNext )
		{
			if ( !( pMarker->uiFlags & MARKERFLAGS_ACTIVE ) )
				continue;

			AUnit* pUnit = pMarker->pUnit;
			pMarker->uiFlags |= MARKERFLAGS_HIDDEN;

			TBOOL bShow;

			if ( pQuestInstance == TNULL || ( pMarker->uiFlags & MARKERFLAGS_TEMPORARY ) )
			{
				bShow = TTRUE;
			}
			else
			{
				bShow = IsUnitInQuest( pUnit, pQuestInstance->GetQuest(), TFALSE );
			}

			if ( bShow && TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
				pMarker->uiFlags &= ~MARKERFLAGS_HIDDEN;

			if ( pMiniMap )
				pMiniMap->SetUnitIconVisible( pUnit->GetSelfProxy(), bShow );
		}

		// The quest giver icon at the barn
		if ( pMiniMap && AGameLoader::IsMainGameLoaded() )
		{
			AQuestManager::QuestTier* pTier     = pQuestManager->GetCurrentQuestTier();
			TBOOL                     bShowIcon = TFALSE;
			TBOOL                     bFound    = TFALSE;

			for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
			{
				if ( bFound )
					break;

				if ( ( pTier->uiCompletedMask & TUINT8( 1 << i ) ) == 0 )
				{
					AQuest* pQuest = pQuestManager->GetQuest( pTier->auiQuests[ i ] );

					if ( pQuest->GetName() != TPS8D( "DUMMY QUEST" ) )
					{
						TPString8 strQuestGiver = pQuest->GetQuestGiver();

						if ( ( !( pQuest->GetFlags() & AQuest::FLAGS_NIGHT ) || AGameTimeManager::GetSingleton()->IsItNight() ) &&
						     ( !( pQuest->GetFlags() & AQuest::FLAGS_DAY ) || !AGameTimeManager::GetSingleton()->IsItNight() ) )
						{
							if ( pQuest->GetFlags() & AQuest::FLAGS_UNK2 )
							{
								bShowIcon = TTRUE;
								bFound    = TTRUE;
							}

							if ( pQuestManager->GetNumAssignedQuests() > 0 && pQuestManager->GetAssignedQuest( 0 )->GetQuest() != pQuest )
								bShowIcon = TFALSE;
						}
					}
				}
			}

			if ( m_iQuestGiverIcon == -1 )
			{
				// NOTE: retail's FindGoal (00553780) returns the first goal of the network if there's none of that name
				TVector4 vPosition;
				AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoal( TPString8( "barn_quest_giver" ) )->GetPosition( vPosition );
				m_iQuestGiverIcon = pMiniMap->AddIcon( vPosition, ASimMiniMap::ICON_MISSION, 0.5f, TFALSE );
			}

			pMiniMap->SetIconVisible( m_iQuestGiverIcon, bShowIcon );
		}

		AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

		if ( m_pExclamationObject && m_pExclamationView && pQuestManager->GetNumAssignedQuests() > 0 && pCurrentState && ( pCurrentState->GetHUDParams().m_uiFlags & 0x80 ) )
		{
			m_pExclamationView->Show();

			// Spin the "!"
			TMatrix44 mTransform;
			m_pExclamationObject->GetModelInstance()->GetLocalTransform( mTransform );
			mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4( 0.3f, 0.5f, 0.0f, 1.0f );
			m_pExclamationObject->GetModelInstance()->SetTransform( mTransform );

			TVector3 vecEuler( TMath::HALF_PI, g_oSystemManager.GetScheduler()->GetTotalTime() * TMath::PI, 0.0f );
			m_pExclamationObject->GetModelInstance()->GetTransform().SetEuler( vecEuler );
		}
		else if ( m_pExclamationView )
		{
			m_pExclamationView->Hide();
		}
	}

	return TTask::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00539bb0
void AAnimalPopulationManager::LoadSpawnData()
{
	m_uiSpawnFlags = 0;
	m_oTRB.Close();
	m_pTRBMemoryCursor = m_pTRBMemory;
	m_uiStoryFlags     = STORYFLAGS_NONE;

	m_oTRB.Load( "Data/Units/AnimalSpawn.trb" );

	m_pAnimalSpawnSet      = m_oTRB.CastSymbol<Set<AnimalSpawn>>( "AnimalSpawnSet" );
	m_pQuestTierSpawnSet   = m_oTRB.CastSymbol<Set<QuestTierSpawn>>( "QuestTierSpawnSet" );
	m_pSaveableUnitSet     = m_oTRB.CastSymbol<Set<SaveableUnit>>( "SaveableUnitSet" );
	m_pStringMappingTables = m_oTRB.CastSymbol<StringTable>( "StringMappingTables" );

#ifdef BARNYARD_LATEST
	// NOTE: the latest version (Wii) loads Data/Units/AnimalSpawn_JP.trb, whose quest giver spawns of act 1 follow its
	// quest order (AQuestManager::ms_aLatestAct1Quests). The spawns of the PC data are moved to those tiers here.
	// The act 1 tiers are the first absolute tiers.
	for ( TINT i = 0; i < m_pQuestTierSpawnSet->iNumEntries; i++ )
	{
		QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ i ];
		const TCHAR*    szQuest    = GetString( STRINGTABLE_QUEST, pQuestTier->uiQuest );

		for ( TINT k = 0; k < AQuestManager::LATEST_ACT1_NUM_QUESTS; k++ )
		{
			const AQuestManager::LatestAct1Quest& rcQuest = AQuestManager::ms_aLatestAct1Quests[ k ];

			if ( pQuestTier->uiTier == rcQuest.iPCTier && TStringManager::String8CompareNoCase( szQuest, rcQuest.szQuest ) == 0 )
			{
				pQuestTier->uiTier = TUINT8( rcQuest.iTier );
				break;
			}
		}
	}
#endif // BARNYARD_LATEST
}

// $Barnyard: FUNCTION 0053e510
TBOOL AAnimalPopulationManager::Populate( SPAWNFLAGS a_uiSpawnFlags, TBOOL a_bNPCBehaviour )
{
	if ( a_uiSpawnFlags != 0 )
		m_uiSpawnFlags = a_uiSpawnFlags;

	// The player is the hero once the story of the save slot is complete
	APlayerProgress::SaveSlot* pSlot = APlayerProgress::GetSingleton()->GetCurrentSlot();

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail reads the flags at a null pointer when there's no save slot
	TBOOL bStoryComplete = pSlot && ( pSlot->eFlags & APlayerProgress::SLOTFLAGS_HERO );
#else
	TBOOL bStoryComplete = pSlot->eFlags & APlayerProgress::SLOTFLAGS_HERO;
#endif

	if ( bStoryComplete )
	{
		if ( !( m_uiStoryFlags & STORYFLAGS_NO_COYOTES ) )
		{
			SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "dag", "dag", SPAWNSTATE_DISABLED );
			SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "generic_coyote", "generic_coyote", SPAWNSTATE_DISABLED );
			m_uiStoryFlags |= STORYFLAGS_NO_COYOTES;
		}

		if ( !( m_uiStoryFlags & STORYFLAGS_NO_OTIS ) )
		{
			SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "otis", "otis", SPAWNSTATE_DISABLED );
			m_uiStoryFlags |= STORYFLAGS_NO_OTIS;
		}

		if ( !( m_uiStoryFlags & STORYFLAGS_NO_DAISY ) )
		{
			SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "daisy", "daisy", SPAWNSTATE_DISABLED );
			m_uiStoryFlags |= STORYFLAGS_NO_DAISY;
		}
	}

	for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
	{
		AnimalSpawn*    pEntry     = &m_pAnimalSpawnSet->pEntries[ i ];
		QuestTierSpawn* pQuestTier = TNULL;
		SpawnData*      pSpawn     = TNULL;

		if ( ( pEntry->uiState & SPAWNSTATE_QUESTTIER ) && pEntry->iQuestTierSpawn > 0 )
		{
			pQuestTier = &m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ];
			pSpawn     = &pQuestTier->oSpawn;
		}
		else if ( !( pEntry->uiState & SPAWNSTATE_QUESTTIER ) && ( pEntry->uiState & SPAWNSTATE_ENABLED ) )
		{
			pSpawn = &pEntry->oSpawn;
		}

		if ( pSpawn )
		{
			SPAWNFLAGS uiFlags = pSpawn->uiFlags;

			if ( ( a_uiSpawnFlags & uiFlags ) == a_uiSpawnFlags )
			{
				if ( pQuestTier )
				{
					if ( !( uiFlags & SPAWNFLAGS_SPAWNED ) || ( uiFlags & SPAWNFLAGS_UNK3 ) )
					{
						SpawnUnits( pSpawn, TFALSE );
						pQuestTier->oSpawn.uiFlags |= SPAWNFLAGS_SPAWNED;
						pEntry->uiState &= ~SPAWNSTATE_QUESTSPAWNED;
					}
				}
				else if ( a_bNPCBehaviour && ( m_uiSpawnFlags & SPAWNFLAGS_UNK2 ) )
				{
					// Only the spawns of a single time of the day
					SPAWNFLAGS uiTime      = ( uiFlags & SPAWNFLAGS_NIGHT ) ? SPAWNFLAGS_NIGHT : SPAWNFLAGS_DAY;
					SPAWNFLAGS uiOtherTime = ( uiTime != SPAWNFLAGS_NIGHT ) ? SPAWNFLAGS_NIGHT : SPAWNFLAGS_DAY;

					if ( ( pSpawn->uiFlags & uiTime ) && !( uiOtherTime & uiFlags ) )
					{
						SpawnUnits( pSpawn, TTRUE );
						pEntry->uiState &= ~SPAWNSTATE_QUESTSPAWNED;
					}
				}
				else
				{
					SpawnUnits( pSpawn, TFALSE );
					pEntry->uiState &= ~SPAWNSTATE_QUESTSPAWNED;
				}
			}
			else if ( a_bNPCBehaviour && !pQuestTier && ( m_uiSpawnFlags & SPAWNFLAGS_UNK2 ) && ( uiFlags & SPAWNFLAGS_UNK2 ) )
			{
				SPAWNFLAGS uiTime      = ( uiFlags & SPAWNFLAGS_NIGHT ) ? SPAWNFLAGS_NIGHT : SPAWNFLAGS_DAY;
				SPAWNFLAGS uiOtherTime = ( uiTime != SPAWNFLAGS_NIGHT ) ? SPAWNFLAGS_NIGHT : SPAWNFLAGS_DAY;

				if ( ( pSpawn->uiFlags & uiTime ) && !( pSpawn->uiFlags & uiOtherTime ) )
				{
					TPString8 strName( GetString( STRINGTABLE_UNITNAME, pSpawn->iName ) );
					AUnit*    pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( strName );

					if ( pUnit && !pUnit->IsDying() && pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
					{
						ANPCBehaviourManager::GetSingleton()->AddVisibilityRequest( pUnit, ANPCBehaviourManager::ACTIVITYFLAGS_HIDE );
						pEntry->uiState &= ~SPAWNSTATE_QUESTSPAWNED;
					}
				}
			}
		}

		if ( pEntry->uiState & SPAWNSTATE_QUESTSPAWNED )
		{
			QuestTierSpawn* pOldQuestTier = &m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ];

			if ( ( a_uiSpawnFlags & pOldQuestTier->oSpawn.uiFlags ) == a_uiSpawnFlags && DespawnUnits( pOldQuestTier ) )
			{
				pOldQuestTier->oSpawn.uiFlags &= ~SPAWNFLAGS_SPAWNED;
				pEntry->uiState &= ~( SPAWNSTATE_QUESTTIER | SPAWNSTATE_QUESTSPAWNED );
				pEntry->iQuestTierSpawn = 0;
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0053ea50
TBOOL AAnimalPopulationManager::SetQuestTier( TINT a_iTier, TBOOL a_bSkipCompleted, SPAWNFLAGS a_uiSpawnFlags )
{
	if ( m_uiSpawnFlags == 0 )
		return TTRUE;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( a_iTier >= pQuestManager->GetAbsQuestTier( 2, 19 ) && !( m_uiStoryFlags & STORYFLAGS_BEN_DEAD ) )
	{
		// Ben is dead
		AUnit* pBen = AUnitManager::GetSingleton()->FindUnitByTypeName( TPS8D( "ben" ) );

		if ( pBen )
			AUnitManager::GetSingleton()->DestroyUnit( pBen );

		SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "ben", "ben", SPAWNSTATE_DISABLED );
		SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "ben_grave", "ben_grave", SPAWNSTATE_ENABLED );
		m_uiStoryFlags |= STORYFLAGS_BEN_DEAD;
	}

	if ( a_iTier > pQuestManager->GetAbsQuestTier( 1, 4 ) && !( m_uiStoryFlags & STORYFLAGS_IGG_BUD_EDDIE ) )
	{
		AnimalSpawn oSource;
		oSource.uiState = SPAWNSTATE_ENABLED;

		OverrideSpawns( "igg", "igg", oSource, PLACEFLAGS_STATE, SPAWNFLAGS_UNK2, SPAWNFLAGS_UNK2 | SPAWNFLAGS_UNK3 );
		OverrideSpawns( "bud", "bud", oSource, PLACEFLAGS_STATE, SPAWNFLAGS_UNK2, SPAWNFLAGS_UNK2 | SPAWNFLAGS_UNK3 );
		OverrideSpawns( "eddie", "eddie", oSource, PLACEFLAGS_STATE, SPAWNFLAGS_UNK2, SPAWNFLAGS_UNK2 | SPAWNFLAGS_UNK3 );
		m_uiStoryFlags |= STORYFLAGS_IGG_BUD_EDDIE;
	}

	// NOTE: retail calculates the tiers of these quests without using them
	TINT iAct;
	TINT iTier;
	pQuestManager->FindQuestTier( AQuestManager::GetFlowersForBenQuest(), &iAct, &iTier );
	pQuestManager->GetAbsQuestTier( iAct, iTier );
	pQuestManager->FindQuestTier( AQuestManager::GetRoundUpChicksQuest(), &iAct, &iTier );
	pQuestManager->GetAbsQuestTier( iAct, iTier );

	TBOOL bBarnOpen = ABarnOpenRequirement::IsBarnOpenAtNight( a_iTier );

	if ( !bBarnOpen )
	{
		SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "nb_bull", "nb_bull", SPAWNSTATE_DISABLED );
		m_uiStoryFlags &= ~STORYFLAGS_NB_BULL;
	}
	else if ( !( m_uiStoryFlags & STORYFLAGS_NB_BULL ) )
	{
		SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "nb_bull", "nb_bull", SPAWNSTATE_ENABLED );
		m_uiStoryFlags |= STORYFLAGS_NB_BULL;
	}

	if ( a_iTier > pQuestManager->GetAbsQuestTier( 3, 17 ) && !( m_uiStoryFlags & STORYFLAGS_ENDING ) )
	{
		SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "end_otis", "end_otis", SPAWNSTATE_ENABLED );
		SetSpawnState( m_pAnimalSpawnSet, m_pStringMappingTables, "end_daisy", "end_daisy", SPAWNSTATE_ENABLED );
		m_uiStoryFlags |= STORYFLAGS_ENDING;
	}

	// Otis is away between these tiers
	SPAWNSTATE uiOtisState = ( a_iTier < pQuestManager->GetAbsQuestTier( 3, 14 ) || a_iTier > pQuestManager->GetAbsQuestTier( 3, 16 ) ) ? SPAWNSTATE_ENABLED : SPAWNSTATE_DISABLED;

	for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
	{
		AnimalSpawn* pEntry  = &m_pAnimalSpawnSet->pEntries[ i ];
		SPAWNFLAGS   uiFlags = pEntry->oSpawn.uiFlags;

		const TCHAR* szName = GetString( STRINGTABLE_UNITNAME, pEntry->oSpawn.iName );
		const TCHAR* szType = GetString( STRINGTABLE_UNITTYPE, pEntry->oSpawn.iType );

		if ( TStringManager::String8Compare( szType, "otis" ) == 0 && TStringManager::String8Compare( szName, "otis" ) == 0 && ( uiFlags & SPAWNFLAGS_UNK2 ) )
			pEntry->uiState = uiOtisState;
	}

	for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
		m_pAnimalSpawnSet->pEntries[ i ].uiState &= ~SPAWNSTATE_QUESTTIER;

	T2Vector<const TCHAR*, 3> vecQuests;

	for ( TINT i = 0; i < m_pQuestTierSpawnSet->iNumEntries; i++ )
	{
		QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ i ];

		if ( pQuestTier->uiTier != a_iTier )
			continue;

		const TCHAR* szQuest = GetString( STRINGTABLE_QUEST, pQuestTier->uiQuest );

		if ( a_bSkipCompleted && pQuestManager->IsQuestComplete( TPString8( szQuest ), 1 ) == TTRUE )
			continue;

		if ( a_uiSpawnFlags != 0 && ( a_uiSpawnFlags & pQuestTier->oSpawn.uiFlags ) != a_uiSpawnFlags )
			continue;

		if ( vecQuests.Find( szQuest ) == vecQuests.End() )
			vecQuests.PushBack( szQuest );

		TBOOL bAssigned = TFALSE;

		for ( TINT k = 0; k < m_pAnimalSpawnSet->iNumEntries; k++ )
		{
			AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ k ];

			if ( pEntry->oSpawn.iType == pQuestTier->oSpawn.iType && ( pEntry->oSpawn.iName == 0 || pEntry->oSpawn.iName == pQuestTier->oSpawn.iName ) )
			{
				if ( !bAssigned )
				{
					pEntry->iQuestTierSpawn = i;
					bAssigned               = TTRUE;
				}

				pEntry->uiState |= SPAWNSTATE_QUESTTIER;

				if ( ( m_uiSpawnFlags & pQuestTier->oSpawn.uiFlags ) == m_uiSpawnFlags )
					pQuestTier->oSpawn.uiFlags |= SPAWNFLAGS_SPAWNED;
			}
		}

		if ( ( pQuestTier->oSpawn.uiFlags & m_uiSpawnFlags ) == m_uiSpawnFlags )
			SpawnUnits( &pQuestTier->oSpawn, TTRUE );
	}

	// Let the scripts know about the quests
	T2_FOREACH( vecQuests, it )
	{
		TCHAR szInteraction[ 128 ];
		T2String8::Format( szInteraction, "%s_init", *it );

		AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPString8( szInteraction ) );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0053f4b0
TBOOL AAnimalPopulationManager::ClearQuestTier( TINT a_iTier, const TCHAR* a_szQuest, SPAWNFLAGS a_uiSpawnFlags )
{
	if ( m_uiSpawnFlags == 0 )
		return TTRUE;

	T2Vector<const TCHAR*, 3> vecQuests;

	for ( TINT i = 0; i < m_pQuestTierSpawnSet->iNumEntries; i++ )
	{
		QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ i ];

		if ( pQuestTier->uiTier != a_iTier )
			continue;

		const TCHAR* szQuest = GetString( STRINGTABLE_QUEST, pQuestTier->uiQuest );

		if ( a_szQuest != TNULL && szQuest != TNULL && TStringManager::String8Compare( szQuest, a_szQuest ) != 0 )
			continue;

		if ( a_uiSpawnFlags != 0 && ( a_uiSpawnFlags & pQuestTier->oSpawn.uiFlags ) != a_uiSpawnFlags )
			continue;

		if ( vecQuests.Find( szQuest ) == vecQuests.End() )
			vecQuests.PushBack( szQuest );

		for ( TINT k = 0; k < m_pAnimalSpawnSet->iNumEntries; k++ )
		{
			AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ k ];

			if ( pEntry->oSpawn.iType != pQuestTier->oSpawn.iType || ( pEntry->oSpawn.iName != 0 && pEntry->oSpawn.iName != pQuestTier->oSpawn.iName ) )
				continue;

			if ( !( pEntry->uiState & SPAWNSTATE_QUESTTIER ) )
				continue;

			SPAWNFLAGS uiFlags = pQuestTier->oSpawn.uiFlags;

			if ( ( uiFlags & m_uiSpawnFlags ) == m_uiSpawnFlags || !( uiFlags & SPAWNFLAGS_SPAWNED ) || ( uiFlags & SPAWNFLAGS_UNK3 ) )
			{
				pEntry->uiState &= ~SPAWNSTATE_QUESTSPAWNED;
			}
			else if ( pEntry->iQuestTierSpawn > 0 )
			{
				// Remove the units the next time the spawns are populated
				pEntry->uiState |= SPAWNSTATE_QUESTSPAWNED;
			}

			if ( !( pEntry->uiState & SPAWNSTATE_QUESTSPAWNED ) )
				pEntry->iQuestTierSpawn = 0;

			pEntry->uiState &= ~SPAWNSTATE_QUESTTIER;
		}

		SPAWNFLAGS uiFlags = pQuestTier->oSpawn.uiFlags;

		if ( ( uiFlags & m_uiSpawnFlags ) == m_uiSpawnFlags && ( uiFlags & SPAWNFLAGS_SPAWNED ) )
		{
			DespawnUnits( pQuestTier );
			pQuestTier->oSpawn.uiFlags &= ~SPAWNFLAGS_SPAWNED;
		}
	}

	T2_FOREACH( vecQuests, it )
	{
		TCHAR szInteraction[ 128 ];
		T2String8::Format( szInteraction, "%s_uninit", *it );

		AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPString8( szInteraction ) );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0053e0f0
TBOOL AAnimalPopulationManager::SpawnQuestUnits( const TPString8& a_rcQuest )
{
	if ( m_uiSpawnFlags != 0 )
	{
		for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
		{
			AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ i ];

			if ( pEntry->iQuestTierSpawn <= 0 )
				continue;

			QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ];

			if ( TPString8( GetString( STRINGTABLE_QUEST, pQuestTier->uiQuest ) ) == a_rcQuest )
			{
				if ( ( pQuestTier->oSpawn.uiFlags & m_uiSpawnFlags ) == m_uiSpawnFlags )
				{
					SpawnUnits( &pQuestTier->oSpawn, TFALSE );
					pEntry->uiState &= ~SPAWNSTATE_QUESTTIER;
				}
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00539df0
void AAnimalPopulationManager::OverrideSpawns( const TCHAR* a_szType, const TCHAR* a_szName, const AnimalSpawn& a_rcSource, PLACEFLAGS a_uiMask, SPAWNFLAGS a_uiFlagsValue, SPAWNFLAGS a_uiFlagsMask )
{
	for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
	{
		AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ i ];

		const TCHAR* szName = GetString( STRINGTABLE_UNITNAME, pEntry->oSpawn.iName );
		const TCHAR* szType = GetString( STRINGTABLE_UNITTYPE, pEntry->oSpawn.iType );

		if ( TStringManager::String8Compare( szType, a_szType ) != 0 || TStringManager::String8Compare( szName, a_szName ) != 0 || a_uiFlagsValue != ( a_uiFlagsMask & pEntry->oSpawn.uiFlags ) )
			continue;

		if ( a_uiMask & PLACEFLAGS_ANIMSET ) pEntry->oSpawn.uiAnimSet = a_rcSource.oSpawn.uiAnimSet;
		if ( a_uiMask & PLACEFLAGS_ANIM ) pEntry->oSpawn.uiAnim = a_rcSource.oSpawn.uiAnim;
		if ( a_uiMask & PLACEFLAGS_NETWORK ) pEntry->oSpawn.iNetwork = a_rcSource.oSpawn.iNetwork;
		if ( a_uiMask & PLACEFLAGS_AREA ) pEntry->oSpawn.iArea = a_rcSource.oSpawn.iArea;
		if ( a_uiMask & PLACEFLAGS_GOAL ) pEntry->oSpawn.iGoal = a_rcSource.oSpawn.iGoal;
		if ( a_uiMask & PLACEFLAGS_BEHAVIOUR ) pEntry->oSpawn.iBehaviour = a_rcSource.oSpawn.iBehaviour;
		if ( a_uiMask & PLACEFLAGS_INTERACTION ) pEntry->oSpawn.iInteraction = a_rcSource.oSpawn.iInteraction;
		if ( a_uiMask & PLACEFLAGS_LOCATOR ) pEntry->oSpawn.iLocator = a_rcSource.oSpawn.iLocator;
		if ( a_uiMask & PLACEFLAGS_STATE ) pEntry->uiState = a_rcSource.uiState;
	}
}

// $Barnyard: FUNCTION 00539f70
AAnimalPopulationManager::QuestTierSpawn* AAnimalPopulationManager::FindQuestTierSpawn( const TCHAR* a_szType, const TCHAR* a_szName, const TCHAR* a_szQuest )
{
	// The first entry is unused
	for ( TINT i = 1; i < m_pQuestTierSpawnSet->iNumEntries; i++ )
	{
		QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ i ];

		const TCHAR* szName  = GetString( STRINGTABLE_UNITNAME, pQuestTier->oSpawn.iName );
		const TCHAR* szType  = GetString( STRINGTABLE_UNITTYPE, pQuestTier->oSpawn.iType );
		const TCHAR* szQuest = GetString( STRINGTABLE_QUEST, pQuestTier->uiQuest );

		if ( ( a_szType == TNULL || TStringManager::String8Compare( szType, a_szType ) == 0 ) &&
		     ( a_szName == TNULL || TStringManager::String8Compare( szName, a_szName ) == 0 ) &&
		     TStringManager::String8Compare( szQuest, a_szQuest ) == 0 )
		{
			return pQuestTier;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0053a7d0
const TCHAR* AAnimalPopulationManager::GetQuestTierGoal( const TCHAR* a_szType )
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TINT           iTier         = pQuestManager->GetCurAbsQuestTier();

	for ( TINT i = 0; i < m_pQuestTierSpawnSet->iNumEntries; i++ )
	{
		QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ i ];

		if ( pQuestTier->uiTier == iTier && TStringManager::String8CompareNoCase( a_szType, m_pStringMappingTables[ STRINGTABLE_UNITTYPE ].ppStrings[ pQuestTier->oSpawn.iType ] ) == 0 )
			return m_pStringMappingTables[ STRINGTABLE_GOAL ].ppStrings[ pQuestTier->oSpawn.iGoal ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0053f7b0
static void PushBackName( AUnitModelStaticData::ModelNameArray& a_rArray, const TPString8& a_rcName )
{
	TINT iNewSize = a_rArray.iSize + 1;

	if ( iNewSize > a_rArray.iCapacity )
	{
		TINT iGrownCapacity = a_rArray.iCapacity + a_rArray.iUnk;
		a_rArray.SetCapacity( ( iGrownCapacity < iNewSize ) ? iNewSize : iGrownCapacity );
	}

	a_rArray.pData[ a_rArray.iSize++ ] = a_rcName;
}

// $Barnyard: FUNCTION 0053f830
void AAnimalPopulationManager::GetQuestUnitNames( const TCHAR* a_szQuest, AUnitModelStaticData::ModelNameArray& a_rNames )
{
	// The first entry is unused
	for ( TINT i = 1; i < m_pQuestTierSpawnSet->iNumEntries; i++ )
	{
		QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ i ];

		if ( TStringManager::String8Compare( GetString( STRINGTABLE_QUEST, pQuestTier->uiQuest ), a_szQuest ) == 0 )
			PushBackName( a_rNames, TPString8( GetString( STRINGTABLE_UNITNAME, pQuestTier->oSpawn.iName ) ) );
	}
}

// $Barnyard: FUNCTION 00539cc0
TBOOL AAnimalPopulationManager::IsSaveableUnit( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_pSaveableUnitSet->iNumEntries; i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_pStringMappingTables[ STRINGTABLE_UNITNAME ].ppStrings[ m_pSaveableUnitSet->pEntries[ i ].uiName ], a_szName ) == 0 )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0053dc40
TBOOL AAnimalPopulationManager::SpawnUnits( SpawnData* a_pSpawn, TBOOL a_bNPCBehaviour )
{
	const TCHAR* szType = m_pStringMappingTables[ STRINGTABLE_UNITTYPE ].ppStrings[ a_pSpawn->iType ];
	const TCHAR* szName = GetString( STRINGTABLE_UNITNAME, a_pSpawn->iName );
	TINT         iCount = a_pSpawn->uiCount;

	AUnitManager*     pUnitManager     = AUnitManager::GetSingleton();
	AUnitNameManager* pUnitNameManager = AUnitNameManager::GetSingleton();

	if ( TUINT( pUnitManager->GetTypeInstanceCount( szType ) ) >= TUINT( iCount ) )
	{
		// All the units already exist, place the named one again
		if ( szName == TNULL )
			return TFALSE;

		AUnit* pUnit = pUnitNameManager->RetrieveByName( TPString8( szName ) );

		if ( pUnit )
		{
			if ( !a_bNPCBehaviour || PrepareNPCBehaviour( pUnit, a_pSpawn, TFALSE, TFALSE, PLACEFLAGS_ALL ) == TTRUE )
				PlaceUnit( pUnit, a_pSpawn, -1, PLACEFLAGS_ALL );

			return TTRUE;
		}
	}

	for ( TINT i = 0; i < iCount; i++ )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, GetCharacterSkin( szType ), -1, TTRUE );
		AUnit*         pUnit = pUnitManager->CreateUnit( pUnitManager->FindUnit( szType ), &oTransform );

		if ( szName )
		{
			TPString8 strName;

			if ( iCount <= 1 )
			{
				strName = TPString8( szName );
			}
			else
			{
				TCHAR szIndexedName[ 128 ];
				TStringManager::String8Format( szIndexedName, sizeof( szIndexedName ), "%s_%d", szName, i );
				strName = TPString8( szIndexedName );
			}

			if ( !pUnitNameManager->RetrieveByName( strName ) )
				pUnitNameManager->SetUnitName( strName, pUnit->GetId() );
		}

		if ( iCount <= 1 )
		{
			if ( !a_bNPCBehaviour || PrepareNPCBehaviour( pUnit, a_pSpawn, TTRUE, TFALSE, PLACEFLAGS_ALL ) == TTRUE )
				PlaceUnit( pUnit, a_pSpawn, -1, PLACEFLAGS_ALL );
		}
		else if ( !a_bNPCBehaviour || PrepareNPCBehaviour( pUnit, a_pSpawn, TTRUE, TFALSE, PLACEFLAGS_ALL ) == TTRUE )
		{
			PlaceUnit( pUnit, a_pSpawn, i + 1, PLACEFLAGS_ALL );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0053e220
TBOOL AAnimalPopulationManager::DespawnUnits( QuestTierSpawn* a_pQuestTierSpawn )
{
	const TCHAR* szName  = GetString( STRINGTABLE_UNITNAME, a_pQuestTierSpawn->oSpawn.iName );
	TBOOL        bResult = TFALSE;

	if ( szName )
	{
		TINT iCount = a_pQuestTierSpawn->oSpawn.uiCount;

		for ( TINT i = 0; i < iCount; i++ )
		{
			TPString8 strName;

			if ( iCount <= 1 )
			{
				strName = TPString8( szName );
			}
			else
			{
				TCHAR szIndexedName[ 128 ];
				TStringManager::String8Format( szIndexedName, sizeof( szIndexedName ), "%s_%d", szName, i );
				strName = TPString8( szIndexedName );
			}

			AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( strName );

			if ( pUnit && !pUnit->IsDying() )
			{
				RemoveMarker( pUnit );

				if ( a_pQuestTierSpawn->oSpawn.uiFlags & SPAWNFLAGS_DESTROY )
				{
					AUnitManager::GetSingleton()->DestroyUnit( pUnit );
				}
				else
				{
					// Put the unit back where its normal spawn wants it
					TINT iSpawn;

					for ( iSpawn = 0; iSpawn < m_pAnimalSpawnSet->iNumEntries; iSpawn++ )
					{
						AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ iSpawn ];

						if ( ( m_uiSpawnFlags & pEntry->oSpawn.uiFlags ) == m_uiSpawnFlags &&
						     pEntry->oSpawn.iType == a_pQuestTierSpawn->oSpawn.iType &&
						     ( a_pQuestTierSpawn->oSpawn.iName == 0 || pEntry->oSpawn.iName == a_pQuestTierSpawn->oSpawn.iName ) )
						{
							break;
						}
					}

					if ( iSpawn < m_pAnimalSpawnSet->iNumEntries )
					{
						SpawnData* pSpawn = &m_pAnimalSpawnSet->pEntries[ iSpawn ].oSpawn;

						if ( PrepareNPCBehaviour( pUnit, pSpawn, TFALSE, TTRUE, 0x263 ) == TTRUE )
						{
							if ( iCount <= 1 )
								PlaceUnit( pUnit, pSpawn, -1, 0x263 );
							else
								PlaceUnit( pUnit, pSpawn, iSpawn + 1, 0x263 );
						}
					}
					else
					{
						AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

						if ( pAttributes )
							pAttributes->SetAttribute( AAnimalAttributes::ATTRIBUTE_COMMAND, 0.0f );
					}
				}

				bResult = TTRUE;
			}
		}
	}

	return bResult;
}

// $Barnyard: FUNCTION 0053bc00
TBOOL AAnimalPopulationManager::PlaceUnit( AUnit* a_pUnit, SpawnData* a_pSpawn, TINT a_iIndex, PLACEFLAGS a_uiFlags )
{
	const TCHAR* szNetwork     = GetString( STRINGTABLE_NETWORK, a_pSpawn->iNetwork );
	const TCHAR* szArea        = GetString( STRINGTABLE_AREA, a_pSpawn->iArea );
	const TCHAR* szGoal        = GetString( STRINGTABLE_GOAL, a_pSpawn->iGoal );
	const TCHAR* szLocator     = GetString( STRINGTABLE_LOCATOR, a_pSpawn->iLocator );
	const TCHAR* szAnimSet     = GetString( STRINGTABLE_ANIMSET, a_pSpawn->uiAnimSet );
	const TCHAR* szAnim        = GetString( STRINGTABLE_ANIM, a_pSpawn->uiAnim );
	const TCHAR* szBehaviour   = GetString( STRINGTABLE_BEHAVIOUR, a_pSpawn->iBehaviour );
	const TCHAR* szInteraction = GetString( STRINGTABLE_INTERACTION, a_pSpawn->iInteraction );
	const TCHAR* szName        = GetString( STRINGTABLE_UNITNAME, a_pSpawn->iName );

	AInteriorNetwork* pNetwork = szNetwork ?
	    AInteriorNetworkManager::GetSingleton()->FindNetwork( TPString8( szNetwork ) ) :
	    AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	// Only interior steers can be placed in networks
	TBOOL bInteriorSteer;

	if ( a_pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
		bInteriorSteer = TTRUE;
	else if ( a_pUnit->GetType()->GetSteerName().GetPooledString() == TNULL && a_pUnit->ChangeSteer( &TGetClass( AInteriorSteer ) ) )
		bInteriorSteer = TTRUE;
	else
		bInteriorSteer = TFALSE;

	TBOOL bPlaced = TFALSE;

	if ( bInteriorSteer )
	{
		AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, a_pUnit->GetSteer() );

		if ( pSteer->GetCurrentNetwork() != pNetwork )
			pSteer->SetNetwork( pNetwork, 1 );

		AQuestManager* pQuestManager = AQuestManager::GetSingleton();
		TINT           iTier         = pQuestManager->GetAct();
		TINT           iQuest        = pQuestManager->GetTier();

		TMatrix44 oLocator;

		if ( IsUnitInQuest( a_pUnit, TNULL, TFALSE ) && FindQuestLocator( oLocator, QUESTLOCATOR_QUEST, szName, iTier, iQuest ) )
		{
			if ( ( szGoal && ( a_uiFlags & PLACEFLAGS_GOAL ) ) || ( szArea && ( a_uiFlags & PLACEFLAGS_AREA ) ) )
			{
				// Stand at the quest locator
				pSteer->SetMode( AInteriorSteer::MODE_STILL );

				TVector4 vecDirection( -oLocator.m_f21, 0.0f, -oLocator.m_f23, 1.0f );
				TVector4 vecPosition = oLocator.GetTranslation();
				vecDirection.Normalise();

				if ( AInteriorNetwork* pSteerNetwork = TSTATICCAST( AInteriorNetwork, pSteer->GetCurrentNetwork() ) )
				{
					TINT iArea = pSteerNetwork->FindContainingArea( vecPosition );

					if ( iArea >= 0 )
						pSteer->SnapToArea( iArea );
				}

				pSteer->SetPosition( vecPosition );
				pSteer->SetDirection( vecDirection );
				pSteer->Command_CancelCurrentCommand();
				bPlaced = TTRUE;
			}
		}

		if ( !bPlaced )
		{
			if ( szGoal && ( a_uiFlags & PLACEFLAGS_GOAL ) )
			{
				pSteer->Command_SnapToGoal( TPString8( szGoal ) );
				pSteer->Command_CancelCurrentCommand();
			}
			else
			{
				if ( szArea && ( a_uiFlags & PLACEFLAGS_AREA ) )
				{
					TINT iArea = pNetwork->FindAreaIndex( TPString8( szArea ) );
					pSteer->SnapToArea( iArea );

					// A random position in the area
					TVector4 vecPosition;
					pNetwork->m_pINetwork->m_pAreas[ iArea ].GetRandomPosition( vecPosition, 0.0f );
					pSteer->SetPosition( vecPosition );

					pSteer->SetStartArea( iArea );
				}

				pSteer->Command_CancelCurrentCommand();
			}
		}
	}
	else if ( szGoal && ( a_uiFlags & PLACEFLAGS_GOAL ) )
	{
		AInteriorGoal* pGoal = TNULL;

		if ( a_iIndex <= 0 )
		{
			pGoal = pNetwork->FindRandomGoal( TPString8( szGoal ) );
		}
		else
		{
			pGoal = pNetwork->FindGoalByIndex( TPString8( szGoal ), a_iIndex - 1 );
		}

		TVector4 vecPosition;
		TBOOL    bHasHeight   = pGoal->GetPosition( vecPosition );
		TVector4 vecDirection = pGoal->m_vForward;

		if ( !vecDirection.IsEqual( TVector4::VEC_ZERO ) )
			a_pUnit->GetSteer()->SetDirection( vecDirection );

		a_pUnit->GetSteer()->SetPosition( vecPosition );

		if ( !bHasHeight && a_pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
			pNetwork->PushGoalInterface( TSTATICCAST( AVisibleUnit, a_pUnit )->GetGoalInterface(), pGoal );
	}
	else if ( szArea && ( a_uiFlags & PLACEFLAGS_AREA ) )
	{
		TINT iArea = pNetwork->FindAreaIndex( TPString8( szArea ) );

		// A random position in the area
		TVector4 vecPosition;
		pNetwork->m_pINetwork->m_pAreas[ iArea ].GetRandomPosition( vecPosition, 0.0f );
		a_pUnit->GetSteer()->SetPosition( vecPosition );
	}
	else if ( szLocator && ( a_uiFlags & PLACEFLAGS_LOCATOR ) )
	{
		ATerrainLocator* pLocator = ATerrainInterface::GetSingleton()->FindLocator( szLocator );

		if ( pLocator )
		{
			TMatrix44 oLocator;
			pLocator->GetMatrix( oLocator );

			// The locators are Z-up
			TMatrix44 oRotation(
			    1.0f, 0.0f, 0.0f, 0.0f,
			    0.0f, 0.0f, 1.0f, 0.0f,
			    0.0f, -1.0f, 0.0f, 0.0f,
			    0.0f, 0.0f, 0.0f, 1.0f
			);

			oLocator = oRotation * oLocator;

			TVector4 vecPosition  = oLocator.GetTranslation();
			TVector4 vecDirection = oLocator.AsBasisVector4( 1 );

			a_pUnit->GetSteer()->SetPosition( vecPosition );
			a_pUnit->GetSteer()->SetDirection( vecDirection );
		}
		else
		{
			AUnitManager::GetSingleton()->DestroyUnit( a_pUnit );
		}
	}

	if ( a_uiFlags & PLACEFLAGS_BEHAVIOUR )
	{
		if ( szBehaviour )
		{
			AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, a_pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) );

#ifdef BARNYARD_COMMUNITY_PATCH
			// Retail expects every unit with a behaviour in AnimalSpawn.trb to have an AUnitBehaviour
			if ( pBehaviour )
#endif
				pBehaviour->SetCommand( TPString8( szBehaviour ), TNULL, TTRUE );
		}
		else
		{
			AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, a_pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

			if ( pAttributes )
				pAttributes->SetAttribute( AAnimalAttributes::ATTRIBUTE_COMMAND, 0.0f );
		}
	}

	SetUnitAnimation( a_pUnit, szAnim, szAnimSet, a_uiFlags );

	if ( szInteraction && ( a_uiFlags & PLACEFLAGS_INTERACTION ) )
		AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPString8( szInteraction ) );

	if ( a_pSpawn->uiFlags & SPAWNFLAGS_MARKER )
	{
		if ( a_pSpawn->uiMarkerFlags & MARKERTYPE_QUESTION )
			SetMarker( a_pUnit, MARKERTYPE_QUESTION, TFALSE );
		else if ( a_pSpawn->uiMarkerFlags & MARKERTYPE_EXCLAMATION )
			SetMarker( a_pUnit, MARKERTYPE_EXCLAMATION, TFALSE );
		else
			SetMarker( a_pUnit, MARKERTYPE_ARROW, TFALSE );
	}

	if ( a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
	{
		// Make sure the model is posed before it is drawn
		TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		AAnimatableObject* pAnimatable = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
		pAnimatable->UpdateAnimControllers( fDeltaTime );
		pAnimatable->GetModelInstance()->GetModel()->Update( fDeltaTime );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0053b5a0
void AAnimalPopulationManager::SetUnitAnimation( AUnit* a_pUnit, const TCHAR* a_szAnim, const TCHAR* a_szAnimSet, PLACEFLAGS a_uiFlags )
{
	if ( !a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
		return;

	TBOOL              bPlayAnim   = ( a_szAnim != TNULL && ( a_uiFlags & PLACEFLAGS_ANIM ) );
	AAnimatableObject* pAnimatable = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();

	TPString8 strAnimSet( a_szAnimSet ? a_szAnimSet : "default" );

	ANamedAnimationSetRef pAnimSet = pAnimatable->GetObjectType()->FindAnimationSet( strAnimSet );

	if ( pAnimSet )
	{
		pAnimatable->GetToshiAnimInterface().SetAnimationSet( pAnimSet );

		AAnimController* pController = pAnimatable->FindAnimController( &TGetClass( ACharacterAnimController ) );

		if ( !pController )
			pController = pAnimatable->FindAnimController( &TGetClass( AAlienAnimController ) );

		if ( pController )
		{
			TSTATICCAST( ACharacterAnimController, pController )->SetAnimationSet( &strAnimSet );
			pAnimatable->KillAllAnimations();
		}
	}

	if ( bPlayAnim )
	{
		pAnimatable->PlayAnimation( TPString8( a_szAnim ) );
	}
	else if ( a_uiFlags & PLACEFLAGS_ANIM )
	{
		// NOTE: inlined AAnimatableObject::HasAnimation, does the animation set of the object have the animation?
		if ( pAnimatable->GetToshiAnimInterface().GetAnimationSet()->GetNamedAnimation( TPS8( idle_lp ) ) )
			pAnimatable->PlayAnimation( TPS8( idle_lp ) );
	}
}

// $Barnyard: FUNCTION 0053ab10
TBOOL AAnimalPopulationManager::PrepareNPCBehaviour( AUnit* a_pUnit, SpawnData* a_pSpawn, TBOOL a_bNewUnit, TBOOL a_bForce, PLACEFLAGS a_uiFlags )
{
	if ( !AGameLoader::IsStoryMode() && !AGameLoader::IsLoadingSavedGame() && AGameLoader::IsChangingTerrain() == TTRUE )
		return TTRUE;

	const TCHAR* szNetwork = GetString( STRINGTABLE_NETWORK, a_pSpawn->iNetwork );
	const TCHAR* szArea    = GetString( STRINGTABLE_AREA, a_pSpawn->iArea );
	const TCHAR* szGoal    = GetString( STRINGTABLE_GOAL, a_pSpawn->iGoal );
	const TCHAR* szLocator = GetString( STRINGTABLE_LOCATOR, a_pSpawn->iLocator );

	TBOOL bHasPlacement = szNetwork || szArea || szGoal || szLocator;

	if ( !a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) || ( !bHasPlacement && !a_bForce ) )
		return TTRUE;

	if ( a_bNewUnit || !TSTATICCAST( AVisibleUnit, a_pUnit )->IsVisible() )
	{
		ANPCBehaviourManager::SetUnitVisible( a_pUnit, TFALSE );
		ANPCBehaviourManager::GetSingleton()->AddVisibilityRequest( a_pUnit, ANPCBehaviourManager::ACTIVITYFLAGS_SHOW );
		return TTRUE;
	}

	if ( !( a_uiFlags & PLACEFLAGS_UNK9 ) )
	{
		ANPCBehaviourManager::GetSingleton()->AddVisibilityRequest( a_pUnit, ANPCBehaviourManager::ACTIVITYFLAGS_HIDE );
	}

	// Place the unit once nobody looks at it
	a_pSpawn->uiFlags &= ~SPAWNFLAGS_DEFERRED;

	for ( TINT i = 0; i < m_vecDeferredSpawns.Size(); i++ )
	{
		SpawnData* pSpawn = m_vecDeferredSpawns[ i ].pSpawn;

		if ( pSpawn == a_pSpawn || ( pSpawn->iType == a_pSpawn->iType && ( a_pSpawn->iName == 0 || pSpawn->iName == a_pSpawn->iName ) ) )
		{
			EraseDeferredSpawn( i );
			break;
		}
	}

	DeferredSpawn oDeferred;
	oDeferred.pSpawn  = a_pSpawn;
	oDeferred.uiFlags = a_uiFlags;
	m_vecDeferredSpawns.PushBack( oDeferred );

	return TFALSE;
}

// $Barnyard: FUNCTION 0053d560
void AAnimalPopulationManager::UpdateDeferredSpawns( TBOOL a_bForce )
{
	TINT i = 0;

	while ( i < m_vecDeferredSpawns.Size() )
	{
		DeferredSpawn& rDeferred = m_vecDeferredSpawns[ i ];
		AUnit*         pUnit     = AUnitNameManager::GetSingleton()->RetrieveByName( TPString8( GetString( STRINGTABLE_UNITNAME, rDeferred.pSpawn->iName ) ) );

		if ( pUnit == TNULL || pUnit->IsDying() )
		{
			i = EraseDeferredSpawn( i );
			continue;
		}

		if ( !a_bForce )
		{
			AVisibleUnit* pVisibleUnit = TSTATICCAST( AVisibleUnit, pUnit );

			if ( ( pVisibleUnit->IsVisible() || ( rDeferred.pSpawn->uiFlags & SPAWNFLAGS_DEFERRED ) ) &&
			     ( !( rDeferred.uiFlags & PLACEFLAGS_UNK9 ) || pVisibleUnit->IsInCameraFrustum() || ( rDeferred.pSpawn->uiFlags & SPAWNFLAGS_DEFERRED ) ) )
			{
				// Still visible, try again later
				i++;
				continue;
			}
		}

		PlaceUnit( pUnit, rDeferred.pSpawn, -1, rDeferred.uiFlags );
		rDeferred.pSpawn->uiFlags |= SPAWNFLAGS_DEFERRED;

		if ( !( rDeferred.uiFlags & PLACEFLAGS_UNK9 ) )
		{
			ANPCBehaviourManager::GetSingleton()->AddVisibilityRequest( pUnit, ANPCBehaviourManager::ACTIVITYFLAGS_SHOW );
		}

		i = EraseDeferredSpawn( i );
	}
}

// $Barnyard: FUNCTION 0053aa00
TINT AAnimalPopulationManager::EraseDeferredSpawn( TINT a_iIndex )
{
	m_vecDeferredSpawns.Erase( m_vecDeferredSpawns.Begin() + a_iIndex );
	return a_iIndex;
}

// $Barnyard: FUNCTION 0053ad00
TBOOL AAnimalPopulationManager::IsUnitInQuest( AUnit* a_pUnit, AQuest* a_pQuest, TBOOL a_bCheckTime )
{
	TPString8 strName = AUnitNameManager::GetSingleton()->GetUnitName( a_pUnit->GetId() );

	if ( strName.GetPooledString() == TNULL || strName.GetString8().Length() == 0 )
		strName = a_pUnit->GetType()->GetName();

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	// TRUE if the quest can't be done at this time of the day
	auto IsOutOfTime = []( AQuest* a_pQuest ) {
		AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
		TBOOL             bIsDay           = !pGameTimeManager->IsItNight() && pGameTimeManager->GetCurrentDayPhase() != AGameTimeManager::DAYPHASE_DUSK;
		TBOOL             bIsNight         = pGameTimeManager->IsItNight();
		AQuest::FLAGS     uiFlags          = a_pQuest->GetFlags();

		return ( ( ( uiFlags & AQuest::FLAGS_DAY ) && !bIsDay ) || ( ( uiFlags & AQuest::FLAGS_NIGHT ) && !bIsNight ) ) &&
		    ( uiFlags & ( AQuest::FLAGS_DAY | AQuest::FLAGS_NIGHT ) ) != ( AQuest::FLAGS_DAY | AQuest::FLAGS_NIGHT );
	};

	if ( a_pQuest )
	{
		if ( a_bCheckTime )
		{
			TBOOL bOutOfTime = IsOutOfTime( a_pQuest );
			return a_pQuest->HasUnit( strName ) && !pQuestManager->IsQuestComplete( a_pQuest->GetName(), 1 ) && !bOutOfTime;
		}

		return a_pQuest->HasUnit( strName );
	}

	// One of the quests of the current tier
	AQuestManager::QuestTier* pTier = pQuestManager->GetCurrentQuestTier();

	for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
	{
		AQuest* pQuest = pQuestManager->GetQuest( pTier->auiQuests[ i ] );

		if ( !pQuest->HasUnit( strName ) )
			continue;

		if ( a_bCheckTime )
		{
			TBOOL bOutOfTime = IsOutOfTime( pQuest );

			if ( pQuestManager->IsQuestComplete( pQuest->GetName(), 1 ) || bOutOfTime )
				continue;
		}

		return TTRUE;
	}

	return TFALSE;
}

#ifdef BARNYARD_LATEST
// Latest version (inlined in FindQuestLocator): the act 1 quests are in another order (see AQuestManager::LoadTierData),
// but the terrain keeps the locators of the PC order, so the quest tier of some quest givers is changed back to it.
static TINT GetPCOrderLocatorTier( const TCHAR* a_szName, TINT a_iAct, TINT a_iTier )
{
	if ( a_iAct == 1 && a_iTier == 3 && TStringManager::String8Compare( a_szName, "otis" ) == 0 ) return 4;
	if ( a_iAct == 1 && a_iTier == 3 && TStringManager::String8Compare( a_szName, "pig" ) == 0 ) return 2;
	if ( a_iAct == 1 && a_iTier == 4 && TStringManager::String8Compare( a_szName, "ben" ) == 0 ) return 5;
	if ( a_iAct == 1 && a_iTier == 4 && TStringManager::String8Compare( a_szName, "duke" ) == 0 ) return 2;

	return a_iTier;
}
#endif // BARNYARD_LATEST

// $Barnyard: FUNCTION 0053a870
// $BarnyardWiiJP: FUNCTION 8029da7c
ATerrainLocator* AAnimalPopulationManager::FindQuestLocator( TMatrix44& a_rOutTransform, QUESTLOCATOR a_iMode, const TCHAR* a_szName, TINT a_iTier, TINT a_iQuest )
{
	if ( TStringManager::String8Compare( "twink", a_szName ) == 0 )
		return TNULL;

	TCHAR              szLocator[ 64 ];
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	if ( a_iMode == QUESTLOCATOR_CONVERSATION )
	{
		if ( TStringManager::String8CompareNoCase( "pip", a_szName ) == 0 )
			return TNULL;

#ifdef BARNYARD_LATEST
		a_iQuest = GetPCOrderLocatorTier( a_szName, a_iTier, a_iQuest );
#endif

		TStringManager::String8Format( szLocator, sizeof( szLocator ), "ql_a%dt%02d_conv_%s", a_iTier, a_iQuest, a_szName );
		ATerrainLocator* pLocator = pTerrain->FindLocator( szLocator );

		if ( pLocator )
		{
			pTerrain->GetLocatorTransform( szLocator, a_rOutTransform );
			a_rOutTransform.RotateZ( TMath::HALF_PI );
		}

		return pLocator;
	}
	else if ( a_iMode == QUESTLOCATOR_QUEST )
	{
#ifdef BARNYARD_LATEST
		a_iQuest = GetPCOrderLocatorTier( a_szName, a_iTier, a_iQuest );
#endif

		TStringManager::String8Format( szLocator, sizeof( szLocator ), "ql_a%dt%02d_%s", a_iTier, a_iQuest, a_szName );
		ATerrainLocator* pLocator = pTerrain->FindLocator( szLocator );

		if ( pLocator )
			pTerrain->GetLocatorTransform( szLocator, a_rOutTransform );

		return pLocator;
	}
	else if ( a_iMode == QUESTLOCATOR_DEFAULT )
	{
		TStringManager::String8Format( szLocator, sizeof( szLocator ), "ql_a%d_default_%s", a_iTier, a_szName );
		ATerrainLocator* pLocator = pTerrain->FindLocator( szLocator );

		if ( pLocator )
			pTerrain->GetLocatorTransform( szLocator, a_rOutTransform );

		return pLocator;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0053bb00
TINT AAnimalPopulationManager::GetCharacterSkin( const TCHAR* a_szType )
{
	TPString8 strType( a_szType );

	for ( TINT i = 0; i < TARRAYSIZE( s_apCharacterTypes ); i++ )
	{
		if ( strType == *s_apCharacterTypes[ i ] )
		{
			TINT iTier = AQuestManager::GetSingleton()->GetAct();
			TINT iSkin = iTier - 1;

			if ( iSkin > 2 )
				iSkin = 2;

			return iSkin;
		}
	}

	return -1;
}

// $Barnyard: FUNCTION 0053c500
void AAnimalPopulationManager::SetMarkersEnabled( TBOOL a_bEnabled )
{
	AModelRepos*              pModelRepos  = AModelRepos::GetSingleton();
	AAnimatableObjectManager* pAnimManager = AAnimatableObjectManager::GetSingleton();

	if ( a_bEnabled )
	{
		m_eFlags |= FLAGS_MARKERS;

		if ( !m_pExclamationModel )
		{
			m_pExclamationModel = pModelRepos->InstantiateNewModel( TPString8( "data/models/exclamation" ), TNULL );
			m_pExclamationModel->SetSkeletonUpdating( TFALSE );
		}

		if ( !m_pQuestionModel )
		{
			m_pQuestionModel = pModelRepos->InstantiateNewModel( TPString8( "data/models/question" ), TNULL );
			m_pQuestionModel->SetSkeletonUpdating( TFALSE );
		}

		if ( !m_pArrowModel )
		{
			m_pArrowModel = pModelRepos->InstantiateNewModel( TPString8( "data/models/arrow" ), TNULL );
			m_pArrowModel->SetSkeletonUpdating( TFALSE );
		}

		if ( !m_pExclamationObject )
		{
			TPString8              strName( "exclamation" );
			AAnimatableObjectType* pType = pAnimManager->FindType( strName );

			if ( !pType )
			{
				pAnimManager->LoadAnimObjType( strName, TNULL, TFALSE );
				pType = pAnimManager->FindType( strName );
			}

			m_pExclamationObject = pAnimManager->CreateAnimatableObject( pType, TNULL, 0 );
		}

		if ( m_pExclamationView )
		{
			if ( m_pExclamationView->GetNumObjects() == 0 )
			{
				m_pExclamationView->AddObject( m_pExclamationObject, TNULL, TNULL );

				if ( m_pExclamationObject )
					ResetExclamationView( TFALSE );
			}
		}
	}
	else
	{
		m_eFlags &= ~FLAGS_MARKERS;

		if ( m_pExclamationView )
		{
			m_pExclamationView->DestroyObjects();
			m_pExclamationObject = TNULL;
		}

		TPString8 strName( "exclamation" );

		if ( pAnimManager->FindType( strName ) )
			pAnimManager->DestroyType( strName );

		if ( m_pExclamationModel )
		{
			pModelRepos->DestroyModelInstance( m_pExclamationModel );
			m_pExclamationModel = TNULL;
		}

		if ( m_pQuestionModel )
		{
			pModelRepos->DestroyModelInstance( m_pQuestionModel );
			m_pQuestionModel = TNULL;
		}

		if ( m_pArrowModel )
		{
			pModelRepos->DestroyModelInstance( m_pArrowModel );
			m_pArrowModel = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 0053b780
void AAnimalPopulationManager::ResetExclamationView( TBOOL a_bSkip )
{
	if ( a_bSkip || !m_pExclamationObject || !m_pExclamationView )
		return;

	// In the bottom right corner
	m_pExclamationView->SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );

	m_pExclamationObject->GetModelInstance()->SetUnlit();
	m_pExclamationObject->GetModelInstance()->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TTRUE)

	TVector3 vecEuler( TMath::HALF_PI, 0.0f, 0.0f );
	m_pExclamationObject->GetModelInstance()->GetTransform().SetEuler( vecEuler );
	m_pExclamationObject->GetModelInstance()->SetOpacity( 1.0f );
	m_pExclamationView->SetObjectOpacity( 0, 1.0f );

	TMatrix44 mTransform;
	m_pExclamationObject->GetModelInstance()->GetLocalTransform( mTransform );

	// NOTE: TMatrix44::Rotate (006c9160) rotates around X, Y and Z
	mTransform.RotateX( 0.0f );
	mTransform.RotateY( 0.0f );
	mTransform.RotateZ( 0.0f );
	mTransform.Scale( 1.0f, 1.0f, 1.0f );
	m_pExclamationView->SetObjectVisible( 0, TTRUE );

	mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_pExclamationObject->GetModelInstance()->SetTransform( mTransform );

	// Look at the "!"
	m_pExclamationView->UpdateViewport();
	m_pExclamationView->GetCamera()->SetFOV( TMath::PI / 3.0f );

	TMatrix44 mCamera                                 = TMatrix44::IDENTITY;
	mCamera.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4( s_fViewCameraY, s_fViewCameraX, -1.5f, 1.0f );
	m_pExclamationView->SetCameraMatrix( mCamera );

	m_pExclamationView->Hide();
}

// $Barnyard: FUNCTION 0053b2b0
void AAnimalPopulationManager::SetMarker( AUnit* a_pUnit, MARKERTYPE a_eType, TBOOL a_bTemporary )
{
	Marker* pMarker;

	for ( pMarker = m_pUsedMarkers; pMarker != TNULL; pMarker = pMarker->pNext )
	{
		if ( pMarker->pUnit == a_pUnit )
			break;
	}

	if ( pMarker )
	{
		if ( pMarker->eType == a_eType )
		{
			pMarker->uiFlags = MARKERFLAGS_ACTIVE;

			if ( a_bTemporary )
				pMarker->uiFlags = MARKERFLAGS_ACTIVE | MARKERFLAGS_TEMPORARY;

			return;
		}

		UnlinkMarker( pMarker, &m_pUsedMarkers );
		pMarker->eType   = a_eType;
		pMarker->uiFlags = MARKERFLAGS_ACTIVE;

		if ( a_bTemporary )
			pMarker->uiFlags = MARKERFLAGS_ACTIVE | MARKERFLAGS_TEMPORARY;
	}
	else
	{
		pMarker = m_pFreeMarkers;

		if ( !pMarker )
			return;

		UnlinkMarker( pMarker, &m_pFreeMarkers );
		pMarker->pUnit   = a_pUnit;
		pMarker->uiFlags = MARKERFLAGS_ACTIVE;

		if ( a_bTemporary )
			pMarker->uiFlags = MARKERFLAGS_ACTIVE | MARKERFLAGS_TEMPORARY;

		pMarker->eType = a_eType;

		if ( ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>() )
			pMiniMap->AddUnitIcon( a_pUnit->GetSelfProxy(), ASimMiniMap::ICON_MISSION, 0.5f, TFALSE );
	}

	if ( a_eType != MARKERTYPE_EXCLAMATION && a_eType != MARKERTYPE_QUESTION && a_eType != MARKERTYPE_ARROW )
		return;

	// Keep the markers of the same type together
	Marker* pSameType;

	for ( pSameType = m_pUsedMarkers; pSameType != TNULL; pSameType = pSameType->pNext )
	{
		if ( pSameType->eType == a_eType )
			break;
	}

	if ( pSameType && pSameType != m_pUsedMarkers )
	{
		// Insert before the first marker of the type
		pMarker->pNext = pSameType;
		pMarker->pPrev = pSameType->pPrev;

		if ( pSameType->pPrev )
			pSameType->pPrev->pNext = pMarker;

		pSameType->pPrev = pMarker;
	}
	else
	{
		// Insert at the head
		pMarker->pNext = m_pUsedMarkers;

		if ( m_pUsedMarkers )
		{
			pMarker->pPrev = m_pUsedMarkers->pPrev;

			if ( m_pUsedMarkers->pPrev )
				m_pUsedMarkers->pPrev->pNext = pMarker;

			m_pUsedMarkers->pPrev = pMarker;
		}

		m_pUsedMarkers = pMarker;
	}
}

// $Barnyard: FUNCTION 0053b4b0
void AAnimalPopulationManager::FreeMarker( Marker* a_pMarker )
{
	if ( ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>() )
		pMiniMap->RemoveUnitIcon( a_pMarker->pUnit->GetSelfProxy() );

	if ( a_pMarker->pNext )
		a_pMarker->pNext->pPrev = a_pMarker->pPrev;

	if ( a_pMarker->pPrev )
		a_pMarker->pPrev->pNext = a_pMarker->pNext;
	else
		m_pUsedMarkers = a_pMarker->pNext;

	a_pMarker->pNext   = TNULL;
	a_pMarker->pPrev   = TNULL;
	a_pMarker->pUnit   = TNULL;
	a_pMarker->uiFlags = 0;

	// Push it to the free list
	a_pMarker->pNext = m_pFreeMarkers;

	if ( m_pFreeMarkers )
	{
		a_pMarker->pPrev = m_pFreeMarkers->pPrev;

		if ( m_pFreeMarkers->pPrev )
			m_pFreeMarkers->pPrev->pNext = a_pMarker;

		m_pFreeMarkers->pPrev = a_pMarker;
	}

	m_pFreeMarkers = a_pMarker;
}

// $Barnyard: FUNCTION 0053d530
void AAnimalPopulationManager::RemoveMarker( AUnit* a_pUnit )
{
	if ( !a_pUnit )
		return;

	for ( Marker* pMarker = m_pUsedMarkers; pMarker != TNULL; pMarker = pMarker->pNext )
	{
		if ( pMarker->pUnit == a_pUnit )
		{
			FreeMarker( pMarker );
			return;
		}
	}
}

// $Barnyard: FUNCTION 00539db0
void AAnimalPopulationManager::SetMarkerActive( AUnit* a_pUnit, TBOOL a_bActive )
{
	if ( !a_pUnit )
		return;

	for ( Marker* pMarker = m_pUsedMarkers; pMarker != TNULL; pMarker = pMarker->pNext )
	{
		if ( pMarker->pUnit == a_pUnit )
		{
			if ( a_bActive )
				pMarker->uiFlags |= MARKERFLAGS_ACTIVE;
			else
				pMarker->uiFlags &= ~MARKERFLAGS_ACTIVE;

			return;
		}
	}
}

// $Barnyard: FUNCTION 0053bbd0
void AAnimalPopulationManager::RemoveTemporaryMarkers()
{
	Marker* pMarker = m_pUsedMarkers;

	while ( pMarker )
	{
		Marker* pNext = pMarker->pNext;

		if ( pMarker->uiFlags & MARKERFLAGS_TEMPORARY )
			FreeMarker( pMarker );

		pMarker = pNext;
	}
}

// $Barnyard: FUNCTION 0053b560
void AAnimalPopulationManager::AddMarkersToMiniMap( ASimMiniMap* a_pMiniMap )
{
	if ( !a_pMiniMap )
		return;

	for ( Marker* pMarker = m_pUsedMarkers; pMarker != TNULL; pMarker = pMarker->pNext )
		a_pMiniMap->AddUnitIcon( pMarker->pUnit->GetSelfProxy(), ASimMiniMap::ICON_MISSION, 0.5f, TFALSE );
}

// $Barnyard: FUNCTION 00539d70
AAnimalPopulationManager::Marker* AAnimalPopulationManager::UnlinkMarker( Marker* a_pMarker, Marker** a_ppHead )
{
	if ( a_pMarker->pNext )
		a_pMarker->pNext->pPrev = a_pMarker->pPrev;

	if ( a_pMarker->pPrev )
		a_pMarker->pPrev->pNext = a_pMarker->pNext;
	else
		*a_ppHead = a_pMarker->pNext;

	a_pMarker->pNext = TNULL;
	a_pMarker->pPrev = TNULL;

	return a_pMarker;
}

// $Barnyard: FUNCTION 0053a320
void AAnimalPopulationManager::RenderMarkers( TBOOL a_bFullBright )
{
	if ( a_bFullBright && !s_bRenderMarkersFullBright )
		return;

	if ( !m_pUsedMarkers || !( m_eFlags & FLAGS_MARKERS ) )
		return;

	TFLOAT fTotalTime = g_oSystemManager.GetScheduler()->GetTotalTime();
	TFLOAT fRotation  = TMath::PI * fTotalTime;
	TFLOAT fBob       = TFLOAT( sin( fmod( fTotalTime, 1.0 ) * TMath::TWO_PI ) * 0.1f );

	TRenderInterface* pRenderer = TRenderInterface::GetSingleton();
	TRenderContext*   pContext  = pRenderer->GetCurrentContext();

	TMatrix44 oSavedLightColour = pRenderer->GetLightColour();
	TVector4  vecSavedAmbient   = pContext->GetAmbientColour();

	TMatrix44 oLightColour = oSavedLightColour;
	TVector4  vecAmbient;
	vecAmbient.w = 0.0f;

	if ( a_bFullBright )
	{
		vecAmbient.x       = 1.0f;
		vecAmbient.y       = 1.0f;
		vecAmbient.z       = 1.0f;
		oLightColour.m_f11 = 0.0f;
		oLightColour.m_f12 = 0.0f;
		oLightColour.m_f13 = 0.0f;
	}
	else
	{
		vecAmbient.x       = 0.75f;
		vecAmbient.y       = 0.75f;
		vecAmbient.z       = 0.75f;
		oLightColour.m_f11 = 0.5f;
		oLightColour.m_f12 = 0.5f;
		oLightColour.m_f13 = 0.5f;
	}

	pContext->SetAmbientColour( vecAmbient );
	oLightColour.m_f14 = 1.0f;
	pRenderer->SetLightColourMatrix( oLightColour );

	// Light the markers from the camera
	TMatrix44 oSavedLightDirection = pRenderer->GetLightDirection();
	TMatrix44 oLightDirection      = oSavedLightDirection;

	{
		const TMatrix44& rcCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();

		TVector4& rLightDir = oLightDirection.AsBasisVector4( 0 );
		rLightDir.x         = -rcCamera.m_f31 + rcCamera.m_f11 + rcCamera.m_f21;
		rLightDir.y         = -rcCamera.m_f32 + rcCamera.m_f12 + rcCamera.m_f22;
		rLightDir.z         = -rcCamera.m_f33 + rcCamera.m_f13 + rcCamera.m_f23;
		rLightDir.Normalise();
	}

	pRenderer->SetLightDirectionMatrix( oLightDirection );

	TVector4 vecCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	for ( Marker* pMarker = m_pUsedMarkers; pMarker != TNULL; pMarker = pMarker->pNext )
	{
		if ( !( pMarker->uiFlags & MARKERFLAGS_ACTIVE ) || ( pMarker->uiFlags & MARKERFLAGS_HIDDEN ) )
			continue;

		AModelInstance* pModel;

		if ( pMarker->eType == MARKERTYPE_QUESTION )
			pModel = m_pQuestionModel;
		else if ( pMarker->eType == MARKERTYPE_ARROW )
			pModel = m_pArrowModel;
		else
			pModel = m_pExclamationModel;

		AVisibleUnit* pUnit        = TSTATICCAST( AVisibleUnit, pMarker->pUnit );
		TSceneObject* pSceneObject = pModel->GetSceneObject();

		TVector4 vecPosition = *pUnit->GetSteer()->GetPosition();
		vecPosition.y += ( -pUnit->GetCollisionObject()->GetHeightScale() - 1.0f ) + fBob;

		// Make the far away markers bigger
		TFLOAT fDistance = TVector4::Distance( vecCameraPos, vecPosition ) - 5.0f;

		if ( fDistance < 0.0f )
			fDistance = 0.0f;
		else if ( fDistance > 40.0f )
			fDistance = 40.0f;

		TFLOAT fScale = fDistance / ( 40.0f - 5.0f ) + 0.5f;

		TMatrix44 oScaledLightDirection = oLightDirection;
		oScaledLightDirection.m_f11 *= fScale;
		oScaledLightDirection.m_f12 *= fScale;
		oScaledLightDirection.m_f13 *= fScale;
		pRenderer->SetLightDirectionMatrix( oScaledLightDirection );

		TMatrix44 oTransform;
		oTransform.Identity();
		TMatrix44::TransformVector( oTransform.GetTranslation(), oTransform, vecPosition );
		oTransform.RotateY( fRotation );
		oTransform.RotateX( TMath::HALF_PI );
		oTransform.Scale( fScale, fScale, fScale );

		pSceneObject->GetTransform().SetMatrix( oTransform );
		pSceneObject->EnableSkeletonUpdate();
		pSceneObject->RenderIfVisible();
		pSceneObject->DisableSkeletonUpdate();
	}

	pContext->SetAmbientColour( vecSavedAmbient );
	pRenderer->SetLightColourMatrix( oSavedLightColour );
	pRenderer->SetLightDirectionMatrix( oSavedLightDirection );
}

// $Barnyard: FUNCTION 0053e000
TBOOL AAnimalPopulationManager::SaveSpawnState( AHackedQuickSave::Buffer* a_pBuffer )
{
	UpdateDeferredSpawns( TTRUE );

	a_pBuffer->WriteInt( m_pAnimalSpawnSet->iNumEntries );

	for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
	{
		AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ i ];

		a_pBuffer->WriteUInt16( pEntry->uiState );
		a_pBuffer->WriteUInt16( pEntry->iQuestTierSpawn );

		if ( pEntry->iQuestTierSpawn > 0 )
			a_pBuffer->WriteUInt8( m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ].oSpawn.uiFlags );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00539c30
TBOOL AAnimalPopulationManager::LoadSpawnState( AHackedQuickSave::Buffer* a_pBuffer )
{
	TINT iNumEntries = 0;
	a_pBuffer->ReadInt( iNumEntries );

	for ( TINT i = 0; i < iNumEntries; i++ )
	{
		AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ i ];

		a_pBuffer->ReadUInt16( pEntry->uiState );

		a_pBuffer->ReadInt16( pEntry->iQuestTierSpawn );

		if ( pEntry->iQuestTierSpawn > 0 )
			a_pBuffer->ReadUInt8( m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ].oSpawn.uiFlags );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0053cc00
TBOOL AAnimalPopulationManager::SaveUnits( AHackedQuickSave::Buffer* a_pBuffer )
{
	TINT iNumSaved = 0;

	for ( TINT i = 0; i < m_pSaveableUnitSet->iNumEntries; i++ )
	{
		if ( SaveUnit( &m_pSaveableUnitSet->pEntries[ i ], a_pBuffer ) )
			iNumSaved++;
	}

	a_pBuffer->WriteInt( iNumSaved );
	return TTRUE;
}

// $Barnyard: FUNCTION 0053e0a0
TBOOL AAnimalPopulationManager::LoadUnits( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < m_pSaveableUnitSet->iNumEntries; i++ )
		LoadUnit( a_pBuffer );

	TINT iNumSaved = 0;
	a_pBuffer->ReadInt( iNumSaved );

	return TTRUE;
}

// $Barnyard: FUNCTION 0053af80
TBOOL AAnimalPopulationManager::SaveUnit( SaveableUnit* a_pUnit, AHackedQuickSave::Buffer* a_pBuffer )
{
	const TCHAR* szType = m_pStringMappingTables[ STRINGTABLE_UNITTYPE ].ppStrings[ a_pUnit->uiType ];
	const TCHAR* szName = m_pStringMappingTables[ STRINGTABLE_UNITNAME ].ppStrings[ a_pUnit->uiName ];

	TPString8 strName( szName );
	TPString8 strType( szType );

	AUnit* pUnit           = AUnitNameManager::GetSingleton()->RetrieveByName( strName );
	TBOOL  bCheckInventory = a_pUnit->bCheckInventory != 0;

	if ( bCheckInventory )
	{
		// Units the player carries aren't saved
		AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
		AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
		AInventoryItem*           pItem      = pInventory->FindItemByType( strType );

		if ( pItem && pItem->GetUnit() == pUnit )
			return TFALSE;
	}

	TBOOL bSave = bCheckInventory;

	if ( !bCheckInventory )
	{
		// Only save it if a quest tier spawn moved it
		for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
		{
			AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ i ];

			if ( pEntry->iQuestTierSpawn > 0 && ( pEntry->uiState & SPAWNSTATE_QUESTTIER ) )
			{
				QuestTierSpawn* pQuestTier = &m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ];

				if ( TStringManager::String8Compare( GetString( STRINGTABLE_UNITNAME, pQuestTier->oSpawn.iName ), szName ) == 0 &&
				     TStringManager::String8Compare( GetString( STRINGTABLE_UNITTYPE, pQuestTier->oSpawn.iType ), szType ) == 0 &&
				     ( pQuestTier->oSpawn.uiFlags & SPAWNFLAGS_SPAWNED ) && ( pQuestTier->oSpawn.uiFlags & SPAWNFLAGS_UNK2 ) )
				{
					bSave = TTRUE;
				}
			}
		}
	}

	if ( !pUnit || pUnit->IsDying() || !bSave )
		return TFALSE;

	a_pBuffer->WriteString( strName );
	a_pBuffer->WriteString( strType );
	a_pBuffer->WriteBool( bCheckInventory );

	// The size of the unit's data is written here once it's known
	TUINT uiSizeCursor = a_pBuffer->GetCursor();
	a_pBuffer->WriteInt( uiSizeCursor );

	a_pBuffer->WriteString( pUnit->GetSteer()->GetClass()->GetName() );
	pUnit->GetSteer()->Serialise( a_pBuffer );

	for ( TINT i = 0; i < TARRAYSIZE( s_apSavedUnitComponents ); i++ )
	{
		AUnitComponent* pComponent = pUnit->FindComponent( s_apSavedUnitComponents[ i ] );

		if ( pComponent )
			pComponent->Save( a_pBuffer );
	}

	// NOTE: the original writes the cursor after the unit's data (where the next unit starts)
	TUINT uiEndCursor = a_pBuffer->GetCursor();
	a_pBuffer->SetCursor( uiSizeCursor );
	a_pBuffer->WriteInt( uiEndCursor );
	a_pBuffer->SetCursor( uiEndCursor );

	return TTRUE;
}

// $Barnyard: FUNCTION 0053cc60
TBOOL AAnimalPopulationManager::LoadUnit( AHackedQuickSave::Buffer* a_pBuffer )
{
	TCHAR szName[ 32 ];
	TCHAR szType[ 32 ];

	if ( a_pBuffer->PeekString( szName, sizeof( szName ) ) != TTRUE || !IsSaveableUnit( szName ) )
		return TFALSE;

	TPString8 strName( szName );
	a_pBuffer->CheckString( strName, TTRUE );
	a_pBuffer->ReadString( szType );
	TPString8 strType( szType );

	TBOOL bCheckInventory;
	a_pBuffer->ReadBool( bCheckInventory );

	TBOOL           bLoad      = bCheckInventory;
	TUINT16         uiMarker   = 0;
	QuestTierSpawn* pQuestTier = TNULL;

	if ( !bCheckInventory )
	{
		for ( TINT i = 0; i < m_pAnimalSpawnSet->iNumEntries; i++ )
		{
			AnimalSpawn* pEntry = &m_pAnimalSpawnSet->pEntries[ i ];

			if ( pEntry->iQuestTierSpawn <= 0 )
				continue;

			QuestTierSpawn* pSpawn = &m_pQuestTierSpawnSet->pEntries[ pEntry->iQuestTierSpawn ];

			if ( pEntry->uiState & SPAWNSTATE_QUESTTIER )
			{
				if ( TStringManager::String8Compare( GetString( STRINGTABLE_UNITNAME, pSpawn->oSpawn.iName ), szName ) == 0 &&
				     TStringManager::String8Compare( GetString( STRINGTABLE_UNITTYPE, pSpawn->oSpawn.iType ), szType ) == 0 &&
				     ( pSpawn->oSpawn.uiFlags & SPAWNFLAGS_SPAWNED ) && ( pSpawn->oSpawn.uiFlags & SPAWNFLAGS_UNK2 ) )
				{
					bLoad      = TTRUE;
					pQuestTier = pSpawn;

					if ( pSpawn->oSpawn.uiFlags & SPAWNFLAGS_MARKER )
						uiMarker = pSpawn->oSpawn.uiMarkerFlags;

					// The units of these quests always get a question mark
					AQuestManager*  pQuestManager = AQuestManager::GetSingleton();
					AQuestInstance* pQuest        = pQuestManager->FindAssignedQuest( TPS8D( "Otis Posse" ), 1 );

					if ( pQuest )
					{
						if ( !pQuest->IsElementDone( 0 ) && TPS8( pig ) == strName ) uiMarker = 2;
						if ( !pQuest->IsElementDone( 1 ) && TPS8( peck ) == strName ) uiMarker = 2;
						if ( !pQuest->IsElementDone( 2 ) && TPS8( freddy ) == strName ) uiMarker = 2;
						if ( !pQuest->IsElementDone( 3 ) && TPS8( pip ) == strName ) uiMarker = 2;
					}

					pQuest = pQuestManager->FindAssignedQuest( TPS8D( "OTIS Flowers For Daisy" ), 1 );

					if ( pQuest && !pQuest->IsElementDone( 0 ) && TPS8( daisy ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Daisy Mud Jumpers 2" ), 1 ) && TPS8( pig ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Pip Hitch-Hiker" ), 1 ) && TPS8( otis ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Daisy Chicken Launch 4" ), 1 ) && TPS8( peck ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Bessy Bike Race 4" ), 1 ) && TPS8( eddie ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Otis Golf 3" ), 1 ) && TPS8( otis ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Daisy Sharp Squirter 5" ), 1 ) && TPS8( ben ) == strName ) uiMarker = 2;
					if ( pQuestManager->FindAssignedQuest( TPS8D( "Otis Golf 3" ), 1 ) && TPS8( otis ) == strName ) uiMarker = 2;
				}
			}
			else if ( pEntry->uiState & SPAWNSTATE_QUESTSPAWNED )
			{
				pEntry->iQuestTierSpawn = 0;
				pEntry->uiState &= ~SPAWNSTATE_QUESTSPAWNED;
			}
		}
	}

	// Where the next unit starts
	TINT iEndCursor = 0;
	a_pBuffer->ReadInt( iEndCursor );

	if ( !bLoad )
	{
		// Skip the unit
		a_pBuffer->SetCursor( iEndCursor );
		return TTRUE;
	}

	AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( strName );

	if ( !pUnit )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, GetCharacterSkin( szType ), -1, TTRUE );
		pUnit = AUnitManager::GetSingleton()->SpawnUnit( strType, &oTransform );
		AUnitNameManager::GetSingleton()->SetUnitName( strName, pUnit->GetId() );
	}

	TPString8 strSteer;
	a_pBuffer->ReadString( strSteer );

	TClass* pSteerClass = const_cast<TClass*>( TClass::Find( strSteer.GetString(), &TGetClass( ASteer ) ) );
	pUnit->ChangeSteer( pSteerClass );

	if ( !bCheckInventory && pSteerClass != &TGetClass( AInteriorSteer ) && pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
	{
		const TCHAR* szNetwork = GetString( STRINGTABLE_NETWORK, pQuestTier->oSpawn.iNetwork );
		const TCHAR* szGoal    = GetString( STRINGTABLE_GOAL, pQuestTier->oSpawn.iGoal );

		if ( szGoal && szNetwork )
		{
			// NOTE: the original ignores the network FindNetwork returns and uses the one in ecx
			AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPString8( szNetwork ) );
			AInteriorGoal*    pGoal    = pNetwork->FindGoal( TPString8( szGoal ) );
			pNetwork->PushGoalInterface( TSTATICCAST( AVisibleUnit, pUnit )->GetGoalInterface(), pGoal );
		}
	}

	pUnit->GetSteer()->Deserialise( a_pBuffer );

	for ( TINT i = 0; i < TARRAYSIZE( s_apSavedUnitComponents ); i++ )
	{
		AUnitComponent* pComponent = pUnit->FindComponent( s_apSavedUnitComponents[ i ] );

		if ( pComponent )
			pComponent->Load( a_pBuffer );
	}

	if ( uiMarker != 0 )
	{
		if ( uiMarker & MARKERTYPE_QUESTION )
			SetMarker( pUnit, MARKERTYPE_QUESTION, TFALSE );
		else if ( uiMarker & MARKERTYPE_EXCLAMATION )
			SetMarker( pUnit, MARKERTYPE_EXCLAMATION, TFALSE );
		else
			SetMarker( pUnit, MARKERTYPE_ARROW, TFALSE );
	}

	if ( !bCheckInventory )
		SetUnitAnimation( pUnit, GetString( STRINGTABLE_ANIM, pQuestTier->oSpawn.uiAnim ), GetString( STRINGTABLE_ANIMSET, pQuestTier->oSpawn.uiAnimSet ), 3 );

	return TTRUE;
}

// $Barnyard: FUNCTION 00539d20
void* __stdcall AAnimalPopulationManager::TRBAllocator( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData )
{
	AAnimalPopulationManager* pManager = TSTATICCAST( AAnimalPopulationManager, a_pUserData );

	TUINT8* pMem = pManager->m_pTRBMemoryCursor;
	pManager->m_pTRBMemoryCursor += a_uiSize;

	return pMem;
}

// NOTE: folded with T2Locale::TRBDeallocator (00539d40)
void __stdcall AAnimalPopulationManager::TRBDeallocator( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData )
{
	// DOES_NOTHING
}
