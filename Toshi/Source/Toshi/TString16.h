#pragma once
#include "TString.h"
#include "T2Allocator.h"

namespace Toshi
{

class TString16
{
public:
    TString16();
    TString16( T2Allocator* allocator );
    TString16( TString16&& src ) noexcept;
    TString16( const TString16& src, T2Allocator* allocator = TNULL );
    TString16( const TWCHAR* src, T2Allocator* allocator = TNULL );
    TString16( TINT size, T2Allocator* allocator = TNULL );
    ~TString16() { FreeBuffer(); }

    void Copy( const TString16& src, TINT size = -1 ) { Copy( src.m_pBuffer, size ); }
    void Copy( const TWCHAR* src, TINT size = -1 );

    void FreeBuffer();

    // Returns TTRUE if allocated memory
    TBOOL AllocBuffer( TINT size, TBOOL freeMemory = TTRUE );

    static TString16 Format( const TWCHAR* a_pcFormat, ... );
    TString16&       VFormat( const TWCHAR* a_pcFormat, va_list a_vargs );

    void ForceSetData( TWCHAR* a_cString, TINT a_ilength );
    void UndoForceSetData() { Reset(); }

    TINT FindReverse( TWCHAR a_findChar, TINT pos = -1 ) const;

    void Truncate( TINT length );

    // Returns position of specified character
    TINT Find( TWCHAR character, TINT pos = 0 ) const;

    // Returns position of specified substring
    TINT Find( const TWCHAR* substr, TINT pos = 0 ) const;

    // Returns string starting from specified index
    const TWCHAR* GetString( TINT index = 0 ) const;

    // Returns string starting from specified a_iIndex. Use only if you are sure about what you do!
    TWCHAR* GetStringUnsafe( TINT a_iIndex = 0 );

    TString16& Concat( const TString16& str, TINT size = -1 ) { return Concat( str.m_pBuffer, size ); };
    TString16& Concat( const TWCHAR* src, TINT size = -1 );

    TINT Compare( const TWCHAR* a_wszString, TINT a_iLength = -1 ) const;
    TINT CompareNoCase( const TWCHAR* a_wszString, TINT a_iLength = -1 ) const;

    // Returns a substring contained in a [a_iFirst; a_iFirst + a_iCount] range
    TString16 Mid( TINT a_iFirst, TINT a_iCount ) const;

    // Returns a substring contained in a [a_iFrom; end of the string] range
    TString16 Right( TINT a_iFrom ) const;

    TString16& MakeUpper()
    {
        _wcsupr( m_pBuffer );
        return *this;
    }
    TString16& MakeLower()
    {
        _wcslwr( m_pBuffer );
        return *this;
    }

    TINT  Length() const { return m_iStrLen; }
    TUINT ExcessLength() const { return m_iExcessLen; }

    TBOOL IsAllLowerCase() const;
    TBOOL IsAllUpperCase() const;
    TBOOL IsIndexValid( TINT index ) const { return index >= 0 && index <= Length(); }
    TBOOL IsEmpty() const { return m_iStrLen == 0; }
    TBOOL IsUnicode() const { return TFALSE; } // Who would have known?

public:
    TString16 operator+( TWCHAR const* a_wszStr ) const
    {
        TString16 str = TString16( *this );
        return std::move( str.Concat( a_wszStr ) );
    }
    TString16* operator+=( TWCHAR const* a_wszStr )
    {
        Concat( a_wszStr, -1 );
        return this;
    }
    TString16* operator+=( TString16& str )
    {
        Concat( str, -1 );
        return this;
    }

    TWCHAR&       operator[]( TINT index ) { return m_pBuffer[ index ]; }
    const TWCHAR& operator[]( TINT index ) const { return *GetString( index ); }
    operator const TWCHAR*() const { return m_pBuffer; }

    TBOOL operator!() { return m_iStrLen == 0; }
    TBOOL operator==( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) == 0; }
    TBOOL operator==( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) == 0; }
    TBOOL operator!=( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) != 0; }
    TBOOL operator!=( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) != 0; }
    TBOOL operator<( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) > -1; };
    TBOOL operator<( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) > -1; };
    TBOOL operator<=( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) > 0; };
    TBOOL operator<=( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) > 0; };

    TString16& operator=( const TWCHAR* a_wszStr )
    {
        Copy( a_wszStr, -1 );
        return *this;
    };
    TString16& operator=( const TString16& str )
    {
        Copy( str, -1 );
        return *this;
    };

private:
    typedef T2Allocator* ( *func_DefaultAllocatorCB )();

    void Reset()
    {
        m_pBuffer    = NullWString;
        m_iStrLen    = 0;
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
    TWCHAR*      m_pBuffer        = NullWString; // 0x0
    TUINT32      m_iExcessLen : 8 = 0;           // 0x4
    TINT32       m_iStrLen : 24   = 0;           // 0x5
    T2Allocator* m_pAllocator;                   // 0x8
};

} // namespace Toshi
