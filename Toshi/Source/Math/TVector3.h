/**
 * @file TVector3.h
 * 3D vector implementation for the Toshi engine
 * 
 * This file provides 3D vector functionality including:
 * - Vector operations (addition, subtraction, multiplication)
 * - Vector normalization and length calculations
 * - Vector transformations
 * - Vector utility functions
 */

#pragma once

TOSHI_NAMESPACE_START

class TVector3
{
public:
	constexpr TVector3() = default;
	constexpr TVector3( TFLOAT x, TFLOAT y, TFLOAT z ) { Set( x, y, z ); }
	constexpr TVector3( TFLOAT floats[ 3 ] ) { Set( floats ); }
	constexpr TVector3( const TVector3& other ) { Set( other.x, other.y, other.z ); }

	void Clip( TFLOAT fVal, TFLOAT fVal2 );
	void CrossProduct( const TVector3&, const TVector3& );

	void RotateX( TFLOAT a_fRotation );
	void RotateY( TFLOAT a_fRotation );
	void RotateZ( TFLOAT a_fRotation );

	void Normalize();

	constexpr void Set( const TVector3& vec )
	{
		TVector3::x = vec.x;
		TVector3::y = vec.y;
		TVector3::z = vec.z;
	}

	constexpr void Set( TFLOAT floats[ 3 ] )
	{
		TVector3::x = floats[ 0 ];
		TVector3::y = floats[ 1 ];
		TVector3::z = floats[ 2 ];
	}

	constexpr void Set( TFLOAT x, TFLOAT y, TFLOAT z )
	{
		TVector3::x = x;
		TVector3::y = y;
		TVector3::z = z;
	}

	// $Barnyard: FUNCTION 006cbba0
	constexpr void Lerp( const TVector3& finish, float t ) { Lerp( *this, finish, t ); }

	// $Barnyard: FUNCTION 006cbb60
	constexpr void Lerp( const TVector3& start, const TVector3& finish, float t )
	{
		TVector3 progress = finish - start;
		progress.Multiply( t );

		Set( start + progress );
	}

	// $Barnyard: FUNCTION 0040f390
	constexpr void Add( const TVector3& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	constexpr void Add( const TVector3& a, const TVector3& b )
	{
		Set( a + b );
	}

	constexpr void Divide( const TVector3& vec )
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

	constexpr void Divide( const TVector3& vec1, const TVector3& vec2 )
	{
		Set( vec1 / vec2 );
	}

	constexpr void Divide( const TVector3& vec, TFLOAT scalar )
	{
		Set( vec );
		Divide( scalar );
	}

	constexpr void Multiply( const TVector3& vec )
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
	}

	constexpr void Multiply( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}

	constexpr void Multiply( const TVector3& vec1, const TVector3& vec2 )
	{
		Set( vec1 * vec2 );
	}

	constexpr void Multiply( const TVector3& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply( scalar );
	}

	// $Barnyard: FUNCTION 006cbb20
	constexpr TBOOL Equals( const TVector3& vec )
	{
		return vec.x == x && vec.y == y && vec.z == z;
	}

	void Abs( const TVector3& vec3 ) { Set( TMath::Abs( vec3.x ), TMath::Abs( vec3.y ), TMath::Abs( vec3.z ) ); }
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ) ); }

	// $Barnyard: FUNCTION 006c74f0
	TFLOAT           Magnitude() const { return TMath::Sqrt( x * x + y * y + z * z ); }
	constexpr TFLOAT MagnitudeSq() const { return x * x + y * y + z * z; }

	constexpr TVector3 operator+( const TVector3& other ) const { return { x + other.x, y + other.y, z + other.z }; }
	constexpr TVector3 operator-( const TVector3& other ) const { return { x - other.x, y - other.y, z - other.z }; }
	constexpr TVector3 operator*( const TVector3& other ) const { return { x * other.x, y * other.y, z * other.z }; }
	constexpr TVector3 operator/( const TVector3& other ) const { return { x / other.x, y / other.y, z / other.z }; }

	constexpr void operator=( const TVector3& other ) { Set( other ); }
	constexpr void operator+=( const TVector3& other ) { Add( other ); }
	constexpr void operator/=( const TVector3& other ) { Divide( other ); }

public:
	static TFLOAT           Distance( const TVector3& vec1, const TVector3& vec2 ) { return ( vec2 - vec1 ).Magnitude(); }
	static constexpr TFLOAT DistanceSq( const TVector3& vec1, const TVector3& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }
	static constexpr TFLOAT DotProduct( const TVector3& vec1, const TVector3& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z; }

public:
	static constinit const TVector3 VEC_ZERO;
	static constinit const TVector3 VEC_POSX;
	static constinit const TVector3 VEC_POSY;
	static constinit const TVector3 VEC_POSZ;
	static constinit const TVector3 VEC_NEGX;
	static constinit const TVector3 VEC_NEGY;
	static constinit const TVector3 VEC_NEGZ;
	static constinit const TVector3 VEC_NEGXPOSZ;
	static constinit const TVector3 VEC_NEGXNEGZ;
	static constinit const TVector3 VEC_POSXPOSZ;
	static constinit const TVector3 VEC_POSXNEGZ;

public:
	TFLOAT x, y, z;
};

TOSHI_NAMESPACE_END
