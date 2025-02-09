#pragma once
#include "TQuaternion.h"
#include "TPlane.h"

#ifdef TRENDERINTERFACE_DX8
#  include <d3dx8math.h>
#endif

TOSHI_NAMESPACE_START

using BASISVECTOR = TINT;
enum BASISVECTOR_ : BASISVECTOR
{
	BASISVECTOR_RIGHT,
	BASISVECTOR_UP,
	BASISVECTOR_FORWARD,
	BASISVECTOR_TRANSLATION
};

class TMatrix44
{
public:
	constexpr TMatrix44() = default;

	constexpr TMatrix44( const TMatrix44& a_rMatrix )
	    : m_f11( a_rMatrix.m_f11 ), m_f12( a_rMatrix.m_f12 ), m_f13( a_rMatrix.m_f13 ), m_f14( a_rMatrix.m_f14 ), m_f21( a_rMatrix.m_f21 ), m_f22( a_rMatrix.m_f22 ), m_f23( a_rMatrix.m_f23 ), m_f24( a_rMatrix.m_f24 ), m_f31( a_rMatrix.m_f31 ), m_f32( a_rMatrix.m_f32 ), m_f33( a_rMatrix.m_f33 ), m_f34( a_rMatrix.m_f34 ), m_f41( a_rMatrix.m_f41 ), m_f42( a_rMatrix.m_f42 ), m_f43( a_rMatrix.m_f43 ), m_f44( a_rMatrix.m_f44 )
	{}

	constexpr TMatrix44(
	    TFLOAT a_f11,
	    TFLOAT a_f12,
	    TFLOAT a_f13,
	    TFLOAT a_f14,
	    TFLOAT a_f21,
	    TFLOAT a_f22,
	    TFLOAT a_f23,
	    TFLOAT a_f24,
	    TFLOAT a_f31,
	    TFLOAT a_f32,
	    TFLOAT a_f33,
	    TFLOAT a_f34,
	    TFLOAT a_f41,
	    TFLOAT a_f42,
	    TFLOAT a_f43,
	    TFLOAT a_f44
	)
	    : m_f11( a_f11 ), m_f12( a_f12 ), m_f13( a_f13 ), m_f14( a_f14 ), m_f21( a_f21 ), m_f22( a_f22 ), m_f23( a_f23 ), m_f24( a_f24 ), m_f31( a_f31 ), m_f32( a_f32 ), m_f33( a_f33 ), m_f34( a_f34 ), m_f41( a_f41 ), m_f42( a_f42 ), m_f43( a_f43 ), m_f44( a_f44 )
	{}

	// $Barnyard: FUNCTION 0041fa30
	constexpr void Set(
	    TFLOAT a_f11,
	    TFLOAT a_f12,
	    TFLOAT a_f13,
	    TFLOAT a_f14,
	    TFLOAT a_f21,
	    TFLOAT a_f22,
	    TFLOAT a_f23,
	    TFLOAT a_f24,
	    TFLOAT a_f31,
	    TFLOAT a_f32,
	    TFLOAT a_f33,
	    TFLOAT a_f34,
	    TFLOAT a_f41,
	    TFLOAT a_f42,
	    TFLOAT a_f43,
	    TFLOAT a_f44
	)
	{
		m_f11 = a_f11;
		m_f12 = a_f12;
		m_f13 = a_f13;
		m_f14 = a_f14;
		m_f21 = a_f21;
		m_f22 = a_f22;
		m_f23 = a_f23;
		m_f24 = a_f24;
		m_f31 = a_f31;
		m_f32 = a_f32;
		m_f33 = a_f33;
		m_f34 = a_f34;
		m_f41 = a_f41;
		m_f42 = a_f42;
		m_f43 = a_f43;
		m_f44 = a_f44;
	}

	constexpr void Set( const TMatrix44& a_rcMatrix )
	{
		Set(
		    a_rcMatrix.m_f11, a_rcMatrix.m_f12, a_rcMatrix.m_f13, a_rcMatrix.m_f14,
		    a_rcMatrix.m_f21, a_rcMatrix.m_f22, a_rcMatrix.m_f23, a_rcMatrix.m_f24,
		    a_rcMatrix.m_f31, a_rcMatrix.m_f32, a_rcMatrix.m_f33, a_rcMatrix.m_f34,
		    a_rcMatrix.m_f41, a_rcMatrix.m_f42, a_rcMatrix.m_f43, a_rcMatrix.m_f44
		);
	}

	// $Barnyard: FUNCTION 006c8500
	constexpr void Identity()
	{
		Set( IDENTITY );
	}

	const TVector3& AsBasisVector3( BASISVECTOR a_iIndex ) const
	{
		return *TREINTERPRETCAST(
		    TVector3*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	TVector3& AsBasisVector3( BASISVECTOR a_iIndex )
	{
		return *TREINTERPRETCAST(
		    TVector3*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	const TVector4& AsBasisVector4( BASISVECTOR a_iIndex ) const
	{
		return *TREINTERPRETCAST(
		    TVector4*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	TVector4& AsBasisVector4( BASISVECTOR a_iIndex )
	{
		return *TREINTERPRETCAST(
		    TVector4*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	TVector3& GetTranslation3()
	{
		return AsBasisVector3( BASISVECTOR_TRANSLATION );
	}

	const TVector3& GetTranslation3() const
	{
		return AsBasisVector3( BASISVECTOR_TRANSLATION );
	}

	TVector4& GetTranslation()
	{
		return AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	const TVector4& GetTranslation() const
	{
		return AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	void SetTranslation( const TVector4& a_rTranslation )
	{
		GetTranslation() = a_rTranslation;
	}

	void LookAtTarget( const TVector4& a_rTarget, const TVector4& a_rUp );
	void LookAtDirection( const TVector4& a_rVec, const TVector4& a_rVec2 );

	// $Barnyard: FUNCTION 006c86f0
	constexpr void Scale( TFLOAT a_fScalar1, TFLOAT a_fScalar2, TFLOAT a_fScalar3 )
	{
		m_f11 *= a_fScalar1;
		m_f12 *= a_fScalar1;
		m_f13 *= a_fScalar1;
		m_f21 *= a_fScalar2;
		m_f22 *= a_fScalar2;
		m_f23 *= a_fScalar2;
		m_f31 *= a_fScalar3;
		m_f32 *= a_fScalar3;
		m_f33 *= a_fScalar3;
	}

	// $Barnyard: FUNCTION 006c8750
	constexpr void Scale( const TVector4& a_rScalars )
	{
		m_f11 *= a_rScalars.x;
		m_f12 *= a_rScalars.x;
		m_f13 *= a_rScalars.x;
		m_f21 *= a_rScalars.y;
		m_f22 *= a_rScalars.y;
		m_f23 *= a_rScalars.y;
		m_f31 *= a_rScalars.z;
		m_f32 *= a_rScalars.z;
		m_f33 *= a_rScalars.z;
	}

	constexpr void Scale( TFLOAT a_fScale )
	{
		Scale( a_fScale, a_fScale, a_fScale );
	}

	constexpr TBOOL IsOrthonormal() const
	{
		TFLOAT fVar1 = ( m_f32 * m_f32 + m_f31 * m_f31 + m_f33 * m_f33 ) - 1.0f;
		TFLOAT fVar2 = ( m_f22 * m_f22 + m_f21 * m_f21 + m_f23 * m_f23 ) - 1.0f;
		TFLOAT fVar3 = m_f22 * m_f32 + m_f31 * m_f21 + m_f23 * m_f33;
		TFLOAT fVar4 = m_f32 * m_f12 + m_f11 * m_f31 + m_f33 * m_f13;
		TFLOAT fVar5 = m_f22 * m_f12 + m_f11 * m_f21 + m_f23 * m_f13;
		TFLOAT fVar6 = ( m_f11 * m_f11 + m_f12 * m_f12 + m_f13 * m_f13 ) - 1.0f;

		return !( ( ( 0.01 <= fVar6 * fVar6 + fVar2 * fVar2 + fVar1 * fVar1 + fVar4 * fVar4 + fVar5 * fVar5 + fVar3 * fVar3 ) || ( m_f14 != 0.0 ) ) || ( m_f24 != 0.0 ) ) || ( ( m_f34 != 0.0 || ( m_f44 != 1.0 ) ) );
	}

	void Multiply( const TMatrix44& a_rLeft, const TMatrix44& a_rRight );
	void Multiply( const TMatrix44& a_rRight );

	TBOOL Invert( const TMatrix44& a_rRight );

	void InvertOrthogonal();
	void InvertOrthogonal( const TMatrix44& a_rRight );
	void InvertOrthonormal();

	TMatrix44& SetFromQuaternion( const TQuaternion& a_rQuaternion );
	TMatrix44& PushQuaternion( const TQuaternion& a_rQuaternion, const TMatrix44& a_rMatrix, const TVector3& a_rOrigin );

	void RotateX( TFLOAT a_fAngle );
	void RotateY( TFLOAT a_fAngle );
	void RotateZ( TFLOAT a_fAngle );

	void GetEulerXYZ( TVector3& a_rOutVec ) const;

	// $Barnyard: FUNCTION 006c8bf0
	static void TransformPlaneOrthogonal( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane )
	{
		RotateVector( a_rOutPlane.AsVector4(), a_rMatrix, a_rcSourcePlane.AsVector4() );

		a_rOutPlane.SetD(
		    a_rcSourcePlane.GetD() +
		    TVector4::DotProduct3(
		        a_rOutPlane.AsVector4(), a_rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION )
		    )
		);
	}

	// $Barnyard: FUNCTION 006c8b80
	constexpr static void RotateVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector )
	{
		TFLOAT fVar1  = a_rMatrix.m_f32;
		TFLOAT fVar2  = a_rVector.z;
		TFLOAT fVar3  = a_rMatrix.m_f12;
		TFLOAT fVar4  = a_rVector.x;
		TFLOAT fVar5  = a_rMatrix.m_f22;
		TFLOAT fVar6  = a_rVector.y;
		TFLOAT fVar7  = a_rMatrix.m_f33;
		TFLOAT fVar8  = a_rVector.z;
		TFLOAT fVar9  = a_rMatrix.m_f13;
		TFLOAT fVar10 = a_rVector.x;
		TFLOAT fVar11 = a_rMatrix.m_f23;
		TFLOAT fVar12 = a_rVector.y;
		TFLOAT fVar13 = a_rMatrix.m_f31;
		TFLOAT fVar14 = a_rVector.z;
		TFLOAT fVar15 = a_rMatrix.m_f21;
		TFLOAT fVar16 = a_rVector.y;
		TFLOAT fVar17 = a_rMatrix.m_f11;
		TFLOAT fVar18 = a_rVector.x;

		a_rOutVector.x = fVar17 * fVar18 + fVar15 * fVar16 + fVar13 * fVar14;
		a_rOutVector.y = fVar5 * fVar6 + fVar3 * fVar4 + fVar1 * fVar2;
		a_rOutVector.z = fVar11 * fVar12 + fVar9 * fVar10 + fVar7 * fVar8;
		a_rOutVector.w = a_rVector.w;
	}

	// $Barnyard: FUNCTION 006c8a20
	constexpr static void TransformVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector )
	{
		TFLOAT fVar1   = a_rMatrix.m_f32;
		TFLOAT fVar2   = a_rVector.z;
		TFLOAT fVar3   = a_rMatrix.m_f12;
		TFLOAT fVar4   = a_rVector.x;
		TFLOAT fVar5   = a_rMatrix.m_f22;
		TFLOAT fVar6   = a_rVector.y;
		TFLOAT fVar7   = a_rMatrix.m_f42;
		TFLOAT fVar8   = a_rMatrix.m_f33;
		TFLOAT fVar9   = a_rVector.z;
		TFLOAT fVar10  = a_rMatrix.m_f13;
		TFLOAT fVar11  = a_rVector.x;
		TFLOAT fVar12  = a_rMatrix.m_f23;
		TFLOAT fVar13  = a_rVector.y;
		TFLOAT fVar14  = a_rMatrix.m_f43;
		a_rOutVector.x = a_rMatrix.m_f11 * a_rVector.x + a_rMatrix.m_f21 * a_rVector.y + a_rMatrix.m_f31 * a_rVector.z + a_rMatrix.m_f41;
		a_rOutVector.y = fVar5 * fVar6 + fVar3 * fVar4 + fVar1 * fVar2 + fVar7;
		a_rOutVector.z = fVar12 * fVar13 + fVar10 * fVar11 + fVar8 * fVar9 + fVar14;
	}

	// $Barnyard: FUNCTION 006c8a80
	constexpr static void TransformVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector )
	{
		TFLOAT fVar1   = a_rMatrix.m_f42;
		TFLOAT fVar2   = a_rVector.w;
		TFLOAT fVar3   = a_rMatrix.m_f12;
		TFLOAT fVar4   = a_rVector.x;
		TFLOAT fVar5   = a_rMatrix.m_f32;
		TFLOAT fVar6   = a_rVector.z;
		TFLOAT fVar7   = a_rMatrix.m_f22;
		TFLOAT fVar8   = a_rVector.y;
		TFLOAT fVar9   = a_rMatrix.m_f43;
		TFLOAT fVar10  = a_rVector.w;
		TFLOAT fVar11  = a_rMatrix.m_f13;
		TFLOAT fVar12  = a_rVector.x;
		TFLOAT fVar13  = a_rMatrix.m_f33;
		TFLOAT fVar14  = a_rVector.z;
		TFLOAT fVar15  = a_rMatrix.m_f23;
		TFLOAT fVar16  = a_rVector.y;
		TFLOAT fVar17  = a_rMatrix.m_f41;
		TFLOAT fVar18  = a_rVector.w;
		TFLOAT fVar19  = a_rMatrix.m_f31;
		TFLOAT fVar20  = a_rVector.z;
		TFLOAT fVar21  = a_rMatrix.m_f21;
		TFLOAT fVar22  = a_rVector.y;
		TFLOAT fVar23  = a_rMatrix.m_f11;
		TFLOAT fVar24  = a_rVector.x;
		a_rOutVector.w = a_rMatrix.m_f24 * a_rVector.y + a_rMatrix.m_f34 * a_rVector.z + a_rMatrix.m_f14 * a_rVector.x + a_rMatrix.m_f44 * a_rVector.w;
		a_rOutVector.x = fVar23 * fVar24 + fVar21 * fVar22 + fVar19 * fVar20 + fVar17 * fVar18;
		a_rOutVector.y = fVar7 * fVar8 + fVar5 * fVar6 + fVar3 * fVar4 + fVar1 * fVar2;
		a_rOutVector.z = fVar15 * fVar16 + fVar13 * fVar14 + fVar11 * fVar12 + fVar9 * fVar10;
	}

	constexpr void operator=( const TMatrix44& a_rcMatrix )
	{
		Set(
		    a_rcMatrix.m_f11, a_rcMatrix.m_f12, a_rcMatrix.m_f13, a_rcMatrix.m_f14,
		    a_rcMatrix.m_f21, a_rcMatrix.m_f22, a_rcMatrix.m_f23, a_rcMatrix.m_f24,
		    a_rcMatrix.m_f31, a_rcMatrix.m_f32, a_rcMatrix.m_f33, a_rcMatrix.m_f34,
		    a_rcMatrix.m_f41, a_rcMatrix.m_f42, a_rcMatrix.m_f43, a_rcMatrix.m_f44
		);
	}

#ifdef TRENDERINTERFACE_DX8
	operator D3DXMATRIX*()
	{
		return TREINTERPRETCAST( D3DXMATRIX*, this );
	}

	operator const D3DXMATRIX*() const
	{
		return TREINTERPRETCAST( const D3DXMATRIX*, this );
	}
#endif

public:
	static constinit TMatrix44 IDENTITY;

public:
	TFLOAT m_f11, m_f12, m_f13, m_f14;
	TFLOAT m_f21, m_f22, m_f23, m_f24;
	TFLOAT m_f31, m_f32, m_f33, m_f34;
	TFLOAT m_f41, m_f42, m_f43, m_f44;
};

TOSHI_NAMESPACE_END
