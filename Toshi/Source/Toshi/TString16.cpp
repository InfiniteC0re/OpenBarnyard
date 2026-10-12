#include "ToshiPCH.h"
#include "TString16.h"
#include "TString8.h"

#include "Toshi/TSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: inline in the original, an instance of it lives in the game code
// $Barnyard: FUNCTION 0042b530
TString16::TString16()
    : TString16( GetAllocator() )
{
}

// $Barnyard: FUNCTION 006cc820
TString16::TString16( T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	AllocBuffer( 0, TTRUE );
}

TString16::TString16( TString16&& src ) noexcept
{
	TString16::m_pAllocator = src.m_pAllocator;
	TString16::m_iExcessLen = src.m_iExcessLen;
	TString16::m_iStrLen    = src.m_iStrLen;
	TString16::m_pBuffer    = src.m_pBuffer;
	src.Reset();
}

TString16::TString16( const TString16& src, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	Copy( src, -1 );
}

TString16::TString16( TINT size, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	AllocBuffer( size );
}

// $Barnyard: FUNCTION 006cc880
TString16::TString16( const TWCHAR* src, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	*this        = src;
}

// $Barnyard: FUNCTION 006cc8f0
TString16::TString16( const TString8& src, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	*this        = src;
}

// $Barnyard: FUNCTION 006cc130
void TString16::Copy( const TString16& src, TINT size )
{
	// NOTE: retail doesn't check for copying a string into itself: the first character is cleared before wcsncpy, so the string ends up empty
	if ( size < 0 || size > src.Length() )
		size = src.Length();

	AllocBuffer( size, TTRUE );

	if ( size != 0 )
	{
		wcsncpy( m_pBuffer, src.m_pBuffer, size );
		m_pBuffer[ size ] = L'\0';
	}
	else
	{
		m_pBuffer[ 0 ] = L'\0';
	}
}

// $Barnyard: FUNCTION 006cc0c0
void TString16::Copy( const TWCHAR* src, TINT size )
{
	if ( src != m_pBuffer )
	{
		TINT srcLen = src ? (TINT)TStringManager::String16Length( src ) : 0;
		TASSERT( srcLen <= 0xFFFFFF, "Too big string" );

		if ( srcLen < size || size == -1 )
		{
			size = srcLen;
		}

		AllocBuffer( size, TTRUE );
		TUtil::MemCopy( m_pBuffer, src, size * sizeof( TWCHAR ) );

		m_pBuffer[ size ] = 0;
	}
}

// $Barnyard: FUNCTION 006cca50
TINT TString16::Find( TWCHAR character, TINT pos ) const
{
	if ( !IsIndexValid( pos ) ) return -1;

	const TWCHAR* foundAt = wcschr( &m_pBuffer[ pos ], character );
	if ( foundAt == TNULL ) return -1;

	return (TINT)( foundAt - m_pBuffer );
}

// $Barnyard: FUNCTION 006cc9f0
TINT TString16::Find( const TWCHAR* substr, TINT pos ) const
{
	if ( !IsIndexValid( pos ) ) return -1;

	const TWCHAR* foundAt = wcsstr( GetString( pos ), substr );
	if ( foundAt == TNULL ) return -1;

	return (TINT)( foundAt - m_pBuffer );
}

// $Barnyard: FUNCTION 006cbfe0
TBOOL TString16::AllocBuffer( TINT a_iLength, TBOOL freeMemory )
{
	TBOOL hasChanged    = TFALSE;
	TINT  currentLength = Length();

	TASSERT( a_iLength >= 0, "Length can't be less than 0" );
	TASSERT( a_iLength <= 0xFFFFFF, "Too big string" );

	if ( a_iLength != currentLength )
	{
		if ( a_iLength == 0 )
		{
			if ( freeMemory ) m_pAllocator->Free( m_pBuffer );

			m_pBuffer    = NullWString;
			m_iExcessLen = 0;

			hasChanged = TTRUE;
		}
		else
		{
			TINT newExcessLen = ( currentLength - a_iLength ) + m_iExcessLen;

			if ( newExcessLen < 0 || newExcessLen > 0xFF )
			{
				if ( currentLength != 0 && freeMemory )
				{
					m_pAllocator->Free( m_pBuffer );
				}

				m_pBuffer    = (TWCHAR*)m_pAllocator->Malloc( ( a_iLength + 1 ) * sizeof( TWCHAR ) );
				m_iExcessLen = 0;

				hasChanged = TTRUE;
			}
			else
			{
				m_iExcessLen = newExcessLen;
				hasChanged   = TFALSE;
			}
		}

		m_iStrLen = a_iLength;
	}

	if ( freeMemory ) m_pBuffer[ 0 ] = L'\0';
	return hasChanged;
}

// $Barnyard: FUNCTION 006cc1a0
TString16& TString16::Format( const TWCHAR* a_pcFormat, ... )
{
	TWCHAR  buffer[ 0x400 ];
	va_list args;

	va_start( args, a_pcFormat );

	TINT iResult = _vsnwprintf( buffer, TARRAYSIZE( buffer ), a_pcFormat, args );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	Copy( buffer, -1 );

	va_end( args );
	return *this;
}

// $Barnyard: FUNCTION 006cc7d0
TString16& TString16::operator=( const TCHAR* a_szStr )
{
	TINT iLength = a_szStr ? TINT( TStringManager::String8Length( a_szStr ) ) : 0;
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, a_szStr, iLength );

	m_pBuffer[ iLength ] = L'\0';
	return *this;
}

TString16& TString16::VFormat( const TWCHAR* a_pcFormat, va_list a_vargs )
{
	TWCHAR buffer[ 0x400 ];

	TINT iResult = _vsnwprintf( buffer, sizeof( buffer ), a_pcFormat, a_vargs );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	Copy( buffer, -1 );

	return *this;
}

void TString16::ForceSetData( TWCHAR* a_cString, TINT a_iLength )
{
	m_pBuffer = a_cString;

	if ( a_iLength < 0 )
	{
		m_iStrLen = TStringManager::String16Length( a_cString );
	}
	else
	{
		m_iStrLen = a_iLength;
	}

	m_iExcessLen = 0;
}

TINT TString16::FindReverse( TWCHAR a_findChar, TINT pos ) const
{
	if ( pos == -1 )
	{
		pos = m_iStrLen;
	}
	else
	{
		if ( !IsIndexValid( pos ) ) return -1;
	}


	for ( ; pos > -1; pos-- )
	{
		if ( a_findChar == m_pBuffer[ pos ] )
		{
			return pos;
		}
	}

	return -1;
}

void TString16::Truncate( TINT length )
{
	if ( Length() < length )
	{
		length = Length();
	}

	TWCHAR* oldBuffer = m_pBuffer;

	TBOOL allocated = AllocBuffer( length, TFALSE );
	if ( allocated )
	{
		TStringManager::String16Copy( m_pBuffer, oldBuffer, length );
	}

	m_pBuffer[ length ] = 0;

	if ( allocated && Length() != 0 )
	{
		m_pAllocator->Free( oldBuffer );
	}
}

// $Barnyard: FUNCTION 006ccab0
void TString16::FreeBuffer()
{
	if ( Length() > 0 ) m_pAllocator->Free( m_pBuffer );
	Reset();
}

// $Barnyard: FUNCTION 006cc950
const TWCHAR* TString16::GetString( TINT a_iIndex ) const
{
	TASSERT( a_iIndex >= 0 && a_iIndex <= m_iStrLen );
	if ( IsIndexValid( a_iIndex ) ) { return &m_pBuffer[ a_iIndex ]; }
	return TNULL;
}

TWCHAR* TString16::GetStringUnsafe( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex <= m_iStrLen );
	if ( IsIndexValid( a_iIndex ) ) { return &m_pBuffer[ a_iIndex ]; }
	return TNULL;
}

// $Barnyard: FUNCTION 006cc320
TString16& TString16::Concat( const TWCHAR* str, TINT size )
{
	TINT len = (TINT)TStringManager::String16Length( str );

	if ( ( len < size ) || ( size == -1 ) )
	{
		size = len;
	}

	TINT    oldLength = m_iStrLen;
	TWCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( m_iStrLen + size, TFALSE );

	if ( allocated )
	{
		// since it has made a new buffer
		// it need to copy the old string
		// to the new buffer

		TStringManager::String16Copy( m_pBuffer, oldString, -1 );
	}

	TStringManager::String16Copy( m_pBuffer + oldLength, str, size );
	m_pBuffer[ m_iStrLen ] = 0;

	if ( allocated && oldLength != 0 )
	{
		m_pAllocator->Free( oldString );
	}

	return *this;
}

// $Barnyard: FUNCTION 006cc210
TString16& TString16::Concat( const TString16& str, TINT size )
{
	if ( ( str.Length() < size ) || ( size == -1 ) )
	{
		size = str.Length();
	}

	TINT    oldLength = m_iStrLen;
	TWCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( m_iStrLen + size, TFALSE );

	if ( allocated )
		TStringManager::String16Copy( m_pBuffer, oldString, -1 );

	TStringManager::String16Copy( m_pBuffer + oldLength, str.m_pBuffer, size );
	m_pBuffer[ m_iStrLen ] = 0;

	if ( allocated && oldLength > 0 )
		m_pAllocator->Free( oldString );

	return *this;
}

// $Barnyard: FUNCTION 006cc430
TString16& TString16::Insert( const TWCHAR* a_wszString, TINT a_iPos, TINT a_iCount )
{
	TINT iStrLength = (TINT)TStringManager::String16Length( a_wszString );

	if ( a_iCount > iStrLength || a_iCount == -1 )
		a_iCount = iStrLength;

	TINT    iOldLength = m_iStrLen;
	TWCHAR* pOldString = m_pBuffer;

	TBOOL bAllocated = AllocBuffer( iOldLength + a_iCount, TFALSE );

	if ( bAllocated )
	{
		// Copy the parts before and after the insertion point into the new buffer
		TStringManager::String16Copy( m_pBuffer, pOldString, a_iPos );
		TStringManager::String16Copy( m_pBuffer + a_iPos + a_iCount, pOldString + a_iPos, iOldLength - a_iPos );
	}
	else
	{
		// Shift the tail (and the terminator) to make room
		T2String16::Move( m_pBuffer + a_iPos + a_iCount, m_pBuffer + a_iPos, iOldLength - a_iPos );
	}

	TStringManager::String16Copy( m_pBuffer + a_iPos, a_wszString, a_iCount );
	m_pBuffer[ m_iStrLen ] = 0;

	if ( bAllocated && iOldLength > 0 )
		m_pAllocator->Free( pOldString );

	return *this;
}

// $Barnyard: FUNCTION 006cc5b0
TString16& TString16::Delete( TINT a_iPos, TINT a_iCount )
{
	TINT    iOldLength = m_iStrLen;
	TWCHAR* pOldString = m_pBuffer;

	TBOOL bAllocated = AllocBuffer( iOldLength - a_iCount, TFALSE );

	if ( bAllocated )
	{
		TStringManager::String16Copy( m_pBuffer, pOldString, a_iPos );
		TStringManager::String16Copy( m_pBuffer + a_iPos, pOldString + a_iPos + a_iCount, iOldLength - a_iPos - a_iCount );
	}
	else
	{
		TStringManager::String16Copy( m_pBuffer + a_iPos, m_pBuffer + a_iPos + a_iCount, iOldLength - a_iPos - a_iCount );
	}

	m_pBuffer[ m_iStrLen ] = 0;

	if ( bAllocated && iOldLength > 0 )
		m_pAllocator->Free( pOldString );

	return *this;
}

// $Barnyard: FUNCTION 006cc740
TString16& TString16::operator=( const TString8& a_rcStr )
{
	TINT iLength = a_rcStr.Length();
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, a_rcStr.GetString(), iLength );

	m_pBuffer[ iLength ] = L'\0';
	return *this;
}

// $Barnyard: FUNCTION 006cc780
TString16& TString16::operator=( const TWCHAR* a_wszStr )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check for self-assignment, which would clear the string
	if ( a_wszStr == m_pBuffer ) return *this;
#endif

	TINT iLength = a_wszStr ? TINT( wcslen( a_wszStr ) ) : 0;
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		wcsncpy( m_pBuffer, a_wszStr, iLength );

	m_pBuffer[ iLength ] = L'\0';
	return *this;
}

// $Barnyard: FUNCTION 006cc980
TINT TString16::Compare( const TWCHAR* a_pcString, TINT a_iLength ) const
{
	TASSERT( a_pcString != TNULL, "TWString::Compare - Passed string cannot be TNULL" );

	if ( a_iLength == -1 )
		return wcscmp( GetString(), a_pcString );

	return wcsncmp( GetString(), a_pcString, a_iLength );
}

TString16& TString16::Reserve( TINT size )
{
	if ( size > Length() + TINT( ExcessLength() ) )
	{
		TINT    iOldLength = m_iStrLen;
		TWCHAR* pOldString = m_pBuffer;
		TINT    iCopySize  = TMath::Min( iOldLength, size );

		TBOOL bAllocated = AllocBuffer( size, TFALSE );

		if ( bAllocated )
		{
			TStringManager::String16Copy( m_pBuffer, pOldString, iCopySize );
			m_pBuffer[ iCopySize ] = 0;
		}

		if ( bAllocated && iOldLength != 0 )
			m_pAllocator->Free( pOldString );
	}

	return *this;
}

TINT TString16::CompareNoCase( const TWCHAR* a_pcString, TINT param_2 ) const
{
	TASSERT( a_pcString != TNULL, "TWString::CompareNoCase - Passed string cannot be TNULL" );
	TASSERT( IsIndexValid( 0 ), "TWString::CompareNoCase - Index 0 is not valid" );
	TASSERT( GetString() != TNULL, "TWString::CompareNoCase - String cannot be TNULL" );

	if ( param_2 == -1 )
	{
		return _wcsicmp( GetString(), a_pcString );
	}

	TASSERT( IsIndexValid( 0 ), "TWString::CompareNoCase - Index 0 is not valid" );

	return _wcsnicmp( GetString(), a_pcString, param_2 );
}

TString16 TString16::Mid( TINT a_iFirst, TINT a_iCount ) const
{
	if ( a_iFirst < 0 )
	{
		a_iFirst = 0;
	}
	else if ( Length() <= a_iFirst )
	{
		// Can't return string bigger that the original
		return TString16();
	}

	if ( a_iCount < 0 || Length() < a_iFirst + a_iCount )
	{
		a_iCount = Length() - a_iFirst;
	}

	TString16 strResult( a_iCount, TNULL );
	TUtil::MemCopy( strResult.GetStringUnsafe(), GetString( a_iFirst ), a_iCount * 2 );
	strResult[ a_iCount ] = '\0';

	return strResult;
}

TString16 TString16::Right( TINT a_iFrom ) const
{
	return Mid( a_iFrom, Length() - a_iFrom );
}

TBOOL TString16::IsAllLowerCase() const
{
	if ( m_iStrLen != 0 )
	{
		TWCHAR* iter   = m_pBuffer;
		TWCHAR* endStr = m_pBuffer + m_iStrLen;

		do
		{
			TWCHAR currentC = *iter++;
			if ( !iswlower( currentC ) ) return TFALSE;
		} while ( iter < endStr );
	}

	return TTRUE;
}

TBOOL TString16::IsAllUpperCase() const
{
	if ( m_iStrLen != 0 )
	{
		TWCHAR* iter   = m_pBuffer;
		TWCHAR* endStr = m_pBuffer + m_iStrLen;

		do
		{
			TWCHAR currentC = *iter++;
			if ( !iswupper( currentC ) ) return TFALSE;
		} while ( iter < endStr );
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
