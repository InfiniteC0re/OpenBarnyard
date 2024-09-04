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
	void PreMultiply( TFLOAT a_fScaleX, TFLOAT a_fScaleY );
	void Transform( Toshi::TVector2& a_rOutVec, const Toshi::TVector2& a_rTransformVec ) const;
	void GetInverse( AGUI2Transform& a_rInverse ) const;

	// Sets scale preserving translation
	void SetScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	// Sets scale and resets translation
	void SetFromScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	constexpr void Set( const Toshi::TVector2& a_rcMatComp1, const Toshi::TVector2& a_rcMatComp2, const Toshi::TVector2& a_rcTranslation )
	{
		m_aRotations[ 0 ] = a_rcMatComp1;
		m_aRotations[ 1 ] = a_rcMatComp2;
		m_vecTranslation  = a_rcTranslation;
	}

	constexpr void SetIdentity()
	{
		m_aRotations[ 0 ] = Toshi::TVector2( 1.0f, 0.0f );
		m_aRotations[ 1 ] = Toshi::TVector2( 0.0f, 1.0f );
		m_vecTranslation  = Toshi::TVector2::VEC_ZERO;
	}

	constexpr void SetZero()
	{
		m_aRotations[ 0 ] = Toshi::TVector2::VEC_ZERO;
		m_aRotations[ 1 ] = Toshi::TVector2::VEC_ZERO;
		m_vecTranslation  = Toshi::TVector2::VEC_ZERO;
	}

	constexpr void SetTranslation( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_vecTranslation.x = a_fX;
		m_vecTranslation.y = a_fY;
	}

	constexpr void SetTranslation( const Toshi::TVector2& a_rcTranslation )
	{
		m_vecTranslation = a_rcTranslation;
	}

	constexpr const Toshi::TVector2& GetTranslation() const { return m_vecTranslation; }

public:
	static void Multiply( AGUI2Transform& a_rOutTransform, const AGUI2Transform& a_rA, const AGUI2Transform& a_rB );

public:
	Toshi::TVector2 m_aRotations[ 2 ];
	Toshi::TVector2 m_vecTranslation;
};
