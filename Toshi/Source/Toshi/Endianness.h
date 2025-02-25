#pragma once
#include <cstdint>
#include <bit>

typedef TUINT8 Endianess;
enum Endianess_ : Endianess
{
	Endianess_Little,
	Endianess_Big
};

inline constexpr Endianess CURRENT_ENDIANESS =
#ifdef TOSHI_SKU_WINDOWS
    Endianess_Little;
#endif

TFORCEINLINE constexpr TUINT32 SWAP32( const TUINT8 bytes[ 4 ] )
{
	return ( bytes[ 3 ] << 0 ) | ( bytes[ 2 ] << 8 ) | ( bytes[ 1 ] << 16 ) | ( bytes[ 0 ] << 24 );
}

TFORCEINLINE constexpr TUINT16 SWAP16( const TUINT8 bytes[ 2 ] )
{
	return ( bytes[ 1 ] << 0 ) | ( bytes[ 0 ] << 8 );
}

TFORCEINLINE TFLOAT SWAPFLOAT( const TUINT8 bytes[ 4 ] )
{
	TUINT32 result = SWAP32( bytes );
	return *(TFLOAT*)( &result );
}

TFORCEINLINE TUINT32 PARSEDWORD( const TUINT32 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return value;
	else return SWAP32( (const TUINT8*)&value );
}

TFORCEINLINE TUINT32 PARSEDWORD( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return *(TUINT32*)value;
	else return SWAP32( value );
}

TFORCEINLINE TUINT32 PARSEDWORD_BIG( const TUINT32 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP32( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE TUINT32 PARSEDWORD_BIG( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP32( value );
	else return *(TUINT32*)value;
}

TFORCEINLINE TUINT16 PARSEWORD( TUINT16 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return value;
	else return SWAP16( (const TUINT8*)&value );
}

TFORCEINLINE TUINT16 PARSEWORD( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return *(TUINT16*)value;
	else return SWAP16( value );
}

TFORCEINLINE TUINT32 PARSEWORD_BIG( TUINT32 value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP16( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE TUINT32 PARSEWORD_BIG( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAP16( value );
	else return *(TUINT16*)value;
}

TFORCEINLINE TFLOAT PARSEFLOAT( TFLOAT value )
{
	if constexpr ( std::endian::native == std::endian::little ) return value;
	else return SWAPFLOAT( (const TUINT8*)&value );
}

TFORCEINLINE TFLOAT PARSEFLOAT( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return *(TFLOAT*)value;
	else return SWAPFLOAT( value );
}

TFORCEINLINE TFLOAT PARSEFLOAT_BIG( TFLOAT value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAPFLOAT( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE TFLOAT PARSEFLOAT_BIG( const TUINT8* value )
{
	if constexpr ( std::endian::native == std::endian::little ) return SWAPFLOAT( value );
	else return *(TFLOAT*)value;
}

template <typename T>
TFORCEINLINE T CONVERTENDIANESS( Endianess a_eEndianess, T a_numValue )
{
	switch ( a_eEndianess )
	{
		case Endianess_Little:
			if constexpr ( sizeof( T ) == 4 )
				return (T)PARSEDWORD( *(TUINT32*)( TREINTERPRETCAST( void*, &a_numValue ) ) );
			else if constexpr ( sizeof( T ) == 2 )
				return (T)PARSEWORD( *(TUINT16*)( TREINTERPRETCAST( void*, &a_numValue ) ) );
			break;
		case Endianess_Big:
			if constexpr ( sizeof( T ) == 4 )
				return (T)PARSEDWORD_BIG( *(TUINT32*)( TREINTERPRETCAST( void*, &a_numValue ) ) );
			else if constexpr ( sizeof( T ) == 2 )
				return (T)PARSEWORD_BIG( *(TUINT16*)( TREINTERPRETCAST( void*, &a_numValue ) ) );
			break;
	}

	return a_numValue;
}
