#include "ToshiPCH.h"
#include "TVector2.h"

TOSHI_NAMESPACE_START

constinit const TVector2 TVector2::VEC_ZERO = TVector2( 0, 0 );
constinit const TVector2 TVector2::VEC_POSX = TVector2( 1, 0 );
constinit const TVector2 TVector2::VEC_POSY = TVector2( 0, 1 );
constinit const TVector2 TVector2::VEC_NEGX = TVector2( -1, 0 );
constinit const TVector2 TVector2::VEC_NEGY = TVector2( 0, -1 );

TOSHI_NAMESPACE_END
