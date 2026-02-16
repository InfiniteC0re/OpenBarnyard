#include "pch.h"
#include <Render/TRenderInterface.h>
#include <File/TTRB.h>

#include "GUI/AGUISystem.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "AMaterialLibraryManager.h"
#include "Locale/ALocaleManager.h"
#include "ALoadScreen.h"
#include "AModelLoader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AMaterialLibraryManager );

// $Barnyard: FUNCTION 00613c10
AMaterialLibraryManager::AMaterialLibraryManager()
    : m_NumRefLibraries( AMemory::GetAllocator( AMemory::POOL_Misc ) ), m_LoadedLibraries( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
	List::CreateSingleton();

	m_iNumUsedTextures = 0;
	m_iNumFreeTextures = 0;

	for ( TUINT i = 0; i < MAX_NUM_TEXTURES; i++ )
	{
		m_FreeTextures.PushFront( &m_aSlots[ i ] );
		m_iNumFreeTextures++;
	}
}

void AMaterialLibraryManager::LoadLibrariesFromProperties( const PBPropertyValue* a_pProperty, TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials )
{
	if ( a_pProperty )
	{
		auto pArray = a_pProperty->GetArray();

		for ( TUINT i = 0; i < pArray->GetSize(); i++ )
		{
			auto matlibName = pArray->GetValue( i )->GetTPString8();
			auto eLangId    = g_pLocaleManager->GetLanguage();

			TBOOL bLocaliseAsset;

			if ( eLangId == ALocaleManager::Lang_English || eLangId == ALocaleManager::Lang_EnglishUK )
			{
				bLocaliseAsset = TFALSE;
			}
			else
			{
				bLocaliseAsset = TPString8( "gui_loc" ) == matlibName;
			}

			if ( bLocaliseAsset )
			{
				TString8 localisedAssetName = matlibName.GetString8();
				localisedAssetName += "_";
				localisedAssetName += ALocaleManager::Interface()->GetCurrentLanguageName();
				localisedAssetName.MakeLower();

				matlibName = TPString8( localisedAssetName );
			}

			LoadLibrary( matlibName, a_pTRB, TFALSE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		TRenderInterface::GetSingleton()->FlushDyingResources();
		TRenderInterface::GetSingleton()->FlushDyingResources();

		if ( a_bUpdateGUIMaterials && g_pGUISystem )
		{
			T2GUITextureSectionManager::UpdateMaterials();
		}

		g_oLoadScreen.Update( 1.0f, TTRUE );
	}
}

// $Barnyard: FUNCTION 00614580
void AMaterialLibraryManager::UnloadLibrariesFromProperties( const PBPropertyValue* a_pProperty, TBOOL a_bUpdateGUIMaterials )
{
	if ( a_pProperty )
	{
		auto pArray = a_pProperty->GetArray();

		for ( TUINT i = 0; i < pArray->GetSize(); i++ )
		{
			auto matlibName = pArray->GetValue( i )->GetTPString8();

			UnloadLibrary( matlibName, TFALSE );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}

		TRenderInterface::GetSingleton()->FlushDyingResources();
		TRenderInterface::GetSingleton()->FlushDyingResources();

		if ( a_bUpdateGUIMaterials && g_pGUISystem )
		{
			T2GUITextureSectionManager::UpdateMaterials();
		}
	}
}

// $Barnyard: FUNCTION 00614a00
void AMaterialLibraryManager::LoadLibrary( const TPString8& a_rLibName, TTRB* a_pTRB, TBOOL a_bIsGUI )
{
	auto pRenderer = TRenderInterface::GetSingleton();

	if ( !m_LoadedLibraries.IsValid( m_LoadedLibraries.Find( a_rLibName ) ) )
	{
		TString8 fileMiddle = "";
		TString8 fileFormat = "";

		if ( !pRenderer->Supports32BitTextures() )
		{
			fileFormat = "16";
		}

		TCHAR matlibPath[ 128 ];
		TStringManager::String8Format(
		    matlibPath,
		    sizeof( matlibPath ),
		    "data/matlibs/%s%s%s.ttl",
		    a_rLibName.GetPooledString()->GetString8().GetString(),
		    fileMiddle.GetString(),
		    fileFormat.GetString()
		);

		AMaterialLibrary* pLibrary = List::GetSingleton()->CreateLibraryFromAsset( matlibPath, a_pTRB );

		if ( pLibrary )
		{
			m_LoadedLibraries.Insert( a_rLibName, pLibrary );
			CreateTextures( pLibrary );
			m_NumRefLibraries.Insert( a_rLibName, 1 );

			if ( a_rLibName.GetString8().Find( "gui_" ) == 0 )
			{
				a_bIsGUI = TTRUE;
			}

			OnLibraryLoaded( a_bIsGUI );
		}
	}
	else
	{
		auto pRefCount = m_NumRefLibraries.Find( a_rLibName );
		pRefCount->GetSecond() += 1;
	}
}

// $Barnyard: FUNCTION 006142c0
void AMaterialLibraryManager::UnloadLibrary( const TPString8& a_rLibName, TBOOL a_bUnused )
{
	TPString8 matlibName = a_rLibName;
	auto      eLangId    = g_pLocaleManager->GetLanguage();

	TBOOL bLocaliseAsset;

	if ( eLangId == ALocaleManager::Lang_English || eLangId == ALocaleManager::Lang_EnglishUK )
	{
		bLocaliseAsset = TFALSE;
	}
	else
	{
		bLocaliseAsset = TPString8( "gui_loc" ) == matlibName;
	}

	if ( bLocaliseAsset )
	{
		TString8 localisedAssetName = matlibName.GetString8();
		localisedAssetName += "_";
		localisedAssetName += ALocaleManager::Interface()->GetCurrentLanguageName();
		localisedAssetName.MakeLower();

		matlibName = TPString8( localisedAssetName );
	}

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	auto pMatlibNode = m_LoadedLibraries.Find( matlibName );

	if ( pMatlibNode != m_LoadedLibraries.End() )
	{
		auto pRefCountNode = m_NumRefLibraries.Find( matlibName );

		if ( m_NumRefLibraries.IsValid( pRefCountNode ) )
		{
			pRefCountNode->GetSecond() -= 1;

			if ( pRefCountNode->GetSecond() == 0 )
			{
				DestroyLibrary( pMatlibNode, a_bUnused );
				m_NumRefLibraries.Remove( pRefCountNode );
			}
		}
	}
}

// $Barnyard: FUNCTION 00614240
void AMaterialLibraryManager::DestroyLibrary( LibrariesMap::Iterator& a_rcMaterialLibraryNode, TBOOL a_bUpdateGUIMaterials )
{
	auto pMaterialLibrary = a_rcMaterialLibraryNode->GetSecond();
	UnloadTexturesOfLibrary( pMaterialLibrary );

	pMaterialLibrary->Destroy();
	m_LoadedLibraries.Remove( a_rcMaterialLibraryNode );

	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( a_bUpdateGUIMaterials && g_pGUISystem )
	{
		T2GUITextureSectionManager::UpdateMaterials();
	}
}

// $Barnyard: FUNCTION 00613d90
void AMaterialLibraryManager::UnloadTexturesOfLibrary( AMaterialLibrary* a_pMaterialLibrary )
{
	for ( auto it = m_UsedTextures.Begin(); it != m_UsedTextures.End(); )
	{
		auto pNextNode = it->Next();

		if ( it->GetLibrary() == a_pMaterialLibrary )
		{
			m_UsedTextures.Erase( it );
			m_iNumUsedTextures -= 1;

			it->GetTexture()->DestroyResource();
			it->SetTexture( TNULL );
			it->ResetName();

			m_FreeTextures.PushFront( it );
			m_iNumFreeTextures += 1;
		}

		it = pNextNode;
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();
	AModelLoader::ResolveTextures();
}

// $Barnyard: FUNCTION 00613b50
void AMaterialLibraryManager::CreateTextures( AMaterialLibrary* a_pMatLibrary )
{
	auto pTextureFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );

	for ( TINT i = 0; i < a_pMatLibrary->GetNumTextures(); i++ )
	{
		auto pMatlibTexture = a_pMatLibrary->GetTexture( i );

		auto pTexSlot = m_FreeTextures.PopFront();
		m_iNumFreeTextures -= 1;

		pTexSlot->SetName( pMatlibTexture->Name );
		pTexSlot->SetLibrary( a_pMatLibrary );
		pTexSlot->SetTexture( pTextureFactory->CreateFromT2Texture( pMatlibTexture->pTexture ) );

		m_UsedTextures.PushFront( pTexSlot );
		m_iNumUsedTextures += 1;
	}
}

// $Barnyard: FUNCTION 00613b20
void AMaterialLibraryManager::OnLibraryLoaded( TBOOL a_bUpdateGUIMaterials )
{
	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	if ( a_bUpdateGUIMaterials && g_pGUISystem )
	{
		T2GUITextureSectionManager::UpdateMaterials();
	}
}

// $Barnyard: FUNCTION 00613d20
TTexture* AMaterialLibraryManager::FindTexture( const TCHAR* a_szTextureName )
{
	for ( auto it = m_UsedTextures.Begin(); it != m_UsedTextures.End(); it = it->Next() )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName(), a_szTextureName ) == 0 )
		{
			return it->GetTexture();
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00615a90
AMaterialLibrary* AMaterialLibraryManager::List::CreateLibraryFromTRB( TTRB* a_pTRB, const TCHAR* a_szFilePath )
{
	TPROFILER_SCOPE();

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

// $Barnyard: FUNCTION 00615930
AMaterialLibrary* AMaterialLibraryManager::List::CreateLibraryFromAsset( const TCHAR* a_szFilePath, TTRB* a_pTRB )
{
	TPROFILER_SCOPE();

	auto pLibrary = new AMaterialLibrary;
	pLibrary->SetPath( a_szFilePath );

	TBOOL bSuccess;

	if ( a_pTRB )
	{
		auto iFilePathLength = TStringManager::String8Length( a_szFilePath );
		auto iFileNamePos    = iFilePathLength - 1;

		while ( a_szFilePath[ iFileNamePos ] != '\\' && a_szFilePath[ iFileNamePos ] != '/' )
			iFileNamePos--;

		iFileNamePos++;

		TCHAR symbolName[ 132 ];
		auto  iLen = iFilePathLength - iFileNamePos - 4;
		TStringManager::String8Copy( symbolName, a_szFilePath + iFileNamePos, iFilePathLength - iFileNamePos - 4 );

		symbolName[ iLen ]     = '_';
		symbolName[ iLen + 1 ] = '\0';
		TStringManager::String8ToLowerCase( symbolName );
		TStringManager::String8Copy( symbolName + iLen + 1, "TTL" );

		auto pTTLData = a_pTRB->GetSymbolAddress( symbolName );

		if ( pTTLData )
		{
			bSuccess = pLibrary->LoadTTLData( pTTLData );
		}
		else
		{
			bSuccess = pLibrary->LoadTTLFile( a_szFilePath );
		}
	}
	else
	{
		bSuccess = pLibrary->LoadTTLFile( a_szFilePath );
	}

	if ( !bSuccess )
	{
		if ( pLibrary )
		{
			delete pLibrary;
			pLibrary = TNULL;
		}
	}
	else
	{
		m_Libraries.PushBack( pLibrary );
	}

	return pLibrary;
}

// $Barnyard: FUNCTION 00615900
void AMaterialLibraryManager::List::DestroyLibrary( AMaterialLibrary* a_pLibrary )
{
	TVALIDPTR( a_pLibrary );
	a_pLibrary->Destroy();
}

// $Barnyard: FUNCTION 00615b40
ATexture* AMaterialLibraryManager::List::FindTexture( const TCHAR* a_szTextureName, AMaterialLibrary** a_ppMaterialLibrary, TINT* a_pTextureIndex )
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
