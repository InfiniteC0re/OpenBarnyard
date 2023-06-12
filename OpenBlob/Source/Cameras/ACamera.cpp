#include "pch.h"
#include "ACamera.h"

const Toshi::TVector4 ACamera::sm_vInitialPos = { 1339.0f, -10.0f, 1495.0f, 1.0f };
const Toshi::TVector4 ACamera::sm_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

void ACamera::LookAtPoint(const Toshi::TVector4& point)
{
	Toshi::TVector4 vec;
	vec = point - m_mMatrices[0].GetTranslation4();
	vec.Normalize();
	m_mMatrices[0].LookAtDirection(vec, sm_vWorldUp);
}
