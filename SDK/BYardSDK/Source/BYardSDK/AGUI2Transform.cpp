#include "pch.h"
#include "AGUI2Transform.h"

void AGUI2Transform::Rotate( TFLOAT a_fAngle )
{
	TFLOAT fSin;
	TFLOAT fCos;
	Toshi::TMath::SinCos( a_fAngle, fSin, fCos );

	TFLOAT f0X           = m_aMatrixRows[ 0 ].x;
	TFLOAT f1X           = m_aMatrixRows[ 1 ].x;
	TFLOAT f0Y           = m_aMatrixRows[ 0 ].y;
	TFLOAT f1Y           = m_aMatrixRows[ 1 ].y;
	m_aMatrixRows[ 0 ].x = f0X * fCos + fSin * f1X;
	m_aMatrixRows[ 0 ].y = f0Y * fCos + f1Y * fSin;
	m_aMatrixRows[ 1 ].x = f1X * fCos - f0X * fSin;
	m_aMatrixRows[ 1 ].y = f1Y * fCos - f0Y * fSin;
}

void AGUI2Transform::PreMultiply( const AGUI2Transform& a_rTransform )
{
	TFLOAT fVar2         = m_aMatrixRows[ 0 ].x;
	TFLOAT fVar8         = m_aMatrixRows[ 0 ].y;
	TFLOAT fVar3         = m_aMatrixRows[ 1 ].x;
	TFLOAT fVar5         = m_aMatrixRows[ 1 ].y;
	m_aMatrixRows[ 1 ].x = a_rTransform.m_aMatrixRows[ 1 ].x * fVar2 + fVar3 * a_rTransform.m_aMatrixRows[ 1 ].y;
	m_aMatrixRows[ 0 ].x = a_rTransform.m_aMatrixRows[ 0 ].y * fVar3 + fVar2 * a_rTransform.m_aMatrixRows[ 0 ].x;
	m_aMatrixRows[ 1 ].y = a_rTransform.m_aMatrixRows[ 1 ].x * fVar8 + fVar5 * a_rTransform.m_aMatrixRows[ 1 ].y;
	m_aMatrixRows[ 0 ].y = a_rTransform.m_aMatrixRows[ 0 ].x * fVar8 + fVar5 * a_rTransform.m_aMatrixRows[ 0 ].y;
}

void AGUI2Transform::PreMultiply( TFLOAT a_fBasisVec1Angle, TFLOAT a_fBasisVec2Angle )
{
	AGUI2Transform transform;
	transform.m_aMatrixRows[ 0 ] = { a_fBasisVec1Angle, 0.0f };
	transform.m_aMatrixRows[ 1 ] = { 0.0f, a_fBasisVec2Angle };
	transform.m_vecTranslation   = { 0.0f, 0.0f };
	PreMultiply( transform );
}

void AGUI2Transform::Transform( Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec ) const
{
	a_rOutVec.x = m_vecTranslation.x + m_aMatrixRows[ 0 ].x * a_rTransformVec.x + m_aMatrixRows[ 1 ].x * a_rTransformVec.y;
	a_rOutVec.y = m_vecTranslation.y + m_aMatrixRows[ 0 ].y * a_rTransformVec.x + m_aMatrixRows[ 1 ].y * a_rTransformVec.y;
}

void AGUI2Transform::GetInverse( AGUI2Transform& a_rInverse ) const
{
	// TODO: refactor
	TFLOAT fVar1;
	TFLOAT fVar2;

	fVar2                             = 1.0f / ( m_aMatrixRows[ 0 ].x * m_aMatrixRows[ 1 ].y - m_aMatrixRows[ 0 ].y * m_aMatrixRows[ 1 ].x );
	fVar1                             = m_aMatrixRows[ 1 ].x;
	a_rInverse.m_aMatrixRows[ 0 ].x   = fVar2 * m_aMatrixRows[ 0 ].x;
	a_rInverse.m_aMatrixRows[ 0 ].y   = fVar2 * fVar1;
	fVar1                             = m_aMatrixRows[ 1 ].y;
	a_rInverse.m_aMatrixRows[ 1 ].x   = fVar2 * m_aMatrixRows[ 0 ].y;
	a_rInverse.m_aMatrixRows[ 1 ].y   = fVar2 * fVar1;
	( a_rInverse.m_vecTranslation ).x = 0.0;
	( a_rInverse.m_vecTranslation ).y = 0.0;
	( a_rInverse.m_vecTranslation ).x = a_rInverse.m_aMatrixRows[ 1 ].x * ( m_vecTranslation ).y + a_rInverse.m_aMatrixRows[ 0 ].x * ( m_vecTranslation ).x + ( a_rInverse.m_vecTranslation ).x;
	( a_rInverse.m_vecTranslation ).y = a_rInverse.m_aMatrixRows[ 0 ].y * ( m_vecTranslation ).x + a_rInverse.m_aMatrixRows[ 1 ].y * ( m_vecTranslation ).y + ( a_rInverse.m_vecTranslation ).y;
	( a_rInverse.m_vecTranslation ).x = -( a_rInverse.m_vecTranslation ).x;
	( a_rInverse.m_vecTranslation ).y = -( a_rInverse.m_vecTranslation ).y;
}

void AGUI2Transform::Multiply( AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB )
{
	a_rOutTransform.m_aMatrixRows[ 0 ].x = a_rB.m_aMatrixRows[ 0 ].y * a_rA.m_aMatrixRows[ 1 ].x + a_rA.m_aMatrixRows[ 0 ].x * a_rB.m_aMatrixRows[ 0 ].x;
	a_rOutTransform.m_aMatrixRows[ 0 ].y = a_rB.m_aMatrixRows[ 0 ].y * a_rA.m_aMatrixRows[ 1 ].y + a_rA.m_aMatrixRows[ 0 ].y * a_rB.m_aMatrixRows[ 0 ].x;
	a_rOutTransform.m_aMatrixRows[ 1 ].x = a_rB.m_aMatrixRows[ 1 ].x * a_rA.m_aMatrixRows[ 0 ].x + a_rB.m_aMatrixRows[ 1 ].y * a_rA.m_aMatrixRows[ 1 ].x;
	a_rOutTransform.m_aMatrixRows[ 1 ].y = a_rB.m_aMatrixRows[ 1 ].x * a_rA.m_aMatrixRows[ 0 ].y + a_rB.m_aMatrixRows[ 1 ].y * a_rA.m_aMatrixRows[ 1 ].y;
	a_rOutTransform.m_vecTranslation.x   = a_rB.m_vecTranslation.x * a_rA.m_aMatrixRows[ 0 ].x + a_rB.m_vecTranslation.y * a_rA.m_aMatrixRows[ 1 ].x + a_rA.m_vecTranslation.x;
	a_rOutTransform.m_vecTranslation.y   = a_rB.m_vecTranslation.x * a_rA.m_aMatrixRows[ 0 ].y + a_rB.m_vecTranslation.y * a_rA.m_aMatrixRows[ 1 ].y + a_rA.m_vecTranslation.y;
}
