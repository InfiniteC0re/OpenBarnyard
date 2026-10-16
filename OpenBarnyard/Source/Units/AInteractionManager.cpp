#include "pch.h"
#include "AInteractionManager.h"
#include "AInteractionComponent.h"
#include "AInteractionEventManager.h"
#include "AInteraction.h"
#include "AInteractionActivity.h"
#include "AInteractionActivityMessageGroups.h"
#include "AInteractionRequirement.h"
#include "APlaceholderRequirement.h"
#include "AGroupRequirement.h"
#include "Quests/AQuestManager.h"
#include "AUnit.h"
#include "AUnitManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Memory/AMemory.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AFunctionPointer.h"
#include "FSM/AMethodPointer.h"
#include "ARandom.h"

#include <File/TTRB.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a2130, CreateObject 005a52c0, CreateObjectInPlace 005a5320
TDEFINE_CLASS_INIT( AInteractionManager );

AFunctionTable AInteractionManager::ms_oFunctionTable( &TGetClass( AInteractionManager ), TNULL );

// FSM fallback: AInteractionManager natives always run on the singleton.
// $Barnyard: FUNCTION 005a22f0
static TObject* __stdcall AInteractionManagerFunctionFallback( void* a_pUnk1, void* a_pUnk2 )
{
	return AInteractionManager::GetSingleton();
}

// NOTE: same as the engine string hash at 006bb240 (also a static in AUnitNameManager.cpp)
static TUINT PersistentLatchHash( const TCHAR* a_szString )
{
	TINT iHash = 0;

	for ( ; *a_szString != '\0'; a_szString++ )
		iHash = iHash * 5 + *a_szString;

	return iHash;
}

// $Barnyard: FUNCTION 005a5640
void AInteractionManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AInteractionManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AInteractionManager>::Create( "CanAssignInteraction", &AInteractionManager::CanAssignInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<AInteractionSequence*, AInteractionManager, AUnit*, TPString8>::Create( "AssignInteraction", &AInteractionManager::AssignInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<AInteractionSequence*, AInteractionManager, AUnit*, TPString8>::Create( "AssignInteractionForced", &AInteractionManager::AssignInteractionForced ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<AInteractionSequence*, AInteractionManager, AUnit*, TPString8>::Create( "AssignInteractionForcedObeyIgnoreList", &AInteractionManager::AssignInteractionForcedObeyIgnoreList ) );
	ms_oFunctionTable.AddFunction( AFunctionPointer2ArgsRT<TBOOL, TINT, TBOOL>::Create( "ForceRequirementSatisfied", &AInteractionManager::ForceRequirementSatisfied ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AInteractionManager, TPString8, TINT>::Create( "SetPersistentLatch", &AInteractionManager::SetPersistentLatch ) );
}

// $Barnyard: FUNCTION 005a2140
void AInteractionManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 005a4500
AInteractionManager::AInteractionManager()
    : m_vecStartInteractions( AMemory::GetAllocator( AMemory::POOL_Misc ), 100, 50 )
    , m_iStartLatch( -1 )
    , m_iFinishLatch( -1 )
    , m_bDebug( TFALSE )
    , m_bEnabled( TTRUE )
    , m_mapPriorityTypes( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
	// NOTE: m_iUnk20, m_pEventManager and m_bUnkf28 aren't initialised by retail

	// PersistentLatchNames / PersistentLatchMap constructors
	m_oPersistentLatchNames.iNumNames = 0;
	m_oPersistentLatches.ppBuckets    = TNULL;
	m_oPersistentLatches.Clear();

	m_oPersistentLatches.ppBuckets   = new PersistentLatchMap::Entry*[ 17 ];
	m_oPersistentLatches.iNumBuckets = 17;
	m_oPersistentLatches.iNumEntries = 0;

	for ( TINT i = 0; i < m_oPersistentLatches.iNumBuckets; i++ )
		m_oPersistentLatches.ppBuckets[ i ] = TNULL;

	m_pMessageGroups = TNULL;
	m_pMessageGroups = new AInteractionActivityMessageGroups();

	m_oCandidates.SetSize( MAX_NUM_CANDIDATES );
}

// $Barnyard: FUNCTION 005a5c60
// $Barnyard: FUNCTION 005a5ff0
AInteractionManager::~AInteractionManager()
{
	// Return every active sequence to the free list
	while ( !m_oSequences.llActive.IsEmpty() )
	{
		AInteractionSequence* pSequence = m_oSequences.llActive.Head();
		pSequence->Reset( TFALSE );

		m_oSequences.llActive.Remove( pSequence );
		m_oSequences.llFree.InsertTail( pSequence );
	}

	if ( m_pMessageGroups )
	{
		// NOTE: the list's destructor deletes the groups (TNodeList::DeleteAll)
		delete m_pMessageGroups;
	}

	m_pMessageGroups = TNULL;

	for ( TINT i = 0; i < m_aInteractions.Size(); i++ )
	{
		if ( m_aInteractions[ i ] )
			delete m_aInteractions[ i ];
	}

	m_aInteractions.Resize( 0 );

	for ( TINT i = 0; i < m_aRequirements.Size(); i++ )
	{
		if ( m_aRequirements[ i ] )
			delete m_aRequirements[ i ];
	}

	m_aRequirements.Resize( 0 );
	m_mapPriorityTypes.Clear();

	for ( TINT i = 0; i < m_aActivities.Size(); i++ )
	{
		if ( m_aActivities[ i ] )
			delete m_aActivities[ i ];
	}

	m_aActivities.Resize( 0 );

	// NOTE: retail destroys the TPString8 elements of m_aLatchNames here (array delete), TArray doesn't
}

// $Barnyard: FUNCTION 005a5f20
TBOOL AInteractionManager::Create()
{
	m_pEventManager = g_oSystemManager.GetScheduler()->CreateTask<AInteractionEventManager>( this );
	m_pEventManager->Create();

	m_iStartLatch  = GetLatch( TPString8( "start" ) ).iLatch;
	m_iFinishLatch = GetLatch( TPString8( "finish" ) ).iLatch;

	if ( !LoadPriorityTypes() )
		return TFALSE;

	TINT iNumActivities = m_aActivities.Size();
	LoadActivitySets( "ActivitySets1", iNumActivities );
	LoadInteractions( "AllInteractions", iNumActivities );
	AllocateActivityData();

	return TTask::Create();
}

// $Barnyard: FUNCTION 005a3bc0
TBOOL AInteractionManager::Reset()
{
	while ( !m_oSequences.llActive.IsEmpty() )
	{
		AInteractionSequence* pSequence = m_oSequences.llActive.Head();
		pSequence->Reset( TFALSE );

		m_oSequences.llActive.Remove( pSequence );
		m_oSequences.llFree.InsertTail( pSequence );
	}

	m_pEventManager->Reset();
	m_oPersistentLatches.Clear();
	m_oPersistentLatchNames.Clear();

	return TTRUE;
}

// $Barnyard: FUNCTION 005a46b0
TBOOL AInteractionManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
		return TTRUE;

	auto itEnd = m_oSequences.llActive.End();

	for ( auto it = m_oSequences.llActive.Begin(); it != itEnd; )
	{
		AInteractionSequence* pSequence = it;
		it                              = pSequence->Next();

		TBOOL                  bNoInteraction   = !pSequence->m_pInteraction || pSequence->m_llComponents.Size() == 0;
		AInteractionComponent* pForcedComponent = TNULL;
		TBOOL                  bWasStarted      = pSequence->m_bStarted;
		TBOOL                  bEnd             = TTRUE;

		if ( !bNoInteraction )
		{
			while ( TTRUE )
			{
				if ( !pSequence->Update( a_fDeltaTime, &pForcedComponent ) || ( pSequence->m_uiFlags & AInteractionSequence::FLAGS_PERSISTENT ) )
				{
					// Still running
					bEnd = TFALSE;
					break;
				}

				// The interaction is over, pick the next one
				if ( !pSequence->m_pInteraction || pSequence->m_llComponents.Size() == 0 )
					break;

				// The interaction would follow itself
				if ( !pSequence->m_strNextInteraction.IsEmpty() && pSequence->m_strNextInteraction == pSequence->m_pInteraction->GetTrigger() )
					break;

				TBOOL bPicked = TFALSE;

				if ( pForcedComponent )
				{
					bPicked = PickInteraction( pSequence, pForcedComponent->GetForcedInteractionArg() );

					if ( bPicked == TTRUE )
					{
						pSequence->MoveUnitToFront( pForcedComponent->GetOwner() );
						continue;
					}
				}
				else
				{
					bPicked = PickInteraction( pSequence, TPString8() );
				}

				if ( bPicked )
					continue;

				if ( !pSequence->m_strNextInteraction.IsEmpty() && PickInteraction( pSequence, pSequence->m_strNextInteraction ) )
					continue;

				break;
			}
		}

		if ( bEnd )
		{
			TGlobalEmitter<AInteractionManagerEvent>::Throw(
			    AInteractionManagerEvent( pSequence, AInteractionManagerEvent::TYPE_SEQUENCE_ENDED, AInteractionManagerEvent::GetSequenceKind( pSequence->m_uiFlags ) )
			);

			pSequence->Reset( TFALSE );
			m_oSequences.llActive.Remove( pSequence );
			m_oSequences.llFree.InsertTail( pSequence );
		}
		else if ( !bWasStarted && pSequence->m_bStarted )
		{
			AInteractionManagerEvent::KIND iKind = AInteractionManagerEvent::KIND_NORMAL;

			if ( pSequence->m_pInteraction )
			{
				if ( pSequence->m_pInteraction->GetFlags() & AInteraction::FLAGS_MISSION_CONVERSATION )
				{
					iKind = AInteractionManagerEvent::KIND_MISSION_CONVERSATION;
					pSequence->m_uiFlags |= AInteractionSequence::FLAGS_MISSION_CONVERSATION;
				}
				else if ( pSequence->m_pInteraction->GetFlags() & AInteraction::FLAGS_MISSION_OBJECTIVE )
				{
					iKind = AInteractionManagerEvent::KIND_MISSION_OBJECTIVE;
					pSequence->m_uiFlags |= AInteractionSequence::FLAGS_MISSION_OBJECTIVE;
				}
			}

			TGlobalEmitter<AInteractionManagerEvent>::Throw( AInteractionManagerEvent( pSequence, AInteractionManagerEvent::TYPE_SEQUENCE_STARTED, iKind ) );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a46a0
AFunctionTable* AInteractionManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 005a2e90
TBOOL AInteractionManager::CanAssignInteraction()
{
	return !m_oSequences.llFree.IsEmpty();
}

// $Barnyard: FUNCTION 005a5360
AInteractionSequence* AInteractionManager::AssignInteraction( AUnit* a_pUnit, TPString8 a_strInteraction )
{
	return AssignInteractionImpl( a_pUnit, a_strInteraction, ASSIGNFLAGS_NONE );
}

// $Barnyard: FUNCTION 005a53e0
AInteractionSequence* AInteractionManager::AssignInteractionForced( AUnit* a_pUnit, TPString8 a_strInteraction )
{
	return AssignInteractionImpl( a_pUnit, a_strInteraction, ASSIGNFLAGS_FORCED );
}

// $Barnyard: FUNCTION 005a5460
AInteractionSequence* AInteractionManager::AssignInteractionForcedObeyIgnoreList( AUnit* a_pUnit, TPString8 a_strInteraction )
{
	return AssignInteractionImpl( a_pUnit, a_strInteraction, ASSIGNFLAGS_FORCED | ASSIGNFLAGS_OBEY_IGNORE_LIST );
}

// $Barnyard: FUNCTION 005a37e0
void AInteractionManager::SetPersistentLatch( TPString8 a_strName, TINT a_iValue )
{
	if ( m_oPersistentLatchNames.iNumNames == MAX_NUM_PERSISTENT_LATCHES )
		return;

	TINT iIndex = m_oPersistentLatchNames.iNumNames++;
	TStringManager::String8Copy( m_oPersistentLatchNames.aszNames[ iIndex ], a_strName.GetString(), 16 );

	m_oPersistentLatches.Set( m_oPersistentLatchNames.aszNames[ iIndex ], a_iValue );
}

// $Barnyard: FUNCTION 005a23a0
TBOOL AInteractionManager::ForceRequirementSatisfied( TINT a_iRequirement, TBOOL a_bSatisfied )
{
	if ( a_iRequirement > -1 )
	{
		// NOTE: TFALSE doesn't force the requirement to be not satisfied, it stops forcing it
		GetSingleton()->m_aRequirements[ a_iRequirement ]->m_eForcedState = a_bSatisfied ? AInteractionRequirement::FORCEDSTATE_SATISFIED : AInteractionRequirement::FORCEDSTATE_NONE;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005a4970
AInteractionSequence* AInteractionManager::AssignInteractionImpl( AUnit* a_pUnit, TPString8 a_strInteraction, ASSIGNFLAGS a_uiFlags )
{
	TBOOL bForced           = ( a_uiFlags & ASSIGNFLAGS_FORCED ) != 0;
	TBOOL bSkipUnitChecks   = ( a_uiFlags & ASSIGNFLAGS_SKIP_UNIT_CHECKS ) != 0;
	TBOOL bIgnoreIgnoreList = bForced && !( a_uiFlags & ASSIGNFLAGS_OBEY_IGNORE_LIST );

	if ( !AInteractionComponent::IsUnitSuitableForInteraction( a_pUnit, bForced ) )
		return TNULL;

	// Spontaneous interactions are limited
	if ( a_strInteraction.IsEmpty() && !( TFLOAT( TINT( m_oSequences.llActive.Count() ) ) < 24.0f ) )
		return TNULL;

	AInteractionParticipants oParticipants;
	oParticipants.Add( a_pUnit );
	oParticipants.iInitiator = 0;

	if ( !( a_uiFlags & ASSIGNFLAGS_NO_NEARBY_UNITS ) )
	{
		// Let the units around join
		TINT                    iNumFound = 0;
		AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( *a_pUnit->GetSteer()->GetPosition(), 12.0f, &iNumFound, TFALSE, 16, TFALSE, TNULL );

		for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
		{
			AUnit* pUnit = pResults[ i ].pUnit;

			if ( a_pUnit != pUnit && !pUnit->IsPlayer() && AInteractionComponent::CanAssignInteraction( a_pUnit, pUnit, bForced, bSkipUnitChecks, bIgnoreIgnoreList ) )
				oParticipants.Add( pUnit );
		}
	}

	if ( bForced )
		oParticipants.bForced = TTRUE;

	return CreateSequence( &oParticipants, a_strInteraction, TFALSE );
}

// $Barnyard: FUNCTION 005a4b90
AInteractionSequence* AInteractionManager::AssignInteractionToUnits( AUnit** a_ppUnits, TINT a_iNumUnits, TPString8 a_strInteraction, ASSIGNFLAGS a_uiFlags )
{
	AInteractionParticipants oParticipants;

	TBOOL bForced           = ( a_uiFlags & ASSIGNFLAGS_FORCED ) != 0;
	TBOOL bSkipUnitChecks   = ( a_uiFlags & ASSIGNFLAGS_SKIP_UNIT_CHECKS ) != 0;
	TBOOL bIgnoreIgnoreList = bForced && !( a_uiFlags & ASSIGNFLAGS_OBEY_IGNORE_LIST );

	// Every unit has to be able to interact with the previous one
	AUnit* pPreviousUnit = TNULL;

	for ( TINT i = 0; i < a_iNumUnits; i++ )
	{
		AUnit* pUnit = a_ppUnits[ i ];

		if ( AInteractionComponent::CanAssignInteraction( pPreviousUnit, pUnit, bForced, bSkipUnitChecks, bIgnoreIgnoreList ) )
		{
			oParticipants.Add( pUnit );
			pPreviousUnit = pUnit;
		}
	}

	if ( oParticipants.iNumUnits < 1 )
		return TNULL;

	if ( a_uiFlags & ASSIGNFLAGS_FIRST_IS_INITIATOR )
		oParticipants.iInitiator = 0;

	if ( a_uiFlags & ASSIGNFLAGS_FORCED )
		oParticipants.bForced = TTRUE;

	AInteractionSequence* pSequence = CreateSequence( &oParticipants, a_strInteraction, TFALSE );

	if ( ( a_uiFlags & ASSIGNFLAGS_PERSISTENT ) && pSequence )
		pSequence->m_uiFlags |= AInteractionSequence::FLAGS_PERSISTENT;

	return pSequence;
}

// $Barnyard: FUNCTION 005a4d10
AInteractionSequence* AInteractionManager::AssignInteractionToAllUnits( TPString8 a_strInteraction )
{
	AInteractionParticipants oParticipants;
	TINT                     iNumUnits = 0;

	TNodeList<AUnit>& rUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = rUnits.Begin(); it != rUnits.End(); it++ )
	{
		if ( AInteractionComponent::IsUnitInteractive( it ) )
		{
			oParticipants.Add( it );

			if ( ++iNumUnits >= AInteractionParticipants::MAX_NUM_UNITS )
				break;
		}
	}

	if ( iNumUnits == 0 )
		return TNULL;

	oParticipants.bForced = TTRUE;
	return CreateSequence( &oParticipants, a_strInteraction, TTRUE );
}

// $Barnyard: FUNCTION 005a3070
TINT AInteractionManager::GetPersistentLatch( const TCHAR* a_szName )
{
	TINT iValue = 0;
	m_oPersistentLatches.Find( a_szName, iValue );

	return iValue;
}

// $Barnyard: FUNCTION 005a30a0
void AInteractionManager::SavePersistentLatches( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( m_oPersistentLatches.iNumEntries );

	for ( PersistentLatchMap::Iterator it( &m_oPersistentLatches ); it != m_oPersistentLatches.End(); it.Next() )
	{
		a_pBuffer->WriteString( it.pEntry->szName );
		a_pBuffer->WriteInt( it.pEntry->iValue );
	}
}

// $Barnyard: FUNCTION 005a3120
void AInteractionManager::LoadPersistentLatches( AHackedQuickSave::Buffer* a_pBuffer )
{
	m_oPersistentLatches.Clear();
	m_oPersistentLatchNames.Clear();

	TINT iNumLatches;
	a_pBuffer->ReadInt( iNumLatches );

	for ( TINT i = 0; i < iNumLatches; i++ )
	{
		TINT iIndex = m_oPersistentLatchNames.iNumNames++;
		a_pBuffer->ReadString( m_oPersistentLatchNames.aszNames[ iIndex ] );

		TINT iValue;
		a_pBuffer->ReadInt( iValue );

		m_oPersistentLatches.Set( m_oPersistentLatchNames.aszNames[ iIndex ], iValue );
	}
}

// $Barnyard: FUNCTION 005a3020
TINT AInteractionManager::GetPriority( const TPString8& a_rcPriorityType )
{
	auto it = m_mapPriorityTypes.Find( a_rcPriorityType );

	if ( it != m_mapPriorityTypes.End() )
		return it->GetSecond();

	return -1;
}

// $Barnyard: FUNCTION 005a3740
TINT AInteractionManager::GetRequirementIndex( TPString8 a_strName )
{
	for ( TINT i = 0; i < m_aRequirements.Size(); i++ )
	{
		if ( a_strName == m_aRequirements[ i ]->GetName() )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005a4000
TINT AInteractionManager::FindOrCreateRequirement( TPString8 a_strName )
{
	TINT iIndex = GetRequirementIndex( a_strName );

	if ( iIndex == -1 )
	{
		// Replaced by the real requirement once it's loaded (see LoadRequirements)
		AInteractionRequirement* pRequirement = new APlaceholderRequirement( a_strName );
		m_aRequirements.Push( pRequirement );
		iIndex = m_aRequirements.Size() - 1;
	}

	return iIndex;
}

// $Barnyard: FUNCTION 005a23e0
void AInteractionManager::ResetForcedRequirements()
{
	for ( TINT i = 0; i < m_aRequirements.Size(); i++ )
		m_aRequirements[ i ]->m_eForcedState = AInteractionRequirement::FORCEDSTATE_NONE;
}

// $Barnyard: FUNCTION 005a5700
ALatchRef AInteractionManager::GetLatch( TPString8 a_strName )
{
	TPString8 strName;
	TBOOL     bNot        = TFALSE;
	TBOOL     bPersistent = TFALSE;

	const TString8& rcName = a_strName.GetString8();

	if ( rcName.Length() > 4 && rcName.CompareNoCase( "not_", 4 ) == 0 )
	{
		bNot    = TTRUE;
		strName = TPString8( rcName.GetString( 4 ) );
	}
	else if ( rcName.Length() > 6 && rcName.CompareNoCase( "latch_", 6 ) == 0 )
	{
		bPersistent = TTRUE;
		strName     = TPString8( rcName.GetString( 6 ) );
	}
	else
	{
		strName = a_strName;
	}

	// Find the latch, add it if it doesn't exist yet
	TINT iLatch = 0;

	for ( ; iLatch < m_aLatchNames.Size(); iLatch++ )
	{
		if ( strName == m_aLatchNames[ iLatch ] )
			break;
	}

	if ( iLatch == m_aLatchNames.Size() )
		m_aLatchNames.Push( strName );

	ALatchRef oRef;
	oRef.iLatch  = iLatch;
	oRef.uiFlags = ALatchRef::FLAGS_NONE;

	if ( bNot )
		oRef.uiFlags |= ALatchRef::FLAGS_NOT;
	else
		oRef.uiFlags &= ~ALatchRef::FLAGS_NOT;

	if ( bPersistent )
		oRef.uiFlags |= ALatchRef::FLAGS_PERSISTENT;
	else
		oRef.uiFlags &= ~ALatchRef::FLAGS_PERSISTENT;

	return oRef;
}

// $Barnyard: FUNCTION 005a2ec0
TBOOL AInteractionManager::StartInteraction( AInteractionSequence* a_pSequence, TINT a_iInteraction )
{
	AInteractionParticipants oParticipants;
	a_pSequence->GetParticipants( oParticipants );
	a_pSequence->SetInteraction( m_aInteractions[ a_iInteraction ], &oParticipants, TFALSE );

	return TTRUE;
}

// $Barnyard: FUNCTION 005a32d0
AInteractionManager::Candidates* AInteractionManager::FindCandidates( AInteractionSequence* a_pSequence, TBOOL a_bStartOnly, AInteractionParticipants* a_pParticipants, TPString8 a_strInteraction )
{
	m_oCandidates.fTotalWeight = 0.0f;
	m_oCandidates.vecCandidates.Clear();

	if ( !a_strInteraction.IsEmpty() || !a_pSequence->m_oLatches.IsSet( m_iFinishLatch ) )
	{
		// Every now and then interactions of any priority are allowed
		TINT iMinPriority = 1;

		if ( ARandom::GetSingleton()->m_oRandom.GetFloat() <= 0.02f )
			iMinPriority = -100;

		TINT iNumInteractions = a_bStartOnly ? m_vecStartInteractions.Size() : m_aInteractions.Size();

		for ( TINT i = 0; i < iNumInteractions; i++ )
		{
			TINT          iInteraction = a_bStartOnly ? m_vecStartInteractions[ i ] : i;
			AInteraction* pInteraction = m_aInteractions[ iInteraction ];

			// The priority of interactions with FLAGS_PRIORITY_MOD is only known once their requirements are evaluated
			TINT iPriority = ( pInteraction->GetFlags() & AInteraction::FLAGS_PRIORITY_MOD ) ? iMinPriority : pInteraction->GetPriority();

			if ( pInteraction->GetTrigger() == a_strInteraction && iMinPriority <= iPriority )
			{
				TFLOAT fWeight = a_pSequence->GetInteractionWeight( pInteraction, a_pParticipants );

				if ( fWeight > 0.0f && ( ( pInteraction->GetFlags() & AInteraction::FLAGS_PRIORITY_MOD ) == 0 || ( iPriority = pInteraction->GetCurrentPriority(), iMinPriority <= iPriority ) ) )
				{
					// Only the interactions of the highest priority are kept
					if ( iMinPriority < iPriority )
					{
						m_oCandidates.fTotalWeight = 0.0f;
						m_oCandidates.vecCandidates.Clear();
						iMinPriority = iPriority;
					}

					if ( CheckCandidateGroup( &m_oCandidates, iInteraction ) )
						m_oCandidates.Add( iInteraction, fWeight );
				}
			}
		}
	}

	return &m_oCandidates;
}

// $Barnyard: FUNCTION 005a3dc0
AInteractionManager::Candidates* AInteractionManager::FindCandidates( AInteractionSequence* a_pSequence, TBOOL a_bStartOnly, TPString8 a_strInteraction )
{
	AInteractionParticipants oParticipants;
	a_pSequence->GetParticipants( oParticipants );

	return FindCandidates( a_pSequence, a_bStartOnly, &oParticipants, a_strInteraction );
}

// $Barnyard: FUNCTION 005a2f10
TBOOL AInteractionManager::CheckCandidateGroup( Candidates* a_pCandidates, TINT a_iInteraction )
{
	AInteraction* pInteraction = m_aInteractions[ a_iInteraction ];

	if ( pInteraction->GetInteractionSet().IsEmpty() )
		return TTRUE;

	for ( TINT i = 0; i < a_pCandidates->vecCandidates.Size(); i++ )
	{
		Candidates::Candidate* pCandidate = &a_pCandidates->vecCandidates[ i ];
		AInteraction*          pOther     = m_aInteractions[ pCandidate->iInteraction ];

		if ( pOther->GetInteractionSet() == pInteraction->GetInteractionSet() )
		{
			// Only one interaction of a set can be a candidate, the one with the higher chance wins
			if ( pInteraction->GetChance() < pOther->GetChance() )
				return TFALSE;

			if ( pOther->GetChance() < pInteraction->GetChance() )
			{
				a_pCandidates->Remove( pCandidate );
				return TTRUE;
			}

			if ( ARandom::GetSingleton()->m_oRandom.GetInt( 100 ) > 49 )
			{
				a_pCandidates->Remove( pCandidate );
				return TTRUE;
			}

			return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a3490
TBOOL AInteractionManager::PickInteraction( AInteractionSequence* a_pSequence, AInteractionParticipants* a_pParticipants, TPString8 a_strInteraction, TBOOL a_bTakeOverUnits )
{
	TINT iNumInteractions = a_pSequence->m_iNumInteractions;

	if ( a_strInteraction.IsEmpty() && a_pSequence->ShouldEnd() )
		return TFALSE;

	Candidates* pCandidates = FindCandidates( a_pSequence, iNumInteractions == 0, a_pParticipants, a_strInteraction );

	// NOTE: retail inlines Candidates::IsEmpty
	if ( !pCandidates->IsEmpty() )
	{
		Candidates::Candidate* pCandidate = pCandidates->Pick();
		a_pSequence->SetInteraction( m_aInteractions[ pCandidate->iInteraction ], a_pParticipants, a_bTakeOverUnits );

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005a3e70
TBOOL AInteractionManager::PickInteraction( AInteractionSequence* a_pSequence, TPString8 a_strInteraction )
{
	AInteractionParticipants oParticipants;
	a_pSequence->GetParticipants( oParticipants );

	return PickInteraction( a_pSequence, &oParticipants, a_strInteraction, TFALSE );
}

// $Barnyard: FUNCTION 005a3c80
AInteractionSequence* AInteractionManager::CreateSequence( AInteractionParticipants* a_pParticipants, TPString8 a_strInteraction, TBOOL a_bTakeOverUnits )
{
	if ( m_oSequences.llFree.IsEmpty() )
		return TNULL;

	AInteractionSequence* pSequence = m_oSequences.Allocate();
	pSequence->Reset( TFALSE );
	pSequence->m_oLatches.Set( m_iStartLatch, TTRUE, TFALSE );

	if ( PickInteraction( pSequence, a_pParticipants, a_strInteraction, a_bTakeOverUnits ) )
		return pSequence;

	// No interaction can be played by the units
	pSequence->Reset( TFALSE );
	m_oSequences.Free( pSequence );

	return TNULL;
}

// $Barnyard: FUNCTION 005a40e0
void AInteractionManager::EndSequence( AInteractionSequence* a_pSequence, TBOOL a_bPlayIdleAnim )
{
	a_pSequence->Reset( a_bPlayIdleAnim );
	m_oSequences.Free( a_pSequence );
}

// $Barnyard: FUNCTION 005a5a30
TBOOL AInteractionManager::LoadPriorityTypes()
{
	TTRB oTRB;

	if ( oTRB.Load( "Data/Interactions/prioritytypes.trb" ) != TTRB::ERROR_OK )
		return TFALSE;

	const PBProperties* pProperties = PBProperties::LoadFromTRB( oTRB );
	oTRB.DeleteSymbolTable();

	TINT iMinPriority = 0;
	TINT iMaxPriority = 0;

	const PBProperties* pRange = pProperties->GetProperty( "priorityrange" )->GetProperties();
	pRange->GetOptionalPropertyValue( iMinPriority, "minpriority" );
	pRange->GetOptionalPropertyValue( iMaxPriority, "maxpriority" );

	const PBPropertyValueArray* pTypes    = pProperties->GetProperty( "prioritytypes" )->GetArray();
	TINT                        iNumTypes = pTypes->GetSize();

	if ( iNumTypes > 0 )
	{
		// The types are listed from the highest priority to the lowest, each one gets the middle of its range
		TFLOAT fStep     = TFLOAT( iMaxPriority - iMinPriority ) / TFLOAT( iNumTypes );
		TFLOAT fPriority = TFLOAT( iMaxPriority ) - fStep * 0.5f;

		for ( TINT i = 0; i < iNumTypes; i++ )
		{
			m_mapPriorityTypes.Insert( pTypes->GetValue( i )->GetTPString8(), TINT( fPriority ) );
			fPriority -= fStep;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a35b0
TBOOL AInteractionManager::LoadActivitySets( const TCHAR* a_szFileName, TINT a_iFirstActivity )
{
	TString8 strPath;
	strPath.Format( "Data/Interactions/%s.trz", a_szFileName );

	TTRB oTRB;

	if ( oTRB.Load( strPath ) != TTRB::ERROR_OK )
		return TFALSE;

	const PBProperties* pProperties = PBProperties::LoadFromTRB( oTRB );
	oTRB.DeleteSymbolTable();

	// NOTE: retail also gets "activityset" from the string pool and doesn't use it, a_iFirstActivity is unused

	for ( auto it = pProperties->Begin(); it != pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "activityset" ) == 0 )
		{
			AInteraction::LoadActivitySet( &m_aActivities, it->GetValue()->GetProperties() );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a4e30
TBOOL AInteractionManager::LoadInteractions( const TCHAR* a_szFileName, TINT a_iFirstActivity )
{
	TString8 strPath;
	strPath.Format( "Data/Interactions/%s.trz", a_szFileName );

	TPString8 strFileName( a_szFileName );

	TTRB oTRB;

	if ( oTRB.Load( strPath ) != TTRB::ERROR_OK )
		return TFALSE;

	const PBProperties* pProperties = PBProperties::LoadFromTRB( oTRB );
	oTRB.DeleteSymbolTable();

	// NOTE: retail also gets "interaction" and "requirements" from the string pool and doesn't use them

	// Requirements first, the interactions refer to them
	for ( auto it = pProperties->Begin(); it != pProperties->End(); it++ )
	{
		TCHAR cFirst = it->GetName().GetString()[ 0 ];

		if ( cFirst == 'r' || cFirst == 'R' )
			LoadRequirements( it->GetValue()->GetProperties() );
	}

	for ( auto it = pProperties->Begin(); it != pProperties->End(); it++ )
	{
		TCHAR cFirst = it->GetName().GetString()[ 0 ];

		if ( cFirst == 'i' || cFirst == 'I' )
		{
			AInteraction* pInteraction = new AInteraction();

			if ( !pInteraction->Load( strFileName, it->GetValue()->GetProperties(), a_iFirstActivity ) )
			{
				if ( pInteraction )
					delete pInteraction;
			}
			else
			{
				m_aInteractions.Push( pInteraction );

				// Interactions that require the start latch or have a trigger can start a sequence
				if ( ALatchRefList_Contains( pInteraction->GetPreconditions(), m_iStartLatch, TFALSE ) || !pInteraction->GetTrigger().IsEmpty() )
					m_vecStartInteractions.PushBack( m_aInteractions.Size() - 1 );
			}
		}
	}

	TGlobalEmitter<AInteractionManagerEvent>::Throw( AInteractionManagerEvent( TNULL, AInteractionManagerEvent::TYPE_INTERACTIONS_LOADED, AInteractionManagerEvent::KIND_NORMAL ) );

#ifdef BARNYARD_LATEST
	if ( TStringManager::String8CompareNoCase( a_szFileName, "AllInteractions" ) == 0 )
		ApplyLatestAct1Order();
#endif

	// The names are only needed while loading
	for ( TINT i = 0; i < m_aRequirements.Size(); i++ )
		m_aRequirements[ i ]->m_strName = TPString8();

	return TTRUE;
}

#ifdef BARNYARD_LATEST

// Data/Interactions/AllInteractions.trz of the latest version (Wii): the groups of act 1 tiers 2-5
struct LatestAct1TierGroup
{
	const TCHAR* szGroup;
	const TCHAR* aszRequirements[ AQuestManager::MAX_NUM_TIER_QUESTS ];
};

static const LatestAct1TierGroup s_aLatestAct1TierGroups[] = {
	{ "Act 1 Tier 2 - Complete", { "Ben Welcome Complete", "Pip Barnyard Secrets Complete", TNULL } },
	{ "Act 1 Tier 3 - Complete", { "Otis Pest Squirting Completed", "Pig Apple Pie Complete", TNULL } },
	{ "Act 1 Tier 4 - Complete", { "Miles Target Squirting 1 Complete", "Duke Farmer Alert Complete", TNULL } },
	{ "Act 1 Tier 5 - Complete", { "Pig Mud Jumpers Complete", "Hanna Missing Eggs Complete", TNULL } },
};

void AInteractionManager::ApplyLatestAct1Order()
{
	if ( !AQuestManager::IsSingletonCreated() || !AQuestManager::IsLatestAct1Order() )
		return;

	// The "Act 1 Tier N - Complete" groups require the quests of the tiers of the latest order
	TINT iNumGroups = 0;

	for ( TINT i = 0; i < TARRAYSIZE( s_aLatestAct1TierGroups ); i++ )
	{
		const LatestAct1TierGroup& rcGroup = s_aLatestAct1TierGroups[ i ];
		TINT                       iGroup  = GetRequirementIndex( TPString8( rcGroup.szGroup ) );

		if ( iGroup == -1 || !m_aRequirements[ iGroup ]->IsA( &TGetClass( AGroupRequirement ) ) )
		{
			TWARN( "AInteractionManager: no group requirement '%s'\n", rcGroup.szGroup );
			continue;
		}

		TINT aiRequirements[ AQuestManager::MAX_NUM_TIER_QUESTS ];
		TINT iNumRequirements = 0;

		for ( TINT k = 0; k < AQuestManager::MAX_NUM_TIER_QUESTS && rcGroup.aszRequirements[ k ]; k++ )
		{
			TINT iRequirement = GetRequirementIndex( TPString8( rcGroup.aszRequirements[ k ] ) );

			if ( iRequirement == -1 )
			{
				TWARN( "AInteractionManager: no requirement '%s' for '%s'\n", rcGroup.aszRequirements[ k ], rcGroup.szGroup );
				iNumRequirements = -1;
				break;
			}

			aiRequirements[ iNumRequirements++ ] = iRequirement;
		}

		if ( iNumRequirements > 0 )
		{
			TSTATICCAST( AGroupRequirement, m_aRequirements[ iGroup ] )->SetRequirements( aiRequirements, iNumRequirements );
			iNumGroups++;
		}
	}

	// The quest givers wait for the tier before the one of their quest in the latest order
	TINT iNumGivers = 0;

	for ( TINT i = 0; i < AQuestManager::LATEST_ACT1_NUM_QUESTS; i++ )
	{
		const AQuestManager::LatestAct1Quest& rcQuest = AQuestManager::ms_aLatestAct1Quests[ i ];

		if ( rcQuest.iTier == rcQuest.iPCTier )
			continue;

		TString8 strName;
		strName.Format( "%s Not Given", rcQuest.szQuest );
		TINT iNotGiven = GetRequirementIndex( TPString8( strName ) );

		strName.Format( "Act 1 Tier %d - Complete", rcQuest.iPCTier - 1 );
		TINT iPCTier = GetRequirementIndex( TPString8( strName ) );

		strName.Format( "Act 1 Tier %d - Complete", rcQuest.iTier - 1 );
		TINT iTier = GetRequirementIndex( TPString8( strName ) );

		if ( iNotGiven == -1 || iPCTier == -1 || iTier == -1 )
			continue;

		for ( TINT k = 0; k < m_aInteractions.Size(); k++ )
		{
			if ( m_aInteractions[ k ]->HasRequirement( iNotGiven ) && m_aInteractions[ k ]->ReplaceRequirement( iPCTier, iTier ) )
				iNumGivers++;
		}
	}

	TINFO( "AInteractionManager: act 1 order of the latest version, %d tier groups and %d quest givers changed\n", iNumGroups, iNumGivers );
}

#endif // BARNYARD_LATEST

// $Barnyard: FUNCTION 005a3f20
TBOOL AInteractionManager::LoadRequirements( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		AInteractionRequirement* pRequirement = CreateRequirement( it );

		if ( pRequirement )
		{
			// Replaces an existing requirement of the same name (e.g. an APlaceholderRequirement)
			TINT iIndex = GetRequirementIndex( pRequirement->GetName() );

			if ( iIndex == -1 )
			{
				m_aRequirements.Push( pRequirement );
			}
			else
			{
				if ( m_aRequirements[ iIndex ] )
					delete m_aRequirements[ iIndex ];

				m_aRequirements[ iIndex ] = pRequirement;
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a2300
void AInteractionManager::AllocateActivityData()
{
	TINT iMaxSize = 0;

	for ( TINT i = 0; i < m_aInteractions.Size(); i++ )
	{
		AInteraction* pInteraction = m_aInteractions[ i ];

		for ( TINT k = 0; k < pInteraction->GetNumActivities(); k++ )
		{
			TINT iSize;
			pInteraction->GetActivity( k )->GetDataSize( &iSize );
			iMaxSize = TMath::Max( iMaxSize, iSize );
		}
	}

	for ( TINT i = 0; i < MAX_NUM_SEQUENCES; i++ )
		m_oSequences.aSequences[ i ].AllocateActivityData( iMaxSize );
}

// $Barnyard: FUNCTION 005a2710
AInteractionRequirement* AInteractionManager::CreateRequirement( const PBProperties::PBProperty* a_pProperty )
{
	TString8 strClassName;
	strClassName.Format( "a%srequirement", a_pProperty->GetName().GetString() );

	const TClass* pClass = TClass::Find( strClassName, &TGetClass( AInteractionRequirement ) );

	if ( pClass )
	{
		AInteractionRequirement* pRequirement = TSTATICCAST( AInteractionRequirement, pClass->CreateObject() );

		if ( pRequirement->Create( a_pProperty->GetValue()->GetProperties() ) )
			return pRequirement;

		delete pRequirement;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005a27e0
AInteractionActivity* AInteractionManager::CreateActivity( const PBProperties::PBProperty* a_pProperty )
{
	TString8 strClassName;
	strClassName.Format( "a%sactivity", a_pProperty->GetName().GetString() );

	const TClass* pClass = TClass::Find( strClassName, &TGetClass( AInteractionActivity ) );

	if ( pClass )
	{
		AInteractionActivity* pActivity = TSTATICCAST( AInteractionActivity, pClass->CreateObject() );

		if ( pActivity->Create( a_pProperty->GetValue()->GetProperties() ) )
			return pActivity;

		delete pActivity;
	}

	return TNULL;
}

//-----------------------------------------------------------------------------
// AInteractionManager::Candidates
//-----------------------------------------------------------------------------

// NOTE: inlined in the AInteractionManager constructor
AInteractionManager::Candidates::Candidates()
    : pRandom( &ARandom::GetSingleton()->m_oRandom )
    , fTotalWeight( 0.0f )
    , vecCandidates( AMemory::GetAllocator( AMemory::POOL_Misc ), 0, 0 )
{
}

// $Barnyard: FUNCTION 005a25b0
void AInteractionManager::Candidates::SetSize( TINT a_iSize )
{
	if ( a_iSize < vecCandidates.Size() )
		vecCandidates.Erase( vecCandidates.MakeIterator( a_iSize ), vecCandidates.End() );

	// New elements are zeroed
	vecCandidates.SetSize( a_iSize );
}

// $Barnyard: FUNCTION 005a2df0
void AInteractionManager::Candidates::Add( TINT a_iInteraction, TFLOAT a_fWeight )
{
	fTotalWeight += a_fWeight;

	Candidate oCandidate;
	oCandidate.iInteraction = a_iInteraction;
	oCandidate.fWeight      = a_fWeight;

	vecCandidates.PushBack( oCandidate );
}

// $Barnyard: FUNCTION 005a2690
// $Barnyard: FUNCTION 005a2e60
void AInteractionManager::Candidates::Remove( Candidate* a_pCandidate )
{
	for ( auto it = vecCandidates.Begin(); it != vecCandidates.End(); it++ )
	{
		if ( a_pCandidate == &it.Value() )
		{
			fTotalWeight -= a_pCandidate->fWeight;
			vecCandidates.Erase( it );
			return;
		}
	}
}

// $Barnyard: FUNCTION 004010d0
TFLOAT AInteractionManager::Candidates::GetRandomWeight( TRandom* a_pRandom, TFLOAT a_fMaxWeight )
{
	TFLOAT fMaxWeight = a_fMaxWeight - TMath::TFLOAT_EPSILON;
	return TFLOAT( a_pRandom->RandRaw() >> 1 ) * fMaxWeight * 4.6566129E-10f;
}

// $Barnyard: FUNCTION 005a28b0
// $Barnyard: FUNCTION 005a32c0
AInteractionManager::Candidates::Candidate* AInteractionManager::Candidates::Pick()
{
	if ( vecCandidates.Size() == 0 || IsWeightZero( fTotalWeight ) )
		return TNULL;

	TFLOAT fRandom = GetRandomWeight( pRandom, fTotalWeight );
	TFLOAT fWeight = 0.0f;

	for ( auto it = vecCandidates.Begin(); it != vecCandidates.End(); it++ )
	{
		fWeight += it.Value().fWeight;

		if ( fRandom < fWeight )
			return &it.Value();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005a2e40
TBOOL AInteractionManager::Candidates::IsEmpty()
{
	if ( vecCandidates.Size() != 0 && !IsWeightZero( fTotalWeight ) )
		return TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005a2e70
void AInteractionManager::Candidates::Clear()
{
	vecCandidates.FreeMemory();
}

//-----------------------------------------------------------------------------
// AInteractionManager::SequencePool
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a4110
AInteractionManager::SequencePool::SequencePool()
{
	for ( TINT i = 0; i < MAX_NUM_SEQUENCES; i++ )
		llFree.InsertTail( &aSequences[ i ] );
}

// $Barnyard: FUNCTION 005a4200
AInteractionManager::SequencePool::~SequencePool()
{
	llFree.RemoveAll();
	llActive.RemoveAll();
}

// $Barnyard: FUNCTION 005a3940
AInteractionSequence* AInteractionManager::SequencePool::Allocate()
{
	AInteractionSequence* pSequence = TNULL;

	if ( !llFree.IsEmpty() )
		pSequence = llFree.Remove( llFree.Tail() );

	// NOTE: retail doesn't check for TNULL
	llActive.InsertTail( pSequence );

	return pSequence;
}

// $Barnyard: FUNCTION 005a38e0
void AInteractionManager::SequencePool::Free( AInteractionSequence* a_pSequence )
{
	llActive.Remove( a_pSequence );
	llFree.InsertTail( a_pSequence );
}

//-----------------------------------------------------------------------------
// AInteractionManager::PersistentLatchNames / PersistentLatchMap
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a2210
void AInteractionManager::PersistentLatchNames::Clear()
{
	for ( TINT i = 0; i < iNumNames; i++ )
		TUtil::MemClear( aszNames[ i ], sizeof( aszNames[ i ] ) );

	iNumNames = 0;
}

// $Barnyard: FUNCTION 005a21e0
AInteractionManager::PersistentLatchMap::Entry::~Entry()
{
	if ( pNext )
		delete pNext;
}

// $Barnyard: FUNCTION 005a2410
AInteractionManager::PersistentLatchMap::Iterator::Iterator( PersistentLatchMap* a_pMap )
{
	iBucket = -1;
	pEntry  = TNULL;
	pMap    = a_pMap;
	Next();
}

// $Barnyard: FUNCTION 005a2170
void AInteractionManager::PersistentLatchMap::Iterator::Next()
{
	if ( pEntry )
		pEntry = pEntry->pNext;

	if ( pEntry == TNULL )
	{
		// Find the next bucket that isn't empty
		for ( iBucket++; iBucket < pMap->iNumBuckets; iBucket++ )
		{
			if ( pMap->ppBuckets[ iBucket ] )
				break;
		}

		if ( iBucket >= pMap->iNumBuckets )
		{
			iBucket = -1;
			pEntry  = TNULL;
			return;
		}

		pEntry = pMap->ppBuckets[ iBucket ];
	}
}

// $Barnyard: FUNCTION 005a3200
AInteractionManager::PersistentLatchMap::~PersistentLatchMap()
{
	Clear();

	if ( ppBuckets )
	{
		delete[] ppBuckets;
		ppBuckets = TNULL;
	}

	iNumBuckets = 0;
}

// $Barnyard: FUNCTION 005a2450
void AInteractionManager::PersistentLatchMap::Clear()
{
	if ( ppBuckets )
	{
		for ( TINT i = 0; i < iNumBuckets; i++ )
		{
			if ( ppBuckets[ i ] )
			{
				delete ppBuckets[ i ];
				ppBuckets[ i ] = TNULL;
			}
		}
	}

	iNumEntries = 0;
}

// $Barnyard: FUNCTION 005a24b0
void AInteractionManager::PersistentLatchMap::Set( const TCHAR* a_szName, TINT a_iValue )
{
	TUINT  uiBucket = PersistentLatchHash( a_szName ) % TUINT( iNumBuckets );
	Entry* pLast    = TNULL;

	for ( Entry* pEntry = ppBuckets[ uiBucket ]; pEntry != TNULL; pEntry = pEntry->pNext )
	{
		if ( TStringManager::String8Compare( pEntry->szName, a_szName ) == 0 )
		{
			pEntry->iValue = a_iValue;
			return;
		}

		pLast = pEntry;
	}

	Entry* pNewEntry  = new Entry;
	pNewEntry->szName = a_szName;
	pNewEntry->iValue = a_iValue;
	pNewEntry->pNext  = TNULL;

	if ( pLast == TNULL )
		ppBuckets[ uiBucket ] = pNewEntry;
	else
		pLast->pNext = pNewEntry;

	iNumEntries++;
}

// $Barnyard: FUNCTION 005a2560
TBOOL AInteractionManager::PersistentLatchMap::Find( const TCHAR* a_szName, TINT& a_rValue ) const
{
	for ( Entry* pEntry = ppBuckets[ PersistentLatchHash( a_szName ) % TUINT( iNumBuckets ) ]; pEntry != TNULL; pEntry = pEntry->pNext )
	{
		if ( TStringManager::String8Compare( pEntry->szName, a_szName ) == 0 )
		{
			a_rValue = pEntry->iValue;
			return TTRUE;
		}
	}

	return TFALSE;
}

// Folded helper returning a copy of a TPString8 (m_aLatchNames elements, T2Map<TPString8, ...> keys)
// $Barnyard: FUNCTION 005a3280
