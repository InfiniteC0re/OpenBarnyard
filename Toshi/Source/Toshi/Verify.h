#pragma once
#include "Defines.h"
#include "Typedefs.h"

// Check if sizes of the integer types are correct
TSTATICASSERT( sizeof( TINT8 ) == 1 );
TSTATICASSERT( sizeof( TUINT8 ) == 1 );
TSTATICASSERT( sizeof( TINT16 ) == 2 );
TSTATICASSERT( sizeof( TUINT16 ) == 2 );
TSTATICASSERT( sizeof( TINT32 ) == 4 );
TSTATICASSERT( sizeof( TUINT32 ) == 4 );
TSTATICASSERT( sizeof( TINT64 ) == 8 );
TSTATICASSERT( sizeof( TUINT64 ) == 8 );

// x64 is not supported
TSTATICASSERT( sizeof( void* ) == 4 );

#ifndef TOSHI_SKU_WINDOWS
#    error At the moment, Windows is the only supported OS
#endif
