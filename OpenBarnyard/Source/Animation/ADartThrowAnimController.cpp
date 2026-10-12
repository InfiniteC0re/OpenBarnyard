#include "pch.h"
#include "ADartThrowAnimController.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058a0b0, CreateObject 0058a230, CreateObjectInPlace 0058a2a0
TDEFINE_CLASS( ADartThrowAnimController );

ADartThrowAnimController::ADartThrowAnimController()
    : m_bPlayIdle( TFALSE )
    , m_fAimX( 0.0f )
    , m_fAimY( 0.0f )
{
}

// $Barnyard: FUNCTION 0058a330
// $Barnyard: FUNCTION 0058a0d0
ADartThrowAnimController::~ADartThrowAnimController()
{
}

// NOTE: also used by ABikeCrashAnimController
// $Barnyard: FUNCTION 0058a350
TBOOL ADartThrowAnimController::OnCreate()
{
	AAnimController::OnCreate();
	return TTRUE;
}

TPSTRING8_DECLARE( darts_throw );
TPSTRING8_DECLARE( darts_idle );
TPSTRING8_DECLARE( darts_raise );

// $Barnyard: FUNCTION 0058a460
void ADartThrowAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	UpdateTransform( a_fDeltaTime );

	// NOTE: retail checks darts_throw twice
	if ( m_pAnimatableObject->IsAnimPlaying( TPS8( darts_throw ), 0.0f ) == TTRUE )
		return;

	if ( m_pAnimatableObject->IsAnimPlaying( TPS8( darts_throw ), 0.0f ) == TTRUE )
		return;

	if ( m_bPlayIdle )
		m_pAnimatableObject->PlayAnimation( TPS8( darts_idle ) );
}

// $Barnyard: FUNCTION 0058a360
void ADartThrowAnimController::UpdateTransform( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	TVector4 vecDirection;
	pSteer->CopyDirection( vecDirection );

	TFLOAT fPitch;
	TFLOAT fRoll;

	if ( pSteer->GetTilt( &fPitch, &fRoll ) )
		m_pAnimatableObject->GetModelInstance()->SetTransformTilted( vecPosition, vecDirection, fPitch, fRoll );
	else
		m_pAnimatableObject->GetModelInstance()->SetTransform( vecPosition, vecDirection );
}

// $Barnyard: FUNCTION 0058a0e0
void ADartThrowAnimController::SetAim( TFLOAT a_fAimX, TFLOAT a_fAimY )
{
	m_fAimX = a_fAimX;
	m_fAimY = a_fAimY;
}

// $Barnyard: FUNCTION 0058a100
void ADartThrowAnimController::Throw()
{
	m_bPlayIdle = TFALSE;

	GetAnimInterface().KillAllAnimations();
	GetAnimInterface().PlayAnim( TPS8( darts_throw ), -1.0f, TFALSE );
}

// $Barnyard: FUNCTION 0058a130
void ADartThrowAnimController::Raise()
{
	m_bRaised   = TTRUE;
	m_bPlayIdle = TFALSE;

	GetAnimInterface().KillAllAnimations();
	GetAnimInterface().PlayAnim( TPS8( darts_raise ), -1.0f, TFALSE );
}
