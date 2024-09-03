#pragma once
#include <Math/TVector2.h>

class AGUI2Transform
{
public:
	enum TUninitialised
	{
		Uninitialised
	};

public:
	// constructors/destructor
	constexpr AGUI2Transform()
	{
		SetIdentity();
	}

	constexpr AGUI2Transform( TUninitialised ) {}

	~AGUI2Transform() = default;

	void Rotate( TFLOAT a_fAngle );
	void PreMultiply( const AGUI2Transform& a_rTransform );
	void PreMultiply( TFLOAT a_fBasisVec1Angle, TFLOAT a_fBasisVec2Angle );
	void Transform( Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec ) const;
	void GetInverse( AGUI2Transform& a_rInverse ) const;

	constexpr void Set( const Toshi::TVector2& a_rcMatComp1, const Toshi::TVector2& a_rcMatComp2, const Toshi::TVector2& a_rcTranslation )
	{
		m_Rotation[ 0 ] = a_rcMatComp1;
		m_Rotation[ 1 ] = a_rcMatComp2;
		m_Translation   = a_rcTranslation;
	}

	constexpr void SetIdentity()
	{
		m_Rotation[ 0 ] = Toshi::TVector2( 1.0f, 0.0f );
		m_Rotation[ 1 ] = Toshi::TVector2( 0.0f, 1.0f );
		m_Translation   = Toshi::TVector2( 0.0f, 0.0f );
	}

	constexpr void SetZero()
	{
		m_Rotation[ 0 ] = Toshi::TVector2( 0.0f, 0.0f );
		m_Rotation[ 1 ] = Toshi::TVector2( 0.0f, 0.0f );
		m_Translation   = Toshi::TVector2( 0.0f, 0.0f );
	}

	constexpr void SetTranslation( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_Translation.x = a_fX;
		m_Translation.y = a_fY;
	}

	constexpr void SetTranslation( const Toshi::TVector2& a_rcTranslation )
	{
		m_Translation = a_rcTranslation;
	}

	constexpr const Toshi::TVector2& GetTranslation() const { return m_Translation; }

public:
	static void Multiply( AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB );

public:
	Toshi::TVector2 m_Rotation[ 2 ];
	Toshi::TVector2 m_Translation;
};
