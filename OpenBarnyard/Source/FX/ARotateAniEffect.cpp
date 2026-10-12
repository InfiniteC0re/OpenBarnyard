#include "pch.h"
#include "ARotateAniEffect.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006316d0, CreateObject 00631830, CreateObjectInPlace 006318a0 (the constructor is inlined in both)
TDEFINE_CLASS( ARotateAniEffect );

// NOTE: m_vRotationSpeed isn't initialised
ARotateAniEffect::ARotateAniEffect()
{
}

// $Barnyard: FUNCTION 00631910
// $Barnyard: FUNCTION 006317c0
ARotateAniEffect::~ARotateAniEffect()
{
}

// $Barnyard: FUNCTION 006317b0
AUnit* ARotateAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00631700
TBOOL ARotateAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_oUnit || !m_oUnit->GetUnit() )
	{
		Finish();
		return TFALSE;
	}

	AUnit* pUnit = m_oUnit ? m_oUnit->GetUnit() : TNULL;

	TVector4 vDirection = *pUnit->GetSteer()->GetDirection();
	TREINTERPRETCAST( TVector3*, &vDirection )->RotateX( a_fDeltaTime * m_vRotationSpeed.x );
	TREINTERPRETCAST( TVector3*, &vDirection )->RotateY( a_fDeltaTime * m_vRotationSpeed.y );
	TREINTERPRETCAST( TVector3*, &vDirection )->RotateZ( a_fDeltaTime * m_vRotationSpeed.z );

	pUnit->GetSteer()->SetDirection( vDirection );
	return TTRUE;
}

// $Barnyard: FUNCTION 00631930
void ARotateAniEffect::SetUnit( AUnit* a_pUnit )
{
	m_oUnit = a_pUnit->GetSelfProxy();
}

// $Barnyard: FUNCTION 006316e0
void ARotateAniEffect::SetRotationSpeed( const TVector4& a_rcSpeed )
{
	m_vRotationSpeed = a_rcSpeed;
}
