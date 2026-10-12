#include "pch.h"
#include "AFunctionCallActivity.h"
#include "AUnit.h"
#include "FSM/AFunctionTable.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>
#include <Plugins/PPropertyParser/PBProperties.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b6100, CreateObject 005b6650, CreateObjectInPlace 005b66b0
TDEFINE_CLASS( AFunctionCallActivity );

TPSTRING8_DECLARE( table );
TPSTRING8_DECLARE( function );
TPSTRING8_DECLARE( blockuntilmet );

// $Barnyard: FUNCTION 005b65a0
AFunctionCallActivity::AFunctionCallActivity()
    : m_pArguments( TNULL )
    , m_pResult( TNULL )
    , m_bBlockUntilMet( TFALSE )
    , m_itUnit( TSTATICCAST( AInteractionComponent, TNULL ) )
{
	m_oFunction.pTable    = TNULL;
	m_oFunction.pFunction = TNULL;

	// The function is called once per unit of the category, only the primary unit by default
	// (a recipe rock's "createandpushnamedstate" would otherwise run for the rock too and give two recipes)
	m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;

	m_oListener.Connect( this, OnInteractionManagerEventCallback );
}

// $Barnyard: FUNCTION 005b64a0
// $Barnyard: FUNCTION 005b6630
AFunctionCallActivity::~AFunctionCallActivity()
{
	if ( m_pArguments )
		delete m_pArguments;

	if ( m_pResult )
		delete m_pResult;

	m_oListener.Disconnect();
}

// $Barnyard: FUNCTION 005b6210
TBOOL AFunctionCallActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strTable;
	TPString8 strFunction;

	a_pProperties->GetOptionalPropertyValue( strTable, TPS8( table ) );
	a_pProperties->GetOptionalPropertyValue( strFunction, TPS8( function ) );
	a_pProperties->GetOptionalPropertyValue( m_bBlockUntilMet, TPS8( blockuntilmet ) );

	// NOTE: an unknown function doesn't make the activity fail, it's just never called
	m_oFunction.Find( strTable, strFunction );

	const PBPropertyValue* pResult = a_pProperties->GetProperty( "result" );

	if ( !pResult )
		m_pResult = new PPropertyValue();
	else
		m_pResult = PBPropertyReader::ToValue( pResult );

	const PBPropertyValue* pArguments = a_pProperties->GetProperty( "arguments" );

	if ( !pArguments )
	{
		// NOTE: value-initialised, retail zeroes the whole array
		m_pArguments = new PPropertyValueArray();
	}
	else if ( pArguments->GetType() != PBPropertyValue::Type::Array )
	{
		// A single argument
		m_pArguments = new PPropertyValueArray( 1 );

		PPropertyValue* pArgument = PBPropertyReader::ToValue( pArguments );

		// NOTE: retail writes the first value directly, the size of the array stays 0
		*m_pArguments->GetValue( 0 ) = *pArgument;

		if ( pArgument )
			delete pArgument;
	}
	else
	{
		m_pArguments = PBPropertyReader::ToValueArray( pArguments->GetArray() );
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b6110
AInteractionActivity::RESULT AFunctionCallActivity::OnStart( AInteractionSequence* a_pSequence )
{
	m_itUnit = a_pSequence->GetCategoryBegin( m_eCategory );

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005b6130
AInteractionActivity::RESULT AFunctionCallActivity::Update( AInteractionSequence* a_pSequence )
{
	PPropertyValue oResult;

	AFunctionCaller oCaller( m_itUnit->GetOwner() );

	if ( m_oFunction.Call( oCaller, &oResult, m_pArguments ) && m_bBlockUntilMet )
	{
		// Call the unit again until the function returns the result
		if ( !( oResult == *m_pResult ) )
			return RESULT_RUNNING;
	}

	m_itUnit++;

	return ( m_itUnit != a_pSequence->GetCategoryEnd( m_eCategory ) ) ? RESULT_RUNNING : RESULT_DONE;
}

// $Barnyard: FUNCTION 005b63f0
void AFunctionCallActivity::OnInteractionManagerEvent( const AInteractionManagerEvent& a_rcEvent )
{
	if ( a_rcEvent.GetType() & AInteractionManagerEvent::TYPE_INTERACTIONS_LOADED )
	{
		if ( T2String8::FindString( m_oFunction.GetName().GetString(), "forcerequirementsatisfied" ) )
		{
			PPropertyValue* pRequirement = m_pArguments->GetValue( 0 );

			// NOTE: retail goes through PPropertyValue::operator=( TINT ) (006b1730, ChangeType + set), same as SetInteger
			pRequirement->SetInteger( AInteractionManager::GetSingleton()->GetRequirementIndex( pRequirement->GetTPString8() ) );
		}
	}
}

// $Barnyard: FUNCTION 005b6580
void AFunctionCallActivity::OnInteractionManagerEventCallback( void* a_pReceiver, const AInteractionManagerEvent& a_rcEvent )
{
	TSTATICCAST( AFunctionCallActivity, a_pReceiver )->OnInteractionManagerEvent( a_rcEvent );
}
