#include "pch.h"
#include "ASnapToAnimationActivity.h"
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bd670, CreateObject 005bd8a0, CreateObjectInPlace 005bd920
TDEFINE_CLASS( ASnapToAnimationActivity );

// NOTE: inlined in CreateObject, m_bWaitForPlayEnd is left uninitialised
ASnapToAnimationActivity::ASnapToAnimationActivity()
    : m_iUnk08( 0 )
{
	m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;
}

// $Barnyard: FUNCTION 005bd6d0
// $Barnyard: FUNCTION 005bd990
ASnapToAnimationActivity::~ASnapToAnimationActivity()
{
}

// $Barnyard: FUNCTION 005bd770
TBOOL ASnapToAnimationActivity::Create( const PBProperties* a_pProperties )
{
	m_strAnimation    = TPString8();
	m_iUnk08          = 0;
	m_bWaitForPlayEnd = TFALSE;

	const PBPropertyValue* pAnimation = a_pProperties->GetProperty( "animation" );

	if ( pAnimation )
	{
		m_strAnimation = pAnimation->GetTPString8();
		m_strWaitAnim  = m_strAnimation;
	}

	const PBPropertyValue* pWaitForPlayEnd = a_pProperties->GetProperty( "waitforplayend" );

	if ( pWaitForPlayEnd )
		m_bWaitForPlayEnd = pWaitForPlayEnd->GetRaw() != TNULL;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bd680
AInteractionActivity::RESULT ASnapToAnimationActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( a_pSequence->GetComponents().Size() > 0 )
	{
		if ( m_bWaitForPlayEnd == TTRUE )
		{
			AUnit*             pUnit             = a_pSequence->GetUnitByCategory( m_eCategory );
			AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();

			return pAnimatableObject->IsAnimationFinished( m_strWaitAnim ) ? RESULT_DONE : RESULT_RUNNING;
		}

		// NOTE: retail ignores the result
		a_pSequence->GetUnit( 0 );
	}

	return RESULT_DONE;
}
