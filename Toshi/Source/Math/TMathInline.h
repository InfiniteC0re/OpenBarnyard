#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/TDebug.h"

namespace Toshi
{

namespace TMath
{

constexpr TINT8  TINT8_MAX  = INT8_MAX;
constexpr TINT8  TINT8_MIN  = INT8_MIN;
constexpr TUINT8 TUINT8_MAX = UINT8_MAX;

constexpr TUINT TINT16_MAX  = INT16_MAX;
constexpr TUINT TINT16_MIN  = INT16_MIN;
constexpr TUINT TUINT16_MAX = UINT16_MAX;

constexpr TINT  TINT32_MAX  = INT32_MAX;
constexpr TINT  TINT32_MIN  = INT32_MIN;
constexpr TUINT TUINT32_MAX = UINT32_MAX;

constexpr TUINT16 MAXWCHAR = WCHAR_MAX;

constexpr TUINTPTR MAXPTR = UINTPTR_MAX;

constexpr TFLOAT MAXFLOAT       = FLT_MAX;
constexpr TFLOAT MINFLOAT       = FLT_MIN;
constexpr TFLOAT TFLOAT_EPSILON = FLT_EPSILON; // Renamed according to de Blob
constexpr TFLOAT FLOATEPSILON   = FLT_EPSILON; // Both exist for some reason

constexpr TFLOAT PI               = 3.1415927f;
constexpr TFLOAT TWO_PI           = PI * 2;
constexpr TFLOAT HALF_PI          = PI / 2;
constexpr TFLOAT ONEOVER_SQRT_TWO = 0.70710677f;
constexpr TFLOAT ONEOVERTWO_PI    = 1 / TWO_PI;

TFORCEINLINE TBOOL  IsFinite( TFLOAT fVal ) { return _finite( fVal ) != 0; }
TFORCEINLINE TFLOAT Sin( TFLOAT fVal ) { return sinf( fVal ); }
TFORCEINLINE TFLOAT Cos( TFLOAT fVal ) { return cosf( fVal ); }
TFORCEINLINE TFLOAT Tan( TFLOAT fVal ) { return tanf( fVal ); }
TFORCEINLINE TFLOAT ASin( TFLOAT fVal ) { return sinf( fVal ); }
TFORCEINLINE TFLOAT ACos( TFLOAT fVal ) { return acos( fVal ); }
TFORCEINLINE TFLOAT ATan( TFLOAT fVal )
{
    TFLOAT fRes = atanf( fVal );
    TASSERT( TMath::IsFinite( fRes ) );
    return fRes;
}
TFORCEINLINE TFLOAT ATan2( TFLOAT fVal1, TFLOAT fVal2 )
{
    TFLOAT fRes = atan2f( fVal1, fVal2 );
    TASSERT( TMath::IsFinite( fRes ) );
    return fRes;
}
TFORCEINLINE TFLOAT           Abs( TFLOAT fVal ) { return (TFLOAT)fabs( fVal ); }
TFORCEINLINE constexpr TFLOAT DegToRad( TFLOAT fDeg ) { return ( fDeg / 180.0f ) * PI; }

TFORCEINLINE constexpr TUINT IntLog2( TUINT32 a_uiValue )
{
    return std::bit_width( a_uiValue ) - 1;
}

inline constexpr TUINT RoundToNextPowerOfTwo( TUINT a_uiValue )
{
    a_uiValue--;
    a_uiValue |= a_uiValue >> 1;
    a_uiValue |= a_uiValue >> 2;
    a_uiValue |= a_uiValue >> 4;
    a_uiValue |= a_uiValue >> 8;
    a_uiValue |= a_uiValue >> 16;
    a_uiValue++;

    return a_uiValue;
}

TFORCEINLINE TFLOAT Sqrt( TFLOAT a_fX )
{
    TASSERT( a_fX != 0.0f );
    TFLOAT fVal = sqrtf( a_fX );
    TASSERT( IsFinite( fVal ) );
    return fVal;
}

TFORCEINLINE TFLOAT OneOverSqrt( TFLOAT a_fX )
{
    return 1.0f / Sqrt( a_fX );
}

TFORCEINLINE TINT  Round( TFLOAT a_fVal ) { return (TINT)roundf( a_fVal ); }
TFORCEINLINE TINT  CeilToInt( TFLOAT a_fVal ) { return TINT( a_fVal ) - TUINT32( 0 < TUINT32( a_fVal - TUINT32( a_fVal ) ) ); }
TFORCEINLINE TINT  FloorToInt( TFLOAT a_fVal ) { return TINT( a_fVal ) - TUINT32( 0x80000000 < TUINT32( a_fVal - TUINT32( a_fVal ) ) ); }
TFORCEINLINE TBOOL IsNaN( TFLOAT fVal ) { return isnan( fVal ); }
TFORCEINLINE TINT  FastMod( TINT a_iNum, TINT a_iModulus )
{
    TASSERT( a_iNum >= 0 );
    TASSERT( a_iModulus > 0 );
    TASSERT( 0 == ( a_iModulus & ( a_iModulus - 1 ) ) );
    return a_iNum & ( a_iModulus - 1 );
}
TFORCEINLINE void SinCos( TFLOAT fVal, TFLOAT& a_rSin, TFLOAT& a_rCos )
{
    a_rSin = sin( fVal );
    a_rCos = cos( fVal );
}
TFORCEINLINE TFLOAT LERP( TFLOAT a, TFLOAT b, TFLOAT t ) { return a + t * ( b - a ); }

template < typename T >
TFORCEINLINE void Clip( T& rVal, const T& Min, const T& Max )
{
    if ( Max < rVal ) { rVal = Max; }
    if ( rVal < Min ) { rVal = Min; }
}

template < typename T >
TFORCEINLINE const T& Min( const T& a, const T& b )
{
    if ( a < b ) { return a; }
    else { return b; }
}

template < typename T >
TFORCEINLINE const T& Max( const T& a, const T& b )
{
    if ( a < b ) { return b; }
    else { return a; }
}

} // namespace TMath

} // namespace Toshi
