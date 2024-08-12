#pragma once

namespace Toshi
{
class TStringTable
{
public:
    TStringTable( const TCHAR* const* strings, TINT numStrings )
    {
        m_Strings    = strings;
        m_NumStrings = numStrings;
    }

    const TCHAR* GetStringFromID( TINT id )
    {
        TASSERT( id >= 0 && id < m_NumStrings );
        return m_Strings[ id ];
    }

    TINT GetStringID( const TCHAR* string );

private:
    const TCHAR* const* m_Strings;
    TINT                m_NumStrings;
};
} // namespace Toshi