#include "ToshiPCH.h"
#include "TString16.h"

#include "Toshi/TSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TString16::TString16()
{
	Reset();
	m_pAllocator = GetAllocator();
}

TString16::TString16( T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
}

TString16::TString16( TString16&& src ) noexcept
{
	TString16::m_pAllocator = src.m_pAllocator;
	TString16::m_iExcessLen = src.m_iExcessLen;
	TString16::m_iStrLen    = src.m_iStrLen;
	TString16::m_pBuffer    = src.m_pBuffer;
	src.Reset();
	src.m_pAllocator = GetAllocator();
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

TString16::TString16( const TWCHAR* src, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	Copy( src );
}

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

TINT TString16::Find( TWCHAR character, TINT pos ) const
{
	if ( !IsIndexValid( pos ) ) return -1;

	const TWCHAR* foundAt = wcschr( &m_pBuffer[ pos ], character );
	if ( foundAt == TNULL ) return -1;

	return (TINT)( foundAt - m_pBuffer );
}

TINT TString16::Find( const TWCHAR* substr, TINT pos ) const
{
	if ( !IsIndexValid( pos ) ) return -1;

	const TWCHAR* foundAt = wcsstr( GetString( pos ), substr );
	if ( foundAt == TNULL ) return -1;

	return (TINT)( foundAt - m_pBuffer );
}

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

TString16 TString16::Format( const TWCHAR* a_pcFormat, ... )
{
	TWCHAR    buffer[ 0x400 ];
	TString16 buffer2;
	va_list   args;

	va_start( args, a_pcFormat );

	TINT iResult = _vsnwprintf( buffer, sizeof( buffer ), a_pcFormat, args );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	buffer2.Copy( buffer, -1 );

	return buffer2;
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

void TString16::FreeBuffer()
{
	if ( Length() != 0 ) m_pAllocator->Free( m_pBuffer );
	Reset();
}

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

TINT TString16::Compare( const TWCHAR* a_pcString, TINT param_2 ) const
{
	TASSERT( a_pcString != TNULL, "TCString::CompareNoCase - Passed string cannot be TNULL" );
	TASSERT( IsIndexValid( 0 ), "TCString::CompareNoCase - Index 0 is not valid" );
	TASSERT( GetString() != TNULL, "TCString::CompareNoCase - String cannot be TNULL" );

	if ( param_2 != -1 )
	{
		return wcsncmp( GetString(), a_pcString, param_2 );
	}

	const TWCHAR* str   = GetString();
	TWCHAR        bVar1 = 0;
	TWCHAR        bVar4 = 0;
	while ( TTRUE )
	{
		bVar1 = *str;
		bVar4 = bVar1 < *a_pcString;

		if ( bVar1 != *a_pcString ) break;
		if ( bVar1 == 0 ) return 0;

		bVar1 = str[ 1 ];
		bVar4 = bVar1 < a_pcString[ 1 ];

		if ( bVar1 != a_pcString[ 1 ] ) break;
		if ( bVar1 == 0 ) return 0;

		str += 2;
		a_pcString += 2;
	}
	return bVar4 | 1;
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
