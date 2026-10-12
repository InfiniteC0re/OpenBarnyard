#include "pch.h"
#include "ADriveActivity.h"
#include "AUnit.h"
#include "AAnimalAttributes.h"
#include "AUnitBehaviour.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b4c00, CreateObject 005b5060
// NOTE: GetClass 005b4c00, CreateObject 005b5060, CreateObjectInPlace 005b50e0
TDEFINE_CLASS( ADriveActivity );

TPSTRING8_DECLARE( suggestfsm );
TPSTRING8_DECLARE( commandfsm );
TPSTRING8_DECLARE( schedulefsm );

ADriveActivity::ADriveActivity()
    : m_eAttribute( ATTRIBUTE_NONE )
    , m_uiFlags( FLAGS_NONE )
    , m_fValue( 0.0f )
    , m_fWait( 0.0f )
{
}

// $Barnyard: FUNCTION 005b4e10
// $Barnyard: FUNCTION 005b5160
ADriveActivity::~ADriveActivity()
{
}

static TFLOAT GetFloatOrInt( const PBPropertyValue* a_pValue )
{
	return ( a_pValue->GetType() == PBPropertyValue::Type::Float ) ? a_pValue->GetFloat() : TFLOAT( a_pValue->GetInteger() );
}

// $Barnyard: FUNCTION 005b4e80
TBOOL ADriveActivity::Create( const PBProperties* a_pProperties )
{
	if ( a_pProperties->GetOptionalPropertyValue( m_strFSM, TPS8( suggestfsm ) ) )
	{
		m_eAttribute = ATTRIBUTE_SUGGESTION;
		m_uiFlags |= FLAGS_SET | FLAGS_SUGGEST_FSM;
		m_fValue = 1.0f;
		m_fWait  = 0.0f;
	}

	if ( a_pProperties->GetOptionalPropertyValue( m_strFSM, TPS8( commandfsm ) ) )
	{
		m_eAttribute = ATTRIBUTE_COMMAND;
		m_uiFlags |= FLAGS_SET | FLAGS_COMMAND_FSM;
		m_fValue = 1.0f;
		m_fWait  = 0.0f;
	}

	if ( a_pProperties->GetOptionalPropertyValue( m_strFSM, TPS8( schedulefsm ) ) )
	{
		m_eAttribute = ATTRIBUTE_SCHEDULE;
		m_uiFlags |= FLAGS_SET | FLAGS_SCHEDULE_FSM;
		m_fValue = 1.0f;
		m_fWait  = 0.0f;
	}

	const PBPropertyValue* pAttribute = a_pProperties->GetProperty( "attribute" );

	if ( pAttribute )
	{
		m_eAttribute = AAnimalAttributes::GetAttributeFromName( pAttribute->GetTPString8().GetString() );

		if ( m_eAttribute == ATTRIBUTE_NUMOF )
			m_eAttribute = ATTRIBUTE_NONE;
	}

	const PBPropertyValue* pWait = a_pProperties->GetProperty( "wait" );

	if ( pWait )
		m_fWait = GetFloatOrInt( pWait );

	const PBPropertyValue* pRate = a_pProperties->GetProperty( "rate" );

	if ( pRate )
	{
		m_uiFlags |= FLAGS_RATE;
		m_fValue = GetFloatOrInt( pRate );
	}

	const PBPropertyValue* pSet = a_pProperties->GetProperty( "set" );

	if ( pSet )
	{
		m_uiFlags |= FLAGS_SET;
		m_fValue = GetFloatOrInt( pSet );
	}

	const PBPropertyValue* pChange = a_pProperties->GetProperty( "change" );

	if ( pChange )
	{
		m_uiFlags |= FLAGS_CHANGE;
		m_fValue = GetFloatOrInt( pChange );
	}

	const PBPropertyValue* pBlock = a_pProperties->GetProperty( "block" );

	if ( pBlock && pBlock->GetRaw() )
		m_uiFlags |= FLAGS_BLOCK;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b4c10
AInteractionActivity::RESULT ADriveActivity::OnStart( AInteractionSequence* a_pSequence )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && !pUnit->IsPlayer() )
		{
			AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

			if ( pAttributes )
			{
				if ( m_uiFlags & FLAGS_SET )
					pAttributes->SetAttribute( m_eAttribute, m_fValue );
				else if ( m_uiFlags & FLAGS_CHANGE )
					pAttributes->ChangeAttribute( m_eAttribute, m_fValue );
				else
					continue;

				AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponentExact( &TGetClass( AUnitBehaviour ) ) );

#ifdef BARNYARD_COMMUNITY_PATCH
				// Retail expects every unit with attributes to have a behaviour
				if ( pBehaviour )
#endif
					pBehaviour->ResetDriveTimer();
			}
		}
	}

	return AInteractionActivity::OnStart( a_pSequence );
}

// $Barnyard: FUNCTION 005b4cd0
AInteractionActivity::RESULT ADriveActivity::Update( AInteractionSequence* a_pSequence )
{
	TBOOL bRunning = TFALSE;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( !pUnit || pUnit->IsPlayer() )
			continue;

		AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

		if ( pAttributes && ( m_uiFlags & FLAGS_RATE ) )
			pAttributes->ChangeAttribute( m_eAttribute, a_pSequence->GetActivityState().fDeltaTime * m_fValue );

		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponentExact( &TGetClass( AUnitBehaviour ) ) );

		if ( !pBehaviour )
			continue;

		if ( !m_strFSM.IsEmpty() )
		{
			TBOOL bStarted = TFALSE;

			if ( m_uiFlags & FLAGS_COMMAND_FSM )
				bStarted = pBehaviour->SetCommand( m_strFSM, TNULL, TTRUE );
			else if ( m_uiFlags & FLAGS_SUGGEST_FSM )
				bStarted = pBehaviour->SetSuggestion( m_strFSM, TNULL, TFALSE );
			else if ( m_uiFlags & FLAGS_SCHEDULE_FSM )
				bStarted = pBehaviour->SetSchedule( m_strFSM, TNULL, TTRUE );

			// Wait for the FSM to start
			if ( !bStarted && ( m_uiFlags & FLAGS_BLOCK ) )
				bRunning = TTRUE;
		}
		else
		{
			pBehaviour->ResetDriveTimer();
		}
	}

	if ( a_pSequence->GetActivityState().fTime < m_fWait )
		bRunning = TTRUE;

	return bRunning ? RESULT_RUNNING : RESULT_DONE;
}
