#include "pch.h"
#include "AKinematicsObstructionUnit.h"
#include "AUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00573b90, CreateObject 00573ce0, CreateObjectInPlace 00573d60
TDEFINE_CLASS( AKinematicsObstructionUnit );

// $Barnyard: FUNCTION 00573e00
AKinematicsObstructionUnit::AKinematicsObstructionUnit( TRef<AUnitProxy> a_pUnitProxy )
    : m_eFlags( FLAGS_NONE )
    , m_pUnitProxy( a_pUnitProxy )
    , m_iShape( SHAPE_CIRCLE )
    , m_iBone( -1 )
{
}

// $Barnyard: FUNCTION 00573c50
// $Barnyard: FUNCTION 00573de0
AKinematicsObstructionUnit::~AKinematicsObstructionUnit()
{
	// m_pUnitProxy is released by its destructor
}

// $Barnyard: FUNCTION 00573e80
void AKinematicsObstructionUnit::SetFlags( FLAGS a_eFlags, TBOOL a_bSet )
{
	if ( a_bSet )
		m_eFlags |= a_eFlags;
	else
		m_eFlags &= ~a_eFlags;

	if ( m_eFlags & FLAGS_USE_BONE )
	{
		AModelInstanceRef pModelInstance = TSTATICCAST( AAnimatedUnit, GetUnit() )->GetAnimatableObject()->GetModelInstance();
		m_iBone                          = pModelInstance->GetSkeletonInstance()->GetBoneID( "C_Pelvis" );
	}
	else
	{
		m_iBone = -1;
	}
}

// $Barnyard: FUNCTION 00573f50
const TVector4* AKinematicsObstructionUnit::GetPosition()
{
	if ( m_eFlags & FLAGS_USE_BONE )
	{
		// 007895a0
		static TMatrix44 s_matBoneTransform;

		AModelInstanceRef pModelInstance = TSTATICCAST( AAnimatedUnit, GetUnit() )->GetAnimatableObject()->GetModelInstance();
		pModelInstance->GetBoneTransform( m_iBone, s_matBoneTransform );

		return &s_matBoneTransform.GetTranslation();
	}

	return GetUnit()->GetSteer()->GetPosition();
}

// $Barnyard: FUNCTION 00573bf0
TVector4 AKinematicsObstructionUnit::GetDirection()
{
	return *GetUnit()->GetSteer()->GetDirection();
}

// $Barnyard: FUNCTION 00573bb0
TVector4 AKinematicsObstructionUnit::GetVelocity()
{
	return GetUnit()->GetSteer()->GetVelocity();
}

// $Barnyard: FUNCTION 00573c30
TFLOAT AKinematicsObstructionUnit::GetSpeed()
{
	return GetUnit()->GetSteer()->GetSpeed();
}

// $Barnyard: FUNCTION 00573bd0
TFLOAT AKinematicsObstructionUnit::GetRadius()
{
	return GetUnit()->GetSteer()->GetRadius();
}

// $Barnyard: FUNCTION 00573ba0
const TVector2* AKinematicsObstructionUnit::GetHalfSize()
{
	// 007743ac
	static const TVector2 s_vecUnk( 1.0f, 0.75f );
	return &s_vecUnk;
}

// $Barnyard: FUNCTION 00573cc0
TVector4& AKinematicsObstructionUnit::PredictPosition( TINT a_iUnk, TVector4& a_rOut )
{
	return GetUnit()->GetSteer()->PredictPosition( a_iUnk, a_rOut );
}
