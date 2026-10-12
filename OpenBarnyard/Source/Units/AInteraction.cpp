#include "pch.h"
#include "AInteraction.h"
#include "AInteractionManager.h"
#include "AInteractionComponent.h"
#include "AInteractionActivity.h"
#include "AInteractionRequirement.h"
#include "AUnit.h"
#include "AUnitProfileComponent.h"
#include "AUnitFlagSetsManager.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( keep );
TPSTRING8_DECLARE( friends );
TPSTRING8_DECLARE( notfriends );
TPSTRING8_DECLARE( chance );
TPSTRING8_DECLARE( displayorder );
TPSTRING8_DECLARE( interactionset );
TPSTRING8_DECLARE( minunitfailbehaviour );
TPSTRING8_DECLARE( hudtext );
TPSTRING8_DECLARE( overrideable );
TPSTRING8_DECLARE( continue );
TPSTRING8_DECLARE( finish );
TPSTRING8_DECLARE( assert );

// Names of AInteraction::MINUNITFAILBEHAVIOUR (0x00775204)
static TPString8* s_apMinUnitFailBehaviourNames[ AInteraction::MINUNITFAILBEHAVIOUR_NUMOF ] = {
	&TPS8( continue ),
	&TPS8( finish ),
	&TPS8( assert ),
};

//-----------------------------------------------------------------------------
// AInteractionRoles
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a60b0
void AInteractionRoles::Reset()
{
	fWeight = 1.0f;

	for ( TINT i = 0; i < NUM_ROLES; i++ )
	{
		for ( TINT k = 0; k < NUM_WORDS; k++ )
			aauiMasks[ i ][ k ] = TUINT( -1 );
	}
}

// $Barnyard: FUNCTION 005a6110
void AInteractionRoles::Reset( const AInteractionParticipants* a_pParticipants )
{
	fWeight = 1.0f;

	for ( TINT i = 0; i < NUM_ROLES; i++ )
	{
		for ( TINT k = 0; k < NUM_WORDS; k++ )
			aauiMasks[ i ][ k ] = TUINT( -1 );
	}

	// Only the unit that started the interaction can be the primary unit
	TUINT uiInitiator = a_pParticipants->iInitiator;

	if ( uiInitiator != TUINT( -1 ) )
	{
		for ( TINT k = 0; k < NUM_WORDS; k++ )
			aauiMasks[ 0 ][ k ] = 0;

		aauiMasks[ 0 ][ uiInitiator >> 5 ] |= 1 << ( uiInitiator & 31 );
	}
}

// $Barnyard: FUNCTION 005a60e0
void AInteractionRoles::Clear()
{
	fWeight = 1.0f;

	for ( TINT i = 0; i < NUM_ROLES; i++ )
	{
		for ( TINT k = 0; k < NUM_WORDS; k++ )
			aauiMasks[ i ][ k ] = 0;
	}
}

// $Barnyard: FUNCTION 005a6180
void AInteractionRoles::Combine( const AInteractionRoles& a_rcOther )
{
	fWeight = a_rcOther.fWeight * fWeight;

	for ( TINT i = 0; i < NUM_ROLES; i++ )
	{
		for ( TINT k = 0; k < NUM_WORDS; k++ )
			aauiMasks[ i ][ k ] &= a_rcOther.aauiMasks[ i ][ k ];
	}
}

// $Barnyard: FUNCTION 005a61f0
void AInteractionRoles::Merge( const AInteractionRoles& a_rcOther )
{
	if ( fWeight < a_rcOther.fWeight )
		fWeight = a_rcOther.fWeight;

	for ( TINT i = 0; i < NUM_ROLES; i++ )
	{
		for ( TINT k = 0; k < NUM_WORDS; k++ )
			aauiMasks[ i ][ k ] |= a_rcOther.aauiMasks[ i ][ k ];
	}
}

// $Barnyard: FUNCTION 005a6260
TINT AInteractionRoles::AssignUnits( TINT* a_pUnits, TINT a_iNumUnits, TINT a_iMinUnits, TINT a_iMaxUnits )
{
	// NOTE: a_iMinUnits is unused
	TUINT auiUsed[ NUM_WORDS ] = { 0 };

	for ( TINT i = 0; i < a_iMaxUnits; i++ )
	{
		// Roles after the auxiliary one use its mask
		const TUINT* pMask = aauiMasks[ ( i > NUM_ROLES - 1 ) ? NUM_ROLES - 1 : i ];

		TUINT uiUnit = 0;

		while ( TTRUE )
		{
			if ( TINT( uiUnit ) >= a_iNumUnits )
				return i;

			TUINT uiBit = 1 << ( uiUnit & 31 );

			if ( ( pMask[ uiUnit >> 5 ] & uiBit ) != 0 && ( auiUsed[ uiUnit >> 5 ] & uiBit ) == 0 )
				break;

			uiUnit++;
		}

		a_pUnits[ i ] = uiUnit;
		auiUsed[ uiUnit >> 5 ] |= 1 << ( uiUnit & 31 );
	}

	return a_iMaxUnits;
}

// $Barnyard: FUNCTION 005a6340
void AInteractionRoles::RemoveUnit( TUINT a_uiUnit )
{
	TUINT uiMask = ~( 1 << ( a_uiUnit & 31 ) );

	for ( TINT i = 0; i < NUM_ROLES; i++ )
		aauiMasks[ i ][ a_uiUnit >> 5 ] &= uiMask;
}

//-----------------------------------------------------------------------------
// AInteraction
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0059e9d0
AInteraction::AInteraction()
    : m_aPreconditions( 10, 0 )
    , m_aPostconditions( 10, 0 )
    , m_fChance( 1.0f )
    , m_iPriority( 1 )
    , m_iCurrentPriority( 1 )
    , m_uiFlags( FLAGS_NONE )
    , m_aActivities( 4, 0 )
    , m_eMinUnitFailBehaviour( MINUNITFAILBEHAVIOUR_ASSERT )
    , m_iMinUnits( 1 )
    , m_iMaxUnits( 10 )
    , m_iDefaultMaxUnits( 10 )
    , m_iDisplayOrder( 0 )
    , m_iNumRequirements( 0 )
    , m_pPriorityMods( TNULL )
{
	m_uiFlags |= FLAGS_SWAP_PRIMARY;
}

// $Barnyard: FUNCTION 0059ea80
// $Barnyard: FUNCTION 0059ef70
AInteraction::~AInteraction()
{
	m_aActivities.Resize( 0 );

	if ( m_pPriorityMods )
	{
		TFree( m_pPriorityMods );
		m_pPriorityMods = TNULL;
	}
}

// $Barnyard: FUNCTION 0059f1c0
TBOOL AInteraction::Load( const TPString8& a_rcFileName, const PBProperties* a_pProperties, TINT a_iFirstActivity )
{
	// NOTE: a_rcFileName is unused

	const PBPropertyValue* pPreconditions = a_pProperties->GetProperty( "preconditions" );

	if ( pPreconditions )
		LoadLatches( &m_aPreconditions, pPreconditions->GetArray() );

	const PBPropertyValue* pPostconditions = a_pProperties->GetProperty( "postconditions" );

	if ( pPostconditions )
		LoadLatches( &m_aPostconditions, pPostconditions->GetArray() );

	const PBPropertyValue* pNextPrimary = a_pProperties->GetProperty( "nextprimary" );

	if ( pNextPrimary && pNextPrimary->GetTPString8() == TPS8( keep ) )
		m_uiFlags &= ~FLAGS_SWAP_PRIMARY;

	const PBPropertyValue* pNumUnits = a_pProperties->GetProperty( "numunits" );

	if ( pNumUnits )
	{
		if ( pNumUnits->GetType() == PBPropertyValue::Type::Int )
		{
			TINT8 iNumUnits    = TINT8( pNumUnits->GetInteger() );
			m_iMinUnits        = iNumUnits;
			m_iMaxUnits        = iNumUnits;
			m_iDefaultMaxUnits = iNumUnits;
		}
		else if ( pNumUnits->GetType() == PBPropertyValue::Type::Array && pNumUnits->GetArray()->GetSize() == 2 )
		{
			// Minimum and maximum number of units
			const PBPropertyValueArray* pRange = pNumUnits->GetArray();

			m_iMinUnits        = TINT8( pRange->GetValue( 0 )->GetInteger() );
			TINT8 iMaxUnits    = TINT8( pRange->GetValue( 1 )->GetInteger() );
			m_iMaxUnits        = iMaxUnits;
			m_iDefaultMaxUnits = iMaxUnits;
		}
	}

	const PBPropertyValue* pRequirements = a_pProperties->GetProperty( "requirements" );

	if ( pRequirements )
		LoadRequirements( pRequirements->GetArray() );

	const PBPropertyValue* pActivities = a_pProperties->GetProperty( "activities" );

	if ( pActivities )
		LoadActivities( &AInteractionManager::GetSingleton()->m_aActivities, pActivities->GetProperties(), TINT16( a_iFirstActivity ), &m_aActivities );

	const PBPropertyValue* pTrigger = a_pProperties->GetProperty( "trigger" );

	if ( pTrigger )
		m_strTrigger = pTrigger->GetTPString8();

	const PBPropertyValue* pEndTrigger = a_pProperties->GetProperty( "endtrigger" );

	if ( pEndTrigger )
		m_strEndTrigger = pEndTrigger->GetTPString8();

	// NOTE: retail uses the TPString8 overload at 006b1f50, same as GetOptionalFloatOrIntValue
	a_pProperties->GetOptionalFloatOrIntValue( m_fChance, TPS8( chance ) );

	const PBPropertyValue* pPriority = a_pProperties->GetProperty( "priority" );

	if ( pPriority )
	{
		if ( pPriority->GetType() == PBPropertyValue::Type::Int )
			m_iPriority = pPriority->GetInteger();
		else if ( pPriority->GetType() == PBPropertyValue::Type::String )
			m_iPriority = AInteractionManager::GetSingleton()->GetPriority( pPriority->GetTPString8() );
	}

	const PBPropertyValue* pPriorityModCategory = a_pProperties->GetProperty( "prioritymodcategory" );

	if ( pPriorityModCategory )
	{
		if ( m_pPriorityMods == TNULL )
		{
			m_pPriorityMods = TSTATICCAST( PriorityMods, TMalloc( sizeof( PriorityMods ) ) );
			TUtil::MemSet( m_pPriorityMods, 0, sizeof( PriorityMods ) );
		}

		m_pPriorityMods->uiCategory = TUINT16( AInteractionSequence::GetCategoryFromName( pPriorityModCategory->GetTPString8().GetString() ) );

		m_uiFlags |= FLAGS_PRIORITY_MOD;
		m_uiFlags |= FLAGS_PRIORITY_MOD_CATEGORY;

		const PBPropertyValue* pPriorityMod = a_pProperties->GetProperty( "prioritymod" );

		if ( pPriorityMod )
		{
			const PBPropertyValueArray* pMods = pPriorityMod->GetArray();

			for ( TINT i = 0; i < TINT( pMods->GetSize() ); i++ )
				m_pPriorityMods->aiRelationMods[ i ] = TINT16( pMods->GetValue( i )->GetInteger() );
		}
	}

	const PBPropertyValue* pPriorityChanceMod = a_pProperties->GetProperty( "prioritychancemod" );

	if ( pPriorityChanceMod )
	{
		if ( m_pPriorityMods == TNULL )
		{
			m_pPriorityMods = TSTATICCAST( PriorityMods, TMalloc( sizeof( PriorityMods ) ) );
			TUtil::MemSet( m_pPriorityMods, 0, sizeof( PriorityMods ) );
		}

		m_uiFlags |= FLAGS_PRIORITY_MOD;
		m_uiFlags |= FLAGS_PRIORITY_CHANCE_MOD;

		const PBPropertyValueArray* pRange = pPriorityChanceMod->GetArray();

		for ( TINT i = 0; i < TINT( pRange->GetSize() ); i++ )
			m_pPriorityMods->aiRandomRange[ i ] = TINT16( pRange->GetValue( i )->GetInteger() );

		if ( m_pPriorityMods->aiRandomRange[ 1 ] < m_pPriorityMods->aiRandomRange[ 0 ] )
		{
			TINT16 iMin                         = m_pPriorityMods->aiRandomRange[ 0 ];
			m_pPriorityMods->aiRandomRange[ 0 ] = m_pPriorityMods->aiRandomRange[ 1 ];
			m_pPriorityMods->aiRandomRange[ 1 ] = iMin;
		}
	}

	const PBPropertyValue* pInterruptible = a_pProperties->GetProperty( "interruptible" );

	if ( pInterruptible )
	{
		if ( pInterruptible->GetBoolean() )
			m_uiFlags |= FLAGS_INTERRUPTIBLE;
		else
			m_uiFlags &= ~FLAGS_INTERRUPTIBLE;
	}

	const PBPropertyValue* pSemantic = a_pProperties->GetProperty( "semantic" );

	m_uiFlags &= ~FLAGS_MISSION_CONVERSATION;
	m_uiFlags &= ~FLAGS_MISSION_OBJECTIVE;

	if ( pSemantic )
	{
		TPString8 strSemantic = pSemantic->GetTPString8();

		if ( strSemantic == TPS8D( "mission conversation" ) )
			m_uiFlags |= FLAGS_MISSION_CONVERSATION;
		else if ( strSemantic == TPS8D( "mission objective" ) )
			m_uiFlags |= FLAGS_MISSION_OBJECTIVE;
	}

	const PBPropertyValue* pStopAnimations = a_pProperties->GetProperty( "stopanimations" );

	if ( !pStopAnimations || pStopAnimations->GetBoolean() )
		m_uiFlags |= FLAGS_STOP_ANIMATIONS;
	else
		m_uiFlags &= ~FLAGS_STOP_ANIMATIONS;

	const PBPropertyValue* pDialogueNext = a_pProperties->GetProperty( "dialoguenext" );

	if ( pDialogueNext )
	{
		if ( pDialogueNext->GetBoolean() )
			m_uiFlags |= FLAGS_DIALOGUE_NEXT;
		else
			m_uiFlags &= ~FLAGS_DIALOGUE_NEXT;
	}

	TINT iDisplayOrder = 0;
	a_pProperties->GetOptionalPropertyValue( iDisplayOrder, TPS8( displayorder ) );
	m_iDisplayOrder = TINT8( iDisplayOrder );

	a_pProperties->GetOptionalPropertyValue( m_strInteractionSet, TPS8( interactionset ) );

	TPString8 strMinUnitFailBehaviour;

	if ( a_pProperties->GetOptionalPropertyValue( strMinUnitFailBehaviour, TPS8( minunitfailbehaviour ) ) )
	{
		for ( TINT i = 0; i < MINUNITFAILBEHAVIOUR_NUMOF; i++ )
		{
			if ( strMinUnitFailBehaviour == *s_apMinUnitFailBehaviourNames[ i ] )
			{
				m_eMinUnitFailBehaviour = MINUNITFAILBEHAVIOUR( i );
				break;
			}
		}
	}

	a_pProperties->GetOptionalPropertyValue( m_oHudText, TPS8( hudtext ) );

	const PBPropertyValue* pPausePlayerSteering = a_pProperties->GetProperty( "pauseplayersteering" );

	if ( !pPausePlayerSteering || pPausePlayerSteering->GetBoolean() )
		m_uiFlags |= FLAGS_PAUSE_PLAYER_STEERING;
	else
		m_uiFlags &= ~FLAGS_PAUSE_PLAYER_STEERING;

	for ( TINT i = 0; i < m_aActivities.Size(); i++ )
	{
		if ( GetActivity( i )->IsWalkActivity() )
		{
			m_uiFlags |= FLAGS_HAS_WALK_ACTIVITY;
			break;
		}
	}

	TBOOL bOverrideable;

	if ( !a_pProperties->GetOptionalPropertyValue( bOverrideable, TPS8( overrideable ) ) )
	{
		// Overrideable if every activity allows it
		bOverrideable = ( m_uiFlags & FLAGS_HAS_WALK_ACTIVITY ) != 0;

		if ( bOverrideable )
		{
			for ( TINT i = 0; i < m_aActivities.Size(); i++ )
			{
				if ( !GetActivity( i )->IsOverrideable() )
				{
					bOverrideable = TFALSE;
					break;
				}
			}
		}
	}

	if ( bOverrideable )
		m_uiFlags |= FLAGS_OVERRIDEABLE;
	else
		m_uiFlags &= ~FLAGS_OVERRIDEABLE;

	return TTRUE;
}

// $Barnyard: FUNCTION 0059e620
TBOOL AInteraction::CanStart( AInteractionLatches* a_pLatches )
{
	return a_pLatches->Matches( m_aPreconditions );
}

// $Barnyard: FUNCTION 0059e650
AInteractionActivity* AInteraction::GetActivity( TINT a_iIndex )
{
	if ( a_iIndex >= m_aActivities.Size() )
		return TNULL;

	return AInteractionManager::GetSingleton()->GetActivity( m_aActivities[ a_iIndex ] );
}

// $Barnyard: FUNCTION 0059e680
void AInteraction::OnSequenceStart( AInteractionParticipants* a_pParticipants, AInteractionSequence* a_pSequence )
{
	for ( TINT i = 0; i < m_iNumRequirements; i++ )
		AInteractionManager::GetSingleton()->GetRequirement( m_auiRequirements[ i ] )->OnSequenceStart( a_pParticipants, a_pSequence );
}

// $Barnyard: FUNCTION 0059e6d0
void AInteraction::OnSequenceEnd( AInteractionSequence* a_pSequence )
{
	for ( TINT i = 0; i < m_iNumRequirements; i++ )
		AInteractionManager::GetSingleton()->GetRequirement( m_auiRequirements[ i ] )->OnSequenceEnd( a_pSequence );
}

// $Barnyard: FUNCTION 0059e7c0
void AInteraction::UpdatePriority( AInteractionParticipants* a_pParticipants, TINT* a_pUnits, TINT a_iNumUnits )
{
	// NOTE: a_iNumUnits is unused
	m_iCurrentPriority = m_iPriority;

	if ( m_uiFlags & FLAGS_PRIORITY_MOD )
	{
		if ( m_uiFlags & FLAGS_PRIORITY_CHANCE_MOD )
		{
			TINT iMin = m_pPriorityMods->aiRandomRange[ 0 ];
			m_iCurrentPriority += ARandom::GetSingleton()->m_oRandom.GetInt( m_pPriorityMods->aiRandomRange[ 1 ] - iMin ) + iMin;
		}

		if ( m_uiFlags & FLAGS_PRIORITY_MOD_CATEGORY )
		{
			AUnit* pUnit = a_pParticipants->Get( a_pUnits[ m_pPriorityMods->uiCategory ] );

			if ( pUnit )
			{
				AUnitProfileComponent* pProfile = TSTATICCAST( AUnitProfileComponent, pUnit->FindComponent( &TGetClass( AUnitProfileComponent ) ) );

				if ( pProfile )
				{
					AUnitFlagSet* pRelationships = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_RELATIONSHIPS );

					TUINT uiFriends    = 0;
					TUINT uiNotFriends = 0;
					pRelationships->GetMask( uiFriends, TPS8( friends ) );
					pRelationships->GetMask( uiNotFriends, TPS8( notfriends ) );

					// NOTE: retail keeps the masks in TBitField<32>s and tests them with TBitField::IsEmpty (0059e630)
					if ( ( uiFriends & pProfile->GetRelationships() ) != 0 )
						m_iCurrentPriority += m_pPriorityMods->aiRelationMods[ 0 ];
					else if ( ( uiNotFriends & pProfile->GetRelationships() ) != 0 )
						m_iCurrentPriority += m_pPriorityMods->aiRelationMods[ 2 ];
					else
						m_iCurrentPriority += m_pPriorityMods->aiRelationMods[ 1 ];
				}
			}
		}

		if ( m_iCurrentPriority < 1 )
			m_iCurrentPriority = 1;
	}
}

// $Barnyard: FUNCTION 0059eba0
void AInteraction::EvaluateRequirements( AInteractionParticipants* a_pParticipants, AInteractionRoles* a_pRoles )
{
	a_pRoles->fWeight = m_fChance * a_pRoles->fWeight;

	for ( TINT i = 0; i < m_iNumRequirements; i++ )
	{
		AInteractionRequirement* pRequirement = AInteractionManager::GetSingleton()->GetRequirement( m_auiRequirements[ i ] );

		// Evaluate once for every set of participants
		if ( pRequirement->m_uiParticipantsId != a_pParticipants->uiId )
		{
			pRequirement->m_oRoles.Reset();
			pRequirement->m_uiParticipantsId = a_pParticipants->uiId;
			pRequirement->Evaluate( a_pParticipants );
		}

		// See AInteractionManager::ForceRequirementSatisfied
		if ( pRequirement->m_eForcedState == AInteractionRequirement::FORCEDSTATE_SATISFIED )
		{
			pRequirement->m_oRoles.Reset();
			pRequirement->m_oRoles.fWeight = 1.0f;
		}
		else if ( pRequirement->m_eForcedState == AInteractionRequirement::FORCEDSTATE_NOT_SATISFIED )
		{
			pRequirement->m_oRoles.Clear();
			pRequirement->m_oRoles.fWeight = 0.0f;
		}

		if ( pRequirement->m_oRoles.fWeight <= 0.0f )
		{
			a_pRoles->fWeight = 0.0f;
			return;
		}

		a_pRoles->Combine( pRequirement->m_oRoles );

		if ( a_pRoles->fWeight <= 0.0f )
			return;
	}

	TINT aiUnits[ AInteractionParticipants::MAX_NUM_UNITS ];

	if ( a_pRoles->AssignUnits( aiUnits, a_pParticipants->iNumUnits, m_iMinUnits, m_iMinUnits ) >= m_iMinUnits )
	{
		UpdatePriority( a_pParticipants, aiUnits, m_iMinUnits );

		if ( !( m_uiFlags & FLAGS_OVERRIDEABLE ) || m_iMaxUnits == 1 )
		{
			TINT iNumUnits = a_pParticipants->iNumUnits;

			for ( TINT i = 0; i < iNumUnits; i++ )
			{
				AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AInteractionComponent ) ) );

				// Units that wait for their forced interaction to start don't take part in other ones
				if ( !pComponent->GetForcedInteractionArg().IsEmpty() && pComponent->GetSequence() == TNULL && m_strTrigger != pComponent->GetForcedInteraction() )
					a_pRoles->RemoveUnit( i );
			}
		}

		if ( !( m_uiFlags & FLAGS_UNK1 ) )
			return;
	}

	a_pRoles->fWeight = 0.0f;
}

// $Barnyard: FUNCTION 0059ede0
TINT AInteraction::SelectUnits( TINT* a_pUnits, AInteractionParticipants* a_pParticipants, AInteractionRoles* a_pRoles )
{
	EvaluateRequirements( a_pParticipants, a_pRoles );
	return a_pRoles->AssignUnits( a_pUnits, a_pParticipants->iNumUnits, m_iMinUnits, m_iMaxUnits );
}

// $Barnyard: FUNCTION 0059ee20
TBOOL AInteraction::LoadRequirements( const PBPropertyValueArray* a_pArray )
{
	AInteractionManager* pManager         = AInteractionManager::GetSingleton();
	TINT                 iNumRequirements = a_pArray->GetSize();

	for ( TINT i = 0; i < iNumRequirements; i++ )
		m_auiRequirements[ i ] = TUINT16( pManager->FindOrCreateRequirement( a_pArray->GetValue( i )->GetTPString8() ) );

	m_iNumRequirements = TINT8( iNumRequirements );
	return TTRUE;
}

// $Barnyard: FUNCTION 0059eef0
TWCHAR* AInteraction::GetActivityText()
{
	TWCHAR* wszText = TNULL;

	for ( TINT i = 0; i < m_aActivities.Size(); i++ )
	{
		if ( GetActivity( i )->GetText( &wszText, TTRUE ) )
			break;
	}

	return wszText;
}

// $Barnyard: FUNCTION 0059ef90
TBOOL AInteraction::LoadLatches( ALatchRefList* a_pLatches, const PBPropertyValueArray* a_pArray )
{
	AInteractionManager* pManager    = AInteractionManager::GetSingleton();
	TINT                 iNumLatches = a_pArray->GetSize();

	if ( iNumLatches > a_pLatches->SizeAllocated() )
		a_pLatches->Resize( iNumLatches );

	a_pLatches->SetSize( iNumLatches );

	for ( TINT i = 0; i < iNumLatches; i++ )
		( *a_pLatches )[ i ] = pManager->GetLatch( a_pArray->GetValue( i )->GetTPString8() );

	return TTRUE;
}

// $Barnyard: FUNCTION 0059f080
TBOOL AInteraction::LoadActivitySet( TArray<AInteractionActivity*>* a_pActivities, const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		AInteractionActivity* pActivity = AInteractionManager::CreateActivity( it );

		if ( pActivity )
			a_pActivities->Push( pActivity );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0059f110
TBOOL AInteraction::LoadActivities( TArray<AInteractionActivity*>* a_pActivities, const PBProperties* a_pProperties, TINT16 a_iFirstActivity, TArray<TUINT16>* a_pIndices )
{
	// NOTE: a_pActivities is unused
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const PBPropertyValueArray* pIndices = it->GetValue()->GetArray();

		for ( TINT i = 0; i < TINT( pIndices->GetSize() ); i++ )
			a_pIndices->Push( TUINT16( TINT16( pIndices->GetValue( i )->GetInteger() ) + a_iFirstActivity ) );
	}

	return TTRUE;
}
