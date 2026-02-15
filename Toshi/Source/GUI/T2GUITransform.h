#pragma once
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

class T2GUITransform
{
public:
	enum TUninitialised
	{
		Uninitialised
	};

public:
	// constructors/destructor
	constexpr T2GUITransform()
	{
		SetIdentity();
	}

	constexpr T2GUITransform( TUninitialised ) {}

	~T2GUITransform() = default;

	void Rotate( TFLOAT a_fAngle );
	void PreMultiply( const T2GUITransform& a_rTransform );
	void Scale( TFLOAT a_fScaleX, TFLOAT a_fScaleY );
	void Transform( TVector2& a_rOutVec, const TVector2& a_rTransformVec ) const;
	void GetInverse( T2GUITransform& a_rInverse ) const;

	// Sets scale preserving translation
	void SetScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	TFLOAT GetScaleX() const { return m_aMatrixRows[ 0 ].x; }
	TFLOAT GetScaleY() const { return m_aMatrixRows[ 1 ].y; }

	// Sets scale and resets translation
	void SetFromScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	constexpr void Set( const TVector2& a_rcMatComp1, const TVector2& a_rcMatComp2, const TVector2& a_rcTranslation )
	{
		m_aMatrixRows[ 0 ] = a_rcMatComp1;
		m_aMatrixRows[ 1 ] = a_rcMatComp2;
		m_vecTranslation   = a_rcTranslation;
	}

	// $Barnyard: FUNCTION 004047f0
	constexpr void SetIdentity()
	{
		m_aMatrixRows[ 0 ] = TVector2( 1.0f, 0.0f );
		m_aMatrixRows[ 1 ] = TVector2( 0.0f, 1.0f );
		m_vecTranslation   = TVector2::VEC_ZERO;
	}

	constexpr void SetZero()
	{
		m_aMatrixRows[ 0 ] = TVector2::VEC_ZERO;
		m_aMatrixRows[ 1 ] = TVector2::VEC_ZERO;
		m_vecTranslation   = TVector2::VEC_ZERO;
	}

	constexpr void SetTranslation( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_vecTranslation.x = a_fX;
		m_vecTranslation.y = a_fY;
	}

	constexpr void SetTranslation( const TVector2& a_rcTranslation )
	{
		m_vecTranslation = a_rcTranslation;
	}

	constexpr const TVector2& GetTranslation() const { return m_vecTranslation; }

public:
	static void Multiply( T2GUITransform& a_rOutTransform, const T2GUITransform& a_rA, const T2GUITransform& a_rB );

public:
	TVector2 m_aMatrixRows[ 2 ];
	TVector2 m_vecTranslation;
};

TOSHI_NAMESPACE_END
