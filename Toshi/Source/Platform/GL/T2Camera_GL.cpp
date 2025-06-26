#include "ToshiPCH.h"
#include "T2Camera_GL.h"
#include "T2Render_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2Camera::T2Camera()
    : m_eCameraMode( CM_PERSPECTIVE )
    , m_fFOV( TMath::DegToRad( 90.0f ) )
    , m_fNearPlane( 1.0f )
    , m_fFarPlane( 100.0f )
{
}

T2Camera::~T2Camera()
{
}

TMatrix44 T2Camera::GetViewMatrix()
{
	TMatrix44 oViewMatrix;
	m_TransformObject.GetLocalMatrixImp( oViewMatrix );
	oViewMatrix.InvertOrthogonal();

	return oViewMatrix;
}

TOSHI_NAMESPACE_END
