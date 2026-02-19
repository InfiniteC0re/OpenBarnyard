#include "pch.h"
#include "AMaterialLibraryManager.h"

#include <Render/TRenderInterface.h>
#include <BYardSDK/SDKHooks.h>

TOSHI_NAMESPACE_USING

void AMaterialLibraryManager::LoadLibrariesFromProperties( const PBPropertyValue* a_pProperty, Toshi::TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials )
{
	CALL_THIS( 0x00613ee0, AMaterialLibraryManager*, void, this, const PBPropertyValue*, a_pProperty, Toshi::TTRB*, a_pTRB, TBOOL, a_bUpdateGUIMaterials );
}

void AMaterialLibraryManager::LoadLibrary( const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI )
{
	CALL_THIS( 0x00614a00, AMaterialLibraryManager*, void, this, const Toshi::TPString8&, a_rLibName, Toshi::TTRB*, a_pTRB, TBOOL, a_bIsGUI );
}

void AMaterialLibraryManager::UnrefLibrary( const TPString8& a_rLibName, TBOOL a_bUnused )
{
	CALL_THIS( 0x006142c0, AMaterialLibraryManager*, void, this, const TPString8&, a_rLibName, TBOOL, a_bUnused );
}

void AMaterialLibraryManager::DestroyLibrary( LibrariesMap::Node*& a_rpMaterialLibraryNode, TBOOL a_bUpdateGUIMaterials )
{
	CALL_THIS( 0x00614240, AMaterialLibraryManager*, void, this, LibrariesMap::Node*&, a_rpMaterialLibraryNode, TBOOL, a_bUpdateGUIMaterials );
}

void AMaterialLibraryManager::UnloadTexturesOfLibrary( AMaterialLibrary* a_pMaterialLibrary )
{
	CALL_THIS( 0x00613d90, AMaterialLibraryManager*, void, this, AMaterialLibrary*, a_pMaterialLibrary );
}

void AMaterialLibraryManager::CreateTextures( AMaterialLibrary* a_pMatLibrary )
{
	CALL_THIS( 0x00613b50, AMaterialLibraryManager*, void, this, AMaterialLibrary*, a_pMatLibrary );
}

void AMaterialLibraryManager::OnLibraryLoaded( TBOOL a_bUpdateGUIMaterials )
{
	CALL_THIS( 0x00613b20, AMaterialLibraryManager*, void, this, TBOOL, a_bUpdateGUIMaterials );
}

Toshi::TTexture* AMaterialLibraryManager::FindTexture( const char* a_szTextureName )
{
	for ( auto it = m_UsedTextures.Begin(); it != m_UsedTextures.End(); it = it->Next() )
	{
		if ( Toshi::TStringManager::String8CompareNoCase( it->GetName(), a_szTextureName ) == 0 )
		{
			return it->GetTexture();
		}
	}

	return TNULL;
}

AMaterialLibrary* AMaterialLibraryManager::List::CreateLibraryFromTRB( Toshi::TTRB* a_pTRB, const char* a_szFilePath )
{
	auto pLibrary = new AMaterialLibrary;
	pLibrary->SetPath( a_szFilePath );

	if ( !pLibrary->LoadTRBFile( a_pTRB ) )
	{
		delete pLibrary;
		return TNULL;
	}

	m_Libraries.PushBack( pLibrary );
	return pLibrary;
}

AMaterialLibrary* AMaterialLibraryManager::List::CreateLibraryFromAsset( const char* a_szFilePath, Toshi::TTRB* a_pTRB )
{
	return CALL_THIS( 0x00615930, AMaterialLibraryManager::List*, AMaterialLibrary*, this, const char*, a_szFilePath, Toshi::TTRB*, a_pTRB );
}

ATexture* AMaterialLibraryManager::List::FindTexture( const char* a_szTextureName, AMaterialLibrary** a_ppMaterialLibrary, TINT* a_pTextureIndex )
{
	TINT iIndex = -1;
	auto it     = m_Libraries.Begin();

	while ( true )
	{
		if ( it == m_Libraries.End() )
		{
			return TNULL;
		}

		iIndex = it->FindTextureIndex( a_szTextureName );
		if ( iIndex != -1 ) break;

		it++;
	}

	if ( a_ppMaterialLibrary )
	{
		*a_ppMaterialLibrary = it;
	}

	if ( a_pTextureIndex )
	{
		*a_pTextureIndex = iIndex;
	}

	return it->GetTexture( iIndex );
}
