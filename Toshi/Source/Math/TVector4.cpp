#include "ToshiPCH.h"
#include "TVector4.h"

TOSHI_NAMESPACE_START

constinit const TVector4 TVector4::VEC_ZERO = TVector4( 0, 0, 0, 1 );
constinit const TVector4 TVector4::VEC_POSX = TVector4( 1, 0, 0, 1 );
constinit const TVector4 TVector4::VEC_POSY = TVector4( 0, 1, 0, 1 );
constinit const TVector4 TVector4::VEC_POSZ = TVector4( 0, 0, 1, 1 );
constinit const TVector4 TVector4::VEC_NEGX = TVector4( -1, 0, 0, 1 );
constinit const TVector4 TVector4::VEC_NEGY = TVector4( 0, -1, 0, 1 );
constinit const TVector4 TVector4::VEC_NEGZ = TVector4( 0, 0, -1, 1 );

// $Barnyard: FUNCTION 006c7600
TFLOAT TVector4::NormaliseMag()
{
	TFLOAT fMagnitudeSq = MagnitudeSq();

	if ( fMagnitudeSq != 0.0f )
	{
		TFLOAT fMultiplier = TMath::OneOverSqrt( fMagnitudeSq );
		x *= fMultiplier;
		y *= fMultiplier;
		z *= fMultiplier;

		return 1.0f / fMultiplier;
	}

	x = 0.0f;
	y = 0.0f;
	z = 1.0f;
	return fMagnitudeSq;
}

// $Barnyard: FUNCTION 006c7590
void TVector4::Normalise()
{
	TFLOAT fMagnitudeSq = MagnitudeSq();

	if ( fMagnitudeSq != 0.0f )
	{
		TFLOAT fMultiplier = TMath::OneOverSqrt( fMagnitudeSq );
		x *= fMultiplier;
		y *= fMultiplier;
		z *= fMultiplier;
		return;
	}

	x = 0.0f;
	y = 0.0f;
	z = 1.0f;
}

// $Barnyard: FUNCTION 006c7930
void TVector4::ProjectNormalToPlane( const TVector4& vec, const TVector4& vec2 )
{
	TVector4 crossProduct;
	crossProduct.CrossProduct( vec2, vec );
	CrossProduct( crossProduct, vec2 );
	Normalise();
}

// $Barnyard: FUNCTION 00544160
void TVector4::Normalise4()
{
	Divide4( Magnitude4() );
}

TOSHI_NAMESPACE_END
