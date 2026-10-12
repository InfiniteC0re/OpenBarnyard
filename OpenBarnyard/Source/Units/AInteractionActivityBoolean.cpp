#include "pch.h"
#include "AInteractionActivityBoolean.h"
#include "AInteraction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0059fc10, CreateObject 0059ff10, CreateObjectInPlace 0059ff70
TDEFINE_CLASS( AInteractionActivityBoolean );

// $Barnyard: FUNCTION 0059fe80
AInteractionActivityBoolean::AInteractionActivityBoolean()
    : m_aTrueActivities( 10, 0 )
    , m_aTruePostconditions( 10, 0 )
    , m_aFalseActivities( 10, 0 )
    , m_aFalsePostconditions( 10, 0 )
{
}

// $Barnyard: FUNCTION 0059fca0
// $Barnyard: FUNCTION 0059fe60
AInteractionActivityBoolean::~AInteractionActivityBoolean()
{
	// The activities of the blocks are owned by the activity
	for ( TINT i = 0; i < m_aTrueActivities.Size(); i++ )
	{
		if ( m_aTrueActivities[ i ] )
			delete m_aTrueActivities[ i ];
	}

	m_aTrueActivities.Resize( 0 );

	for ( TINT i = 0; i < m_aFalseActivities.Size(); i++ )
	{
		if ( m_aFalseActivities[ i ] )
			delete m_aFalseActivities[ i ];
	}

	m_aFalseActivities.Resize( 0 );
}

// $Barnyard: FUNCTION 0059fdb0
TBOOL AInteractionActivityBoolean::Create( const PBProperties* a_pProperties )
{
	const PBProperties* pTrue = a_pProperties->FindProperties( "true" );

	if ( pTrue )
	{
		const PBPropertyValue* pPostconditions = pTrue->GetProperty( "postconditions" );

		if ( pPostconditions )
			AInteraction::LoadLatches( &m_aTruePostconditions, pPostconditions->GetArray() );

		const PBProperties* pActivities = pTrue->FindProperties( "activities" );

		if ( pActivities )
			AInteraction::LoadActivitySet( &m_aTrueActivities, pActivities );
	}

	const PBProperties* pFalse = a_pProperties->FindProperties( "false" );

	if ( pFalse )
	{
		const PBPropertyValue* pPostconditions = pFalse->GetProperty( "postconditions" );

		if ( pPostconditions )
			AInteraction::LoadLatches( &m_aFalsePostconditions, pPostconditions->GetArray() );

		const PBProperties* pActivities = pFalse->FindProperties( "activities" );

		if ( pActivities )
			AInteraction::LoadActivitySet( &m_aFalseActivities, pActivities );
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 0059fc20
AInteractionActivity::RESULT AInteractionActivityBoolean::OnStart( AInteractionSequence* a_pSequence )
{
	m_oActivityState.Reset();
	return AInteractionActivity::OnStart( a_pSequence );
}

// $Barnyard: FUNCTION 0059fc40
void AInteractionActivityBoolean::UpdateTrue( AInteractionSequence* a_pSequence )
{
	UpdateActivities( TNULL, &m_aTrueActivities, a_pSequence, &m_oActivityState, a_pSequence->GetActivityState().fDeltaTime, TFALSE );
	a_pSequence->GetLatches().Apply( m_aTruePostconditions );
}

// $Barnyard: FUNCTION 0059fc70
void AInteractionActivityBoolean::UpdateFalse( AInteractionSequence* a_pSequence )
{
	UpdateActivities( TNULL, &m_aFalseActivities, a_pSequence, &m_oActivityState, a_pSequence->GetActivityState().fDeltaTime, TFALSE );
	a_pSequence->GetLatches().Apply( m_aFalsePostconditions );
}
