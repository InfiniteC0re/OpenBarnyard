#pragma once

#include <cctype>
#include <type_traits>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Helper class that gives you the required methods to operate
// with characters of different types using the same interface
//-----------------------------------------------------------------------------
template <typename CharT>
class T2CharTraits
{
public:
	using CharType                     = CharT;
	using UCharType                    = std::make_unsigned_t<CharT>;
	constexpr static TSIZE    CharSize = sizeof( CharType );
	constexpr static CharType NullChar = 0;

public:
	static TBOOL    IsSpace( TINT a_cChar ) { TUNREACHABLE(); }
	static TBOOL    IsUpperCase( TINT a_cChar ) { TUNREACHABLE(); }
	static TBOOL    IsLowerCase( TINT a_cChar ) { TUNREACHABLE(); }
	static CharType ToUpperCase( TINT a_cChar ) { TUNREACHABLE(); }
	static CharType ToLowerCase( TINT a_cChar ) { TUNREACHABLE(); }
};

using T2Char8  = T2CharTraits<TCHAR>;
using T2Char16 = T2CharTraits<TWCHAR>;

//-----------------------------------------------------------------------------
// TCHAR - 1 byte character
//-----------------------------------------------------------------------------

template <>
TINLINE TBOOL T2CharTraits<TCHAR>::IsSpace( TINT a_cChar )
{
	return isspace( a_cChar ) != 0;
}

template <>
TINLINE TBOOL T2CharTraits<TCHAR>::IsUpperCase( TINT a_cChar )
{
	return isupper( a_cChar ) != 0 || isalpha( a_cChar ) == 0;
}

template <>
TINLINE TBOOL T2CharTraits<TCHAR>::IsLowerCase( TINT a_cChar )
{
	return islower( a_cChar ) != 0 || isalpha( a_cChar ) == 0;
}

template <>
TINLINE T2CharTraits<TCHAR>::CharType T2CharTraits<TCHAR>::ToUpperCase( TINT a_cChar )
{
	return (TCHAR)toupper( a_cChar );
}

template <>
TINLINE T2CharTraits<TCHAR>::CharType T2CharTraits<TCHAR>::ToLowerCase( TINT a_cChar )
{
	return (TCHAR)tolower( a_cChar );
}

//-----------------------------------------------------------------------------
// TWCHAR - 2 bytes character
//-----------------------------------------------------------------------------

template <>
TINLINE TBOOL T2CharTraits<TWCHAR>::IsSpace( TINT a_cChar )
{
	return iswspace( a_cChar ) != 0;
}

template <>
TINLINE TBOOL T2CharTraits<TWCHAR>::IsUpperCase( TINT a_cChar )
{
	return iswupper( a_cChar ) != 0 || iswalpha( a_cChar ) == 0;
}

template <>
TINLINE TBOOL T2CharTraits<TWCHAR>::IsLowerCase( TINT a_cChar )
{
	return iswlower( a_cChar ) != 0 || iswalpha( a_cChar ) == 0;
}

template <>
TINLINE T2CharTraits<TWCHAR>::CharType T2CharTraits<TWCHAR>::ToUpperCase( TINT a_cChar )
{
	return (TWCHAR)towupper( a_cChar );
}

template <>
TINLINE T2CharTraits<TWCHAR>::CharType T2CharTraits<TWCHAR>::ToLowerCase( TINT a_cChar )
{
	return (TWCHAR)towlower( a_cChar );
}

TOSHI_NAMESPACE_END
