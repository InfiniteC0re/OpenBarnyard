#pragma once

TOSHI_NAMESPACE_START

class TVector2
{
public:
	constexpr TVector2() = default;
	constexpr TVector2( TFLOAT x, TFLOAT y ) { Set( x, y ); }
	constexpr TVector2( TFLOAT coords[ 2 ] ) { Set( coords[ 0 ], coords[ 1 ] ); }
	constexpr TVector2( const TVector2& other ) { Set( other ); }

	constexpr void Set( TFLOAT x, TFLOAT y )
	{
		TVector2::x = x;
		TVector2::y = y;
	}

	constexpr void Set( TFLOAT coords[ 2 ] )
	{
		TVector2::x = coords[ 0 ];
		TVector2::y = coords[ 1 ];
	}

	constexpr void Set( const TVector2& other )
	{
		TVector2::x = other.x;
		TVector2::y = other.y;
	}

	constexpr void Add( const TVector2& vec )
	{
		x += vec.x;
		y += vec.y;
	}

	constexpr void Add( const TVector2& a, const TVector2& b )
	{
		Set( a + b );
	}

	constexpr void Divide( const TVector2& vec )
	{
		x /= vec.x;
		y /= vec.y;
	}

	constexpr void Divide( TFLOAT scalar )
	{
		TFLOAT ratio = 1.0f / scalar;
		x *= ratio;
		y *= ratio;
	}

	constexpr void Divide( const TVector2& vec1, const TVector2& vec2 )
	{
		Set( vec1 / vec2 );
	}

	constexpr void Divide( const TVector2& vec, TFLOAT scalar )
	{
		Set( vec );
		Divide( scalar );
	}

	constexpr void Multiply( const TVector2& vec )
	{
		x *= vec.x;
		y *= vec.y;
	}

	constexpr void Multiply( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
	}

	constexpr void Multiply( const TVector2& vec1, const TVector2& vec2 )
	{
		Set( vec1 * vec2 );
	}

	constexpr void Multiply( const TVector2& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply( scalar );
	}

	void Normalize()
	{
		if ( MagnitudeSq() != 0.0f )
		{
			float magnitude = TMath::OneOverSqrt( MagnitudeSq() );
			Set( x * magnitude, y * magnitude );
		}
		else
		{
			Set( 1.0f, 0.0f );
		}
	}

	void Abs( const TVector2& vec3 ) { Set( TMath::Abs( vec3.x ), TMath::Abs( vec3.y ) ); }
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ) ); }

	TFLOAT Magnitude() const { return TMath::Sqrt( x * x + y * y ); }
	constexpr TFLOAT MagnitudeSq() const { return x * x + y * y; }

	TBOOL IsEqual( const TVector2& vec ) { return TMath::Abs( x - vec.x ) < 0.00001f && TMath::Abs( y - vec.y ) < 0.00001f; }

	constexpr TVector2 operator+( const TVector2& other ) const { return { x + other.x, y + other.y }; }
	constexpr TVector2 operator-( const TVector2& other ) const { return { x - other.x, y - other.y }; }
	constexpr TVector2 operator*( const TVector2& other ) const { return { x * other.x, y * other.y }; }
	constexpr TVector2 operator/( const TVector2& other ) const { return { x / other.x, y / other.y }; }

	constexpr void operator=( const TVector2& other ) { Set( other ); }
	constexpr void operator+=( const TVector2& other ) { Add( other ); }
	constexpr void operator/=( const TVector2& other ) { Divide( other ); }

public:
	static TFLOAT Distance( const TVector2& vec1, const TVector2& vec2 ) { return ( vec2 - vec1 ).Magnitude(); }
	static constexpr TFLOAT DistanceSq( const TVector2& vec1, const TVector2& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }
	static constexpr TFLOAT DotProduct( const TVector2& vec1, const TVector2& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y; }

public:
	constinit const static TVector2 VEC_ZERO;
	constinit const static TVector2 VEC_POSX;
	constinit const static TVector2 VEC_POSY;
	constinit const static TVector2 VEC_NEGX;
	constinit const static TVector2 VEC_NEGY;

public:
	TFLOAT x, y;
};

TOSHI_NAMESPACE_END
