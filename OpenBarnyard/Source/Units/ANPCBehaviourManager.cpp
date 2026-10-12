#include "pch.h"
#include "ANPCBehaviourManager.h"
#include "ANPCBehaviourComponent.h"
#include "AAnimalPopulationManager.h"
#include "AInteraction.h"
#include "AInteractionComponent.h"
#include "AInteractionSequence.h"
#include "AUnitFactory.h"
#include "AUnitManager.h"
#include "AUnitType.h"
#include "AUnitNameManager.h"
#include "AVisibleUnit.h"
#include "ARandom.h"
#include "Assets/AAssetLoader.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Helpers/AGameLoader.h"
#include "Quests/AQuestManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/Steering/AInteriorSteer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006346e0, CreateObject 006352b0, CreateObjectInPlace 00635310
TDEFINE_CLASS( ANPCBehaviourManager );

TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( ASimState );

// NOTE: inlined in retail, the act of the story
static TINT GetQuestTier()
{
	return AQuestManager::GetSingleton()->GetAct();
}

// $Barnyard: FUNCTION 006351f0
ANPCBehaviourManager::ANPCBehaviourManager()
    : m_uiFlags( FLAGS_NONE )
    , m_iUnk38( 0 )
    , m_pSchedules( TNULL )
    , m_pActivities( TNULL )
    , m_pStringTables( TNULL )
    , m_iCurrentSchedule( -1 )
{
	m_oVisibilityRequests.iNumRequests = 0;

	for ( TINT i = 0; i < MAX_NUM_VISIBILITY_REQUESTS; i++ )
	{
		m_oVisibilityRequests.aRequests[ i ].pUnit   = TNULL;
		m_oVisibilityRequests.aRequests[ i ].uiFlags = ACTIVITYFLAGS_NONE;
	}

	m_oDateChangeListener.Connect( *AGameTimeManager::GetSingleton()->GetDateChangeEmitter(), this, OnDateChangeCallback, 0 );
}

// $Barnyard: FUNCTION 00634850
// $Barnyard: FUNCTION 00634c50
ANPCBehaviourManager::~ANPCBehaviourManager()
{
	m_oDateChangeListener.Disconnect();

	m_iUnk38        = 0;
	m_pSchedules    = TNULL;
	m_pActivities   = TNULL;
	m_pStringTables = TNULL;

	m_oDateChangeListener.Disconnect();
}

// $Barnyard: FUNCTION 006346f0
TBOOL ANPCBehaviourManager::Create()
{
	m_pSchedules    = TSTATICCAST( ScheduleList, AAssetLoader::GetSymbolAddress( "npcschedules", "NPCSchedules", AAssetType_Startup ) );
	m_pActivities   = TSTATICCAST( ActivityList, AAssetLoader::GetSymbolAddress( "npcschedules", "NPCActivities", AAssetType_Startup ) );
	m_pStringTables = TSTATICCAST( StringMappingTables, AAssetLoader::GetSymbolAddress( "npcschedules", "StringMappingTables", AAssetType_Startup ) );

	return TTask::Create();
}

// $Barnyard: FUNCTION 006348e0
TBOOL ANPCBehaviourManager::Reset()
{
	m_uiFlags          = FLAGS_NONE;
	m_iCurrentSchedule = -1;

	ResetActivityFlags();

	for ( TINT i = 0; i < m_oVisibilityRequests.iNumRequests; i++ )
	{
		m_oVisibilityRequests.aRequests[ i ].pUnit   = TNULL;
		m_oVisibilityRequests.aRequests[ i ].uiFlags = ACTIVITYFLAGS_NONE;
	}

	m_oVisibilityRequests.iNumRequests = 0;

	return TTRUE;
}

// $Barnyard: FUNCTION 00635160
TBOOL ANPCBehaviourManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !ASSeqState::IsPlaying() && !AGameLoader::IsBarnExploreMiniGame() && AGameLoader::IsMainGameLoaded() == TTRUE )
	{
		if ( m_uiFlags & FLAGS_UPDATE_SCHEDULES )
			UpdateSchedules();

		UpdateVisibility();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00634820
void ANPCBehaviourManager::RequestScheduleUpdate()
{
	m_iCurrentSchedule = 0;
	m_uiFlags |= FLAGS_UPDATE_SCHEDULES;
}

// $Barnyard: FUNCTION 006347c0
ANPCBehaviourManager::Activity* ANPCBehaviourManager::GetActivity( TINT a_iActivity )
{
	return &m_pActivities->pActivities[ a_iActivity ];
}

// $Barnyard: FUNCTION 006347e0
const TCHAR* ANPCBehaviourManager::GetFSMName( TINT a_iFSM )
{
	return m_pStringTables->ppFSMNames[ a_iFSM ];
}

// $Barnyard: FUNCTION 00634800
const TCHAR* ANPCBehaviourManager::GetLocationName( TINT a_iLocation )
{
	if ( a_iLocation == -1 )
		return TNULL;

	return m_pStringTables->ppLocationNames[ a_iLocation ];
}

// $Barnyard: FUNCTION 00634740
void ANPCBehaviourManager::ResetActivityFlags()
{
	if ( !m_pSchedules || !m_pActivities )
		return;

	for ( TINT i = 0; i < m_pSchedules->iNumSchedules; i++ )
	{
		Schedule* pSchedule = &m_pSchedules->pSchedules[ i ];

		for ( TINT k = 0; k < pSchedule->iNumActivities; k++ )
			pSchedule->puiActivityFlags[ k ] = m_pActivities->pActivities[ pSchedule->piActivities[ k ] ].uiFlags;
	}
}

// $Barnyard: FUNCTION 00634920
TINT ANPCBehaviourManager::PickActivity( TINT a_iSchedule )
{

	AGameTimeManager::DAYPHASE eDayPhase = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();
	TINT                       iWeekDay  = AGameTimeManager::GetSingleton()->GetWeekDay();
	Schedule*                  pSchedule = &m_pSchedules->pSchedules[ a_iSchedule ];

	TUINT8 uiTierMask = TUINT8( 1 << ( GetQuestTier() - 1 ) );

	// The end of the story
	if ( AQuestManager::GetSingleton()->IsGameComplete() )
		uiTierMask = 4;

	TINT aCandidates[ 4 ];
	TINT iNumCandidates = 0;

	for ( TINT i = 0; i < pSchedule->iNumActivities; i++ )
	{
		Activity* pActivity = &m_pActivities->pActivities[ pSchedule->piActivities[ i ] ];

		if ( pActivity->uiDayPhase == eDayPhase && ( uiTierMask & pActivity->uiTierMask ) )
		{
			if ( ( pActivity->uiWeekDay == 7 || pActivity->uiWeekDay == iWeekDay ) && !( pSchedule->puiActivityFlags[ i ] & ACTIVITYFLAGS_DISABLED ) )
			{
				aCandidates[ iNumCandidates++ ] = pSchedule->piActivities[ i ];

				if ( iNumCandidates >= TARRAYSIZE( aCandidates ) )
					break;
			}
		}
	}

	if ( iNumCandidates <= 0 )
		return -1;

	return aCandidates[ ARandom::GetSingleton()->m_oRandom.GetInt( iNumCandidates ) ];
}

// $Barnyard: FUNCTION 00634a20
ANPCBehaviourManager::ACTIVITYFLAGS ANPCBehaviourManager::GetVisibilityFlags( TINT a_iSchedule )
{
	Schedule* pSchedule  = &m_pSchedules->pSchedules[ a_iSchedule ];
	TUINT8    uiTierMask = TUINT8( 1 << ( GetQuestTier() - 1 ) );

	for ( TINT i = 0; i < pSchedule->iNumActivities; i++ )
	{
		Activity* pActivity = &m_pActivities->pActivities[ pSchedule->piActivities[ i ] ];

		if ( pActivity->uiDayPhase == AGameTimeManager::GetSingleton()->GetCurrentDayPhase() && ( uiTierMask & pActivity->uiTierMask ) )
		{
			ACTIVITYFLAGS uiFlags = pSchedule->puiActivityFlags[ i ];

			if ( !( uiFlags & ACTIVITYFLAGS_DISABLED ) && ( uiFlags & ( ACTIVITYFLAGS_SHOW | ACTIVITYFLAGS_HIDE ) ) )
				return uiFlags;
		}
	}

	return ACTIVITYFLAGS_NONE;
}

// $Barnyard: FUNCTION 00634ad0
void ANPCBehaviourManager::SetUnitVisible( AUnit* a_pUnit, TBOOL a_bVisible )
{
	AInteriorNetwork* pNetwork     = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	AVisibleUnit*     pVisibleUnit = TSTATICCAST( AVisibleUnit, a_pUnit );

	if ( a_bVisible )
	{
		pVisibleUnit->SetVisible( TTRUE );
		AUnitManager::GetSingleton()->SetUnitActive( a_pUnit, TTRUE );
		a_pUnit->SetNoQuery( TFALSE );
		a_pUnit->SetHashed( TTRUE );

		AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pInteraction )
		{
			pInteraction->SetAllowNormalInteractions( TTRUE );
			pInteraction->SetAllowConversation( TTRUE );
		}

		if ( a_pUnit->GetType()->GetName() != TPS8( pip ) && a_pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
		{
			pNetwork->AddUnitObstruction( a_pUnit );
		}
	}
	else
	{
		pVisibleUnit->SetVisible( TFALSE );
		AUnitManager::GetSingleton()->SetUnitActive( a_pUnit, TFALSE );
		a_pUnit->SetNoQuery( TTRUE );
		a_pUnit->SetHashed( TFALSE );

		AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pInteraction )
		{
			pInteraction->SetAllowNormalInteractions( TFALSE );
			pInteraction->SetAllowConversation( TFALSE );
		}

		if ( a_pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
		{
			if ( pNetwork->HasUnitObstruction( a_pUnit ) )
				pNetwork->RemoveUnitObstruction( a_pUnit );
		}
	}
}

// $Barnyard: FUNCTION 00634c00
TINT ANPCBehaviourManager::VisibilityRequestList::Erase( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < iNumRequests - 1; i++ )
		aRequests[ i ] = aRequests[ i + 1 ];

	aRequests[ iNumRequests - 1 ].pUnit   = TNULL;
	aRequests[ iNumRequests - 1 ].uiFlags = ACTIVITYFLAGS_NONE;
	iNumRequests--;

	return a_iIndex;
}

// $Barnyard: FUNCTION 00634c70
void ANPCBehaviourManager::UpdateVisibility()
{
	// NOTE: retail takes the end of the list once and steps over the request that
	// follows an erased one, it's handled on the next update
	TINT iEnd = m_oVisibilityRequests.iNumRequests;

	for ( TINT i = 0; i != iEnd; i++ )
	{
		VisibilityRequest* pRequest = &m_oVisibilityRequests.aRequests[ i ];
		AUnit*             pUnit    = pRequest->pUnit;

		if ( !pUnit )
			continue;

		if ( pUnit->IsDying() )
		{
			m_oVisibilityRequests.Erase( i );
			continue;
		}

		// Wait until the unit is out of view, unless it's forced
		if ( TSTATICCAST( AVisibleUnit, pUnit )->IsInCameraFrustumDistance( 25.0f ) && !( pRequest->uiFlags & ACTIVITYFLAGS_FORCE ) )
			continue;

		// Don't hide units that are in a mission interaction
		AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pInteraction && pInteraction->GetSequence() && pInteraction->GetSequence()->GetInteraction() &&
		     ( pInteraction->GetSequence()->GetInteraction()->GetFlags() & ( AInteraction::FLAGS_MISSION_CONVERSATION | AInteraction::FLAGS_MISSION_OBJECTIVE ) ) )
			continue;

		if ( !AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork() )
			continue;

		if ( pRequest->uiFlags & ACTIVITYFLAGS_HIDE )
			SetUnitVisible( pUnit, TFALSE );
		else if ( pRequest->uiFlags & ACTIVITYFLAGS_SHOW )
			SetUnitVisible( pUnit, TTRUE );

		m_oVisibilityRequests.Erase( i );
	}
}

// $Barnyard: FUNCTION 00634db0
void ANPCBehaviourManager::ForceVisibility( AUnit* a_pUnit )
{
	for ( TINT i = 0; i < m_oVisibilityRequests.iNumRequests; i++ )
	{
		if ( m_oVisibilityRequests.aRequests[ i ].pUnit == a_pUnit )
		{
			m_oVisibilityRequests.aRequests[ i ].uiFlags |= ACTIVITYFLAGS_FORCE;
			return;
		}
	}
}

// $Barnyard: FUNCTION 00634e10
void ANPCBehaviourManager::AddVisibilityRequest( AUnit* a_pUnit, ACTIVITYFLAGS a_uiFlags )
{
	if ( !a_pUnit )
		return;

	// Replace the earlier requests of the unit
	// NOTE: same as UpdateVisibility, the request after an erased one isn't checked
	TINT iEnd = m_oVisibilityRequests.iNumRequests;

	for ( TINT i = 0; i != iEnd; i++ )
	{
		if ( m_oVisibilityRequests.aRequests[ i ].pUnit == a_pUnit )
			m_oVisibilityRequests.Erase( i );
	}

	TASSERT( m_oVisibilityRequests.iNumRequests < MAX_NUM_VISIBILITY_REQUESTS );

	VisibilityRequest* pRequest = &m_oVisibilityRequests.aRequests[ m_oVisibilityRequests.iNumRequests ];
	pRequest->pUnit             = a_pUnit;
	pRequest->uiFlags           = a_uiFlags;
	m_oVisibilityRequests.iNumRequests++;
}

// $Barnyard: FUNCTION 00634eb0
void ANPCBehaviourManager::UpdateSchedules()
{
	if ( AGameLoader::IsBarnExploreMiniGame() == TTRUE || !AGameLoader::IsMainGameLoaded() )
		return;

	TSIZE uiGenericLength = TStringManager::String8Length( "generic_" );

	for ( ; m_iCurrentSchedule < m_pSchedules->iNumSchedules; m_iCurrentSchedule++ )
	{
		ACTIVITYFLAGS uiVisibility = GetVisibilityFlags( m_iCurrentSchedule );
		TINT          iActivity    = PickActivity( m_iCurrentSchedule );

		if ( iActivity < 0 )
			continue;

		TPString8 strUnitName( m_pStringTables->ppUnitNames[ m_pSchedules->pSchedules[ m_iCurrentSchedule ].iUnitName ] );
		AUnit*    pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( strUnitName );

		if ( pUnit && !pUnit->IsDying() )
		{
			ANPCBehaviourComponent* pComponent = TSTATICCAST( ANPCBehaviourComponent, pUnit->FindComponent( &TGetClass( ANPCBehaviourComponent ) ) );

			if ( pComponent && TStringManager::String8CompareNoCase( "none", GetFSMName( GetActivity( iActivity )->iFSM ) ) != 0 )
			{
				pComponent->SetActivity( iActivity );
				pUnit->SetAIEnabled( TTRUE );
			}

			if ( !AAnimalPopulationManager::IsUnitInQuest( pUnit, TNULL, TTRUE ) && uiVisibility )
				AddVisibilityRequest( pUnit, uiVisibility );
		}
		else if ( TStringManager::String8Compare( "generic_", strUnitName.GetString(), uiGenericLength ) == 0 )
		{
			// Every unit of the type follows the schedule
			AUnitType* pType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strUnitName );

			if ( pType )
			{
				TNodeList<AUnit>::Iterator it    = AUnitManager::GetSingleton()->GetUnits().Begin();
				AUnit*                     pNext = AUnitManager::GetSingleton()->FindNextUnitOfType( pType, it );

				while ( pNext )
				{
					ANPCBehaviourComponent* pComponent = TSTATICCAST( ANPCBehaviourComponent, pNext->FindComponent( &TGetClass( ANPCBehaviourComponent ) ) );

					if ( pComponent )
						pComponent->SetActivity( iActivity );

					if ( !AAnimalPopulationManager::IsUnitInQuest( pNext, TNULL, TTRUE ) && uiVisibility )
						AddVisibilityRequest( pNext, uiVisibility );

					pNext = AUnitManager::GetSingleton()->FindNextUnitOfType( pType, it );
				}
			}
		}
	}

	m_uiFlags &= ~FLAGS_UPDATE_SCHEDULES;
}

// $Barnyard: FUNCTION 006351a0
TBOOL ANPCBehaviourManager::OnDateChangeCallback( ANPCBehaviourManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent )
{
	if ( a_pEvent->m_eUpdateType != AGameTimeManager::DATECHANGETYPE_DAYPHASE )
		return TTRUE;

	a_pCaller->m_uiFlags |= FLAGS_UPDATE_SCHEDULES;
	a_pCaller->m_iCurrentSchedule = 0;

	// NOTE: retail calls AGameStateController::IsStateOnStack, the TBOOL twin of FindGameStateOnStack
	if ( AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) )
		a_pCaller->UpdateSchedules();

	return TTRUE;
}
