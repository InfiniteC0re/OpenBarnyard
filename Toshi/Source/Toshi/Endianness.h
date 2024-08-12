#pragma once
#include <cstdint>
#include <bit>

static __forceinline TUINT32 SWAP32( const uint8_t bytes[ 4 ] )
{
    return ( bytes[ 3 ] << 0 ) | ( bytes[ 2 ] << 8 ) | ( bytes[ 1 ] << 16 ) | ( bytes[ 0 ] << 24 );
}

static __forceinline uint16_t SWAP16( const uint8_t bytes[ 2 ] )
{
    return ( bytes[ 1 ] << 0 ) | ( bytes[ 0 ] << 8 );
}

static __forceinline float SWAPFLOAT( const uint8_t bytes[ 4 ] )
{
    TUINT32 result = SWAP32( bytes );
    return *(float*)( &result );
}

static __forceinline TUINT32 PARSEDWORD( const TUINT32 value )
{
    if constexpr ( std::endian::native == std::endian::little ) return value;
    else
        return SWAP32( (const uint8_t*)&value );
}

static __forceinline TUINT32 PARSEDWORD( const uint8_t* value )
{
    if constexpr ( std::endian::native == std::endian::little ) return *(TUINT32*)value;
    else
        return SWAP32( value );
}

static __forceinline TUINT32 PARSEDWORD_BIG( const TUINT32 value )
{
    if constexpr ( std::endian::native == std::endian::little ) return SWAP32( (const uint8_t*)&value );
    else
        return value;
}

static __forceinline TUINT32 PARSEDWORD_BIG( const uint8_t* value )
{
    if constexpr ( std::endian::native == std::endian::little ) return SWAP32( value );
    else
        return *(TUINT32*)value;
}

static __forceinline uint16_t PARSEWORD( uint16_t value )
{
    if constexpr ( std::endian::native == std::endian::little ) return value;
    else
        return SWAP16( (const uint8_t*)&value );
}

static __forceinline uint16_t PARSEWORD( const uint8_t* value )
{
    if constexpr ( std::endian::native == std::endian::little ) return *(uint16_t*)value;
    else
        return SWAP16( value );
}

static __forceinline TUINT32 PARSEWORD_BIG( TUINT32 value )
{
    if constexpr ( std::endian::native == std::endian::little ) return SWAP16( (const uint8_t*)&value );
    else
        return value;
}

static __forceinline TUINT32 PARSEWORD_BIG( const uint8_t* value )
{
    if constexpr ( std::endian::native == std::endian::little ) return SWAP16( value );
    else
        return *(uint16_t*)value;
}

static __forceinline float PARSEFLOAT( float value )
{
    if constexpr ( std::endian::native == std::endian::little ) return value;
    else
        return SWAPFLOAT( (const uint8_t*)&value );
}

static __forceinline float PARSEFLOAT( const uint8_t* value )
{
    if constexpr ( std::endian::native == std::endian::little ) return *(float*)value;
    else
        return SWAPFLOAT( value );
}

static __forceinline float PARSEFLOAT_BIG( float value )
{
    if constexpr ( std::endian::native == std::endian::little ) return SWAPFLOAT( (const uint8_t*)&value );
    else
        return value;
}

static __forceinline float PARSEFLOAT_BIG( const uint8_t* value )
{
    if constexpr ( std::endian::native == std::endian::little ) return SWAPFLOAT( value );
    else
        return *(float*)value;
}
