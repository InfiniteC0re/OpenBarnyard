#include "pch.h"
#include "AKidBikeSteer.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Helpers/ASpline.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00566ac0, CreateObject 00567080, CreateObjectInPlace 005670e0
TDEFINE_CLASS( AKidBikeSteer );

TPSTRING8_DECLARE( frontwheel );
TPSTRING8_DECLARE( rearwheel );

// $Barnyard: FUNCTION 00566c70
AKidBikeSteer::AKidBikeSteer()
    : m_pSpline( TNULL )
    , m_oTransform( TMatrix44::IDENTITY )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_vDirection( TVector4::VEC_POSZ )
    , m_fProgress( 0.0f )
    , m_fSpeed( 0.0f )
    , m_fDesiredSpeed( 0.0f )
    , m_fSideOffset( 0.0f )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail leaves it uninitialised until Create/CreateFrom
	m_fWheelBase = 0.0f;
#endif
}

// $Barnyard: FUNCTION 00566d00
// $Barnyard: FUNCTION 00566ad0
AKidBikeSteer::~AKidBikeSteer()
{
}

// $Barnyard: FUNCTION 005673e0
TBOOL AKidBikeSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );
	InitialiseWheels();

	a_pOther->CopyPosition( m_vPosition );
	a_pOther->CopyDirection( m_vDirection );

	return TTRUE;
}

// $Barnyard: FUNCTION 00567370
TBOOL AKidBikeSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );
	InitialiseWheels();

	m_vPosition  = a_rcPosition;
	m_vDirection = a_rcDirection;

	return TTRUE;
}

// $Barnyard: FUNCTION 00567250
TBOOL AKidBikeSteer::Update( TFLOAT a_fDeltaTime )
{
	UpdateProgress( a_fDeltaTime );

	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, &GetParentUnit() );
	pUnit->GetAnimatableObject()->GetModelInstance()->SetTransform( m_oTransform );

	return TTRUE;
}

// $Barnyard: FUNCTION 00567120
TBOOL AKidBikeSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;

	// NOTE: the direction is reset to +Z
	UpdateTransform( TVector4::VEC_POSZ );
	return TTRUE;
}

// NOTE: folded with ACarSteer::SetPosition3 (0055c910) in retail
TBOOL AKidBikeSteer::SetPosition3( const TVector3& a_rcPosition )
{
	TVector4 vPosition( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	SetPosition( vPosition );

	return TTRUE;
}

// $Barnyard: FUNCTION 00567180
void AKidBikeSteer::SetDirection( const TVector4& a_rcDirection )
{
	// NOTE: retail ignores a_rcDirection and reapplies the current direction (and the side offset once more)
	UpdateTransform( m_vDirection );
}

// $Barnyard: FUNCTION 00566ae0
void AKidBikeSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vPosition;
}

// NOTE: folded with ACarSteer's CopyPosition3 (0055c9a0)
void AKidBikeSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vPosition.AsVector3();
}

// $Barnyard: FUNCTION 00566b00
const TVector4* AKidBikeSteer::GetPosition()
{
	return &m_vPosition;
}

// $Barnyard: FUNCTION 00566b40
void AKidBikeSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vDirection;
}

// $Barnyard: FUNCTION 00566b20
void AKidBikeSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vDirection.AsVector3();
}

// $Barnyard: FUNCTION 00566b10
const TVector4* AKidBikeSteer::GetDirection()
{
	return &m_vDirection;
}

// $Barnyard: FUNCTION 00566b60
TFLOAT AKidBikeSteer::GetSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 00566b70
void AKidBikeSteer::SetSpeed( TFLOAT a_fSpeed )
{
	m_fDesiredSpeed = a_fSpeed;
	m_fSpeed        = a_fSpeed;
}

// $Barnyard: FUNCTION 00566c10
void AKidBikeSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vDirection    = a_rcVelocity;
	m_fSpeed        = m_vDirection.NormaliseMag();
	m_fDesiredSpeed = m_fSpeed;
}

// $Barnyard: FUNCTION 00566c50
TFLOAT AKidBikeSteer::GetRadius()
{
	return m_fWheelBase;
}

// $Barnyard: FUNCTION 00566bd0
void AKidBikeSteer::CopyVelocity2( TVector4& a_rOutVelocity )
{
	TVector4 vVelocity;
	vVelocity.Multiply( m_vDirection, m_fSpeed );
	a_rOutVelocity = vVelocity;
}

// $Barnyard: FUNCTION 00566b90
TVector4 AKidBikeSteer::GetVelocity2()
{
	TVector4 vVelocity;
	vVelocity.Multiply( m_vDirection, m_fSpeed );
	return vVelocity;
}

// $Barnyard: FUNCTION 00566c60
void AKidBikeSteer::SetSpline( const ASpline* a_pSpline )
{
	m_pSpline = a_pSpline;
}

// $Barnyard: FUNCTION 00567020
void AKidBikeSteer::SetProgress( TFLOAT a_fProgress )
{
	m_fProgress = a_fProgress;
	TMath::Clip( m_fProgress, 0.0f, 1.0f );

	m_pSpline->GetPositionAndDirection( m_fProgress, m_vPosition, m_vDirection );
	UpdateTransform( m_vDirection );
}

// $Barnyard: FUNCTION 005672a0
void AKidBikeSteer::InitialiseWheels()
{
	AAnimatedUnit*  pUnit          = TSTATICCAST( AAnimatedUnit, &GetParentUnit() );
	AModelInstance* pModelInstance = pUnit->GetAnimatableObject()->GetModelInstance();

	TMatrix44 mBone;
	pModelInstance->GetScaledBoneTransform( pModelInstance->GetBoneID( TPS8( frontwheel ) ), mBone );
	m_avWheelOffsets[ WHEEL_FRONT ] = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	pModelInstance->GetScaledBoneTransform( pModelInstance->GetBoneID( TPS8( rearwheel ) ), mBone );
	m_avWheelOffsets[ WHEEL_REAR ] = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

	m_fWheelBase = TVector4::Distance( m_avWheelOffsets[ WHEEL_FRONT ], m_avWheelOffsets[ WHEEL_REAR ] );
}

// $Barnyard: FUNCTION 005671b0
void AKidBikeSteer::UpdateProgress( TFLOAT a_fDeltaTime )
{
	if ( !m_pSpline )
		return;

	m_fSpeed = TMath::LERP( m_fSpeed, m_fDesiredSpeed, a_fDeltaTime * 0.001f );
	m_fProgress += ( m_fSpeed * a_fDeltaTime ) / m_pSpline->GetLength();
	TMath::Clip( m_fProgress, 0.0f, 1.0f );

	m_pSpline->GetPositionAndDirection( m_fProgress, m_vPosition, m_vDirection );
	UpdateTransform( m_vDirection );
}

// $Barnyard: FUNCTION 00566d20
void AKidBikeSteer::UpdateTransform( TVector4 a_vDirection )
{
	// The flat direction
	m_vDirection   = a_vDirection;
	m_vDirection.y = 0.0f;
	m_vDirection.Normalise();

	m_vUp = TVector4::VEC_NEGY;
	m_vRight.CrossProduct( m_vDirection, m_vUp );

	// Move to the side of the spline and onto the ground
	TVector4 vOffset;
	vOffset.Multiply( m_vRight, m_fSideOffset );
	m_vPosition.x += vOffset.x;
	m_vPosition.y += vOffset.y;
	m_vPosition.z += vOffset.z;
	m_vPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( m_vPosition.x, m_vPosition.z, 0 );

	// NOTE: the model of the bike rides along its second row (BASISVECTOR_UP) and the third row
	// (BASISVECTOR_FORWARD) points up (-Y is up in the world)

	// Put the wheels on the ground (the rear one first)
	for ( TINT i = WHEEL_NUMOF - 1; i >= 0; i-- )
	{
		m_oTransform                                           = TMatrix44::IDENTITY;
		m_oTransform.AsBasisVector3( BASISVECTOR_RIGHT )       = m_vRight.AsVector3();
		m_oTransform.AsBasisVector3( BASISVECTOR_UP )          = m_vDirection.AsVector3();
		m_oTransform.AsBasisVector3( BASISVECTOR_FORWARD )     = m_vUp.AsVector3();
		m_oTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = m_vPosition;

		// NOTE: the result is written over the translation of the matrix it's transformed by, like retail does
		TMatrix44::TransformVector( m_oTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ), m_oTransform, m_avWheelOffsets[ i ] );

		m_avWheels[ i ]   = m_oTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
		m_avWheels[ i ].y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( m_avWheels[ i ].x, m_avWheels[ i ].z, 0 );
	}

	// Pitch the bike along the line between its wheels
	TVector4 vForward;
	vForward.x = m_avWheels[ WHEEL_FRONT ].x - m_avWheels[ WHEEL_REAR ].x;
	vForward.y = m_avWheels[ WHEEL_FRONT ].y - m_avWheels[ WHEEL_REAR ].y;
	vForward.z = m_avWheels[ WHEEL_FRONT ].z - m_avWheels[ WHEEL_REAR ].z;
	vForward.w = m_avWheels[ WHEEL_REAR ].w;
	vForward.Normalise();

	TVector4 vSide;
	vSide.CrossProduct( vForward, TVector4::VEC_NEGY );

	m_vUp.CrossProduct( vSide, vForward );
	m_vUp.Normalise();

	m_vRight.CrossProduct( vForward, m_vUp );
	m_vRight.Normalise();

	m_oTransform                                           = TMatrix44::IDENTITY;
	m_oTransform.AsBasisVector3( BASISVECTOR_RIGHT )       = m_vRight.AsVector3();
	m_oTransform.AsBasisVector3( BASISVECTOR_UP )          = vForward.AsVector3();
	m_oTransform.AsBasisVector3( BASISVECTOR_FORWARD )     = m_vUp.AsVector3();
	m_oTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = m_vPosition;
}
