#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{

TKeyframeLibrary* TKeyframeLibraryManager::GetLibrary( const TCHAR* a_szName )
{
    for ( auto it = m_List.Begin(); it != m_List.End(); ++it )
    {
        if ( TStringManager::String8CompareNoCase( it->GetName(), a_szName ) == 0 )
        {
            return it;
        }
    }

    return TNULL;
}

TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary( const TCHAR* a_szFilePath )
{
    TVALIDPTR( a_szFilePath );

    auto pTRB = new TTRB();

    if ( pTRB->Load( a_szFilePath, 0 ) != TTRB::ERROR_OK )
    {
        delete pTRB;
        return TNULL;
    }

    TKeyframeLibrary* pLibrary = TKeyframeLibrary::CreateFromTRB( pTRB, "keylib" );

    m_List.InsertHead( pLibrary );
    m_iNumLibraries++;

    pLibrary->SetTRB( pTRB );
    return pLibrary;
}

TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary( TTRB* a_pTRB, const TCHAR* a_szSymbolName )
{
    TKeyframeLibrary* pLibrary = TKeyframeLibrary::CreateFromTRB( a_pTRB, a_szSymbolName );

    m_List.InsertHead( pLibrary );
    m_iNumLibraries++;

    return pLibrary;
}

void TKeyframeLibraryManager::UnloadLibrary( TKeyframeLibrary* a_pLibrary )
{
    TVALIDPTR( a_pLibrary );

    a_pLibrary->Remove();
    a_pLibrary->Delete();
    m_iNumLibraries--;
}

} // namespace Toshi
