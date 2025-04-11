#include "pch.h"
#include "ACamera.h"

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

ACamera::ACamera()
{
	m_Matrix             = TMatrix44::IDENTITY;
	m_fFOV               = sm_fCurrentFOV;
	m_fProjectionCentreX = 0.5f;
	m_fProjectionCentreY = 0.5f;
}

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

void ACamera::LookAtPoint( const Toshi::TVector4& a_rPoint )
{
	TVector4 vDirection = a_rPoint - m_Matrix.GetTranslation3();
	vDirection.Normalise();

	LookAtDirection( vDirection );
}

void ACamera::LookAtDirection( const Toshi::TVector4& a_rDirection )
{
	TASSERT( a_rDirection.isNormalised() );
	m_Matrix.LookAtDirection( a_rDirection, sm_vWorldUp );
}

const Toshi::TMatrix44& ACamera::GetMatrix() const
{
	return m_Matrix;
}

Toshi::TMatrix44& ACamera::GetMatrix()
{
	return m_Matrix;
}
