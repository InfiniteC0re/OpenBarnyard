#pragma once
#include "Math/TVector3.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

class TVector4
{
public:
	constexpr TVector4() = default;
	constexpr TVector4( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW = 1.0f )
	    : x( a_fX ), y( a_fY ), z( a_fZ ), w( a_fW )
	{}

	constexpr TVector4( TFLOAT a_pFloats[ 4 ] )
	    : x( a_pFloats[ 0 ] ), y( a_pFloats[ 1 ] ), z( a_pFloats[ 2 ] ), w( a_pFloats[ 3 ] )
	{}

	constexpr TVector4( const TVector3& a_rcOther )
	    : x( a_rcOther.x ), y( a_rcOther.y ), z( a_rcOther.z ), w( 1.0f )
	{}

	constexpr TVector4( const TVector4& a_rcOther )
	    : x( a_rcOther.x ), y( a_rcOther.y ), z( a_rcOther.z ), w( a_rcOther.w )
	{}

	constexpr void Set( const TVector3& vec )
	{
		TVector4::x = vec.x;
		TVector4::y = vec.y;
		TVector4::z = vec.z;
		TVector4::w = 1.0f;
	}

	// $Barnyard: FUNCTION 0046b360
	constexpr void Set( const TVector4& vec )
	{
		TVector4::x = vec.x;
		TVector4::y = vec.y;
		TVector4::z = vec.z;
		TVector4::w = vec.w;
	}

	constexpr void Set( TFLOAT floats[ 4 ] )
	{
		TVector4::x = floats[ 0 ];
		TVector4::y = floats[ 1 ];
		TVector4::z = floats[ 2 ];
		TVector4::w = floats[ 3 ];
	}

	constexpr void Set( TFLOAT x, TFLOAT y, TFLOAT z, TFLOAT w )
	{
		TVector4::x = x;
		TVector4::y = y;
		TVector4::z = z;
		TVector4::w = w;
	}

	TFLOAT          NormaliseMag();
	void            Normalise();
	void            Normalise4();
	constexpr TBOOL isNormalised( TFLOAT fVal = 0.05f ) const { return ( ( ( 1.0f - fVal ) * ( 1.0f - fVal ) ) < MagnitudeSq() ) && ( ( ( 1.0f + fVal ) * ( 1.0f + fVal ) ) >= MagnitudeSq() ); }

	void ProjectNormalToPlane( const TVector4& vec, const TVector4& vec2 );

	constexpr void Add( const TVector3& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	constexpr void Add( const TVector4& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	// $Barnyard: FUNCTION 0043cce0
	constexpr void Add( const TVector4& a, const TVector4& b )
	{
		Set( a + b );
		w = a.w;
	}

	constexpr void Substract( const TVector4& vec )
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
	}

	// $Barnyard: FUNCTION 0041cd70
	constexpr void Substract( const TVector4& a, const TVector4& b )
	{
		Set( a - b );
		w = a.w;
	}

	constexpr void Divide( const TVector4& vec )
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
	}

	constexpr void Divide( TFLOAT scalar )
	{
		TFLOAT ratio = 1.0f / scalar;
		x *= ratio;
		y *= ratio;
		z *= ratio;
	}

	// $Barnyard: FUNCTION 006c9250
	constexpr void Divide4( TFLOAT scalar )
	{
		if ( scalar != 0.0f )
		{
			TFLOAT ratio = 1.0f / scalar;
			x *= ratio;
			y *= ratio;
			z *= ratio;
			w *= ratio;
		}
	}

	constexpr void Divide( const TVector4& vec1, const TVector4& vec2 )
	{
		Set( vec1 / vec2 );
	}

	constexpr void Divide( const TVector4& vec, TFLOAT scalar )
	{
		Set( vec );
		Divide( scalar );
	}

	// $Barnyard: FUNCTION 006c7430
	constexpr void Multiply( const TVector4& vec )
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
	}

	// $Barnyard: FUNCTION 006c7450
	constexpr void Multiply( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}

	// $Barnyard: FUNCTION 006c7380
	constexpr void Multiply( const TVector4& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply( scalar );
	}

	constexpr void Multiply4( const TVector4& vec )
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		w *= vec.w;
	}

	constexpr void Multiply4( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
	}

	constexpr void Multiply4( const TVector4& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply4( scalar );
	}

	void Cos( const TVector4& vec )
	{
		x = TMath::Cos( vec.x );
		y = TMath::Cos( vec.y );
		z = TMath::Cos( vec.z );
		w = TMath::Cos( vec.w );
	}

	// $Barnyard: FUNCTION 006c7740
	void CrossProduct( const TVector4& vec1, const TVector4& vec2 )
	{
		x = vec2.z * vec1.y - vec2.y * vec1.z;
		y = vec1.z * vec2.x - vec2.z * vec1.x;
		z = vec1.x * vec2.y - vec1.y * vec2.x;
		w = vec1.w;
	}

	constexpr void Lerp3( const TVector4& finish, TFLOAT t ) { Lerp3( *this, finish, t ); }

	// $Barnyard: FUNCTION 006c77e0
	constexpr void Lerp3( const TVector4& start, const TVector4& finish, TFLOAT t )
	{
		TVector4::x = ( finish.x - start.x ) * t + start.x;
		TVector4::y = ( finish.y - start.y ) * t + start.y;
		TVector4::z = ( finish.z - start.z ) * t + start.z;
		TVector4::w = start.w;
	}

	constexpr void Lerp4( const TVector4& finish, TFLOAT t ) { Lerp4( *this, finish, t ); }

	// $Barnyard: FUNCTION 006c7790
	constexpr void Lerp4( const TVector4& start, const TVector4& finish, TFLOAT t )
	{
		TVector4::x = ( finish.x - start.x ) * t + start.x;
		TVector4::y = ( finish.y - start.y ) * t + start.y;
		TVector4::z = ( finish.z - start.z ) * t + start.z;
		TVector4::w = ( finish.w - start.w ) * t + start.w;
	}

	constexpr double GetScalarProjectionOnUnit( const TVector4& vec )
	{
		return (double)( TVector4::z * vec.z + TVector4::x * vec.x + TVector4::y * vec.y );
	}

	void Abs( const TVector4& vec4 ) { Set( TMath::Abs( vec4.x ), TMath::Abs( vec4.y ), TMath::Abs( vec4.z ), w ); }
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ), w ); }

	constexpr void Negate( const TVector4& vec ) { Negate3( vec ); }
	constexpr void Negate() { Negate3(); }

	// $Barnyard: FUNCTION 005f1740
	constexpr void Negate3( const TVector4& vec ) { Set( -vec.x, -vec.y, -vec.z, vec.w ); }

	constexpr void Negate3()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	constexpr void Negate4( const TVector4& vec ) { Set( -vec.x, -vec.y, -vec.z, -vec.w ); }
	constexpr void Negate4() { Set( -x, -y, -z, -w ); }

	TFLOAT           Magnitude() const { return TMath::Sqrt( x * x + y * y + z * z ); }
	TFLOAT           Magnitude4() const { return TMath::Sqrt( w * w + x * x + y * y + z * z ); }
	constexpr TFLOAT MagnitudeSq() const { return x * x + y * y + z * z; }
	constexpr TFLOAT MagnitudeSq4() const { return w * w + x * x + y * y + z * z; }
	TFLOAT           MagnitudeXZ() const { return TMath::Sqrt( x * x + z * z ); }
	constexpr TFLOAT MagnitudeSqXZ() const { return x * x + z * z; }

	constexpr TVector4& Progress( const TVector4& a_rVec, TFLOAT a_fScalar )
	{
		x = a_rVec.x * a_fScalar;
		y = a_rVec.y * a_fScalar;
		z = a_rVec.z * a_fScalar;
		w = a_rVec.w * a_fScalar;

		return *this;
	}

	// $Barnyard: FUNCTION 006c92f0
	constexpr TBOOL IsEqual( const TVector4& a_rcVec ) const
	{
		return a_rcVec.x == x && a_rcVec.y == y && a_rcVec.z == z;
	}

	constexpr TVector4 operator+( const TVector3& other ) const { return { x + other.x, y + other.y, z + other.z, w }; }
	constexpr TVector4 operator-( const TVector3& other ) const { return { x - other.x, y - other.y, z - other.z, w }; }
	constexpr TVector4 operator*( const TVector3& other ) const { return { x * other.x, y * other.y, z * other.z, w }; }
	constexpr TVector4 operator/( const TVector3& other ) const { return { x / other.x, y / other.y, z / other.z, w }; }

	constexpr TVector4 operator+( const TVector4& other ) const { return { x + other.x, y + other.y, z + other.z, other.w }; }
	constexpr TVector4 operator-( const TVector4& other ) const { return { x - other.x, y - other.y, z - other.z, other.w }; }

	// $Barnyard: FUNCTION 004159e0
	constexpr TVector4 operator*( const TVector4& other ) const { return { x * other.x, y * other.y, z * other.z, other.w }; }
	constexpr TVector4 operator/( const TVector4& other ) const { return { x / other.x, y / other.y, z / other.z, other.w }; }

	void operator=( const TVector3& other ) { Set( other ); }
	void operator=( const TVector4& other ) { Set( other ); }
	void operator+=( const TVector4& other ) { Add( other ); }

	TVector4& operator-=( const TVector4& other )
	{
		Substract( other );
		return *this;
	}

	void operator/=( const TVector4& other ) { Divide( other ); }
	void operator*=( const TVector4& other ) { Multiply( other ); }

	void operator/=( TFLOAT a_fScalar ) { Divide( a_fScalar ); }
	void operator*=( TFLOAT a_fScalar ) { Multiply( a_fScalar ); }

	TFLOAT*       AsArray() { return TREINTERPRETCAST( TFLOAT*, this ); }
	const TFLOAT* AsArray() const { return TREINTERPRETCAST( const TFLOAT*, this ); }

	TVector3&       AsVector3() { return TREINTERPRETCAST( TVector3&, *this ); }
	const TVector3& AsVector3() const { return TREINTERPRETCAST( const TVector3&, *this ); }

	TVector2&       AsVector2() { return TREINTERPRETCAST( TVector2&, *this ); }
	const TVector2& AsVector2() const { return TREINTERPRETCAST( const TVector2&, *this ); }

public:
	static TFLOAT Distance( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).Magnitude(); }

	// $Barnyard: FUNCTION 006c76c0
	static constexpr TFLOAT DistanceSq( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }
	// $Barnyard: FUNCTION 006cbaf0
	static TFLOAT           DistanceXZ( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).MagnitudeXZ(); }
	static constexpr TFLOAT DistanceSqXZ( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).MagnitudeSqXZ(); }

	TFLOAT constexpr DotProduct( const TVector4& vec ) const { return DotProduct3( *this, vec ); }
	static constexpr TFLOAT DotProduct( const TVector4& vec1, const TVector4& vec2 ) { return DotProduct3( vec1, vec2 ); }

	TFLOAT constexpr DotProduct3( const TVector4& vec ) const { return DotProduct3( *this, vec ); }

	// $Barnyard: FUNCTION 006c7670
	static constexpr TFLOAT DotProduct3( const TVector4& vec1, const TVector4& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z; }

	TFLOAT constexpr DotProduct4( const TVector4& vec ) const { return DotProduct4( *this, vec ); }

	// $Barnyard: FUNCTION 006c7690
	static constexpr TFLOAT DotProduct4( const TVector4& vec1, const TVector4& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w; }

	// $Barnyard: FUNCTION 006085d0
	static void Swap( TVector4& a_rVec1, TVector4& a_rVec2 )
	{
		TVector4 temp = a_rVec1;
		a_rVec1       = a_rVec2;
		a_rVec2       = temp;
	}

public:
	static const constinit TVector4 VEC_ZERO;
	static const constinit TVector4 VEC_POSX;
	static const constinit TVector4 VEC_POSY;
	static const constinit TVector4 VEC_POSZ;
	static const constinit TVector4 VEC_NEGX;
	static const constinit TVector4 VEC_NEGY;
	static const constinit TVector4 VEC_NEGZ;

public:
	TFLOAT x, y, z, w;
};

TOSHI_NAMESPACE_END
