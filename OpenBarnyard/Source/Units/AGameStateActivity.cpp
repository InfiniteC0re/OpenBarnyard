#include "pch.h"
#include "AGameStateActivity.h"
#include "GameInterface/AGameStateController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b66f0, CreateObject 005b6980, CreateObjectInPlace 005b6a00
TDEFINE_CLASS( AGameStateActivity );

// NOTE: inlined into CreateObject, retail leaves m_iUnk10 uninitialised
AGameStateActivity::AGameStateActivity()
    : m_iUnk10( 0 )
{
}

// $Barnyard: FUNCTION 005b6730
// $Barnyard: FUNCTION 005b6a70
AGameStateActivity::~AGameStateActivity()
{
}

// $Barnyard: FUNCTION 005b67d0
TBOOL AGameStateActivity::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strState1, TPS8D( "state1" ) );
	a_pProperties->GetOptionalPropertyValue( m_strState2, TPS8D( "state2" ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b6700
void AGameStateActivity::GetDataSize( TINT* a_pSize )
{
	// TBOOL, set once m_strState1 was the current state
	*a_pSize = 1;
}

// $Barnyard: FUNCTION 005b6710
AInteractionActivity::RESULT AGameStateActivity::OnStart( AInteractionSequence* a_pSequence )
{
	*TSTATICCAST( TBOOL, a_pSequence->GetActivityData() ) = TFALSE;

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005b68a0
AInteractionActivity::RESULT AGameStateActivity::Update( AInteractionSequence* a_pSequence )
{
	TBOOL*    pReachedState1 = TSTATICCAST( TBOOL, a_pSequence->GetActivityData() );
	TPString8 strState( AGameStateController::GetSingleton()->GetCurrentState()->GetClass()->GetName() );

	if ( !*pReachedState1 )
	{
		if ( strState == m_strState1 )
			*pReachedState1 = TTRUE;
	}
	else if ( strState == m_strState2 )
	{
		return RESULT_DONE;
	}

	return RESULT_RUNNING;
}
