#include "pch.h"
#include "APrecisionScarecrow.h"
#include "Physics/ACollisionObject.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( APrecisionScarecrow );

// $Barnyard: FUNCTION 00502a30
// $Barnyard: FUNCTION 00502ac0
// NOTE: the constructor is inlined in the TClass create functions
APrecisionScarecrow::APrecisionScarecrow()
{
	m_bHit         = TFALSE;
	m_pKnockedOver = TNULL;
	m_pPumpkin     = TNULL;
	m_pPlayer      = TNULL;
}

APrecisionScarecrow::~APrecisionScarecrow()
{
}

// $Barnyard: FUNCTION 00502c10
TBOOL APrecisionScarecrow::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	AddFlags( FLAGS_AI_ENABLED );
	SetHashed( TTRUE );

	if ( !GetCollisionObject()->IsCollisionObjectLinked() )
		GetCollisionObject()->Activate();

	GetCollisionObject()->SetType( ACollisionObject::TYPE_SIMPLE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00502a00
void APrecisionScarecrow::OnDestroy()
{
	if ( m_pKnockedOver )
		m_pKnockedOver = TNULL;

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00502a20
void APrecisionScarecrow::SetPlayer( AUnit* a_pPlayer )
{
	m_pPlayer = a_pPlayer;
}

// $Barnyard: FUNCTION 00502b40
void APrecisionScarecrow::SetPumpkin( AUnit* a_pPumpkin )
{
	m_pPumpkin = a_pPumpkin;

	if ( a_pPumpkin )
	{
		AVisibleUnit* pPumpkin = TSTATICCAST( AVisibleUnit, a_pPumpkin );
		pPumpkin->SetVisible( TFALSE );

		if ( pPumpkin->GetCollisionObject()->IsCollisionObjectLinked() )
			pPumpkin->GetCollisionObject()->UnlinkCollisionObject();

		m_pPumpkin->GetSteer()->SetPosition( *GetSteer()->GetPosition() );
	}
}

// $Barnyard: FUNCTION 00502ba0
void APrecisionScarecrow::SetKnockedOver( AUnit* a_pScarecrow )
{
	if ( !a_pScarecrow )
	{
		m_pKnockedOver = TNULL;
		return;
	}

	m_pKnockedOver = a_pScarecrow;

	AVisibleUnit* pScarecrow = TSTATICCAST( AVisibleUnit, a_pScarecrow );

	if ( pScarecrow->GetCollisionObject()->IsCollisionObjectLinked() )
		pScarecrow->GetCollisionObject()->UnlinkCollisionObject();

	TSTATICCAST( AVisibleUnit, m_pKnockedOver )->SetVisible( TFALSE );
	m_pKnockedOver->GetSteer()->SetPosition( *GetSteer()->GetPosition() );
}

// $Barnyard: FUNCTION 00502c60
void APrecisionScarecrow::SetHit( TBOOL a_bHit )
{
	if ( !m_pKnockedOver )
		return;

	m_bHit = a_bHit;

	if ( !a_bHit )
	{
		SetVisible( TTRUE );
		AddFlags( FLAGS_MILK_TARGET );
		TSTATICCAST( AVisibleUnit, m_pKnockedOver )->SetVisible( TFALSE );
		return;
	}

	// Swap for the knocked over scarecrow
	SetVisible( TFALSE );

	AVisibleUnit* pKnockedOver = TSTATICCAST( AVisibleUnit, m_pKnockedOver );
	pKnockedOver->SetVisible( TTRUE );

	if ( !pKnockedOver->GetCollisionObject()->IsCollisionObjectLinked() )
		pKnockedOver->GetCollisionObject()->Activate();

	// The pumpkin falls off
	AVisibleUnit* pPumpkin = TSTATICCAST( AVisibleUnit, m_pPumpkin );

	if ( pPumpkin )
	{
		TVector4 vVelocity = TVector4::VEC_POSZ;
		TVector4 vPosition = *GetSteer()->GetPosition();
		vPosition.y -= 2.0f;

		m_pPumpkin->GetSteer()->SetPosition( vPosition );
		pPumpkin->SetVisible( TTRUE );

		if ( !pPumpkin->GetCollisionObject()->IsCollisionObjectLinked() )
			pPumpkin->GetCollisionObject()->Activate();

		pPumpkin->GetCollisionObject()->SetType( ACollisionObject::TYPE_SIMPLE );

		vVelocity.Multiply( 3.0f );
		m_pPumpkin->GetSteer()->SetVelocity( vVelocity );
		m_pPumpkin = TNULL;
	}
}

// $Barnyard: FUNCTION 00502dd0
void APrecisionScarecrow::UpdateAI( TFLOAT a_fDeltaTime )
{
	TVector4 vPosition  = *GetSteer()->GetPosition();
	TVector4 vDirection = *GetSteer()->GetDirection();

	// Face the player
	if ( m_pPlayer )
	{
		const TVector4* pPlayerPosition = m_pPlayer->GetSteer()->GetPosition();
		vDirection                      = TVector4( pPlayerPosition->x - vPosition.x, pPlayerPosition->y - vPosition.y, pPlayerPosition->z - vPosition.z, pPlayerPosition->w );
		vDirection.Normalise();
		GetSteer()->SetDirection( vDirection );
	}

	GetAnimatableObject()->GetModelInstance()->SetTransform( vPosition, vDirection );
	AUnit::UpdateAI( a_fDeltaTime );
}
