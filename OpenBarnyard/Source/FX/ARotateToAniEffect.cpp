#include "pch.h"
#include "ARotateToAniEffect.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006319c0, CreateObject 00631b00, CreateObjectInPlace 00631b70 (the constructor is inlined in both)
TDEFINE_CLASS( ARotateToAniEffect );

// NOTE: m_vDirection and m_iUnk1 aren't initialised
ARotateToAniEffect::ARotateToAniEffect()
{
}

// $Barnyard: FUNCTION 00631be0
// $Barnyard: FUNCTION 00631a90
ARotateToAniEffect::~ARotateToAniEffect()
{
}

// $Barnyard: FUNCTION 00631a80
AUnit* ARotateToAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 006319d0
TBOOL ARotateToAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_oUnit || !m_oUnit->GetUnit() )
	{
		Finish();
		return TFALSE;
	}

	AUnit* pUnit = m_oUnit ? m_oUnit->GetUnit() : TNULL;

	TVector4 vDirection = *pUnit->GetSteer()->GetDirection();
	vDirection.x        = TMath::LERP( vDirection.x, m_vDirection.x, a_fDeltaTime );
	vDirection.y        = TMath::LERP( vDirection.y, m_vDirection.y, a_fDeltaTime );
	vDirection.z        = TMath::LERP( vDirection.z, m_vDirection.z, a_fDeltaTime );

	pUnit->GetSteer()->SetDirection( vDirection );
	return TTRUE;
}
