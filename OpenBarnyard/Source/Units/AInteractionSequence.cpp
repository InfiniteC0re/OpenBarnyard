#include "pch.h"
#include "AInteractionSequence.h"
#include "Helpers/AConversationManager.h"
#include "AInteractionManager.h"
#include "AInteraction.h"
#include "AInteractionActivity.h"
#include "AInteractionActivityMessage.h"
#include "AInteractionActivityMessageGroups.h"
#include "AUnit.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail registers RETURNS_ZERO as both create functions and an alignment of 1
// NOTE: GetClass 005a6620
TDEFINE_CLASS_NORUNTIME( AInteractionSequence );

//-----------------------------------------------------------------------------
// ALatchRefList / AInteractionLatches
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a1f40
TBOOL ALatchRefList_Contains( const ALatchRefList& a_rcList, TINT a_iLatch, TBOOL a_bNot )
{
	for ( TINT i = 0; i < a_rcList.Size(); i++ )
	{
		if ( a_iLatch == a_rcList[ i ].iLatch && ( a_rcList[ i ].uiFlags & ALatchRef::FLAGS_NOT ) == a_bNot )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005a1f80
AInteractionLatches::AInteractionLatches()
{
	for ( TINT i = 0; i < NUM_WORDS; i++ )
	{
		auiValues[ i ]     = 0;
		auiPersistent[ i ] = 0;
	}
}

// $Barnyard: FUNCTION 005a1f20
void AInteractionLatches::Reset()
{
	for ( TINT i = 0; i < NUM_WORDS; i++ )
	{
		auiValues[ i ]     = 0;
		auiPersistent[ i ] = 0;
	}
}

// $Barnyard: FUNCTION 005a1fa0
void AInteractionLatches::ClearTemporary()
{
	for ( TUINT i = 0; i < MAX_NUM_LATCHES; i++ )
	{
		TUINT uiBit = 1 << ( i & 31 );

		if ( ( auiPersistent[ i >> 5 ] & uiBit ) == 0 )
			auiValues[ i >> 5 ] &= ~uiBit;
	}
}

// $Barnyard: FUNCTION 005a1fe0
void AInteractionLatches::Set( TUINT a_uiLatch, TBOOL a_bValue, TBOOL a_bPersistent )
{
	TUINT uiBit  = 1 << ( a_uiLatch & 31 );
	TUINT uiWord = a_uiLatch >> 5;

	if ( a_bValue )
		auiValues[ uiWord ] |= uiBit;
	else
		auiValues[ uiWord ] &= ~uiBit;

	if ( a_bPersistent )
		auiPersistent[ uiWord ] |= uiBit;
	else
		auiPersistent[ uiWord ] &= ~uiBit;
}

// $Barnyard: FUNCTION 005a2040
TBOOL AInteractionLatches::Matches( const ALatchRefList& a_rcList ) const
{
	for ( TINT i = 0; i < a_rcList.Size(); i++ )
	{
		const ALatchRef& rcRef = a_rcList[ i ];

		if ( IsSet( rcRef.iLatch ) != ( ( rcRef.uiFlags & ALatchRef::FLAGS_NOT ) == 0 ) )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a20a0
void AInteractionLatches::Apply( const ALatchRefList& a_rcList )
{
	for ( TINT i = 0; i < a_rcList.Size(); i++ )
	{
		const ALatchRef& rcRef = a_rcList[ i ];
		Set( rcRef.iLatch, ( rcRef.uiFlags & ALatchRef::FLAGS_NOT ) == 0, ( rcRef.uiFlags & ALatchRef::FLAGS_PERSISTENT ) != 0 );
	}
}

//-----------------------------------------------------------------------------
// AInteractionParticipants
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a6020
AInteractionParticipants::AInteractionParticipants()
{
	iNumUnits  = 0;
	bForced    = TFALSE;
	bKeepOrder = TFALSE;
	uiId       = ms_uiNextId;
	iInitiator = -1;
	ms_uiNextId++;
}

// $Barnyard: FUNCTION 005a6050
void AInteractionParticipants::Add( AUnit* a_pUnit )
{
	if ( a_pUnit )
		apUnits[ iNumUnits++ ] = a_pUnit;
}

// $Barnyard: FUNCTION 005a6070
AUnit* AInteractionParticipants::Get( TINT a_iIndex ) const
{
	return apUnits[ a_iIndex ];
}

// $Barnyard: FUNCTION 005a6080
TINT AInteractionParticipants::Find( AUnit* a_pUnit ) const
{
	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		if ( apUnits[ i ] == a_pUnit )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// AInteractionActivityState / AInteractionActivityData
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0059ffe0
AInteractionActivityState::AInteractionActivityState()
{
	iFlags     = FLAGS_NONE;
	fDeltaTime = 0.0f;
	fTime      = 0.0f;
	iActivity  = 0;
	iFlags     = FLAGS_NONE;
}

// $Barnyard: FUNCTION 0059ffb0
void AInteractionActivityState::Reset()
{
	fDeltaTime = 0.0f;
	fTime      = 0.0f;
	iActivity  = 0;
	iFlags     = FLAGS_NONE;
}

// $Barnyard: FUNCTION 0059ffc0
void AInteractionActivityState::Next()
{
	fDeltaTime = 0.0f;
	fTime      = 0.0f;
	iActivity++;
	iFlags = FLAGS_NONE;
}

// $Barnyard: FUNCTION 005c9ba0
AInteractionActivityData::~AInteractionActivityData()
{
	TFree( pData );
	pData = TNULL;
}

// $Barnyard: FUNCTION 005c9b80
void AInteractionActivityData::Allocate( TSIZE a_uiSize )
{
	// NOTE: retail doesn't free the previous buffer
	pData = TMalloc( a_uiSize );
}

// $Barnyard: FUNCTION 005c9bc0
void AInteractionActivityData::AllocateFromSize( const TINT& a_rcSize )
{
	// NOTE: retail doesn't free the previous buffer
	pData = TMalloc( a_rcSize );
}

//-----------------------------------------------------------------------------
// AInteractionSequence
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a7430
AInteractionSequence::AInteractionSequence()
    : m_uiId( 0 )
    , m_pInteraction( TNULL )
    , m_iNumInteractions( 0 )
    , m_uiFlags( FLAGS_NONE )
    , m_bStarted( TFALSE )
{
}

// $Barnyard: FUNCTION 005a74e0
// $Barnyard: FUNCTION 005a76f0
AInteractionSequence::~AInteractionSequence()
{
	// NOTE: m_llMessages deletes its messages (TNodeList::DeleteAll, 005a3230)
}

// $Barnyard: FUNCTION 005a6630
void AInteractionSequence::AllocateActivityData( TINT a_iSize )
{
	if ( a_iSize > 0 )
		m_oActivityData.Allocate( a_iSize );
}

// $Barnyard: FUNCTION 005a6650
void AInteractionSequence::RemoveComponent( AInteractionComponent* a_pComponent )
{
	a_pComponent->T2DList<AInteractionComponent>::Node::Remove();
}

// $Barnyard: FUNCTION 005a6660
TBOOL AInteractionSequence::AddUnit( AUnit* a_pUnit )
{
	AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pComponent )
	{
		pComponent->SetSequence( this );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005a6690
AInteractionActivity* AInteractionSequence::GetCurrentActivity()
{
	if ( m_pInteraction && m_oActivityState.iActivity > -1 && m_oActivityState.iActivity < m_pInteraction->GetNumActivities() )
		return m_pInteraction->GetActivity( m_oActivityState.iActivity );

	return TNULL;
}

// $Barnyard: FUNCTION 005a6700
void AInteractionSequence::GetParticipants( AInteractionParticipants& a_rParticipants )
{
	for ( auto it = GetComponentsBegin(); it != GetComponentsEnd(); it++ )
		a_rParticipants.Add( it->GetOwner() );

	if ( a_rParticipants.iNumUnits > 1 )
	{
		// The primary unit of the next interaction is the current secondary one unless the interaction keeps it
		if ( m_pInteraction && ( m_pInteraction->GetFlags() & AInteraction::FLAGS_SWAP_PRIMARY ) == 0 )
		{
			a_rParticipants.iInitiator = 0;
			return;
		}

		a_rParticipants.iInitiator = 1;
	}
}

// $Barnyard: FUNCTION 005a6790
void AInteractionSequence::RemoveAllComponents( TBOOL a_bPlayIdleAnim )
{
	while ( GetComponentsBegin() != GetComponentsEnd() )
	{
		AInteractionComponent* pComponent = GetComponentsBegin();
		pComponent->LeaveSequence( a_bPlayIdleAnim );

		// NOTE: retail gets the owner here and does nothing with it (stripped debug output)
		if ( AInteractionManager::GetSingleton()->IsDebugEnabled() )
			pComponent->GetOwner();
	}
}

// $Barnyard: FUNCTION 005a67f0
TFLOAT AInteractionSequence::GetInteractionWeight( AInteraction* a_pInteraction, AInteractionParticipants* a_pParticipants )
{
	// The current interaction can't follow itself
	if ( m_pInteraction == a_pInteraction )
		return 0.0f;

	if ( !a_pInteraction->CanStart( &m_oLatches ) )
		return 0.0f;

	AInteractionRoles oRoles;
	oRoles.Reset( a_pParticipants );
	a_pInteraction->EvaluateRequirements( a_pParticipants, &oRoles );

	TFLOAT fWeight = TMath::Max( oRoles.fWeight, 0.0f );

	if ( fWeight > 0.0f && !a_pParticipants->bForced )
	{
		if ( ARandom::GetSingleton()->m_oRandom.GetFloat() <= fWeight )
			return fWeight;

		return 0.0f;
	}

	return fWeight;
}

// $Barnyard: FUNCTION 005a68f0
void AInteractionSequence::IgnoreEachOther()
{
	for ( auto it = GetComponentsBegin(); it != GetComponentsEnd(); it++ )
	{
		for ( auto itOther = GetComponentsBegin(); itOther != GetComponentsEnd(); itOther++ )
		{
			if ( it != itOther )
				it->AddToIgnoreList( itOther->GetOwner() );
		}
	}
}

// $Barnyard: FUNCTION 005a6970
TBOOL AInteractionSequence::ShouldEnd()
{
	if ( ( m_iNumInteractions < 10 || ( m_uiFlags & FLAGS_PERSISTENT ) ) && !m_oLatches.IsSet( AInteractionManager::GetSingleton()->GetFinishLatch() ) )
		return TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005a69b0
AUnit* AInteractionSequence::GetUnit( TINT a_iIndex )
{
	auto it = GetComponentsBegin();

	for ( TINT i = a_iIndex; i > 0; i-- )
	{
		if ( it != GetComponentsEnd() )
			it++;
	}

	if ( it != GetComponentsEnd() )
		return it->GetOwner();

	return TNULL;
}

// $Barnyard: FUNCTION 005a6a20
void AInteractionSequence::FinishInteraction()
{
	m_uiFlags |= FLAGS_INTERACTION_DONE;
}

// $Barnyard: FUNCTION 005a6a30
TBOOL AInteractionSequence::GetForcedComponent( AInteractionComponent** a_ppComponent )
{
	if ( m_iNumInteractions == 1 && m_pInteraction->GetTrigger().IsEmpty() )
	{
		for ( auto it = GetComponentsBegin(); it != GetComponentsEnd(); it++ )
		{
			if ( !it->GetForcedInteractionArg().IsEmpty() )
			{
				*a_ppComponent = it;
				return TTRUE;
			}
		}
	}

	*a_ppComponent = TNULL;
	return TFALSE;
}

// $Barnyard: FUNCTION 005a6ad0
void AInteractionSequence::MoveUnitToFront( AUnit* a_pUnit )
{
	auto itBegin = GetComponentsBegin();

	for ( auto it = itBegin; it != GetComponentsEnd(); it++ )
	{
		if ( it->GetOwner() == a_pUnit && it != itBegin )
		{
			AInteractionComponent* pComponent = it;
			AInteractionComponent* pHead      = itBegin;

			// NOTE: retail inserts after the list root (T2DList::PushFront is ambiguous since AUnitComponent is also a T2SList node)
			pComponent->T2DList<AInteractionComponent>::Node::Remove();
			pComponent->T2DList<AInteractionComponent>::Node::InsertBefore( pHead );
			return;
		}
	}
}

// $Barnyard: FUNCTION 005a6b80
T2DList<AInteractionComponent>::Iterator AInteractionSequence::GetCategoryBegin( CATEGORY a_eCategory )
{
	// Only the first categories are indices of units, everything else starts from the first unit
	TINT iIndex = ( a_eCategory >= CATEGORY_PRIMARY && a_eCategory <= CATEGORY_AUXILIARY5 ) ? a_eCategory : 0;

	auto it = GetComponentsBegin();

	for ( ; iIndex > 0; iIndex-- )
	{
		if ( it != GetComponentsEnd() )
			it++;
	}

	return it;
}

// $Barnyard: FUNCTION 005a6bf0
T2DList<AInteractionComponent>::Iterator AInteractionSequence::GetCategoryEnd( CATEGORY a_eCategory )
{
	// Primary and secondary categories are a single unit, the others last until the end
	if ( a_eCategory != CATEGORY_PRIMARY && a_eCategory != CATEGORY_SECONDARY )
		return GetComponentsEnd();

	auto it = GetComponentsBegin();

	for ( TINT i = a_eCategory + 1; i > 0; i-- )
	{
		if ( it != GetComponentsEnd() )
			it++;
	}

	return it;
}

// $Barnyard: FUNCTION 005a6c60
AUnit* AInteractionSequence::GetUnitByCategory( CATEGORY a_eCategory )
{
	if ( a_eCategory == CATEGORY_ACTIVE )
	{
		return m_pInteraction->GetActivity( m_oActivityState.iActivity )->GetActiveUnit( this );
	}

	if ( a_eCategory == CATEGORY_TARGET )
	{
		return m_pInteraction->GetActivity( m_oActivityState.iActivity )->GetTargetUnit( this );
	}

	auto it = GetCategoryBegin( a_eCategory );

	if ( it == GetComponentsEnd() )
		return TNULL;

	return it->GetOwner();
}

// $Barnyard: FUNCTION 005a6d20
void AInteractionSequence::Reset( TBOOL a_bPlayIdleAnim )
{
	if ( m_pInteraction )
		m_pInteraction->OnSequenceEnd( this );

	if ( m_uiId )
		AConversationManager::GetSingleton()->OnSequenceEnd( m_uiId );

	m_uiId = ++ms_uiLastId;

	// Return every message to its group
	while ( !m_llMessages.IsEmpty() )
	{
		AInteractionActivityMessage* pMessage = m_llMessages.Remove( m_llMessages.Head() );
		AInteractionManager::GetSingleton()->m_pMessageGroups->Free( pMessage );
	}

	RemoveAllComponents( a_bPlayIdleAnim );

	m_iNumInteractions = 0;
	m_pInteraction     = TNULL;
	m_oLatches.Reset();
	m_strNextInteraction = TPString8();
	m_uiFlags            = FLAGS_NONE;
	m_bStarted           = TFALSE;
}

// $Barnyard: FUNCTION 005a6e20
TBOOL AInteractionSequence::Update( TFLOAT a_fDeltaTime, AInteractionComponent** a_ppForcedComponent )
{
	if ( m_uiFlags & FLAGS_ENDED )
		return TFALSE;

	m_bStarted = TTRUE;

	if ( m_pInteraction )
	{
		for ( auto it = m_llMessages.Begin(); it != m_llMessages.End(); it++ )
		{
			it->Update( a_fDeltaTime );

			// The message can end the interaction
			if ( !m_pInteraction )
				break;
		}
	}

	*a_ppForcedComponent = TNULL;

	TBOOL bDone          = TFALSE;
	TINT  iNumComponents = m_llComponents.Size();

	// Not enough units can end the interaction
	if ( m_oActivityState.iActivity > -1 && m_pInteraction && iNumComponents != 0 && ( m_uiFlags & FLAGS_INTERACTION_DONE ) == 0 &&
	     ( m_pInteraction->GetMinUnits() <= iNumComponents || m_pInteraction->GetMinUnitFailBehaviour() != AInteraction::MINUNITFAILBEHAVIOUR_FINISH ) )
	{
		TBOOL                        bForced = GetForcedComponent( a_ppForcedComponent );
		AInteractionActivity::RESULT iResult;

		iResult = AInteractionActivity::UpdateActivities( m_pInteraction, TNULL, this, &m_oActivityState, a_fDeltaTime, bForced );

		if ( iResult == AInteractionActivity::RESULT_DONE )
		{
			bDone = TTRUE;
		}
		else if ( iResult == AInteractionActivity::RESULT_FINISH_SEQUENCE )
		{
			bDone                = TTRUE;
			*a_ppForcedComponent = TNULL;
			m_strNextInteraction = TPString8();
			m_oLatches.auiValues[ AInteractionManager::GetSingleton()->GetFinishLatch() >> 5 ] |= 1 << ( AInteractionManager::GetSingleton()->GetFinishLatch() & 31 );
		}
	}
	else
	{
		bDone = TTRUE;
	}

	if ( bDone )
		m_uiFlags |= FLAGS_INTERACTION_DONE;
	else
		m_uiFlags &= ~FLAGS_INTERACTION_DONE;

	return bDone;
}

// $Barnyard: FUNCTION 005a6fa0
void AInteractionSequence::SetInteraction( AInteraction* a_pInteraction, AInteractionParticipants* a_pParticipants, TBOOL a_bTakeOverUnits )
{
	TINT aiSelected[ AInteractionParticipants::MAX_NUM_UNITS ];
	TINT iNumSelected = 0;

	if ( a_pInteraction )
		a_pInteraction->ResetMaxUnits();

	AInteractionRoles oRoles;
	oRoles.Reset( a_pParticipants );

	// Let the interaction pick and sort the units
	if ( !a_pParticipants->bKeepOrder )
		iNumSelected = a_pInteraction->SelectUnits( aiSelected, a_pParticipants, &oRoles );

	if ( a_pParticipants->bKeepOrder )
	{
		iNumSelected = a_pParticipants->iNumUnits;

		for ( TINT i = 0; i < iNumSelected; i++ )
			aiSelected[ i ] = i;
	}

	// Keep the components of the selected units that are already in this sequence, the others leave
	T2DList<AInteractionComponent> llKept;

	for ( auto it = GetComponentsBegin(); it != GetComponentsEnd(); )
	{
		AInteractionComponent* pComponent = it++;

		TINT  iIndex    = a_pParticipants->Find( pComponent->GetOwner() );
		TBOOL bSelected = TFALSE;

		for ( TINT i = 0; i < iNumSelected; i++ )
		{
			if ( aiSelected[ i ] == iIndex )
			{
				pComponent->T2DList<AInteractionComponent>::Node::Remove();
				llKept.PushBack( pComponent );
				bSelected = TTRUE;
				break;
			}
		}

		if ( !bSelected )
			pComponent->LeaveSequence( TFALSE );
	}

	// Add the selected units in their order
	for ( TINT i = 0; i < iNumSelected; i++ )
	{
		AUnit*                 pUnit      = a_pParticipants->Get( aiSelected[ i ] );
		AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( !pComponent )
			continue;

		if ( a_bTakeOverUnits && pComponent->IsInSequence() )
		{
			// Take the unit away from the sequence it is in
			AInteractionSequence* pOtherSequence = pComponent->GetSequence();

			TGlobalEmitter<AInteractionManagerEvent>::Throw(
			    AInteractionManagerEvent( pOtherSequence, AInteractionManagerEvent::TYPE_SEQUENCE_ENDED, AInteractionManagerEvent::GetSequenceKind( pOtherSequence->m_uiFlags ) )
			);

			AInteractionManager::GetSingleton()->EndSequence( pOtherSequence, TTRUE );
		}

		if ( llKept.Exists( pComponent ) )
		{
			pComponent->T2DList<AInteractionComponent>::Node::Remove();
			m_llComponents.PushBack( pComponent );
		}
		else
		{
			// NOTE: retail looks the component up again
			AInteractionComponent* pNewComponent = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

			if ( pNewComponent )
				pNewComponent->SetSequence( this );
		}
	}

	if ( m_pInteraction )
		m_pInteraction->OnSequenceEnd( this );

	m_pInteraction = a_pInteraction;
	m_iNumInteractions++;
	m_oLatches.ClearTemporary();
	m_oLatches.Apply( a_pInteraction->GetPostconditions() );
	m_oActivityState.Reset();

	m_pInteraction->OnSequenceStart( a_pParticipants, this );

	if ( !m_pInteraction->GetEndTrigger().IsEmpty() && m_strNextInteraction != m_pInteraction->GetEndTrigger() )
		m_strNextInteraction = m_pInteraction->GetEndTrigger();

	IgnoreEachOther();
	m_uiFlags &= ~FLAGS_INTERACTION_DONE;
}

// $Barnyard: FUNCTION 005a72f0
AInteractionActivityMessage* AInteractionSequence::FindActivityMessage( const TClass* a_pClass, TPString8 a_strName )
{
	if ( a_strName.IsEmpty() )
		a_strName = TPString8( a_pClass->GetName() );

	for ( auto it = m_llMessages.Begin(); it != m_llMessages.End(); it++ )
	{
		if ( a_strName == it->GetName() )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005a7580
AInteractionActivityMessage* AInteractionSequence::GetActivityMessage( const TClass* a_pClass, TPString8 a_strName )
{
	if ( a_strName.IsEmpty() )
		a_strName = TPString8( a_pClass->GetName() );

	AInteractionActivityMessage* pMessage = FindActivityMessage( a_pClass, a_strName );

	if ( pMessage == TNULL )
	{
		pMessage = AInteractionManager::GetSingleton()->m_pMessageGroups->Allocate( a_pClass );

		if ( pMessage == TNULL )
			return TNULL;
	}
#ifdef BARNYARD_COMMUNITY_PATCH
	else
	{
		// NOTE: retail links the message again although it's in the list already
		m_llMessages.Remove( pMessage );
	}
#endif

	pMessage->SetName( a_strName );
	pMessage->SetSequence( this );
	m_llMessages.InsertTail( pMessage );

	return pMessage;
}

// $Barnyard: FUNCTION 005a77c0
AInteractionSequence::CATEGORY AInteractionSequence::GetCategoryFromName( const TCHAR* a_szName )
{
	switch ( a_szName[ 0 ] )
	{
		case 'A':
		case 'a':
			if ( a_szName[ 1 ] == 'u' )
			{
				if ( TStringManager::String8CompareNoCase( a_szName, "auxiliary" ) == 0 ) return CATEGORY_AUXILIARY;
				if ( TStringManager::String8CompareNoCase( a_szName, "auxiliary2" ) == 0 ) return CATEGORY_AUXILIARY2;
				if ( TStringManager::String8CompareNoCase( a_szName, "auxiliary3" ) == 0 ) return CATEGORY_AUXILIARY3;
				if ( TStringManager::String8CompareNoCase( a_szName, "auxiliary4" ) == 0 ) return CATEGORY_AUXILIARY4;
				if ( TStringManager::String8CompareNoCase( a_szName, "auxiliary5" ) == 0 ) return CATEGORY_AUXILIARY5;
			}
			else
			{
				if ( TStringManager::String8CompareNoCase( a_szName, "active" ) == 0 ) return CATEGORY_ACTIVE;

				// NOTE: retail compares with "all" and ignores the result, CATEGORY_ALL is returned anyway
				TStringManager::String8CompareNoCase( a_szName, "all" );
			}
			break;
		case 'P':
		case 'p':
			return CATEGORY_PRIMARY;
		case 'S':
		case 's':
			return CATEGORY_SECONDARY;
		case 'T':
		case 't':
			return CATEGORY_TARGET;
	}

	return CATEGORY_ALL;
}

//-----------------------------------------------------------------------------
// AInteractionSequenceRef
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a7710
AInteractionSequenceRef::AInteractionSequenceRef( AInteractionSequence* a_pSequence )
{
	pSequence = a_pSequence;

	if ( a_pSequence )
	{
		uiId = a_pSequence->GetId();
		bSet = TTRUE;
	}
	else
	{
		uiId = 0;
		bSet = TFALSE;
	}
}

// $Barnyard: FUNCTION 005a7790
AInteractionSequenceRef::AInteractionSequenceRef( AInteractionSequenceRef& a_rOther )
{
	a_rOther.Validate();

	pSequence = a_rOther.pSequence;
	uiId      = a_rOther.uiId;
}

// $Barnyard: FUNCTION 005a7760
AInteractionSequenceRef& AInteractionSequenceRef::operator=( AInteractionSequence* a_pSequence )
{
	pSequence = a_pSequence;

	if ( a_pSequence )
	{
		uiId = a_pSequence->GetId();
		bSet = TTRUE;
	}
	else
	{
		uiId = 0;
		bSet = TFALSE;
	}

	return *this;
}

// $Barnyard: FUNCTION 005a7740
void AInteractionSequenceRef::Validate()
{
	if ( pSequence && pSequence->GetId() != uiId )
	{
		pSequence = TNULL;
		uiId      = 0;
	}
}
