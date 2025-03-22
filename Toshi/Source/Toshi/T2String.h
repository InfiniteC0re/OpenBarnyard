#pragma once
#include "T2CharTraits.h"
#include "T2StringTraits.h"
#include "T2String8.h"
#include "T2String16.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Template wrapper for constant strings that allows using an interface
// instead of static methods from T2String8 or T2String16.
//
// Use T2StringView to work with TCHAR strings
// Use T2ConstString16 to work with TWCHAR strings
// Note: To support other encodings, pass your own StringTraits class.
//-----------------------------------------------------------------------------
template <class StringTraits = T2StringTraits<TCHAR>>
class T2StringViewBase
{
public:
	using CharType = typename StringTraits::CharType;

public:
	TFORCEINLINE constexpr T2StringViewBase( const CharType* a_pszString )
	    : m_pszString( a_pszString )
	{}

	TFORCEINLINE constexpr T2StringViewBase()
	    : m_pszString( TNULL )
	{}

	TFORCEINLINE constexpr T2StringViewBase( const T2StringViewBase& a_rOther )
	    : m_pszString( a_rOther.m_pszString )
	{}

	TFORCEINLINE TSIZE Length() const { return StringTraits::Length( m_pszString ); }
	TFORCEINLINE TBOOL IsLowerCase() const { return StringTraits::IsLowerCase( m_pszString ); }
	TFORCEINLINE TBOOL IsUpperCase() const { return StringTraits::IsUpperCase( m_pszString ); }
	TFORCEINLINE TBOOL IsEqual( T2StringViewBase a_otherString ) { return StringTraits::Compare( m_pszString, a_otherString.m_pszString ) == 0; }
	TFORCEINLINE TBOOL IsEqualNoCase( T2StringViewBase a_otherString ) { return StringTraits::CompareNoCase( m_pszString, a_otherString.m_pszString ) == 0; }

	TFORCEINLINE const CharType* FindChar( CharType a_cCharacter ) { return StringTraits::FindChar( m_pszString, a_cCharacter ); }
	TFORCEINLINE const CharType* FindString( const CharType* a_pszSubstr ) { return StringTraits::FindString( m_pszString, a_pszSubstr ); }
	TFORCEINLINE const CharType* SkipSpaces() { return StringTraits::SkipSpaces( m_pszString ); }

	TFORCEINLINE constexpr T2StringViewBase& operator=( const CharType* a_pszString )
	{
		m_pszString = a_pszString;
		return *this;
	}

	TFORCEINLINE constexpr T2StringViewBase operator+( int a_iSize ) const { return T2StringViewBase( m_pszString + a_iSize ); }
	TFORCEINLINE constexpr T2StringViewBase operator-( int a_iSize ) const { return T2StringViewBase( m_pszString - a_iSize ); }
	TFORCEINLINE constexpr T2StringViewBase operator+=( int a_iSize )
	{
		m_pszString += a_iSize;
		return *this;
	}
	TFORCEINLINE constexpr T2StringViewBase operator-=( int a_iSize )
	{
		m_pszString -= a_iSize;
		return *this;
	}

	TFORCEINLINE TBOOL operator==( T2StringViewBase a_otherString )
	{
		return StringTraits::Compare( m_pszString, a_otherString.m_pszString ) == 0;
	}

	TFORCEINLINE constexpr const CharType* Get() const { return m_pszString; }
	TFORCEINLINE constexpr operator const CharType*() const { return m_pszString; }

private:
	const CharType* m_pszString;
};

using T2StringView  = T2StringViewBase<T2StringTraits<TCHAR>>;
using T2StringView16 = T2StringViewBase<T2StringTraits<TWCHAR>>;

//-----------------------------------------------------------------------------
// Purpose: Wrapper for a constant size string that has an interface.
// Note: To support other encodings, pass your own StringTraits class.
//-----------------------------------------------------------------------------
template <TSIZE Size, typename TStringTraits = T2StringTraits<TCHAR>>
class T2FormatString
{
public:
	TSTATICASSERT( Size > 0 );

	using StringTraits = TStringTraits;
	using CharTraits   = typename StringTraits::CharTraits;
	using CharType     = typename StringTraits::CharType;
	using UCharType    = typename StringTraits::UCharType;

public:
	constexpr T2FormatString()
	{
		m_szBuffer[ 0 ] = CharTraits::NullChar;
	}

	T2FormatString( const CharType* a_szString )
	{
		Copy( a_szString );
	}

	void Copy( const CharType* a_szString )
	{
		StringTraits::CopySafe( m_szBuffer, a_szString, Size );
	}

	/**
		 * Parses a line from specified buffer and saves it.
		 * @param a_szString buffer to parse buffer
		 * @param a_uiSize size of the a_szString buffer
		 * @param a_pStringLength if not TNULL, string length is saved there
		 * @param a_bTrimSpaces if TTRUE, removes spaces at the start and at the end
		 * @return number of characters to skip in the buffer
		 */
	TINT ParseLine( const CharType* a_szString, TINT a_uiSize = -1, TINT* a_pStringLength = TNULL, TBOOL a_bTrimStartSpaces = TTRUE, TBOOL a_bTrimEndSpaces = TTRUE )
	{
		TINT  uiPos        = 0;
		TINT  uiStringPos  = 0;
		TBOOL bTextStarted = TFALSE;

		if ( a_uiSize == -1 )
			a_uiSize = INT_MAX;

		while ( uiPos < Size && uiStringPos < a_uiSize && a_szString[ uiStringPos ] != '\0' )
		{
			if ( a_szString[ uiStringPos ] != '\n' )
			{
				// Trim spaced at the start of the string
				if ( !bTextStarted && a_bTrimStartSpaces )
				{
					bTextStarted = !CharTraits::IsSpace( (UCharType)a_szString[ uiStringPos ] );

					if ( !bTextStarted )
					{
						uiStringPos++;
						continue;
					}
				}

				m_szBuffer[ uiPos++ ] = a_szString[ uiStringPos++ ];
			}
			else
			{
				break;
			}
		}

		uiPos -= 1;

		if ( a_bTrimEndSpaces )
		{
			while ( uiPos >= 0 && m_szBuffer[ uiPos ] != '\n' && CharTraits::IsSpace( (UCharType)m_szBuffer[ uiPos ] ) )
			{
				uiPos--;
			}
		}

		m_szBuffer[ uiPos + 1 ] = CharTraits::NullChar;

		if ( a_pStringLength )
		{
			*a_pStringLength = uiPos + 1;
		}

		return uiStringPos + 1;
	}

	void Format( const CharType* a_szFormat, ... )
	{
		va_list args;

		va_start( args, a_szFormat );
		StringTraits::FormatV( m_szBuffer, Size, a_szFormat, args );
		va_end( args );

		m_szBuffer[ Size - 1 ] = CharTraits::NullChar;
	}

	void FormatV( const CharType* a_szFormat, va_list a_Args )
	{
		StringTraits::FormatV( m_szBuffer, Size, a_szFormat, a_Args );
		m_szBuffer[ Size - 1 ] = CharTraits::NullChar;
	}

	void Append( const CharType* a_szString )
	{
		StringTraits::Concat( m_szBuffer, a_szString );
	}

	void Clear()
	{
		m_szBuffer[ 0 ] = CharTraits::NullChar;
	}

	TUINT Length()
	{
		return StringTraits::Length( m_szBuffer );
	}

	constexpr CharType* Get()
	{
		return m_szBuffer;
	}

	constexpr const CharType* Get() const
	{
		return m_szBuffer;
	}

	constexpr CharType* Get( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < Size );
		return &m_szBuffer[ a_uiIndex ];
	}

	constexpr const CharType* Get( TUINT a_uiIndex ) const
	{
		TASSERT( a_uiIndex < Size );
		return &m_szBuffer[ a_uiIndex ];
	}

	constexpr CharType& operator[]( TUINT a_uiIndex )
	{
		return *Get( a_uiIndex );
	}

	constexpr const CharType& operator[]( TUINT a_uiIndex ) const
	{
		return *Get( a_uiIndex );
	}

private:
	CharType m_szBuffer[ Size ];
};

using T2FormatString32   = T2FormatString<32, T2StringTraits<TCHAR>>;
using T2FormatString64   = T2FormatString<64, T2StringTraits<TCHAR>>;
using T2FormatString128  = T2FormatString<128, T2StringTraits<TCHAR>>;
using T2FormatString256  = T2FormatString<256, T2StringTraits<TCHAR>>;
using T2FormatString512  = T2FormatString<512, T2StringTraits<TCHAR>>;
using T2FormatString1024 = T2FormatString<1024, T2StringTraits<TCHAR>>;
using T2FormatString2048 = T2FormatString<2048, T2StringTraits<TCHAR>>;
using T2FormatString4096 = T2FormatString<4096, T2StringTraits<TCHAR>>;

using T2FormatWString32   = T2FormatString<32, T2StringTraits<TWCHAR>>;
using T2FormatWString64   = T2FormatString<64, T2StringTraits<TWCHAR>>;
using T2FormatWString128  = T2FormatString<128, T2StringTraits<TWCHAR>>;
using T2FormatWString256  = T2FormatString<256, T2StringTraits<TWCHAR>>;
using T2FormatWString512  = T2FormatString<512, T2StringTraits<TWCHAR>>;
using T2FormatWString1024 = T2FormatString<1024, T2StringTraits<TWCHAR>>;
using T2FormatWString2048 = T2FormatString<2048, T2StringTraits<TWCHAR>>;
using T2FormatWString4096 = T2FormatString<4096, T2StringTraits<TWCHAR>>;

//-----------------------------------------------------------------------------
// Purpose: Wrapper class that makes parsing text data easier.
// Note: To support other encodings, pass your own StringTraits class.
//-----------------------------------------------------------------------------
template <class StringTraits = T2StringTraits<TCHAR>>
class T2StringParser
{
public:
	using CharType    = typename StringTraits::CharType;
	using ConstString = typename T2StringViewBase<StringTraits>;

public:
	T2StringParser( ConstString a_pchBuffer, TSIZE a_uiBufferSize = -1 )
	{
		SetBuffer( a_pchBuffer, a_uiBufferSize );
	}

	T2StringParser()
	{
		m_uiBufferSize = 0;
	}

	void SetBuffer( ConstString a_pchBuffer, TSIZE a_uiBufferSize = -1 )
	{
		m_Buffer    = a_pchBuffer;
		m_BufferPos = a_pchBuffer;

		if ( a_pchBuffer )
			m_uiBufferSize = ( a_uiBufferSize == -1 ) ? a_pchBuffer.Length() : a_uiBufferSize;
		else
			m_uiBufferSize = 0;

		m_BufferEnd = m_Buffer + m_uiBufferSize;
	}

	void Reset()
	{
		m_BufferPos = m_Buffer;
	}

	TSIZE Tell() const
	{
		return m_BufferPos - m_Buffer;
	}

	void Set( TSIZE a_uiPos )
	{
		m_BufferPos = m_Buffer + a_uiPos;
	}

	TBOOL PeekToken( CharType* a_pszToken, TSIZE a_uiTokenMaxSize )
	{
		TASSERT( TNULL != a_pszToken );
		TASSERT( a_uiTokenMaxSize > 1 );

		TSIZE       uiSizeLeft    = a_uiTokenMaxSize;
		CharType*   pchCopyCursor = a_pszToken;
		ConstString cursor        = m_BufferPos.SkipSpaces();

		TBOOL bIsInQuotes       = TFALSE;
		TBOOL bIsPrevCharEscape = TFALSE;
		TBOOL bIsValid          = TFALSE;

		// Copy until we have the last byte for the \0 character or the buffer is over
		while ( uiSizeLeft > 1 && cursor < m_BufferEnd )
		{
			CharType cChar         = *cursor;
			TBOOL    bIsEscapeChar = cChar == '\\';
			TBOOL    bIsBreakToken = isspace( cChar );

			if ( !bIsPrevCharEscape && ( cChar == '"' || cChar == '\'' ) )
			{
				bIsInQuotes = !bIsInQuotes;

				if ( !bIsInQuotes )
				{
					// The quotes are closed so the token is over
					cursor += 1;
					bIsValid = TTRUE;
					break;
				}
			}
			else if ( !bIsInQuotes && bIsBreakToken )
			{
				// Break (stop) character is found and it's not in quotes
				cursor += 1;
				bIsValid = TTRUE;
				break;
			}
			else
			{
				// Copy the character
				*( pchCopyCursor++ ) = cChar;
			}

			cursor += 1;
			bIsPrevCharEscape = bIsEscapeChar;
		}

		if ( cursor >= m_BufferEnd && !bIsInQuotes )
			bIsValid = TTRUE;

		m_BufferLastPeek = cursor;
		*pchCopyCursor   = '\0';
		return bIsValid;
	}

	TBOOL ReadToken( CharType* a_pszToken, TSIZE a_uiTokenMaxSize )
	{
		TBOOL bResult = PeekToken( a_pszToken, a_uiTokenMaxSize );
		m_BufferPos   = m_BufferLastPeek;

		return bResult;
	}

	TBOOL IsOver() const
	{
		return m_BufferPos >= m_BufferEnd;
	}

private:
	ConstString m_Buffer;
	ConstString m_BufferPos;
	ConstString m_BufferEnd;
	ConstString m_BufferLastPeek;
	TSIZE       m_uiBufferSize;
};

using T2String8Parser  = T2StringParser<T2StringTraits<TCHAR>>;
using T2String16Parser = T2StringParser<T2StringTraits<TWCHAR>>;

//-----------------------------------------------------------------------------
// Purpose: Wrapper class that allows other classes to compare strings.
// Note: To support other encodings, pass your own StringTraits class.
//-----------------------------------------------------------------------------
template <class StringTraits = T2StringTraits<TCHAR>>
class T2StringComparator
{
public:
	using CharType = typename StringTraits::CharType;

public:
	static TBOOL IsEqual( const CharType* a, const CharType* b )
	{
		return StringTraits::Compare( a, b ) == 0;
	}

	static TBOOL IsGreater( const CharType* a, const CharType* b )
	{
		return StringTraits::Compare( a, b ) > 0;
	}

	static TBOOL IsLess( const CharType* a, const CharType* b )
	{
		return StringTraits::Compare( a, b ) < 0;
	}

	static TBOOL IsLessOrEqual( const CharType* a, const CharType* b )
	{
		return StringTraits::Compare( a, b ) <= 0;
	}

	static TBOOL IsGreaterOrEqual( const CharType* a, const CharType* b )
	{
		return StringTraits::Compare( a, b ) >= 0;
	}
};

using T2String8Comparator  = T2StringComparator<T2StringTraits<TCHAR>>;
using T2String16Comparator = T2StringComparator<T2StringTraits<TWCHAR>>;

template <TINT Size, typename TStringTraits = T2StringTraits<TCHAR>>
class T2StringBuffer
{
public:
	TSTATICASSERT( Size > 0 );

	using StringTraits = TStringTraits;
	using CharTraits   = typename StringTraits::CharTraits;
	using CharType     = typename StringTraits::CharType;

public:
	constexpr T2StringBuffer()
	{
		m_szBuffer[ 0 ] = CharTraits::NullChar;
		m_iPosition     = 0;
	}

	T2StringBuffer( const CharType* a_szString )
	{
		Copy( a_szString );
	}

	void Copy( const CharType* a_szString )
	{
		StringTraits::CopySafe( m_szBuffer, a_szString, Size );
		m_iPosition = StringTraits::Length( a_szString );
	}

	void Format( const CharType* a_szFormat, ... )
	{
		va_list args;

		va_start( args, a_szFormat );
		m_iPosition = StringTraits::FormatV( m_szBuffer, Size, a_szFormat, args );
		va_end( args );

		m_szBuffer[ Size - 1 ] = CharTraits::NullChar;
	}

	void FormatV( const CharType* a_szFormat, va_list a_Args )
	{
		m_iPosition            = StringTraits::FormatV( m_szBuffer, Size, a_szFormat, a_Args );
		m_szBuffer[ Size - 1 ] = CharTraits::NullChar;
	}

	void AppendFormat( const CharType* a_szFormat, ... )
	{
		if ( Size - m_iPosition - 1 <= 0 ) return;

		va_list args;

		va_start( args, a_szFormat );
		m_iPosition += StringTraits::FormatV( m_szBuffer + m_iPosition, Size - m_iPosition, a_szFormat, args );
		va_end( args );
	}

	void AppendFormatV( const CharType* a_szFormat, va_list a_Args )
	{
		if ( Size - m_iPosition - 1 <= 0 ) return;
		m_iPosition += StringTraits::FormatV( m_szBuffer + m_iPosition, Size - m_iPosition, a_szFormat, a_Args );
	}

	void Append( const CharType* a_szString )
	{
		TINT iStringLength = StringTraits::Length( a_szString );
		if ( Size - m_iPosition - iStringLength - 1 <= 0 ) return;

		StringTraits::Concat( m_szBuffer + m_iPosition, a_szString, iStringLength );
		m_iPosition += StringTraits::Length( a_szString );
	}

	constexpr void Clear()
	{
		m_szBuffer[ 0 ] = CharTraits::NullChar;
		m_iPosition     = 0;
	}

	constexpr TINT Length()
	{
		return m_iPosition;
	}

	constexpr CharType* Get()
	{
		return m_szBuffer;
	}

	constexpr const CharType* Get() const
	{
		return m_szBuffer;
	}

	constexpr CharType* Get( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < Size );
		return &m_szBuffer[ a_uiIndex ];
	}

	constexpr const CharType* Get( TUINT a_uiIndex ) const
	{
		TASSERT( a_uiIndex < Size );
		return &m_szBuffer[ a_uiIndex ];
	}

	constexpr CharType& operator[]( TUINT a_uiIndex )
	{
		return *Get( a_uiIndex );
	}

	constexpr const CharType& operator[]( TUINT a_uiIndex ) const
	{
		return *Get( a_uiIndex );
	}

	T2StringBuffer& operator+=( const CharType* a_szString )
	{
		Append( a_szString );
		return *this;
	}

private:
	CharType m_szBuffer[ Size ];
	TINT     m_iPosition;
};

TOSHI_NAMESPACE_END
