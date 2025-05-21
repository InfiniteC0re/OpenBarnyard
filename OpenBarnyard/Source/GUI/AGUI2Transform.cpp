#include "pch.h"
#include "AGUI2Transform.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006c2060
void AGUI2Transform::Rotate( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );

	TVector2 vecRot0 = m_aMatrixRows[ 0 ];
	TVector2 vecRot1 = m_aMatrixRows[ 1 ];

	m_aMatrixRows[ 0 ].x = vecRot0.x * fCos + vecRot1.x * fSin;
	m_aMatrixRows[ 0 ].y = vecRot0.y * fCos + vecRot1.y * fSin;
	m_aMatrixRows[ 1 ].x = vecRot1.x * fCos - vecRot0.x * fSin;
	m_aMatrixRows[ 1 ].y = vecRot1.y * fCos - vecRot0.y * fSin;
}

// $Barnyard: FUNCTION 00404810
void AGUI2Transform::PreMultiply( const AGUI2Transform& a_rTransform )
{
	TVector2 vecRot0 = m_aMatrixRows[ 0 ];
	TVector2 vecRot1 = m_aMatrixRows[ 1 ];

	m_aMatrixRows[ 0 ].x = a_rTransform.m_aMatrixRows[ 0 ].x * vecRot0.x + a_rTransform.m_aMatrixRows[ 0 ].y * vecRot1.x;
	m_aMatrixRows[ 0 ].y = a_rTransform.m_aMatrixRows[ 0 ].x * vecRot0.y + a_rTransform.m_aMatrixRows[ 0 ].y * vecRot1.y;
	m_aMatrixRows[ 1 ].x = a_rTransform.m_aMatrixRows[ 1 ].x * vecRot0.x + a_rTransform.m_aMatrixRows[ 1 ].y * vecRot1.x;
	m_aMatrixRows[ 1 ].y = a_rTransform.m_aMatrixRows[ 1 ].x * vecRot0.y + a_rTransform.m_aMatrixRows[ 1 ].y * vecRot1.y;
}

void AGUI2Transform::Scale( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	AGUI2Transform transform;
	transform.m_aMatrixRows[ 0 ] = { a_fScaleX, 0.0f };
	transform.m_aMatrixRows[ 1 ] = { 0.0f, a_fScaleY };
	transform.m_vecTranslation   = { 0.0f, 0.0f };
	PreMultiply( transform );
}

// $Barnyard: FUNCTION 006c20f0
void AGUI2Transform::Transform( TVector2& a_rOutVec, const TVector2& a_rTransformVec ) const
{
	a_rOutVec.x = m_vecTranslation.x + m_aMatrixRows[ 0 ].x * a_rTransformVec.x + m_aMatrixRows[ 1 ].x * a_rTransformVec.y;
	a_rOutVec.y = m_vecTranslation.y + m_aMatrixRows[ 0 ].y * a_rTransformVec.x + m_aMatrixRows[ 1 ].y * a_rTransformVec.y;
}

// $Barnyard: FUNCTION 006c21a0
void AGUI2Transform::GetInverse( AGUI2Transform& a_rInverse ) const
{
	TFLOAT fMultiplier = 1.0f / ( m_aMatrixRows[ 0 ].x * m_aMatrixRows[ 1 ].y - m_aMatrixRows[ 0 ].y * m_aMatrixRows[ 1 ].x );

	// NOTE: this code seems to be returning incorrect inverse matrices but
	// it's the way it works in Barnyard. In de Blob, however, it was rewritten
	// to do math the correct way.
	a_rInverse.m_aMatrixRows[ 0 ].x = fMultiplier * m_aMatrixRows[ 0 ].x;
	a_rInverse.m_aMatrixRows[ 0 ].y = fMultiplier * m_aMatrixRows[ 1 ].x;
	a_rInverse.m_aMatrixRows[ 1 ].x = fMultiplier * m_aMatrixRows[ 0 ].y;
	a_rInverse.m_aMatrixRows[ 1 ].y = fMultiplier * m_aMatrixRows[ 1 ].y;

	// Someone probably forgot to remove some extra code?
	a_rInverse.m_vecTranslation   = TVector2( 0.0f, 0.0f );
	a_rInverse.m_vecTranslation.x = a_rInverse.m_aMatrixRows[ 1 ].x * m_vecTranslation.y + a_rInverse.m_aMatrixRows[ 0 ].x * m_vecTranslation.x + a_rInverse.m_vecTranslation.x;
	a_rInverse.m_vecTranslation.y = a_rInverse.m_aMatrixRows[ 0 ].y * m_vecTranslation.x + a_rInverse.m_aMatrixRows[ 1 ].y * m_vecTranslation.y + a_rInverse.m_vecTranslation.y;
	a_rInverse.m_vecTranslation.Negate();
}

void AGUI2Transform::SetScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	TVector2 vecTranslation = m_vecTranslation;
	SetIdentity();
	Scale( a_fScaleX, a_fScaleY );
	m_vecTranslation = vecTranslation;
}

void AGUI2Transform::SetFromScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	SetIdentity();
	Scale( a_fScaleX, a_fScaleY );
}

// $Barnyard: FUNCTION 006c2120
void AGUI2Transform::Multiply( AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB )
{
	a_rOutTransform.m_aMatrixRows[ 0 ].x = a_rB.m_aMatrixRows[ 0 ].y * a_rA.m_aMatrixRows[ 1 ].x + a_rA.m_aMatrixRows[ 0 ].x * a_rB.m_aMatrixRows[ 0 ].x;
	a_rOutTransform.m_aMatrixRows[ 0 ].y = a_rB.m_aMatrixRows[ 0 ].y * a_rA.m_aMatrixRows[ 1 ].y + a_rA.m_aMatrixRows[ 0 ].y * a_rB.m_aMatrixRows[ 0 ].x;
	a_rOutTransform.m_aMatrixRows[ 1 ].x = a_rB.m_aMatrixRows[ 1 ].x * a_rA.m_aMatrixRows[ 0 ].x + a_rB.m_aMatrixRows[ 1 ].y * a_rA.m_aMatrixRows[ 1 ].x;
	a_rOutTransform.m_aMatrixRows[ 1 ].y = a_rB.m_aMatrixRows[ 1 ].x * a_rA.m_aMatrixRows[ 0 ].y + a_rB.m_aMatrixRows[ 1 ].y * a_rA.m_aMatrixRows[ 1 ].y;
	a_rOutTransform.m_vecTranslation.x   = a_rB.m_vecTranslation.x * a_rA.m_aMatrixRows[ 0 ].x + a_rB.m_vecTranslation.y * a_rA.m_aMatrixRows[ 1 ].x + a_rA.m_vecTranslation.x;
	a_rOutTransform.m_vecTranslation.y   = a_rB.m_vecTranslation.x * a_rA.m_aMatrixRows[ 0 ].y + a_rB.m_vecTranslation.y * a_rA.m_aMatrixRows[ 1 ].y + a_rA.m_vecTranslation.y;
}
