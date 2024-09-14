#include "ToshiPCH.h"
#include "TVector3.h"

TOSHI_NAMESPACE_START

constinit const TVector3 TVector3::VEC_ZERO     = TVector3( 0, 0, 0 );
constinit const TVector3 TVector3::VEC_POSX     = TVector3( 1, 0, 0 );
constinit const TVector3 TVector3::VEC_POSY     = TVector3( 0, 1, 0 );
constinit const TVector3 TVector3::VEC_POSZ     = TVector3( 0, 0, 1 );
constinit const TVector3 TVector3::VEC_NEGX     = TVector3( -1, 0, 0 );
constinit const TVector3 TVector3::VEC_NEGY     = TVector3( 0, -1, 0 );
constinit const TVector3 TVector3::VEC_NEGZ     = TVector3( 0, 0, -1 );
constinit const TVector3 TVector3::VEC_POSXPOSZ = TVector3( TMath::ONEOVER_SQRT_TWO, 0, TMath::ONEOVER_SQRT_TWO );
constinit const TVector3 TVector3::VEC_POSXNEGZ = TVector3( TMath::ONEOVER_SQRT_TWO, 0, -TMath::ONEOVER_SQRT_TWO );
constinit const TVector3 TVector3::VEC_NEGXPOSZ = TVector3( -TMath::ONEOVER_SQRT_TWO, 0, TMath::ONEOVER_SQRT_TWO );
constinit const TVector3 TVector3::VEC_NEGXNEGZ = TVector3( -TMath::ONEOVER_SQRT_TWO, 0, -TMath::ONEOVER_SQRT_TWO );

void TVector3::Clip( TFLOAT fVal, TFLOAT fVal2 )
{
	TMath::Clip( x, fVal, fVal2 );
	TMath::Clip( y, fVal, fVal2 );
	TMath::Clip( z, fVal, fVal2 );
}

void TVector3::CrossProduct( const TVector3& vec1, const TVector3& vec2 )
{
	TFLOAT fX = vec2.z * vec1.y - vec2.y * vec1.z;
	TFLOAT fY = vec1.z * vec2.x - vec2.z * vec1.x;
	TFLOAT fZ = vec1.x * vec2.y - vec1.y * vec2.x;
	Set( fX, fY, fZ );
}

// $Barnyard: FUNCTION 006cbbe0
void TVector3::RotateX( TFLOAT a_fRotation )
{
	TFLOAT f1;
	TFLOAT f2;
	TFLOAT oldY = y;

	TMath::SinCos( a_fRotation, f1, f2 );

	y = -f1 * z + f2 * oldY;
	z = oldY * f1 + z * f2;
}

// $Barnyard: FUNCTION 006c7820
void TVector3::RotateY( TFLOAT a_fRotation )
{
	TFLOAT f1;
	TFLOAT f2;
	TFLOAT oldX = x;

	TMath::SinCos( a_fRotation, f1, f2 );

	x = f1 * z + f2 * oldX;
	z = z * f2 + -f1 * oldX;
}

void TVector3::RotateZ( TFLOAT a_fRotation )
{
	TFLOAT f1;
	TFLOAT f2;
	TFLOAT oldX = x;

	TMath::SinCos( a_fRotation, f1, f2 );

	x = -f1 * y + f2 * oldX;
	y = f1 * oldX + y * f2;
}

void TVector3::Normalize()
{
	if ( MagnitudeSq() != 0.0f )
	{
		TFLOAT magnitude = TMath::OneOverSqrt( MagnitudeSq() );
		Set( x * magnitude, y * magnitude, z * magnitude );
	}
	else
	{
		Set( 0.0f, 0.0f, 1.0f );
	}
}

TOSHI_NAMESPACE_END
