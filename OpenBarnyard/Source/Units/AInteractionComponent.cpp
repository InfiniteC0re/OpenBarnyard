#include "pch.h"
#include "AInteractionComponent.h"
#include "AInteractionComponentManager.h"
#include "AInteractionManager.h"
#include "AInteractionSequence.h"
#include "AInteraction.h"
#include "ASpeechActivity.h"
#include "AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ACommonSteer.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 005a1c60 / CreateObjectInPlace 005a1cc0
// NOTE: GetClass 005a0300, CreateObject 005a1c60, CreateObjectInPlace 005a1cc0
TDEFINE_CLASS_INIT( AInteractionComponent, 2, 1 );

TPSTRING8_DECLARE( idle_lp );

AFunctionTable AInteractionComponent::ms_oFunctionTable( &TGetClass( AInteractionComponent ), &AUnitComponent::ms_oFunctionTable );

// $Barnyard: FUNCTION 005a1d00
void AInteractionComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AInteractionComponent>::Create( "IsSuitableForInteraction", &AInteractionComponent::IsSuitableForInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AInteractionComponent>::Create( "IsSuitableForForcedInteraction", &AInteractionComponent::IsSuitableForForcedInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AInteractionComponent>::Create( "ResetTimeSinceLastInteraction", &AInteractionComponent::ResetTimeSinceLastInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AInteractionComponent, TBOOL>::Create( "SetAllowNormalInteractions", &AInteractionComponent::SetAllowNormalInteractions ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AInteractionComponent>::Create( "IsAllowNormalInteractions", &AInteractionComponent::IsAllowNormalInteractions ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AInteractionComponent>::Create( "ResetForcedInteraction", &AInteractionComponent::ResetForcedInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer3Args<AInteractionComponent, const TPString8&, const TPString8&, TFLOAT>::Create( "SetForcedInteraction", &AInteractionComponent::SetForcedInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AInteractionComponent>::Create( "ResetIgnoreList", &AInteractionComponent::ResetIgnoreList ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AInteractionComponent>::Create( "GetTimeSinceLastInteraction", &AInteractionComponent::GetTimeSinceLastInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AInteractionComponent, TBOOL>::Create( "SetAllowConversation", &AInteractionComponent::SetAllowConversation ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AInteractionComponent>::Create( "IsAllowConversation", &AInteractionComponent::IsAllowConversation ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AInteractionComponent, TBOOL>::Create( "SetInteractionsDisabled", &AInteractionComponent::SetInteractionsDisabled ) );
}

// $Barnyard: FUNCTION 005a0380
void AInteractionComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 005a1650
AInteractionComponent::AInteractionComponent()
    : m_fTimeSinceLastInteraction( 0.0f )
    , m_fInterval( 0.0f )
    , m_fReadyTime( 0.0f )
    , m_pSequence( TNULL )
    , m_fForcedInteractionTime( 0.0f )
    , m_uiFlags( FLAGS_NONE )
{
	m_uiFlags |= FLAGS_ALLOW_NORMAL_INTERACTIONS;

	for ( TINT i = 0; i < NUM_INTERVALS; i++ )
		m_afIntervals[ i ] = -1.0f;

	m_iIntervalIndex = 0;
}

// $Barnyard: FUNCTION 005a16f0
// $Barnyard: FUNCTION 005a1030
AInteractionComponent::~AInteractionComponent()
{
}

// $Barnyard: FUNCTION 005a10e0
AFunctionTable* AInteractionComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 005a0680
AUnitComponentManager* AInteractionComponent::GetManager()
{
	return AInteractionComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005a0410
void AInteractionComponent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "InteractionComponent" );
	a_pBuffer->WriteFloat( m_fTimeSinceLastInteraction );
	a_pBuffer->WriteFloat( m_fInterval );
	a_pBuffer->WriteFloat( m_fReadyTime );

	for ( TINT i = 0; i < NUM_INTERVALS; i++ )
		a_pBuffer->WriteFloat( m_afIntervals[ i ] );

	a_pBuffer->WriteInt( m_iIntervalIndex );
}

// $Barnyard: FUNCTION 005a0480
void AInteractionComponent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "InteractionComponent" );
	a_pBuffer->ReadFloat( m_fTimeSinceLastInteraction );
	a_pBuffer->ReadFloat( m_fInterval );
	a_pBuffer->ReadFloat( m_fReadyTime );

	for ( TINT i = 0; i < NUM_INTERVALS; i++ )
		a_pBuffer->ReadFloat( m_afIntervals[ i ] );

	a_pBuffer->ReadInt( m_iIntervalIndex );
}

// $Barnyard: FUNCTION 005a1270
TBOOL AInteractionComponent::IsSuitableForInteraction()
{
	if ( !IsInteractionsDisabled() && IsUnitSuitableForInteraction( GetOwner(), TFALSE ) )
		return TTRUE;

	return TFALSE;
}

// $Barnyard: FUNCTION 005a12a0
TBOOL AInteractionComponent::IsSuitableForForcedInteraction()
{
	// NOTE: retail inlines IsUnitSuitableForInteraction( GetOwner(), TTRUE )
	if ( !IsInteractionsDisabled() && IsUnitSuitableForInteraction( GetOwner(), TTRUE ) )
		return TTRUE;

	return TFALSE;
}

// $Barnyard: FUNCTION 005a0390
void AInteractionComponent::ResetTimeSinceLastInteraction()
{
	m_fTimeSinceLastInteraction = 0.0f;
	m_fReadyTime                = 0.0f;
}

// $Barnyard: FUNCTION 005a0f30
void AInteractionComponent::SetAllowNormalInteractions( TBOOL a_bAllow )
{
	if ( a_bAllow )
	{
		m_uiFlags |= FLAGS_ALLOW_NORMAL_INTERACTIONS;
		ResetTimeSinceLastInteraction();
	}
	else
	{
		m_uiFlags &= ~FLAGS_ALLOW_NORMAL_INTERACTIONS;
	}
}

// $Barnyard: FUNCTION 005a0c80
TBOOL AInteractionComponent::IsAllowNormalInteractions()
{
	return m_uiFlags & FLAGS_ALLOW_NORMAL_INTERACTIONS;
}

// $Barnyard: FUNCTION 005a1160
void AInteractionComponent::ResetForcedInteraction()
{
	m_fForcedInteractionTime  = 0.0f;
	m_strForcedInteraction    = TPString8();
	m_strForcedInteractionArg = TPString8();
}

// $Barnyard: FUNCTION 005a10f0
void AInteractionComponent::SetForcedInteraction( const TPString8& a_rcInteraction, const TPString8& a_rcArgument, TFLOAT a_fTime )
{
	if ( IsInteractionsDisabled() )
		return;

	m_strForcedInteraction    = a_rcInteraction;
	m_strForcedInteractionArg = a_rcArgument;
	m_fForcedInteractionTime  = a_fTime;
}

// $Barnyard: FUNCTION 005a18e0
void AInteractionComponent::ResetIgnoreList()
{
	m_oIgnoreList.Clear();
}

// $Barnyard: FUNCTION 005a0650
TFLOAT AInteractionComponent::GetTimeSinceLastInteraction()
{
	if ( 0.0f < m_fTimeSinceLastInteraction )
		return m_fTimeSinceLastInteraction;

	return 0.0f;
}

// $Barnyard: FUNCTION 005a0f50
void AInteractionComponent::SetAllowConversation( TBOOL a_bAllow )
{
	if ( a_bAllow )
		m_uiFlags |= FLAGS_ALLOW_CONVERSATION;
	else
		m_uiFlags &= ~FLAGS_ALLOW_CONVERSATION;
}

// $Barnyard: FUNCTION 005a0c90
TBOOL AInteractionComponent::IsAllowConversation()
{
	return HASANYFLAG( m_uiFlags, FLAGS_ALLOW_CONVERSATION );
}

// $Barnyard: FUNCTION 005a0cb0
void AInteractionComponent::SetInteractionsDisabled( TBOOL a_bDisabled )
{
	if ( a_bDisabled )
		m_uiFlags |= FLAGS_INTERACTIONS_DISABLED;
	else
		m_uiFlags &= ~FLAGS_INTERACTIONS_DISABLED;
}

// $Barnyard: FUNCTION 005a0ca0
TBOOL AInteractionComponent::IsInteractionsDisabled()
{
	return HASANYFLAG( m_uiFlags, FLAGS_INTERACTIONS_DISABLED );
}

// $Barnyard: FUNCTION 005a03a0
void AInteractionComponent::RecordInteraction()
{
	if ( --m_iIntervalIndex < 0 )
		m_iIntervalIndex = NUM_INTERVALS - 1;

	m_afIntervals[ m_iIntervalIndex ] = m_fInterval;
	m_fInterval                       = 0.0f;
}

// $Barnyard: FUNCTION 005a03c0
TINT AInteractionComponent::GetNumInteractionsWithin( TFLOAT a_fTime )
{
	TINT   iIndex           = m_iIntervalIndex;
	TFLOAT fTime            = m_fInterval;
	TINT   iNumInteractions = 0;

	for ( TINT i = 0; m_afIntervals[ iIndex ] >= 0.0f; )
	{
		if ( a_fTime < fTime )
			return iNumInteractions;

		fTime += m_afIntervals[ iIndex ];
		iNumInteractions++;

		if ( ++iIndex > NUM_INTERVALS - 1 )
			iIndex = 0;

		if ( ++i > NUM_INTERVALS - 1 )
			return iNumInteractions;
	}

	return iNumInteractions;
}

// $Barnyard: FUNCTION 005a0cd0
TFLOAT AInteractionComponent::GetInteractionChance()
{
	if ( IsInteractionsDisabled() )
		return 0.0f;

	if ( m_fTimeSinceLastInteraction < 1.0f )
		return 0.0f;

	if ( m_fTimeSinceLastInteraction <= 3.0f && m_fForcedInteractionTime <= 0.0f )
		return ( m_fTimeSinceLastInteraction - 1.0f ) * 0.5f;

	return 1.0f;
}

// $Barnyard: FUNCTION 005a0690
TBOOL AInteractionComponent::IsUnitIgnored( AUnit* a_pUnit )
{
	for ( TINT i = 0; i < m_oIgnoreList.iNumElements; i++ )
	{
		IgnoredUnit& rIgnored = m_oIgnoreList.aElements[ i ];
		AUnit*       pUnit    = rIgnored.oUnit.Get() ? rIgnored.oUnit->GetUnit() : TNULL;

		if ( a_pUnit == pUnit )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005a18f0
void AInteractionComponent::AddToIgnoreList( AUnit* a_pUnit )
{
	if ( GetOwner() == a_pUnit || a_pUnit == TNULL )
		return;

	// Already ignored, restart the timer
	for ( TINT i = 0; i < m_oIgnoreList.iNumElements; i++ )
	{
		IgnoredUnit& rIgnored = m_oIgnoreList.aElements[ i ];
		AUnit*       pUnit    = rIgnored.oUnit.Get() ? rIgnored.oUnit->GetUnit() : TNULL;

		if ( a_pUnit == pUnit )
		{
			rIgnored.fTime = 15.0f;
			return;
		}
	}

	if ( m_oIgnoreList.iNumElements == MAX_NUM_IGNORED_UNITS )
	{
		// The list is full, replace the unit that would be forgotten first
		TFLOAT fMinTime = TMath::MAXFLOAT;
		TINT   iMin     = -1;

		for ( TINT i = 0; i < MAX_NUM_IGNORED_UNITS; i++ )
		{
			if ( m_oIgnoreList.aElements[ i ].fTime < fMinTime )
			{
				fMinTime = m_oIgnoreList.aElements[ i ].fTime;
				iMin     = i;
			}
		}

		m_oIgnoreList.Erase( iMin );
	}

	IgnoredUnit& rIgnored = m_oIgnoreList.aElements[ m_oIgnoreList.iNumElements++ ];
	rIgnored.oUnit        = a_pUnit->GetSelfProxy();
	rIgnored.fTime        = 15.0f;
}

// $Barnyard: FUNCTION 005a0ed0
void AInteractionComponent::SetSequence( AInteractionSequence* a_pSequence )
{
	T2DList<AInteractionComponent>::Node::Remove();
	m_pSequence = a_pSequence;

	a_pSequence->GetComponents().PushBack( this );

	m_fTimeSinceLastInteraction = 0.0f;
	m_fReadyTime                = 0.0f;

	// NOTE: retail passes the sequence pointer itself
	GetOwner()->SetHasInteraction( TINT( TREINTERPRETCAST( TUINTPTR, m_pSequence ) ) );
}

// $Barnyard: FUNCTION 005a1370
void AInteractionComponent::LeaveSequence( TBOOL a_bPlayIdleAnim )
{
	if ( m_pSequence == TNULL )
		return;

	AInteraction* pInteraction = m_pSequence->GetInteraction();

	// NOTE: retail calls GetOwner() once more here and discards the result
	AUnit* pOwner = GetOwner();

	if ( pOwner && !pOwner->IsDying() )
	{
		if ( a_bPlayIdleAnim && pInteraction && pInteraction->ShouldStopAnimations() )
		{
			if ( GetOwner()->IsA( &TGetClass( AAnimatedUnit ) ) )
			{
				AAnimatableObject* pAnimatable = TSTATICCAST( AAnimatedUnit, GetOwner() )->GetAnimatableObject();
				pAnimatable->KillAllAnimations();

				ANamedAnimationSetRef pAnimSet = pAnimatable->GetToshiAnimInterface().GetAnimationSet();
				ANamedAnimation*      pAnim    = pAnimSet->GetNamedAnimation( TPS8( idle_lp ) );

				if ( pAnim )
					pAnimatable->PlayAnimation( TPS8( idle_lp ) );
			}
		}

		GetOwner()->SetHasInteraction( 0 );
	}

	m_pSequence->RemoveComponent( this );

	m_pSequence = TNULL;
	AInteractionComponentManager::GetSingleton()->m_llComponents.PushBack( this );
}

// $Barnyard: FUNCTION 005a0f70
TBOOL AInteractionComponent::IsInSequence()
{
	if ( m_pSequence && !m_pSequence->IsEnded() )
		return TTRUE;

	return TFALSE;
}

// $Barnyard: FUNCTION 005a06d0
TBOOL AInteractionComponent::GetSpeechActivity( ASpeechActivity** a_ppActivity )
{
	if ( m_pSequence == TNULL )
		return TFALSE;

	AInteractionActivity* pActivity = m_pSequence->GetCurrentActivity();

	if ( pActivity && pActivity->IsA( &TGetClass( ASpeechActivity ) ) )
	{
		*a_ppActivity = TSTATICCAST( ASpeechActivity, pActivity );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005a1710
TBOOL AInteractionComponent::UpdateTimers( TFLOAT a_fDeltaTime )
{
	if ( 0.0f < m_fForcedInteractionTime )
	{
		m_fForcedInteractionTime -= a_fDeltaTime;

		if ( m_fForcedInteractionTime <= 0.0f )
		{
			m_strForcedInteraction    = TPString8();
			m_strForcedInteractionArg = TPString8();
		}
	}

	// Forget units whose time ran out or that don't exist anymore
	for ( TINT i = 0; i != m_oIgnoreList.iNumElements; )
	{
		IgnoredUnit& rIgnored = m_oIgnoreList.aElements[ i ];
		rIgnored.fTime -= a_fDeltaTime;

		if ( 0.0f < rIgnored.fTime && rIgnored.oUnit.Get() && rIgnored.oUnit->GetUnit() )
		{
			i++;
		}
		else
		{
			rIgnored.oUnit = TNULL;
			i              = m_oIgnoreList.Erase( i );
		}
	}

	if ( m_uiFlags & FLAGS_ALLOW_NORMAL_INTERACTIONS )
	{
		m_fInterval += a_fDeltaTime;
		m_fTimeSinceLastInteraction += a_fDeltaTime;

		if ( m_fTimeSinceLastInteraction < 5.0f )
			m_fReadyTime = 0.0f;
		else
			m_fReadyTime += a_fDeltaTime;
	}
	else
	{
		m_fTimeSinceLastInteraction = 0.0f;
		m_fReadyTime                = 0.0f;
	}

	return !( m_fReadyTime < 1.0f );
}

// $Barnyard: FUNCTION 005a11e0
TBOOL AInteractionComponent::TryAssignInteraction()
{
	m_fReadyTime = 0.0f;

	if ( IsInteractionsDisabled() )
		return TFALSE;

	return AInteractionManager::GetSingleton()->AssignInteraction( GetOwner(), m_strForcedInteraction ) != TNULL;
}

// $Barnyard: FUNCTION 005a0d30
TBOOL AInteractionComponent::IsUnitSuitableForInteraction( AUnit* a_pUnit, TBOOL a_bForced )
{
	if ( a_pUnit->HasInteraction() || a_pUnit->IsDying() || a_pUnit->Unk_0x4c() )
		return TFALSE;

	AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pComponent == TNULL || pComponent->m_pSequence != TNULL || pComponent->IsInteractionsDisabled() )
		return TFALSE;

	if ( a_bForced )
		return TTRUE;

	TFLOAT fChance = pComponent->GetInteractionChance();
	return ARandom::GetSingleton()->m_oRandom.GetFloat() < fChance;
}

// $Barnyard: FUNCTION 005a0dc0
TBOOL AInteractionComponent::IsUnitInteractive( AUnit* a_pUnit )
{
	if ( a_pUnit->IsDying() )
		return TFALSE;

	AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pComponent == TNULL || pComponent->IsInteractionsDisabled() )
		return TFALSE;

	// NOTE: retail repeats the checks of an inlined helper
	if ( !a_pUnit->IsDying() && !a_pUnit->Unk_0x4c() && a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) != TNULL )
		return TTRUE;

	return TFALSE;
}

// $Barnyard: FUNCTION 005a0e20
TBOOL AInteractionComponent::CanUnitsInteract( AUnit* a_pUnit, AUnit* a_pOther, TBOOL a_bIgnoreIgnoreList )
{
	if ( a_pUnit == TNULL )
		return TTRUE;

	// NOTE: retail doesn't check a_pOther for TNULL here
	if ( a_pUnit->IsPlayer() || a_pOther->IsPlayer() )
		return TTRUE;

	// NOTE: retail doesn't check the found components for TNULL
	if ( a_pOther && !a_pOther->IsDying() )
	{
		AInteractionComponent* pOtherComponent = TSTATICCAST( AInteractionComponent, a_pOther->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pOtherComponent->IsInteractionsDisabled() )
			return TFALSE;
	}

	AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pComponent->IsInteractionsDisabled() )
		return TFALSE;

	if ( !a_bIgnoreIgnoreList && pComponent->IsUnitIgnored( a_pOther ) )
		return TFALSE;

	ASteer* pSteer = a_pUnit->GetSteer();

	if ( pSteer->IsA( &TGetClass( ACommonSteer ) ) )
		return TSTATICCAST( ACommonSteer, pSteer )->IsInSameOrLinkedArea( a_pOther );

	return TTRUE;
}

// $Barnyard: FUNCTION 005a1300
TBOOL AInteractionComponent::CanAssignInteraction( AUnit* a_pUnit, AUnit* a_pOther, TBOOL a_bForced, TBOOL a_bSkipUnitChecks, TBOOL a_bIgnoreIgnoreList )
{
	if ( a_pUnit && !a_pUnit->IsDying() )
	{
		// NOTE: retail doesn't check the component for TNULL
		AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pComponent->IsInteractionsDisabled() )
			return TFALSE;
	}

	if ( !IsUnitSuitableForInteraction( a_pOther, a_bForced ) )
		return TFALSE;

	if ( !a_bSkipUnitChecks && !CanUnitsInteract( a_pUnit, a_pOther, a_bIgnoreIgnoreList ) )
		return TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005a14f0
void AInteractionComponent::IgnoreList::Clear()
{
	for ( TINT i = 0; i < iNumElements; i++ )
		aElements[ i ] = IgnoredUnit();

	iNumElements = 0;
}

// $Barnyard: FUNCTION 005a1570
TINT AInteractionComponent::IgnoreList::Erase( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < iNumElements - 1; i++ )
		aElements[ i ] = aElements[ i + 1 ];

	aElements[ iNumElements - 1 ] = IgnoredUnit();
	iNumElements--;

	// NOTE: retail returns an iterator (index + container)
	return a_iIndex;
}
