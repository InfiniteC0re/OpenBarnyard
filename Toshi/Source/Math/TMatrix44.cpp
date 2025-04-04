#include "ToshiPCH.h"
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

constinit TMatrix44 TMatrix44::IDENTITY(
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f
);

void TMatrix44::LookAtTarget( const TVector4& target, const TVector4& up )
{
	auto& forward = AsBasisVector3( 2 );

	forward = target.AsVector3() - up.AsVector3();
	forward.Normalize();

	AsBasisVector3( 0 ) = { m_f33, 0, -m_f31 };
	AsBasisVector3( 0 ).Normalize();

	m_f22 = m_f11 * m_f33 - m_f13 * m_f31;
	m_f23 = m_f12 * m_f31 - m_f11 * m_f32;
	m_f21 = m_f13 * m_f32 - m_f12 * m_f33;
}

// $Barnyard: FUNCTION 006c8970
void TMatrix44::LookAtDirection( const Toshi::TVector4& a_rDirection, const Toshi::TVector4& a_rUpAxis )
{
	TVector4 vProjection;
	TVector4 vCrossProduct;

	vProjection.ProjectNormalToPlane( a_rUpAxis, a_rDirection );
	vProjection.Multiply( -1.0f );
	vCrossProduct.CrossProduct( vProjection, a_rDirection );

	AsBasisVector4( 0 )   = vCrossProduct;
	AsBasisVector4( 0 ).w = 0.0f;

	AsBasisVector4( 1 )   = vProjection;
	AsBasisVector4( 1 ).w = 0.0f;

	AsBasisVector4( 2 )   = a_rDirection;
	AsBasisVector4( 2 ).w = 0.0f;
}

// $Barnyard: FUNCTION 006c85f0
void TMatrix44::Multiply( const TMatrix44& a_rLeft, const TMatrix44& a_rRight )
{
	TASSERT( ( this != &a_rLeft ) && ( this != &a_rRight ) );

	for ( TINT i = 0; i < 4; i++ )
	{
		TFLOAT* pBasis = AsBasisVector4( i ).AsArray();

		for ( TINT k = 0; k < 4; k++ )
		{
			pBasis[ k ] =
			    a_rLeft.AsBasisVector4( 3 ).AsArray()[ k ] * a_rRight.AsBasisVector4( i ).w +
			    a_rLeft.AsBasisVector4( 2 ).AsArray()[ k ] * a_rRight.AsBasisVector4( i ).z +
			    a_rLeft.AsBasisVector4( 1 ).AsArray()[ k ] * a_rRight.AsBasisVector4( i ).y +
			    a_rLeft.AsBasisVector4( 0 ).AsArray()[ k ] * a_rRight.AsBasisVector4( i ).x;
		}
	}
}

// $Barnyard: FUNCTION 0041fb30
void TMatrix44::Multiply( const TMatrix44& a_rRight )
{
	TMatrix44 temp = *this;
	Multiply( temp, a_rRight );
}

// $Barnyard: FUNCTION 006c8ef0
TBOOL TMatrix44::Invert( const TMatrix44& a_rRight )
{
	TASSERT( a_rRight.m_f14 == 0.0f );
	TASSERT( a_rRight.m_f24 == 0.0f );
	TASSERT( a_rRight.m_f34 == 0.0f );
	TASSERT( a_rRight.m_f44 == 1.0f );

	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fVar3;
	TFLOAT fVar4;

	fVar1 = a_rRight.m_f22 * a_rRight.m_f33 - a_rRight.m_f32 * a_rRight.m_f23;
	fVar4 = -( a_rRight.m_f33 * a_rRight.m_f12 - a_rRight.m_f32 * a_rRight.m_f13 );
	fVar3 = a_rRight.m_f23 * a_rRight.m_f12 - a_rRight.m_f22 * a_rRight.m_f13;
	fVar2 = fVar4 * a_rRight.m_f21 + fVar1 * a_rRight.m_f11 + fVar3 * a_rRight.m_f31;

	if ( fVar2 == 0.0 )
		return TFALSE;

	fVar2 = 1.0f / fVar2;
	Set( fVar2 * fVar1, fVar4 * fVar2, fVar3 * fVar2, a_rRight.m_f14, -( ( a_rRight.m_f33 * a_rRight.m_f21 - a_rRight.m_f31 * a_rRight.m_f23 ) * fVar2 ), ( a_rRight.m_f11 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f13 ) * fVar2, -( ( a_rRight.m_f11 * a_rRight.m_f23 - a_rRight.m_f21 * a_rRight.m_f13 ) * fVar2 ), a_rRight.m_f24, ( a_rRight.m_f32 * a_rRight.m_f21 - a_rRight.m_f31 * a_rRight.m_f22 ) * fVar2, -( ( a_rRight.m_f32 * a_rRight.m_f11 - a_rRight.m_f31 * a_rRight.m_f12 ) * fVar2 ), ( a_rRight.m_f22 * a_rRight.m_f11 - a_rRight.m_f21 * a_rRight.m_f12 ) * fVar2, a_rRight.m_f34, -a_rRight.m_f41, -a_rRight.m_f42, -a_rRight.m_f43, a_rRight.m_f44 );
	RotateVector( AsBasisVector4( 3 ), *this, AsBasisVector4( 3 ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 006c9050
void TMatrix44::InvertOrthogonal( const TMatrix44& a_rRight )
{
	TASSERT( a_rRight.m_f14 == 0.0f );
	TASSERT( a_rRight.m_f24 == 0.0f );
	TASSERT( a_rRight.m_f34 == 0.0f );
	TASSERT( a_rRight.m_f44 == 1.0f );

	TFLOAT fVar7  = a_rRight.m_f13;
	TFLOAT fVar1  = a_rRight.m_f44;
	TFLOAT fVar2  = a_rRight.m_f43;
	TFLOAT fVar8  = a_rRight.m_f24;
	TFLOAT fVar3  = a_rRight.m_f42;
	TFLOAT fVar9  = a_rRight.m_f32;
	TFLOAT fVar4  = a_rRight.m_f41;
	TFLOAT fVar10 = a_rRight.m_f22;
	TFLOAT fVar5  = a_rRight.m_f34;
	TFLOAT fVar11 = a_rRight.m_f12;
	TFLOAT fVar6  = a_rRight.m_f33;
	TFLOAT fVar12 = a_rRight.m_f14;
	TFLOAT fVar13 = a_rRight.m_f31;
	TFLOAT fVar14 = a_rRight.m_f21;
	TFLOAT fVar15 = a_rRight.m_f11;

	m_f32 = a_rRight.m_f23;
	m_f11 = fVar15;
	m_f33 = fVar6;
	m_f13 = fVar13;
	m_f34 = fVar5;
	m_f21 = fVar11;
	m_f41 = -fVar4;
	m_f23 = fVar9;
	m_f42 = -fVar3;
	m_f31 = fVar7;
	m_f43 = -fVar2;
	m_f12 = fVar14;
	m_f44 = fVar1;
	m_f14 = fVar12;
	m_f22 = fVar10;
	m_f24 = fVar8;
	RotateVector( AsBasisVector4( 3 ), *this, AsBasisVector4( 3 ) );
}

// $Barnyard: FUNCTION 006c9110
void TMatrix44::InvertOrthogonal()
{
	InvertOrthogonal( *this );
}

void TMatrix44::InvertOrthonormal()
{
	TASSERT( IsOrthonormal() );

	std::swap( m_f12, m_f21 );
	std::swap( m_f13, m_f31 );
	std::swap( m_f23, m_f32 );

	m_f42 *= -1;
	m_f41 *= -1;
	m_f43 *= -1;
	m_f42 = m_f22 * m_f42 + m_f41 * m_f12 + m_f43 * m_f32;
	m_f43 = m_f42 * m_f23 + m_f41 * m_f13 + m_f33 * m_f43;
	m_f41 = m_f42 * m_f21 + m_f41 * m_f11 + m_f43 * m_f31;
}

// $Barnyard: FUNCTION 006c8d90
TMatrix44& TMatrix44::SetFromQuaternion( const TQuaternion& a_rQuaternion )
{
	TFLOAT fVal1 = a_rQuaternion.z * 2.0f;
	TFLOAT fVal2 = a_rQuaternion.y * 2.0f;
	TFLOAT fVal3 = a_rQuaternion.x * 2.0f * a_rQuaternion.w;
	TFLOAT fVal4 = a_rQuaternion.x * 2.0f * a_rQuaternion.x;
	TFLOAT fVal5 = fVal1 * a_rQuaternion.z;

	m_f14 = 0.0f;
	m_f24 = 0.0f;
	m_f34 = 0.0f;
	m_f41 = 0.0f;
	m_f42 = 0.0f;
	m_f43 = 0.0f;
	m_f44 = 1.0f;
	m_f11 = 1.0f - ( fVal5 + fVal2 * a_rQuaternion.y );
	m_f21 = fVal2 * a_rQuaternion.x - fVal1 * a_rQuaternion.w;
	m_f12 = fVal2 * a_rQuaternion.x + fVal1 * a_rQuaternion.w;
	m_f31 = fVal1 * a_rQuaternion.x + fVal2 * a_rQuaternion.w;
	m_f13 = fVal1 * a_rQuaternion.x - fVal2 * a_rQuaternion.w;
	m_f22 = 1.0f - ( fVal5 + fVal4 );
	m_f32 = fVal1 * a_rQuaternion.y - fVal3;
	m_f23 = fVal1 * a_rQuaternion.y + fVal3;
	m_f33 = 1.0f - ( fVal2 * a_rQuaternion.y + fVal4 );

	return *this;
}

// $Barnyard: FUNCTION 006c8ea0
TMatrix44& TMatrix44::PushQuaternion( const TQuaternion& a_rQuaternion, const TMatrix44& a_rMatrix, const TVector3& a_rOrigin )
{
	TMatrix44 matrix;

	matrix.SetFromQuaternion( a_rQuaternion );
	matrix.AsBasisVector3( 3 ) = a_rOrigin;
	Multiply( a_rMatrix, matrix );

	return *this;
}

// $Barnyard: FUNCTION 006c8840
void TMatrix44::RotateX( TFLOAT angle )
{
	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fCos;
	TFLOAT fSin;

	TMath::SinCos( angle, fSin, fCos );
	fVar1 = m_f31;
	m_f31 = fVar1 * fCos - m_f21 * fSin;
	fVar2 = m_f32;
	m_f21 = fVar1 * fSin + m_f21 * fCos;
	fVar1 = m_f23;
	m_f32 = fVar2 * fCos - m_f22 * fSin;
	m_f22 = fVar2 * fSin + m_f22 * fCos;
	m_f23 = m_f33 * fSin + fVar1 * fCos;
	m_f33 = m_f33 * fCos - fVar1 * fSin;
}

// $Barnyard: FUNCTION 006c88e0
void TMatrix44::RotateY( TFLOAT angle )
{
	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fSin;
	TFLOAT fCos;

	TMath::SinCos( angle, fSin, fCos );
	fVar1 = m_f31;
	m_f31 = fVar1 * fCos + m_f11 * fSin;
	fVar2 = m_f32;
	m_f11 = m_f11 * fCos - fVar1 * fSin;
	fVar1 = m_f13;
	m_f32 = fVar2 * fCos + m_f12 * fSin;
	m_f12 = m_f12 * fCos - fVar2 * fSin;
	m_f13 = fVar1 * fCos - m_f33 * fSin;
	m_f33 = m_f33 * fCos + fVar1 * fSin;
}

// $Barnyard: FUNCTION 006c87b0
void TMatrix44::RotateZ( TFLOAT angle )
{
	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fCos;
	TFLOAT fSin;

	TMath::SinCos( angle, fSin, fCos );
	fVar1 = m_f21;
	m_f21 = fVar1 * fCos - m_f11 * fSin;
	fVar2 = m_f22;
	m_f11 = fVar1 * fSin + m_f11 * fCos;
	fVar1 = m_f13;
	m_f22 = fVar2 * fCos - m_f12 * fSin;
	m_f12 = fVar2 * fSin + m_f12 * fCos;
	m_f13 = m_f23 * fSin + fVar1 * fCos;
	m_f23 = m_f23 * fCos - fVar1 * fSin;
}

// $Barnyard: FUNCTION 005e7bb0
void TMatrix44::GetEulerXYZ( TVector3& a_rOutVec ) const
{
	TFLOAT fVal1 = m_f31;
	TMath::Clip( fVal1, -1.0f, 1.0f );

	TFLOAT fVal2 = TMath::Sqrt( 1.0f - fVal1 * fVal1 );

	if ( m_f11 < 0 && m_f33 < 0 )
		fVal2 *= -1;

	if ( 0.001f < fVal2 || fVal2 < -0.001 )
	{
		TFLOAT fVal3 = 1.0f / fVal2;

		a_rOutVec.x = TMath::ATan2( m_f32 * fVal3, m_f33 * fVal3 );
		a_rOutVec.y = TMath::ATan2( fVal1, fVal2 );
		a_rOutVec.z = TMath::ATan2( m_f21 * fVal3, m_f11 * fVal3 );
	}
	else
	{
		a_rOutVec.x = TMath::ATan2( -m_f32, m_f22 );
		a_rOutVec.y = -TMath::ASin( fVal1 );
		a_rOutVec.z = 0.0f;
	}
}

TOSHI_NAMESPACE_END
