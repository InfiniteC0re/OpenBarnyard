#include "pch.h"
#include "AAnimationActivity.h"
#include "AAnimatedUnit.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimationBreakpointUnitAction.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/ANamedAnimationSet.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b27f0, CreateObject 005b2e50
// NOTE: GetClass 005b27f0, CreateObject 005b2e50, CreateObjectInPlace 005b2ee0
TDEFINE_CLASS( AAnimationActivity );

TPSTRING8_DECLARE( namedanimationset );
TPSTRING8_DECLARE( animation );
TPSTRING8_DECLARE( block );
TPSTRING8_DECLARE( run );
TPSTRING8_DECLARE( breakpoint );
TPSTRING8_DECLARE( rand_dance );
TPSTRING8_DECLARE( pickup_bp );
TPSTRING8_DECLARE( hd_throw_bp );
TPSTRING8_DECLARE( start_bp );
TPSTRING8_DECLARE( end_bp );
TPSTRING8_DECLARE( start_climb_getoff_bp );
TPSTRING8_DECLARE( end_climb_getoff_bp );
TPSTRING8_DECLARE( attach_bp );
TPSTRING8_DECLARE( detach_bp );
TPSTRING8_DECLARE( putdown_bp );

// 0x007740f0
static TPString8* s_apBreakpointNames[ AAnimationActivity::BREAKPOINT_NUMOF ] = {
	&TPS8( pickup_bp ),
	&TPS8( hd_throw_bp ),
	&TPS8( start_bp ),
	&TPS8( end_bp ),
	&TPS8( start_climb_getoff_bp ),
	&TPS8( end_climb_getoff_bp ),
	&TPS8( attach_bp ),
	&TPS8( detach_bp ),
	&TPS8( putdown_bp ),
};

// "rand_dance" never plays the same dance move twice in a row
static TCHAR s_szDanceMove[]  = "dancemove1"; // 0x0077521c
static TINT  s_iLastDanceMove = 1;            // 0x00775228

// NOTE: retail leaves m_bRun and m_eBreakpoint uninitialised
AAnimationActivity::AAnimationActivity()
    : m_bBlock( TFALSE )
{
}

// $Barnyard: FUNCTION 005b2920
// $Barnyard: FUNCTION 005b2b20
AAnimationActivity::~AAnimationActivity()
{
}

// $Barnyard: FUNCTION 005b2830
TBOOL AAnimationActivity::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strNamedAnimationSet, TPS8( namedanimationset ) );
	a_pProperties->GetOptionalPropertyValue( m_strAnimation, TPS8( animation ) );
	a_pProperties->GetOptionalPropertyValue( m_bBlock, TPS8( block ) );

	m_bRun = TTRUE;
	a_pProperties->GetOptionalPropertyValue( m_bRun, TPS8( run ) );

	TPString8 strBreakpoint;
	a_pProperties->GetOptionalPropertyValue( strBreakpoint, TPS8( breakpoint ) );

	TINT i = 0;
	for ( ; i < BREAKPOINT_NUMOF; i++ )
	{
		if ( strBreakpoint == *s_apBreakpointNames[ i ] )
			break;
	}

	m_eBreakpoint = i;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b2b40
AInteractionActivity::RESULT AAnimationActivity::OnStart( AInteractionSequence* a_pSequence )
{
	if ( m_strAnimation.IsEmpty() )
		return m_bBlock ? RESULT_RUNNING : RESULT_DONE;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AAnimatableObject* pAnimObj = TSTATICCAST( AAnimatedUnit, it->GetOwner() )->GetAnimatableObject();

		if ( !m_strNamedAnimationSet.IsEmpty() )
		{
			// NOTE: inlined AAnimatableObjectType::FindAnimationSetOrDefault, the named animation set of the type or its default one
			ANamedAnimationSetRef pAnimationSet = pAnimObj->GetObjectType()->FindAnimationSet( m_strNamedAnimationSet );

			if ( !pAnimationSet.Get() )
				pAnimationSet = pAnimObj->GetObjectType()->GetDefaultAnimationSet();

			if ( pAnimObj->GetToshiAnimInterface().GetAnimationSet().Get() != pAnimationSet.Get() )
				pAnimObj->GetToshiAnimInterface().SetAnimationSet( pAnimationSet );
		}

		if ( !m_bRun )
		{
			m_strPlayingAnim = m_strAnimation;
		}
		else
		{
			if ( m_strAnimation == TPS8( rand_dance ) )
			{
				TINT iMove;

				do
				{
					iMove = ARandom::GetSingleton()->m_oRandom.GetInt( 6 ) + 1;
				} while ( iMove == s_iLastDanceMove );

				s_szDanceMove[ 9 ] = TCHAR( '0' + iMove );
				s_iLastDanceMove   = iMove;
				m_strPlayingAnim   = TPString8( s_szDanceMove );
			}
			else
			{
				m_strPlayingAnim = m_strAnimation;
			}

			// NOTE: inlined AAnimatableObject::HasAnimation, does the animation set of the object have the animation?
			if ( pAnimObj->GetToshiAnimInterface().GetAnimationSet()->GetNamedAnimation( m_strPlayingAnim ) )
				pAnimObj->PlayAnimation( m_strPlayingAnim );
		}
	}

	if ( m_eBreakpoint == BREAKPOINT_NONE )
	{
		if ( m_bRun )
			return m_bBlock ? RESULT_RUNNING : RESULT_DONE;
	}
	else
	{
		// Wait for the breakpoint
		m_oAnimEventListener.pCaller   = this;
		m_oAnimEventListener.pCallback = OnAnimEventCallback;

		// Insert at the tail of the global animation event list (same layout as its nodes)
		AnimEventListener* pHead = TREINTERPRETCAST( AnimEventListener*, &AAnimationBreakpointUnitAction::GetListeners() );

		m_oAnimEventListener.pNext = pHead;
		m_oAnimEventListener.pPrev = pHead->pPrev;
		pHead->pPrev->pNext        = &m_oAnimEventListener;
		pHead->pPrev               = &m_oAnimEventListener;
	}

	m_bBlock = TTRUE;
	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005b2a30
AInteractionActivity::RESULT AAnimationActivity::Update( AInteractionSequence* a_pSequence )
{
	TBOOL bPlaying = TFALSE;

	if ( !m_strPlayingAnim.IsEmpty() )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AAnimatableObject* pAnimObj = TSTATICCAST( AAnimatedUnit, it->GetOwner() )->GetAnimatableObject();

			// NOTE: the last unit decides
			bPlaying = !pAnimObj->IsAnimationFinished( m_strPlayingAnim );
		}
	}

	if ( ( !m_bBlock || !bPlaying ) && m_eBreakpoint != BREAKPOINT_NONE )
		m_oAnimEventListener.Disconnect();

	return ( m_bBlock && bPlaying ) ? RESULT_RUNNING : RESULT_DONE;
}

// $Barnyard: FUNCTION 005b2810
void AAnimationActivity::OnAnimEvent( void* a_pCaller, void* a_pEvent )
{
	if ( m_eBreakpoint == TSTATICCAST( TINT, a_pEvent )[ 1 ] )
		m_bBlock = TFALSE;
}

// $Barnyard: FUNCTION 005b2800
void __stdcall AAnimationActivity::OnAnimEventCallback( AAnimationActivity* a_pActivity, void* a_pEvent )
{
	a_pActivity->OnAnimEvent( a_pActivity, a_pEvent );
}

void AAnimationActivity::AnimEventListener::Disconnect()
{
	if ( pNext != this )
	{
		pPrev->pNext = pNext;
		pNext->pPrev = pPrev;
		pNext        = this;
		pPrev        = this;
	}

	pCaller   = TNULL;
	pCallback = TNULL;
}
