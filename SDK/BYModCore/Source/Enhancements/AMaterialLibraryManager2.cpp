#include "pch.h"
#include "AMaterialLibraryManager2.h"

#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/SDKHooks.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AMaterialLibraryManager2 );

AMaterialLibraryManager2::AMaterialLibraryManager2()
{
	m_iNumUsedTextures = 0;
	m_iNumFreeTextures = 0;

	for ( TUINT i = 0; i < MAX_NUM_TEXTURES; i++ )
	{
		m_FreeTextures.PushFront( &m_aSlots[ i ] );
		m_iNumFreeTextures++;
	}
}

void AMaterialLibraryManager2::CreateTextures( AMaterialLibrary* a_pMatLibrary )
{
	auto pTextureFactory = THookedRenderD3DInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );

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

void AMaterialLibraryManager2::UnloadTextures( AMaterialLibrary* a_pMaterialLibrary )
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

	THookedRenderD3DInterface::GetSingleton()->FlushDyingResources();
	CALL( 0x006120e0, void );
}

TTexture* AMaterialLibraryManager2::FindTexture( const TCHAR* a_szTextureName )
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
