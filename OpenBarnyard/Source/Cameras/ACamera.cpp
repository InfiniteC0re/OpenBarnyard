#include "pch.h"
#include "ACamera.h"

#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ACamera );

TFLOAT                ACamera::sm_fCurrentFOV           = Toshi::TMath::DegToRad( 60.0f );
const Toshi::TVector4 ACamera::sm_vWorldUp              = { 0.0f, -1.0f, 0.0f, 1.0f };
const Toshi::TVector4 ACamera::sm_vInitialLookDirection = { 0.0f, 0.0f, 1.0f, 1.0f };

// $Barnyard: FUNCTION 0045b040
ACamera::ACamera()
{
	m_Matrix             = TMatrix44::IDENTITY;
	m_fFOV               = sm_fCurrentFOV;
	m_fProjectionCentreX = 0.5f;
	m_fProjectionCentreY = 0.5f;
}

// $Barnyard: FUNCTION 0045b080
// $Barnyard: FUNCTION 0045b210
ACamera::~ACamera()
{
}

// $Barnyard: FUNCTION 00420490
TVector4 ACamera::GetOrientation() const
{
	TVector4 vOrientation = m_Matrix.AsBasisVector4( Toshi::BASISVECTOR_FORWARD );
	vOrientation.w        = 1.0f;

	return vOrientation;
}

// $Barnyard: FUNCTION 0045b0b0
void ACamera::GetFlatDirection( TVector4& a_rOutDirection ) const
{
	TVector4 vForward( m_Matrix.m_f31, m_Matrix.m_f32, m_Matrix.m_f33, 1.0f );
	a_rOutDirection.ProjectNormalToPlane( vForward, sm_vWorldUp );
}

// $Barnyard: FUNCTION 0045b110
TBOOL ACamera::IsPointInView( const Toshi::TVector4& a_rcPoint, TFLOAT a_fMaxDistance ) const
{
	Toshi::TVector4 vecPosition = m_Matrix.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION );
	Toshi::TVector4 vecForward( m_Matrix.m_f31, m_Matrix.m_f32, m_Matrix.m_f33, 1.0f );

	return Toshi::TMath::ConeVsSphere( vecPosition, vecForward, m_fFOV * 0.5f, a_fMaxDistance, a_rcPoint );
}

// $Barnyard: FUNCTION 0045b1d0
void ACamera::GetDirection( Toshi::TVector4& a_rOutDirection ) const
{
	TQuaternion quatRotation;
	TMatrix44::MatToQuat( quatRotation, m_Matrix );

	TQuaternion::RotateVector( a_rOutDirection.AsVector3(), quatRotation, TVector3( 0.0f, 0.0f, 1.0f ) );
	a_rOutDirection.w = 0.0f;
}

// $Barnyard: FUNCTION 0045b2c0
void ACamera::LookAtPoint( const Toshi::TVector4& a_rPoint )
{
	TVector4 vDirection = a_rPoint - m_Matrix.GetTranslation3();
	vDirection.Normalise();

	LookAtDirection( vDirection );
}

// $Barnyard: FUNCTION 0045b090
void ACamera::LookAtDirection( const Toshi::TVector4& a_rDirection )
{
	TASSERT( a_rDirection.isNormalised() );
	m_Matrix.LookAtDirection( a_rDirection, sm_vWorldUp );
}

// $Barnyard: FUNCTION 0045b230
void ACamera::LookAt( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcTarget )
{
	TVector4 vDirection;
	vDirection.x = a_rcTarget.x - a_rcPosition.x;
	vDirection.y = a_rcTarget.y - a_rcPosition.y;
	vDirection.z = a_rcTarget.z - a_rcPosition.z;
	vDirection.w = a_rcTarget.w;
	vDirection.Normalise();

	m_Matrix.LookAtDirection( vDirection, sm_vWorldUp );
	m_Matrix.GetTranslation() = a_rcPosition;
}

// $Barnyard: FUNCTION 0046cd40
void ACamera::CreateCameraDirection_RotateY( Toshi::TVector4& a_rOutDirection, TFLOAT a_fAngle )
{
	TVector3 vDirection = sm_vInitialLookDirection.AsVector3();
	vDirection.RotateY( -a_fAngle );

	a_rOutDirection.Set( vDirection.x, vDirection.y, vDirection.z, sm_vInitialLookDirection.w );
}

const Toshi::TMatrix44& ACamera::GetMatrix() const
{
	return m_Matrix;
}

Toshi::TMatrix44& ACamera::GetMatrix()
{
	return m_Matrix;
}
