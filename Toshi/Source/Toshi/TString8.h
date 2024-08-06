#pragma once
#include "TString.h"
#include "T2Allocator.h"

namespace Toshi {

	class TString16;

	class TString8
	{
	public:
		TString8();
		TString8( T2Allocator* allocator );
		TString8( TString8&& src ) noexcept;
		TString8( const TString8& src, T2Allocator* allocator = TNULL );
		TString8( const TString16& src, T2Allocator* allocator = TNULL );
		TString8( const TCHAR* src, T2Allocator* allocator = TNULL );
		TString8( TINT size, T2Allocator* allocator = TNULL );
		~TString8() { FreeBuffer(); }

		void Copy( const TString8& src, TINT size = -1 ) { Copy( src.m_pBuffer, size ); }
		void Copy( const TString16& src, TINT size = -1 );
		void Copy( const TCHAR* src, TINT size = -1 );
		void Copy( const TWCHAR* src, TINT size = -1 );

		void FreeBuffer();

		// Returns TTRUE if allocated memory
		TBOOL AllocBuffer( TINT a_iLength, TBOOL a_bFreeMemory = TTRUE );

		TString8& Format( const TCHAR* a_pcFormat, ... );
		TString8& VFormat( const TCHAR* a_pcFormat, va_list a_vargs );

		static TString8 VarArgs( const TCHAR* a_pcFormat, ... );

		void UndoForceSetData() { Reset(); }
		void ForceSetData( TCHAR* a_pchString, TINT a_iLength );

		TINT FindReverse( TCHAR a_findChar, TINT pos = -1 ) const;

		void Truncate( TINT length );

		// Returns position of specified character
		TINT Find( TCHAR character, TINT pos = 0 ) const;

		// Returns position of specified substring
		TINT Find( const TCHAR* substr, TINT pos = 0 ) const;

		// Returns const string starting from specified a_iIndex
		const TCHAR* GetString( TINT a_iIndex = 0 ) const;

		// Returns string starting from specified a_iIndex. Use only if you are sure about what you do!
		TCHAR* GetStringUnsafe( TINT a_iIndex = 0 );

		TString8& Concat( const TString8& str, TINT size = -1 ) { return Concat( str.m_pBuffer, size ); };
		TString8& Concat( const TString16& src, TINT size = -1 );
		TString8& Concat( const TCHAR* src, TINT size = -1 );

		TINT Compare( const TCHAR* a_szString, TINT a_iLength = -1 ) const;
		TINT CompareNoCase( const TCHAR* a_szString, TINT a_iLength = -1 ) const;

		// Returns a substring contained in a [a_iFirst; a_iFirst + a_iCount] range
		TString8 Mid( TINT a_iFirst, TINT a_iCount ) const;

		// Returns a substring contained in a [a_iFrom; end of the string] range
		TString8 Right( TINT a_iFrom ) const;

		TString8& MakeUpper() { _strupr( m_pBuffer ); return *this; }
		TString8& MakeLower() { _strlwr( m_pBuffer ); return *this; }

		TINT Length() const { return m_iStrLen; }
		TUINT ExcessLength() const { return m_iExcessLen; }

		TBOOL IsAllLowerCase() const;
		TBOOL IsAllUpperCase() const;
		TBOOL IsIndexValid( TINT a_iIndex ) const { return a_iIndex >= 0 && a_iIndex <= Length(); }
		TBOOL IsEmpty() const { return m_iStrLen == 0; }
		TBOOL IsUnicode() const { return TFALSE; } // Who would have known?

	public:
		TString8 operator+( TCHAR const* cstr ) const { TString8 str = TString8( *this ); return std::move( str.Concat( cstr ) ); }
		TString8* operator+=( TCHAR const* cstr ) { Concat( cstr, -1 ); return this; }
		TString8* operator+=( TString8& str ) { Concat( str, -1 ); return this; }

		TCHAR& operator[]( TINT a_iIndex ) { return m_pBuffer[ a_iIndex ]; }
		const TCHAR& operator[]( TINT a_iIndex ) const { return *GetString( a_iIndex ); }

		operator const TCHAR* ( ) const { return m_pBuffer; }
		operator const TBOOL() const { return !IsEmpty(); }

		TBOOL operator!() { return m_iStrLen == 0; }
		TBOOL operator==( const TCHAR* cstr ) const { return Compare( cstr, -1 ) == 0; }
		TBOOL operator==( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) == 0; }
		TBOOL operator!=( const TCHAR* cstr ) const { return Compare( cstr, -1 ) != 0; }
		TBOOL operator!=( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) != 0; }
		TBOOL operator<( const TCHAR* cstr ) const { return Compare( cstr, -1 ) > -1; };
		TBOOL operator<( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) > -1; };
		TBOOL operator<=( const TCHAR* cstr ) const { return Compare( cstr, -1 ) > 0; };
		TBOOL operator<=( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) > 0; };

		TString8& operator=( const TCHAR* cstr ) { Copy( cstr, -1 ); return *this; };
		TString8& operator=( const TWCHAR* wcstr ) { Copy( wcstr, -1 ); return *this; };
		TString8& operator=( const TString8& str ) { Copy( str, -1 ); return *this; };
		TString8& operator=( const TString16& str ) { Copy( str, -1 ); return *this; };

	private:
		typedef T2Allocator* ( *func_DefaultAllocatorCB )( );

		void Reset()
		{
			m_pBuffer = NullString;
			m_iStrLen = 0;
			m_iExcessLen = 0;
		}

		T2Allocator* GetAllocator()
		{
			return sm_pDefaultAllocatorCB();
		}

		static T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	private:
		static inline func_DefaultAllocatorCB sm_pDefaultAllocatorCB = &GetDefaultAllocatorCB;

	private:
		TCHAR* m_pBuffer = NullString; // 0x0
		TUINT32 m_iExcessLen : 8 = 0;  // 0x4
		TINT32 m_iStrLen : 24 = 0;     // 0x5
		T2Allocator* m_pAllocator;     // 0x8
	};

}
