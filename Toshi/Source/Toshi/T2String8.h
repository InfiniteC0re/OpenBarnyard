#pragma once
#include "Toshi/T2Allocator.h"
#include "Math/TMathInline.h"

#include <stdlib.h>

namespace Toshi {

	//-----------------------------------------------------------------------------
	// Purpose: Stores all common methods that let you operate with TCHAR* strings
	//-----------------------------------------------------------------------------
	class T2String8
	{
	public:
		static constexpr TSIZE SCRATCH_MEM_SIZE = 2048;

	public:
		static TINT FormatV( TCHAR* a_pcString, TINT size, const TCHAR* a_pcFormat, va_list args );
		static TINT FormatV( TCHAR* a_pcString, const TCHAR* a_pcFormat, va_list args );

		static TINT Format( TCHAR* a_pcString, TINT size, const TCHAR* a_pcFormat, ... );
		static TINT Format( TCHAR* a_pcString, const TCHAR* a_pcFormat, ... );

		static TINT Compare( const TCHAR* str1, const TCHAR* str2, TSIZE size );
		static TINT CompareNoCase( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 );

		static TCHAR* Copy( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );
		static TCHAR* CopySafe( TCHAR* dst, const TCHAR* src, TSIZE size );

		static TCHAR* Concat( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );

		static TCHAR* FindChar( TCHAR* str, TCHAR character );
		static const TCHAR* FindChar( const TCHAR* str, TCHAR character );
		static const TCHAR* FindString( const TCHAR* str, const TCHAR* substr );

		static TSIZE Length( const TCHAR* str );

		static TBOOL IsLowerCase( const TCHAR* str );
		static TBOOL IsUpperCase( const TCHAR* str );

		static void ToLowerCase( TCHAR* str );
		static void ToUpperCase( TCHAR* str );

		static void IntToString( TINT value, TCHAR* dst, TINT unused, TINT radix );
		static void IntToString( TINT value, TCHAR* dst, TINT radix );

		static TINT StringToInt( const TCHAR* src );
		static TFLOAT StringToFloat( const TCHAR* src );

		static TCHAR* SkipSpaces( TCHAR* str );
		static const TCHAR* SkipSpaces( const TCHAR* str );

		static T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	public:
		static inline T2Allocator* sm_pDefaultAllocatorCB = GetDefaultAllocatorCB();
		static TCHAR ms_aScratchMem[ SCRATCH_MEM_SIZE ];
	};

	//-----------------------------------------------------------------------------
	// Purpose: Wrapper for const TCHAR* strings that provides an interface
	// to make it easier to work with them.
	//-----------------------------------------------------------------------------
	class T2ConstString8
	{
	public:
		TFORCEINLINE constexpr T2ConstString8( const TCHAR* a_pszString ) :
			m_pszString( a_pszString )
		{ }

		TFORCEINLINE constexpr T2ConstString8() :
			m_pszString( TNULL )
		{ }

		TFORCEINLINE constexpr T2ConstString8( const T2ConstString8& a_rOther ) :
			m_pszString( a_rOther.m_pszString )
		{ }

		~T2ConstString8() = default;

		TFORCEINLINE TSIZE Length() const { return T2String8::Length( m_pszString ); }
		TFORCEINLINE TBOOL IsLowerCase() const { return T2String8::IsLowerCase( m_pszString ); }
		TFORCEINLINE TBOOL IsUpperCase() const { return T2String8::IsUpperCase( m_pszString ); }

		TFORCEINLINE const TCHAR* FindChar( TCHAR a_cCharacter ) { return T2String8::FindChar( m_pszString, a_cCharacter ); }
		TFORCEINLINE const TCHAR* FindString( const TCHAR* a_pszSubstr ) { return T2String8::FindString( m_pszString, a_pszSubstr ); }
		TFORCEINLINE const TCHAR* SkipSpaces() { return T2String8::SkipSpaces( m_pszString ); }

		TFORCEINLINE constexpr T2ConstString8& operator=( const TCHAR* a_pszString )
		{
			m_pszString = a_pszString;
			return *this;
		}

		TFORCEINLINE constexpr T2ConstString8 operator+( int a_iSize ) const { return T2ConstString8( m_pszString + a_iSize ); }
		TFORCEINLINE constexpr T2ConstString8 operator-( int a_iSize ) const { return T2ConstString8( m_pszString - a_iSize ); }
		TFORCEINLINE constexpr T2ConstString8 operator+=( int a_iSize ) { m_pszString += a_iSize; return *this; }
		TFORCEINLINE constexpr T2ConstString8 operator-=( int a_iSize ) { m_pszString -= a_iSize; return *this; }

		TFORCEINLINE constexpr operator const char* ( ) const { return m_pszString; }

	private:
		const TCHAR* m_pszString;
	};
}