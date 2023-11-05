#pragma once
#include <Toshi/Math/TVector2.h>

class AGUI2Transform
{
public:
	AGUI2Transform()
	{
		m_Rotation[0] = { 1.0f, 0.0f };
		m_Rotation[1] = { 0.0f, 1.0f };
		m_Position    = { 0.0f, 0.0f };
	}

	void Rotate(TFLOAT a_fAngle);

	void PreMultiply(const AGUI2Transform& a_rTransform);

	void Transform(Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec) const;

public:
	static void Multiply(AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB);

private:
	Toshi::TVector2 m_Rotation[2];
	Toshi::TVector2 m_Position;
};
