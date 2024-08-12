#include "ToshiPCH.h"
#include "TMatrix44.h"

namespace Toshi
{
TMatrix44 TMatrix44::IDENTITY(
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
    1.0f );

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

void TMatrix44::Multiply( const TMatrix44& a_rLeft, const TMatrix44& a_rRight )
{
    /*float fVar2;
		bool bVar3;
		TINT iVar4;
		TINT iVar5;

		TMatrix44* pThis = this;

		iVar4 = 0;
		do {
			iVar5 = 0;
			while (true) {
				fVar2 = (&a_rLeft.m_f11)[iVar5] * *(float*)((TINT)&a_rRight.m_f11 + iVar4);
				if (3 < iVar5 + 1) break;
				(&pm_f11)[iVar5] = fVar2;
				iVar5 = iVar5 + 1;
			}
			(&pm_f11)[iVar5] = fVar2;
			iVar4 = iVar4 + 0x10;
			pThis = (TMatrix44*)&pm_f21;
		} while (iVar4 < 0x40);*/

    TASSERT( ( this != &a_rLeft ) && ( this != &a_rRight ) );

    for ( TINT i = 0; i < 4; i++ )
    {
        float* pBasis = AsBasisVector4( i ).AsArray();

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

TBOOL TMatrix44::Invert( const TMatrix44& a_rRight )
{
    TASSERT( a_rRight.m_f14 == 0.0f );
    TASSERT( a_rRight.m_f24 == 0.0f );
    TASSERT( a_rRight.m_f34 == 0.0f );
    TASSERT( a_rRight.m_f44 == 1.0f );

    float fVal1 = a_rRight.m_f22 * a_rRight.m_f33 - a_rRight.m_f23 * a_rRight.m_f32;
    float fVal2 = -( a_rRight.m_f12 * a_rRight.m_f33 - a_rRight.m_f13 * a_rRight.m_f32 );
    float fVal3 = a_rRight.m_f12 * a_rRight.m_f23 - a_rRight.m_f13 * a_rRight.m_f22;
    float fDet  = a_rRight.m_f11 * fVal1 + a_rRight.m_f21 * fVal2 + a_rRight.m_f31 * fVal3;

    if ( fDet == 0.0 )
    {
        // Matrix is not invertible
        return TFALSE;
    }

    float fInvDet = 1.0f / fDet;

    m_f11 = fInvDet * fVal1;
    m_f12 = fInvDet * fVal2;
    m_f13 = fInvDet * fVal3;
    m_f14 = a_rRight.m_f14;

    m_f21 = -( ( a_rRight.m_f21 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f23 ) * fInvDet );
    m_f22 = ( a_rRight.m_f11 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f13 ) * fInvDet;
    m_f23 = -( ( a_rRight.m_f11 * a_rRight.m_f23 - a_rRight.m_f21 * a_rRight.m_f13 ) * fInvDet );
    m_f24 = a_rRight.m_f24;

    m_f31 = ( a_rRight.m_f21 * a_rRight.m_f32 - a_rRight.m_f31 * a_rRight.m_f22 ) * fInvDet;
    m_f32 = -( ( a_rRight.m_f11 * a_rRight.m_f32 - a_rRight.m_f31 * a_rRight.m_f12 ) * fInvDet );
    m_f33 = ( a_rRight.m_f11 * a_rRight.m_f22 - a_rRight.m_f21 * a_rRight.m_f12 ) * fInvDet;
    m_f34 = a_rRight.m_f34;

    m_f41 = -a_rRight.m_f41 * m_f11 - a_rRight.m_f42 * m_f21 - a_rRight.m_f43 * m_f31;
    m_f42 = -a_rRight.m_f41 * m_f12 - a_rRight.m_f42 * m_f22 - a_rRight.m_f43 * m_f32;
    m_f43 = -a_rRight.m_f41 * m_f13 - a_rRight.m_f42 * m_f23 - a_rRight.m_f43 * m_f33;
    m_f44 = a_rRight.m_f44;

    return TTRUE;
}

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

TMatrix44& TMatrix44::SetFromQuaternion( const TQuaternion& a_rQuaternion )
{
    float fVal1 = a_rQuaternion.z * 2.0f;
    float fVal2 = a_rQuaternion.y * 2.0f;
    float fVal3 = a_rQuaternion.x * 2.0f * a_rQuaternion.w;
    float fVal4 = a_rQuaternion.x * 2.0f * a_rQuaternion.x;
    float fVal5 = fVal1 * a_rQuaternion.z;

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

TMatrix44& TMatrix44::PushQuaternion( const TQuaternion& a_rQuaternion, const TMatrix44& a_rMatrix, const TVector3& a_rOrigin )
{
    TMatrix44 matrix;

    matrix.SetFromQuaternion( a_rQuaternion );
    matrix.AsBasisVector3( 3 ) = a_rOrigin;
    Multiply( a_rMatrix, matrix );

    return *this;
}

void TMatrix44::RotateX( float angle )
{
    float fVar1;
    float fVar2;
    float fCos;
    float fSin;

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

void TMatrix44::RotateY( float angle )
{
    float fVar1;
    float fVar2;
    float fSin;
    float fCos;

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

void TMatrix44::RotateZ( float angle )
{
    float fVar1;
    float fVar2;
    float fCos;
    float fSin;

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

} // namespace Toshi