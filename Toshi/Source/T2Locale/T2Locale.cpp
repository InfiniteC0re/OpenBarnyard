#include "ToshiPCH.h"
#include "T2Locale.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
T2Locale::T2Locale( TINT langCount, size_t bufferSize, void* buffer )
{
    // 00662d40
    m_pBuffer     = buffer;
    m_BufferPos   = buffer;
    m_StringTable = TNULL;
    m_LangId      = -1;
    m_bOwnsBuffer = TFALSE;

    if ( m_pBuffer == TNULL )
    {
        size_t symbolTableSize = TTRB::GetSymbolTableSize( 1 );
        size_t headerSize      = TTRB::GetHeaderSize( 1 );
        m_pBuffer              = TMalloc( headerSize + bufferSize + symbolTableSize );
        m_BufferPos            = m_pBuffer;
        m_bOwnsBuffer          = TTRUE;
    }

    m_Locale.SetMemoryFunctions( TRBAllocator, TRBDeallocator, this );
}

T2Locale::~T2Locale()
{
    m_Locale.Close();

    if ( m_bOwnsBuffer )
    {
        TFree( m_pBuffer );
    }
}

void* T2Locale::TRBAllocator( TTRB::AllocType alloctype, size_t size, short unk, size_t unk2, void* userData )
{
    return TSTATICCAST( T2Locale, userData )->TRBAlloc( size );
}

void T2Locale::TRBDeallocator( TTRB::AllocType alloctype, void* ptr, short unk, size_t unk2, void* userData )
{
    // T2Locale doesn't have deallocator
}

void T2Locale::SetLanguage( Lang langid )
{
    // 00662e30
    if ( langid != m_LangId && ( m_Locale.Close(), -1 < langid ) )
    {
        m_Locale.Load( GetLanguageFilename( langid ) );
        m_StringTable = m_Locale.CastSymbol< LocaleStrings >( "LocaleStrings" );

        m_BufferPos = m_pBuffer;
        m_LangId    = langid;
    }
}

Toshi::T2Locale::Lang T2Locale::GetLanguage() const
{
    return m_LangId;
}

TINT T2Locale::GetNumStrings() const
{
    return m_StringTable->m_numstrings;
}

T2LocalisedString T2Locale::GetString( TINT a_iNumString )
{
    TASSERT( a_iNumString >= 0 && a_iNumString < GetNumStrings() );
    return m_StringTable->Strings[ a_iNumString ];
}

} // namespace Toshi
