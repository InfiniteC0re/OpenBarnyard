#include "pch.h"
#include "ACamera.h"
#include "Toshi/Math/TMath.h"

const Toshi::TVector4 ACamera::sm_vInitialPos = { 1339.0f, -10.0f, 1495.0f, 1.0f };
const Toshi::TVector4 ACamera::sm_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

using namespace Toshi;

TBOOL ACamera::IsInViewCone(const TVector4& a_rvPos, float a_fSphereRadius) const
{
	const TMatrix44& transformMatrix = m_Matrix.m_mMatrix;
	TVector4 conePosition = { transformMatrix.m, transformMatrix.n, transformMatrix.o, transformMatrix.p };
	TVector4 coneDirection = { transformMatrix.i, transformMatrix.j, transformMatrix.k, 1.0f };

	return TMath::ConeVsSphere(conePosition, coneDirection, m_Matrix.m_fFOV * 0.5f, a_fSphereRadius, a_rvPos);
}

void ACamera::LookAtPoint(const TVector4& point)
{
	TVector4 vec;
	vec = point - m_Matrix.m_mMatrix.GetTranslation4();
	vec.Normalize();
	LookAtDirection(vec);
}
