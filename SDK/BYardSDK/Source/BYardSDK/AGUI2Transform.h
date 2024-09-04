#pragma once
#include <Math/TVector2.h>

class AGUI2Transform
{
public:
	AGUI2Transform()
	{
		m_aRotations[ 0 ] = { 1.0f, 0.0f };
		m_aRotations[ 1 ] = { 0.0f, 1.0f };
		m_vecTranslation  = { 0.0f, 0.0f };
	}

	void Rotate( TFLOAT a_fAngle );
	void PreMultiply( const AGUI2Transform& a_rTransform );
	void PreMultiply( TFLOAT a_fBasisVec1Angle, TFLOAT a_fBasisVec2Angle );
	void Transform( Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec ) const;
	void GetInverse( AGUI2Transform& a_rInverse ) const;

	void SetTranslation( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_vecTranslation.x = a_fX;
		m_vecTranslation.y = a_fY;
	}

	void SetTranslation( const Toshi::TVector2& a_rPos )
	{
		m_vecTranslation.x = a_rPos.x;
		m_vecTranslation.y = a_rPos.y;
	}

public:
	static void Multiply( AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB );

public:
	Toshi::TVector2 m_aRotations[ 2 ];
	Toshi::TVector2 m_vecTranslation;
};
