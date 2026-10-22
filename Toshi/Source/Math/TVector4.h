#pragma once
#include "Math/TVector3.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

class MODAPI TVector4
{
public:
	constexpr TVector4() = default;
	constexpr TVector4( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW = 1.0f )
	    : x( a_fX ), y( a_fY ), z( a_fZ ), w( a_fW )
	{}

	constexpr TVector4( TFLOAT a_pFloats[ 4 ] )
	    : x( a_pFloats[ 0 ] ), y( a_pFloats[ 1 ] ), z( a_pFloats[ 2 ] ), w( a_pFloats[ 3 ] )
	{}

	// $Barnyard: FUNCTION 0041cd50
	constexpr TVector4( const TVector3& a_rcOther )
	    : x( a_rcOther.x ), y( a_rcOther.y ), z( a_rcOther.z ), w( 1.0f )
	{}

	// $Barnyard: FUNCTION 006c7330
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

	// Rotates the vector around a_rcAxis (normalised) by a_fAngle radians (Rodrigues), w is kept
	void RotateAroundAxis( const TVector4& a_rcAxis, TFLOAT a_fAngle );

	// Writes a_rcVec rotated around a_rcAxis (normalised) by a_fAngle radians (Rodrigues) to a_rOut, w is copied
	static void RotateAroundAxis( const TVector4& a_rcVec, const TVector4& a_rcAxis, TFLOAT a_fAngle, TVector4& a_rOut );

	// Returns the angle around the Y axis from +Z to the vector (atan2( x, z ))
	TFLOAT GetYaw() const;

	constexpr void Add( const TVector3& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	// $Barnyard: FUNCTION 0041cd00
	constexpr void Add( const TVector4& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	constexpr void Add4( const TVector4& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		w += vec.w;
	}

	// $Barnyard: FUNCTION 0043ccb0
	constexpr void Add( const TVector4& a, const TVector4& b )
	{
		Set( a + b );
		w = a.w;
	}

	constexpr void Add4( const TVector4& a, const TVector4& b )
	{
		Set( a );
		Add4( b );
	}

	constexpr void Substract4( const TVector4& vec )
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		w -= vec.w;
	}

	constexpr void Substract( const TVector4& vec )
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
	}

	// $Barnyard: FUNCTION 0041cd20
	constexpr void Substract( const TVector4& a, const TVector4& b )
	{
		x = a.x - b.x;
		y = a.y - b.y;
		z = a.z - b.z;
		w = a.w;
	}

	constexpr void Substract4( const TVector4& a, const TVector4& b )
	{
		Set( a );
		Substract4( b );
	}

	constexpr void Divide( const TVector4& vec )
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
	}

	// $Barnyard: FUNCTION 006c74a0
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

	// $Barnyard: FUNCTION 006c73e0
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

	// $Barnyard: FUNCTION 006c7350
	constexpr void Multiply( const TVector4& vec1, const TVector4& vec2 )
	{
		x = vec1.x * vec2.x;
		y = vec1.y * vec2.y;
		z = vec1.z * vec2.z;
		w = vec1.w;
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

	// $Barnyard: FUNCTION 006c7470
	constexpr void Multiply4( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
	}

	// $Barnyard: FUNCTION 006c73b0
	constexpr void Multiply4( const TVector4& vec, TFLOAT scalar )
	{
		x = scalar * vec.x;
		y = scalar * vec.y;
		z = scalar * vec.z;
		w = scalar * vec.w;
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
		// NOTE: computed before storing like the original, so this can be vec1 or vec2 (ARigidBody::AddAngularResponse)
		TFLOAT fX = vec2.z * vec1.y - vec2.y * vec1.z;
		TFLOAT fY = vec1.z * vec2.x - vec2.z * vec1.x;
		TFLOAT fZ = vec1.x * vec2.y - vec1.y * vec2.x;
		w         = vec1.w;
		x         = fX;
		y         = fY;
		z         = fZ;
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

	// Replaces the vector with its projection onto a_rcVec (w is taken from a_rcVec)
	// $Barnyard: FUNCTION 006c7d70
	constexpr void ProjectOnto( const TVector4& a_rcVec )
	{
		TFLOAT fScale = ( z * a_rcVec.z + y * a_rcVec.y + a_rcVec.x * x ) / ( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z );
		Set( fScale * a_rcVec.x, fScale * a_rcVec.y, fScale * a_rcVec.z, a_rcVec.w );
	}

	// Clears y and normalises x and z ((0, 0, 1) if both are 0). Returns the length on the XZ plane
	// $Barnyard: FUNCTION 006c7540
	TFLOAT NormaliseXZ()
	{
		y             = 0.0f;
		TFLOAT fMagSq = x * x + z * z;

		if ( fMagSq == 0.0f )
		{
			x = 0.0f;
			z = 1.0f;
			return fMagSq;
		}

		TFLOAT fMag = TMath::Sqrt( fMagSq );
		x           = ( 1.0f / fMag ) * x;
		z           = ( 1.0f / fMag ) * z;
		return fMag;
	}

	void Abs( const TVector4& vec4 ) { Set( TMath::Abs( vec4.x ), TMath::Abs( vec4.y ), TMath::Abs( vec4.z ), w ); }
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ), w ); }

	// $Barnyard: FUNCTION 006c7410
	void Abs4() { Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ), TMath::Abs( w ) ); }

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

	// Returns angle between this and a_rcVec in radians (0..PI)
	TFLOAT GetAngleBetweenVector( const TVector4& a_rcVec ) const;

	TFLOAT Magnitude() const { return TMath::Sqrt( x * x + y * y + z * z ); }
	TFLOAT Magnitude4() const { return TMath::Sqrt( w * w + x * x + y * y + z * z ); }
	// $Barnyard: FUNCTION 006a7a60
	constexpr TFLOAT MagnitudeSq() const { return x * x + y * y + z * z; }
	constexpr TFLOAT MagnitudeSq4() const { return w * w + x * x + y * y + z * z; }
	// $Barnyard: FUNCTION 006c7520
	TFLOAT MagnitudeXZ() const { return TMath::Sqrt( x * x + z * z ); }
	// $Barnyard: FUNCTION 006c74d0
	constexpr TFLOAT MagnitudeSqXZ() const { return x * x + z * z; }

	// $Barnyard: FUNCTION 006c92f0
	// NOTE: compares all four components, w included
	constexpr TBOOL IsEqual( const TVector4& a_rcVec ) const
	{
		return a_rcVec.x == x && a_rcVec.y == y && a_rcVec.z == z && a_rcVec.w == w;
	}

	// Compares xyz only (used by T2Vector<TVector4>::Find)
	constexpr TBOOL operator==( const TVector4& a_rcVec ) const { return a_rcVec.x == x && a_rcVec.y == y && a_rcVec.z == z; }

	// Returns TTRUE if all four components are within [-a_fTolerance, a_fTolerance].
	// $Barnyard: FUNCTION 006c78a0
	constexpr TBOOL IsZero4( TFLOAT a_fTolerance ) const
	{
		TFLOAT fMin = -a_fTolerance;
		return x >= fMin && x <= a_fTolerance && y >= fMin && y <= a_fTolerance && z >= fMin && z <= a_fTolerance && w >= fMin && w <= a_fTolerance;
	}

	constexpr TVector4 operator+( const TVector3& other ) const { return { x + other.x, y + other.y, z + other.z, w }; }
	constexpr TVector4 operator-( const TVector3& other ) const { return { x - other.x, y - other.y, z - other.z, w }; }
	constexpr TVector4 operator*( const TVector3& other ) const { return { x * other.x, y * other.y, z * other.z, w }; }
	constexpr TVector4 operator/( const TVector3& other ) const { return { x / other.x, y / other.y, z / other.z, w }; }

	// NOTE: the binary operators work on xyz and keep the w of this (like Add/Substract/Multiply( a, b ) keep a.w)
	// $Barnyard: FUNCTION 0043cce0
	constexpr TVector4 operator+( const TVector4& other ) const { return { x + other.x, y + other.y, z + other.z, w }; }
	// $Barnyard: FUNCTION 0041cd70
	constexpr TVector4 operator-( const TVector4& other ) const { return { x - other.x, y - other.y, z - other.z, w }; }
	constexpr TVector4 operator*( const TVector4& other ) const { return { x * other.x, y * other.y, z * other.z, w }; }
	constexpr TVector4 operator/( const TVector4& other ) const { return { x / other.x, y / other.y, z / other.z, w }; }

	// $Barnyard: FUNCTION 004159e0
	constexpr TVector4 operator*( TFLOAT a_fScalar ) const
	{
		TVector4 vResult;
		vResult.Multiply( *this, a_fScalar );
		return vResult;
	}

	// $Barnyard: FUNCTION 00464ab0
	constexpr TVector4 operator/( TFLOAT a_fScalar ) const
	{
		TVector4 vResult;
		vResult.Divide( *this, a_fScalar );
		return vResult;
	}

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
	// $Barnyard: FUNCTION 006c7700
	static TFLOAT Distance( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).Magnitude(); }

	// $Barnyard: FUNCTION 006c76c0
	static constexpr TFLOAT DistanceSq( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }

	// $Barnyard: FUNCTION 006cbaf0
	static TFLOAT DistanceXZ( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).MagnitudeXZ(); }
	// $Barnyard: FUNCTION 00414e00
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
