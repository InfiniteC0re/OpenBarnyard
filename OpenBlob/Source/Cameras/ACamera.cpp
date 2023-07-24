#include "pch.h"
#include "ACamera.h"
#include "Toshi/Math/TMath.h"

const Toshi::TVector4 ACamera::sm_vInitialPos = { 1339.0f, -10.0f, 1495.0f, 1.0f };
const Toshi::TVector4 ACamera::sm_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

using namespace Toshi;

void ACamera::ResetCameraMatrix(CameraMatrix& camMat)
{
	camMat.m_mMatrix.Identity();
	camMat.m_mShakeMatrix.Identity();
	camMat.m_fNear = 5.0f;
	camMat.m_fFar = 500.0f;
	camMat.m_fFOV = sm_fDefaultFOV;
	camMat.m_fAspect = Toshi::TMath::Tan(sm_fDefaultFOV * 0.5f);
	camMat.m_fProjectionCentreX = 0.5f;
	camMat.m_fProjectionCentreY = 0.5f;
}

void ACamera::RotateAroundAxis(const Toshi::TVector4& a_vAxis, float rotation)
{
	TVector4 quat = m_Matrix.m_mMatrix.AsBasisVector3(2);
	TQuaternion res;
	res.SetRotation(a_vAxis.AsVector3(), rotation);
	TQuaternion::RotateVector(quat.AsVector3(), res, quat.AsVector3());
	LookAtDirection(quat);
}

void ACamera::RotateAroundRight(float rotation, float a_fVal)
{
	TVector4 vAxis = m_Matrix.m_mMatrix.AsBasisVector3(2);

	TVector4 vRight;
	vRight.CrossProduct(vAxis, sm_vWorldUp);

	TASSERT(vRight.MagnitudeSq() > 0.0001f);

	vRight.Normalise();

	TQuaternion quat;
	quat.SetRotation(vRight.AsVector3(), rotation);

	TQuaternion::RotateVector(vAxis.AsVector3(), quat, vAxis.AsVector3());

	float fDotProduct = vAxis.DotProduct(sm_vWorldUp);
	float fVal = 1.0f - a_fVal;
	TVector4 vec = sm_vWorldUp;

	if (fDotProduct <= fVal)
	{
		if (fDotProduct > a_fVal - 1.0f)
		{
			LookAtDirection(vAxis);
			return;
		}
		vec.Negate();
		quat.SetRotation(vRight.AsVector3(), TMath::ACos(fVal));
	}
	else
	{
		quat.SetRotation(vRight.AsVector3(), -TMath::ACos(fVal));
	}

	TQuaternion::RotateVector(vAxis.AsVector3(), quat, vec.AsVector3());
	LookAtDirection(vAxis);
}

TBOOL ACamera::IsInViewCone(const TVector4& a_rvPos, float a_fSphereRadius) const
{
	const TMatrix44& transformMatrix = m_Matrix.m_mMatrix;
	
	return
		TMath::ConeVsSphere(
			m_Matrix.m_mMatrix.AsBasisVector3(3),
			m_Matrix.m_mMatrix.AsBasisVector3(2),
			m_Matrix.m_fFOV * 0.5f,
			a_fSphereRadius,
			a_rvPos
		);
}

void ACamera::LookAtPoint(const TVector4& point)
{
	TVector4 vec;
	vec = point - m_Matrix.m_mMatrix.GetTranslation4();
	vec.Normalise();
	LookAtDirection(vec);
}
