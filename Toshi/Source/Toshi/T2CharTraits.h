#pragma once

#include <cctype>

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
	constexpr static TSIZE    CharSize = sizeof( CharType );
	constexpr static CharType NullChar = 0;

public:
	static TBOOL    IsSpace( CharType a_cChar ) { TUNREACHABLE(); }
	static TBOOL    IsUpperCase( CharType a_cChar ) { TUNREACHABLE(); }
	static TBOOL    IsLowerCase( CharType a_cChar ) { TUNREACHABLE(); }
	static CharType ToUpperCase( CharType a_cChar ) { TUNREACHABLE(); }
	static CharType ToLowerCase( CharType a_cChar ) { TUNREACHABLE(); }
};

using T2Char8  = T2CharTraits<TCHAR>;
using T2Char16 = T2CharTraits<TWCHAR>;

//-----------------------------------------------------------------------------
// TCHAR - 1 byte character
//-----------------------------------------------------------------------------

template <>
TINLINE TBOOL T2CharTraits<TCHAR>::IsSpace( CharType a_cChar )
{
	return isspace( a_cChar ) != 0;
}

template <>
TINLINE TBOOL T2CharTraits<TCHAR>::IsUpperCase( CharType a_cChar )
{
	return isupper( a_cChar ) != 0 || isalpha( a_cChar ) == 0;
}

template <>
TINLINE TBOOL T2CharTraits<TCHAR>::IsLowerCase( CharType a_cChar )
{
	return islower( a_cChar ) != 0 || isalpha( a_cChar ) == 0;
}

template <>
TINLINE T2CharTraits<TCHAR>::CharType T2CharTraits<TCHAR>::ToUpperCase( CharType a_cChar )
{
	return (TCHAR)toupper( a_cChar );
}

template <>
TINLINE T2CharTraits<TCHAR>::CharType T2CharTraits<TCHAR>::ToLowerCase( CharType a_cChar )
{
	return (TCHAR)tolower( a_cChar );
}

//-----------------------------------------------------------------------------
// TWCHAR - 2 bytes character
//-----------------------------------------------------------------------------

template <>
TINLINE TBOOL T2CharTraits<TWCHAR>::IsSpace( CharType a_cChar )
{
	return iswspace( a_cChar ) != 0;
}

template <>
TINLINE TBOOL T2CharTraits<TWCHAR>::IsUpperCase( CharType a_cChar )
{
	return iswupper( a_cChar ) != 0 || iswalpha( a_cChar ) == 0;
}

template <>
TINLINE TBOOL T2CharTraits<TWCHAR>::IsLowerCase( CharType a_cChar )
{
	return iswlower( a_cChar ) != 0 || iswalpha( a_cChar ) == 0;
}

template <>
TINLINE T2CharTraits<TWCHAR>::CharType T2CharTraits<TWCHAR>::ToUpperCase( CharType a_cChar )
{
	return (TWCHAR)towupper( a_cChar );
}

template <>
TINLINE T2CharTraits<TWCHAR>::CharType T2CharTraits<TWCHAR>::ToLowerCase( CharType a_cChar )
{
	return (TWCHAR)towlower( a_cChar );
}

TOSHI_NAMESPACE_END
