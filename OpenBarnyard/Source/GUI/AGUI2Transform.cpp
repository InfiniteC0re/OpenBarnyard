#include "pch.h"
#include "AGUI2Transform.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void AGUI2Transform::Rotate( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	Toshi::TMath::SinCos( a_fAngle, fSin, fCos );

	Toshi::TVector2 vecRot0 = m_Rotation[ 0 ];
	Toshi::TVector2 vecRot1 = m_Rotation[ 1 ];

	m_Rotation[ 0 ].x = vecRot0.x * fCos + vecRot1.x * fSin;
	m_Rotation[ 0 ].y = vecRot0.y * fCos + vecRot1.y * fSin;
	m_Rotation[ 1 ].x = vecRot1.x * fCos - vecRot0.x * fSin;
	m_Rotation[ 1 ].y = vecRot1.y * fCos - vecRot0.y * fSin;
}

void AGUI2Transform::PreMultiply( const AGUI2Transform& a_rTransform )
{
	Toshi::TVector2 vecRot0 = m_Rotation[ 0 ];
	Toshi::TVector2 vecRot1 = m_Rotation[ 1 ];

	m_Rotation[ 0 ].x = a_rTransform.m_Rotation[ 0 ].x * vecRot0.x + a_rTransform.m_Rotation[ 0 ].y * vecRot1.x;
	m_Rotation[ 0 ].y = a_rTransform.m_Rotation[ 0 ].x * vecRot0.y + a_rTransform.m_Rotation[ 0 ].y * vecRot1.y;
	m_Rotation[ 1 ].x = a_rTransform.m_Rotation[ 1 ].x * vecRot0.x + a_rTransform.m_Rotation[ 1 ].y * vecRot1.x;
	m_Rotation[ 1 ].y = a_rTransform.m_Rotation[ 1 ].x * vecRot0.y + a_rTransform.m_Rotation[ 1 ].y * vecRot1.y;
}

void AGUI2Transform::PreMultiply( TFLOAT a_fBasisVec1Angle, TFLOAT a_fBasisVec2Angle )
{
	AGUI2Transform transform;
	transform.m_Rotation[ 0 ] = { a_fBasisVec1Angle, 0.0f };
	transform.m_Rotation[ 1 ] = { 0.0f, a_fBasisVec2Angle };
	transform.m_Translation   = { 0.0f, 0.0f };
	PreMultiply( transform );
}

void AGUI2Transform::Transform( Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec ) const
{
	a_rOutVec.x = m_Translation.x + m_Rotation[ 0 ].x * a_rTransformVec.x + m_Rotation[ 1 ].x * a_rTransformVec.y;
	a_rOutVec.y = m_Translation.y + m_Rotation[ 0 ].y * a_rTransformVec.x + m_Rotation[ 1 ].y * a_rTransformVec.y;
}

void AGUI2Transform::GetInverse( AGUI2Transform& a_rInverse ) const
{
	TFLOAT fMultiplier = 1.0f / ( m_Rotation[ 0 ].x * m_Rotation[ 1 ].y - m_Rotation[ 0 ].y * m_Rotation[ 1 ].x );

	// NOTE: this code seems to be returning incorrect inverse matrices but
	// it's the way it works in Barnyard. In de Blob, however, it was rewritten
	// to do math the correct way.
	a_rInverse.m_Rotation[ 0 ].x = fMultiplier * m_Rotation[ 0 ].x;
	a_rInverse.m_Rotation[ 0 ].y = fMultiplier * m_Rotation[ 1 ].x;
	a_rInverse.m_Rotation[ 1 ].x = fMultiplier * m_Rotation[ 0 ].y;
	a_rInverse.m_Rotation[ 1 ].y = fMultiplier * m_Rotation[ 1 ].y;

	// Someone probably forgot to remove some extra code?
	a_rInverse.m_Translation   = Toshi::TVector2( 0.0f, 0.0f );
	a_rInverse.m_Translation.x = a_rInverse.m_Rotation[ 1 ].x * m_Translation.y + a_rInverse.m_Rotation[ 0 ].x * m_Translation.x + a_rInverse.m_Translation.x;
	a_rInverse.m_Translation.y = a_rInverse.m_Rotation[ 0 ].y * m_Translation.x + a_rInverse.m_Rotation[ 1 ].y * m_Translation.y + a_rInverse.m_Translation.y;
	a_rInverse.m_Translation.Negate();
}

void AGUI2Transform::Multiply( AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB )
{
	a_rOutTransform.m_Rotation[ 0 ].x = a_rB.m_Rotation[ 0 ].y * a_rA.m_Rotation[ 1 ].x + a_rA.m_Rotation[ 0 ].x * a_rB.m_Rotation[ 0 ].x;
	a_rOutTransform.m_Rotation[ 0 ].y = a_rB.m_Rotation[ 0 ].y * a_rA.m_Rotation[ 1 ].y + a_rA.m_Rotation[ 0 ].y * a_rB.m_Rotation[ 0 ].x;
	a_rOutTransform.m_Rotation[ 1 ].x = a_rB.m_Rotation[ 1 ].x * a_rA.m_Rotation[ 0 ].x + a_rB.m_Rotation[ 1 ].y * a_rA.m_Rotation[ 1 ].x;
	a_rOutTransform.m_Rotation[ 1 ].y = a_rB.m_Rotation[ 1 ].x * a_rA.m_Rotation[ 0 ].y + a_rB.m_Rotation[ 1 ].y * a_rA.m_Rotation[ 1 ].y;
	a_rOutTransform.m_Translation.x   = a_rB.m_Translation.x * a_rA.m_Rotation[ 0 ].x + a_rB.m_Translation.y * a_rA.m_Rotation[ 1 ].x + a_rA.m_Translation.x;
	a_rOutTransform.m_Translation.y   = a_rB.m_Translation.x * a_rA.m_Rotation[ 0 ].y + a_rB.m_Translation.y * a_rA.m_Rotation[ 1 ].y + a_rA.m_Translation.y;
}
