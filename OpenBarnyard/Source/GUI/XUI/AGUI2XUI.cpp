#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUI.h"
#  include "AGUI2XUIResource.h"
#  include "AGUI2XUIScene.h"
#  include "AGUI2XUIVisual.h"
#  include "GUI/AGUI2.h"
#  include "Assets/AMaterialLibraryManager.h"

#  include <GUI/T2GUIFontManager.h>
#  include <Toshi/T2String8.h>
#  include <Toshi/T2String16.h>
#  include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $BarnyardWiiJP: FUNCTION 80415e58
AGUI2XUI::AGUI2XUI()
{
	m_pMainResource = TNULL;
}

// $BarnyardWiiJP: FUNCTION 80415eb4
AGUI2XUI::~AGUI2XUI()
{
	while ( !m_llScenes.IsEmpty() )
	{
		SceneNode* pNode = m_llScenes.PopBack();

		if ( pNode )
		{
			if ( pNode->pScene )
				delete pNode->pScene;

			delete pNode;
		}
	}

	for ( auto it = m_mapUnk.Begin(); it != m_mapUnk.End(); it++ )
	{
		if ( it->GetSecond() )
			delete it->GetSecond();
	}

	m_mapUnk.Clear();

	// NOTE: the resources aren't deleted here, see DeleteResources
}

// $BarnyardWiiJP: FUNCTION 80415fcc
void AGUI2XUI::AddScene( AGUI2XUIScene* a_pScene )
{
	SceneNode* pNode = new SceneNode();
	pNode->pScene    = a_pScene;

	m_llScenes.PushBack( pNode );
}

// $BarnyardWiiJP: FUNCTION 8041603c
void AGUI2XUI::DeleteScene( AGUI2XUIScene* a_pScene )
{
	for ( auto it = m_llScenes.Begin(); it != m_llScenes.End(); )
	{
		SceneNode* pNode = it.Get();
		it++;

		if ( pNode->pScene == a_pScene )
		{
			pNode->Remove();

			if ( pNode )
			{
				if ( pNode->pScene )
					delete pNode->pScene;

				delete pNode;
			}

			return;
		}
	}
}

// $BarnyardWiiJP: FUNCTION 80416108
AGUI2XUIVisual* AGUI2XUI::CreateVisual( const TWCHAR* a_wszName )
{
	if ( m_pMainResource )
		return m_pMainResource->CreateVisual( a_wszName );

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 80416120
void AGUI2XUI::SetMainResource( AGUI2XUIResource* a_pResource )
{
	if ( m_pMainResource )
	{
		while ( !m_mapUnk.IsEmpty() )
		{
			auto it = m_mapUnk.Begin();

			if ( it->GetSecond() )
				delete it->GetSecond();

			m_mapUnk.Remove( it );
		}

		delete m_pMainResource;
	}

	m_pMainResource = a_pResource;
}

// $BarnyardWiiJP: FUNCTION 804161cc
void AGUI2XUI::AddResource( const TCHAR* a_szName, AGUI2XUIResource* a_pResource )
{
	ResourceNode* pNode = new ResourceNode();
	pNode->pResource    = a_pResource;
	pNode->strName      = a_szName;

	m_llResources.PushBack( pNode );
}

// $BarnyardWiiJP: FUNCTION 80416260
void AGUI2XUI::DeleteResources()
{
	while ( !m_llResources.IsEmpty() )
	{
		ResourceNode* pNode = m_llResources.PopBack();

		if ( pNode )
		{
			if ( pNode->pResource )
				delete pNode->pResource;

			delete pNode;
		}
	}

	if ( m_pMainResource )
	{
		delete m_pMainResource;
		m_pMainResource = TNULL;
	}
}

// $BarnyardWiiJP: FUNCTION 804162fc
void AGUI2XUI::UnloadMaterialLibraries()
{
	while ( !m_llMaterialLibs.IsEmpty() )
	{
		NameNode* pNode = m_llMaterialLibs.PopBack();

		if ( pNode )
		{
			AMaterialLibraryManager::GetSingleton()->UnrefLibrary( pNode->strName, TTRUE );
			delete pNode;
		}
	}
}

// $BarnyardWiiJP: FUNCTION 804163c4
void AGUI2XUI::AddMaterialLibrary( const TPString8& a_rcName )
{
	NameNode* pNode = new NameNode();
	pNode->strName  = a_rcName;

	m_llMaterialLibs.PushBack( pNode );
}

// $BarnyardWiiJP: FUNCTION 8041648c
void AGUI2XUI::AddFont( const TPString8& a_rcName )
{
	NameNode* pNode = new NameNode();
	pNode->strName  = a_rcName;

	m_llFonts.PushBack( pNode );
}

// $BarnyardWiiJP: FUNCTION 80416554
void AGUI2XUI::CloseFonts()
{
	while ( !m_llFonts.IsEmpty() )
	{
		NameNode* pNode = m_llFonts.PopBack();

		if ( pNode )
		{
			T2GUIFontManager::Close( pNode->strName.GetString8().GetString() );
			delete pNode;
		}
	}
}

// $BarnyardWiiJP: FUNCTION 80416638
AGUI2XUIResource* AGUI2XUI::FindResource( const TCHAR* a_szName )
{
	for ( auto it = m_llResources.Begin(); it != m_llResources.End(); it++ )
	{
		if ( T2String8::Compare( a_szName, it->strName.GetString() ) == 0 )
			return it->pResource;
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 804166c8
AGUI2XUIScene* AGUI2XUI::FindScene( const TWCHAR* a_wszId )
{
	for ( auto it = m_llScenes.Begin(); it != m_llScenes.End(); it++ )
	{
		if ( T2String16::Compare( a_wszId, it->pScene->GetId() ) == 0 )
			return it->pScene;
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8041675c
void AGUI2XUI::DeleteScenes()
{
	while ( !m_llScenes.IsEmpty() )
	{
		SceneNode* pNode = m_llScenes.PopBack();

		if ( pNode )
		{
			if ( pNode->pScene )
				delete pNode->pScene;

			delete pNode;
		}
	}
}

// $BarnyardWiiJP: FUNCTION 8041a924
AGUI2XUIResource* AGUI2XUI::LoadResource( const TCHAR* a_szName, AGUI2XUI* a_pXUI, TBOOL a_bMainResource, TBOOL a_bUnused, TTRB* a_pTRB )
{
	if ( !a_szName )
		return TNULL;

	AGUI2XUIResource* pResource  = new AGUI2XUIResource();
	const TCHAR*      szLanguage = AGUI2::GetLanguage();

	TCHAR szStringTablePath[ 256 ];
	TCHAR szResourcePath[ 280 ];
	TStringManager::String8Format( szStringTablePath, sizeof( szStringTablePath ), "Data/XUI/StringTables/%s/%s_xus.trb", szLanguage, a_szName );
	TStringManager::String8Format( szResourcePath, sizeof( szResourcePath ), "Data/XUI/%s/%s.trb", szLanguage, a_szName );

	if ( pResource->Load( szResourcePath, szStringTablePath, TTRUE, a_pTRB ) )
	{
		if ( a_bMainResource == TTRUE )
			a_pXUI->SetMainResource( pResource );
		else
			a_pXUI->AddResource( a_szName, pResource );

		return pResource;
	}

	delete pResource;
	return TNULL;
}

#endif // BARNYARD_LATEST
