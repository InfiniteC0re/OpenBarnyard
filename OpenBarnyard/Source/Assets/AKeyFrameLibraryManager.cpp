#include "pch.h"
#include "AKeyFrameLibraryManager.h"
#include "ALoadScreen.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AKeyFrameLibraryManager );

AKeyFrameLibraryManager::AKeyFrameLibraryManager() :
    m_Libraries( AMemory::GetAllocator( AMemory::POOL_Misc ) ),
    m_LibrariesRefs( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
}

AKeyFrameLibraryManager::~AKeyFrameLibraryManager()
{
	UnloadAllLibraries();
}

void AKeyFrameLibraryManager::LoadLibrariesFromProperties( const PBPropertyValue* a_pArray, Toshi::TTRB* a_pTRB )
{
	TPROFILER_SCOPE();

	if ( TNULL != a_pArray )
	{
		auto pArray = a_pArray->GetArray();

		for ( TUINT i = 0; i < pArray->GetSize(); i++ )
		{
			LoadLibrary( pArray->GetValue( i )->GetString(), a_pTRB );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		g_oLoadScreen.Update( 1.0f, TTRUE );
	}
}

TBOOL AKeyFrameLibraryManager::LoadLibrary( const Toshi::TPString8& a_rLibraryName, TTRB* a_pTRB )
{
	TPROFILER_SCOPE();

	if ( m_Libraries.IsValid( m_Libraries.Find( a_rLibraryName ) ) )
	{
		// The library is already loaded
		// Increment reference count
		m_LibrariesRefs.Find( a_rLibraryName )->GetSecond() += 1;
		return TTRUE;
	}

	TKeyframeLibrary* pKeyFrameLibrary;
	auto              pRenderInterface = TRenderInterface::GetSingleton();
	auto              pKeyFrameLibMngr = &pRenderInterface->GetKeyframeLibraryManager();

	if ( TNULL == a_pTRB )
	{
	LoadFromModelsFolder:
		TString8 filePath;
		filePath.Format( "data/models/%s.tkl", a_rLibraryName.GetString() );

		pKeyFrameLibrary = pKeyFrameLibMngr->LoadLibrary( filePath );
	}
	else
	{
		TString8 symbolname;
		symbolname.Format( "%s_keylib", a_rLibraryName.GetString() );

		// If no symbol was found, load from models folder
		if ( TNULL == a_pTRB->GetSymbol( symbolname ) )
			goto LoadFromModelsFolder;

		pKeyFrameLibrary = pKeyFrameLibMngr->LoadLibrary( a_pTRB, symbolname );

		// If no success in loading library, try to load it from models folder
		if ( !pKeyFrameLibrary )
			goto LoadFromModelsFolder;
	}

	TVALIDPTR( pKeyFrameLibrary );

	if ( TNULL != pKeyFrameLibrary )
		m_Libraries.Insert( a_rLibraryName, pKeyFrameLibrary );

	m_LibrariesRefs.Insert( a_rLibraryName, 1 );

	return TTRUE;
}

TBOOL AKeyFrameLibraryManager::UnrefLibrary( const Toshi::TPString8& a_rLibraryName )
{
	TPROFILER_SCOPE();

	auto pResultNode = m_Libraries.Find( a_rLibraryName );

	if ( m_Libraries.IsValid( pResultNode ) )
	{
		auto pRefResultNode = m_LibrariesRefs.Find( pResultNode->GetFirst() );

		if ( pRefResultNode != m_LibrariesRefs.End() )
		{
			auto& rNumRefs = pResultNode->GetSecond();

			if ( rNumRefs != 0 )
				rNumRefs--;

			if ( rNumRefs == 0 )
			{
				UnloadLibrary( pResultNode );
				m_LibrariesRefs.Remove( pRefResultNode );
			}
		}
	}

	return TTRUE;
}

void AKeyFrameLibraryManager::UnloadAllLibraries()
{
	TPROFILER_SCOPE();

	for ( auto it = m_Libraries.Begin(); it != m_Libraries.End(); )
	{
		auto pNext = it.Next();
		UnloadLibrary( it );
		it = pNext;
	}

	m_Libraries.Clear();
	m_LibrariesRefs.Clear();
}

void AKeyFrameLibraryManager::UnloadLibrary( LibraryMap::Iterator& a_rLibrary )
{
	TPROFILER_SCOPE();

	if ( a_rLibrary != m_Libraries.End() )
	{
		auto pKeyFrameLibMngr = &TRenderInterface::GetSingleton()->GetKeyframeLibraryManager();
		pKeyFrameLibMngr->UnloadLibrary( a_rLibrary.GetValue()->GetSecond() );
		m_Libraries.Remove( a_rLibrary );
	}
}
