#include "pch.h"
#include "AFallingCowAnimController.h"
#include "UnitMovement/Steering/AFallingCowSteer.h"
#include "GameInterface/AGameStateController.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( bk_crashoffbike_loop );
TPSTRING8_DECLARE( bk_crashoffbike_land );
TPSTRING8_DECLARE( bk_crashoffbike_getup );
TPSTRING8_DECLARE( bk_crashoffbike_back_loop );
TPSTRING8_DECLARE( bk_crashoffbike_back_land );
TPSTRING8_DECLARE( bk_crashoffbike_back_getup );
TPSTRING8_DECLARE( ABikeRaceMicroGame );

// NOTE: GetClass 0058a4c0, CreateObject 0058a5b0, CreateObjectInPlace 0058a630
TDEFINE_CLASS( AFallingCowAnimController );

AFallingCowAnimController::AFallingCowAnimController()
    : m_eState( 0 )
    , m_iDirection( 0 )
{
}

// $Barnyard: FUNCTION 0058a6c0
// $Barnyard: FUNCTION 0058a4e0
AFallingCowAnimController::~AFallingCowAnimController()
{
}

// $Barnyard: FUNCTION 0058a6e0
TBOOL AFallingCowAnimController::OnCreate()
{
	SetDisabled( TTRUE );
	m_eState = STATE_FALLING;

	return TTRUE;
}

// Whether the animation called a_rcName ends in this frame (or isn't playing)
// NOTE: inlined in retail
static TBOOL IsAnimationEnding( AToshiAnimationInterface& a_rAnimInterface, const TPString8& a_rcName, TFLOAT a_fDeltaTime, TBOOL a_bTrueIfMissing )
{
	AToshiAnimationRef* pAnimRef = a_rAnimInterface.FindAnimationRef( a_rcName );

	if ( !pAnimRef )
		return a_bTrueIfMissing;

	TAnimation* pAnimation = a_rAnimInterface.GetAnimation( *pAnimRef );

	if ( !pAnimation )
		return TFALSE;

	TSkeletonSequence* pSequence = pAnimation->GetSkeletonInstance()->GetSkeleton()->GetSequence( pAnimation->GetSequence() );
	return pSequence->GetDuration() - pAnimation->GetSeqTime() < a_fDeltaTime;
}

// $Barnyard: FUNCTION 0058a7a0
void AFallingCowAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	if ( !pSteer->IsA( &TGetClass( AFallingCowSteer ) ) )
		return;

	UpdateTransform( a_fDeltaTime );

	AToshiAnimationInterface& rAnimInterface = GetAnimInterface();
	AModelInstance*           pModelInstance = m_pAnimatableObject->GetModelInstance();

	if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() )
	{
		AToshiAnimationRef* pAnimRef = rAnimInterface.GetAnim();

		if ( pAnimRef )
			rAnimInterface.StopAnim( *pAnimRef );

		return;
	}

	AFallingCowSteer* pFallingSteer = TSTATICCAST( AFallingCowSteer, TSTATICCAST( AUnit, m_pOwner )->GetSteer() );

	// Thrown again
	if ( !pFallingSteer->HasLanded() && pFallingSteer->GetTime() < 0.1f )
		m_eState = STATE_FALLING;

	const TPString8* pLoopAnim  = &TPS8( bk_crashoffbike_loop );
	const TPString8* pLandAnim  = &TPS8( bk_crashoffbike_land );
	const TPString8* pGetUpAnim = &TPS8( bk_crashoffbike_getup );

	if ( m_iDirection == 1 )
	{
		pLoopAnim  = &TPS8( bk_crashoffbike_back_loop );
		pLandAnim  = &TPS8( bk_crashoffbike_back_land );
		pGetUpAnim = &TPS8( bk_crashoffbike_back_getup );
	}

	switch ( m_eState )
	{
		case STATE_LAND:
			if ( !rAnimInterface.IsAnimPlaying( *pLandAnim, TFALSE ) )
			{
				rAnimInterface.PlayAnim( *pLandAnim, -1.0f, TFALSE );
				m_eState = STATE_LANDING;
			}
			break;
		case STATE_LANDING:
			if ( pFallingSteer->IsFinished() &&
			     !AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABikeRaceMicroGame ) ) &&
			     IsAnimationEnding( rAnimInterface, *pLandAnim, a_fDeltaTime, TTRUE ) )
			{
				m_eState = STATE_GETUP;
			}
			break;
		case STATE_GETUP:
			if ( !rAnimInterface.IsAnimPlaying( *pGetUpAnim, TFALSE ) )
			{
				rAnimInterface.PlayAnim( *pGetUpAnim, -1.0f, TFALSE );
				m_eState = STATE_GETTING_UP;
			}
			break;
		case STATE_GETTING_UP:
			if ( IsAnimationEnding( rAnimInterface, *pGetUpAnim, a_fDeltaTime, TFALSE ) )
				m_eState = STATE_DONE;
			break;
		case STATE_DONE:
			break;
		default:
			if ( !rAnimInterface.IsAnimPlaying( *pLoopAnim, TFALSE ) )
			{
				rAnimInterface.PlayAnim( *pLoopAnim, -1.0f, TFALSE );
				m_pAnimatableObject->Update( 0.0f );
			}

			if ( pFallingSteer->HasLanded() )
				m_eState = STATE_LAND;
			break;
	}
}

// $Barnyard: FUNCTION 0058a700
void AFallingCowAnimController::UpdateTransform( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	TVector4 vecDirection;
	pSteer->CopyDirection( vecDirection );

	m_pAnimatableObject->GetModelInstance()->SetTransform( vecPosition, vecDirection );
}
