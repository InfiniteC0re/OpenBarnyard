#pragma once
#include <cstdint>
#include <bit>

TFORCEINLINE constexpr TUINT32 SWAP32( const TUINT8 bytes[ 4 ] )
{
	return ( bytes[ 3 ] << 0 ) | ( bytes[ 2 ] << 8 ) | ( bytes[ 1 ] << 16 ) | ( bytes[ 0 ] << 24 );
}

TFORCEINLINE constexpr TUINT16 SWAP16( const TUINT8 bytes[ 2 ] )
{
	return ( bytes[ 1 ] << 0 ) | ( bytes[ 0 ] << 8 );
}

TFORCEINLINE constexpr TFLOAT SWAPFLOAT( const TUINT8 bytes[ 4 ] )
{
	TUINT32 result = SWAP32( bytes );
	return *(TFLOAT*)( &result );
}

TFORCEINLINE constexpr TUINT32 PARSEDWORD( const TUINT32 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return value;
	else return SWAP32( (const TUINT8*)&value );
}

TFORCEINLINE constexpr TUINT32 PARSEDWORD( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return *(TUINT32*)value;
	else return SWAP32( value );
}

TFORCEINLINE constexpr TUINT32 PARSEDWORD_BIG( const TUINT32 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP32( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE constexpr TUINT32 PARSEDWORD_BIG( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP32( value );
	else return *(TUINT32*)value;
}

TFORCEINLINE constexpr TUINT16 PARSEWORD( TUINT16 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return value;
	else return SWAP16( (const TUINT8*)&value );
}

TFORCEINLINE constexpr TUINT16 PARSEWORD( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return *(TUINT16*)value;
	else return SWAP16( value );
}

TFORCEINLINE constexpr TUINT32 PARSEWORD_BIG( TUINT32 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP16( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE constexpr TUINT32 PARSEWORD_BIG( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP16( value );
	else return *(TUINT16*)value;
}

TFORCEINLINE constexpr TFLOAT PARSEFLOAT( TFLOAT value )
{
	if constexpr ( std::endian::native == std::endian::little ) return value;
	else return SWAPFLOAT( (const TUINT8*)&value );
}

TFORCEINLINE constexpr TFLOAT PARSEFLOAT( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return *(TFLOAT*)value;
	else return SWAPFLOAT( value );
}

TFORCEINLINE constexpr TFLOAT PARSEFLOAT_BIG( TFLOAT value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAPFLOAT( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE constexpr TFLOAT PARSEFLOAT_BIG( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAPFLOAT( value );
	else return *(TFLOAT*)value;
}
