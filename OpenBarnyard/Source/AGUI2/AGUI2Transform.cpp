#include "pch.h"
#include "AGUI2Transform.h"

void AGUI2Transform::Rotate(TFLOAT a_fAngle)
{
	TFLOAT fSin;
	TFLOAT fCos;
	Toshi::TMath::SinCos(a_fAngle, fSin, fCos);

	TFLOAT f0X = m_Rotation[0].x;
	TFLOAT f1X = m_Rotation[1].x;
	TFLOAT f0Y = m_Rotation[0].y;
	TFLOAT f1Y = m_Rotation[1].y;
	m_Rotation[0].x = f0X * fCos + fSin * f1X;
	m_Rotation[0].y = f0Y * fCos + f1Y * fSin;
	m_Rotation[1].x = f1X * fCos - f0X * fSin;
	m_Rotation[1].y = f1Y * fCos - f0Y * fSin;
}

void AGUI2Transform::PreMultiply(const AGUI2Transform& a_rTransform)
{
	TFLOAT fVar2 = m_Rotation[0].x;
	TFLOAT fVar8 = m_Rotation[0].y;
	TFLOAT fVar3 = m_Rotation[1].x;
	TFLOAT fVar5 = m_Rotation[1].y;
	m_Rotation[1].x = a_rTransform.m_Rotation[1].x * fVar2 + fVar3 * a_rTransform.m_Rotation[1].y;
	m_Rotation[0].x = a_rTransform.m_Rotation[0].y * fVar3 + fVar2 * a_rTransform.m_Rotation[0].x;
	m_Rotation[1].y = a_rTransform.m_Rotation[1].x * fVar8 + fVar5 * a_rTransform.m_Rotation[1].y;
	m_Rotation[0].y = a_rTransform.m_Rotation[0].x * fVar8 + fVar5 * a_rTransform.m_Rotation[0].y;
}

void AGUI2Transform::Transform(Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec) const
{
	a_rOutVec.x = m_Position.x + m_Rotation[0].x * a_rTransformVec.x + m_Rotation[1].x * a_rTransformVec.y;
	a_rOutVec.y = m_Position.y + m_Rotation[0].y * a_rTransformVec.x + m_Rotation[1].y * a_rTransformVec.y;
}

void AGUI2Transform::Multiply(AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB)
{
	a_rOutTransform.m_Rotation[0].x = a_rB.m_Rotation[0].y * a_rA.m_Rotation[1].x + a_rA.m_Rotation[0].x * a_rB.m_Rotation[0].x;
	a_rOutTransform.m_Rotation[0].y = a_rB.m_Rotation[0].y * a_rA.m_Rotation[1].y + a_rA.m_Rotation[0].y * a_rB.m_Rotation[0].x;
	a_rOutTransform.m_Rotation[1].x = a_rB.m_Rotation[1].x * a_rA.m_Rotation[0].x + a_rB.m_Rotation[1].y * a_rA.m_Rotation[1].x;
	a_rOutTransform.m_Rotation[1].y = a_rB.m_Rotation[1].x * a_rA.m_Rotation[0].y + a_rB.m_Rotation[1].y * a_rA.m_Rotation[1].y;
	a_rOutTransform.m_Position.x = a_rB.m_Position.x * a_rA.m_Rotation[0].x + a_rB.m_Position.y * a_rA.m_Rotation[1].x + a_rA.m_Position.x;
	a_rOutTransform.m_Position.y = a_rB.m_Position.x * a_rA.m_Rotation[0].y + a_rB.m_Position.y * a_rA.m_Rotation[1].y + a_rA.m_Position.y;
}
