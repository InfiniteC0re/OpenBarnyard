#include "pch.h"
#include "AMaterialLibrary.h"
#include "AMaterialLibraryManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AMaterialLibrary::AMaterialLibrary()
{
	m_iNumTextures = 0;
}

TBOOL AMaterialLibrary::LoadTTLData( void* a_pTTLData )
{
	TTL* pTTL = TSTATICCAST( TTL, a_pTTLData );

	auto pLibList     = AMaterialLibraryManager::List::GetSingleton();
	TINT iNumTextures = 0;

	if ( ms_bSkipLoadedTextures )
	{
		for ( TINT i = 0; i < pTTL->m_iNumTextures; i++ )
		{
			if ( !pLibList->FindTexture( pTTL->m_pTextureInfos[ i ].m_szFileName, TNULL, TNULL ) )
			{
				iNumTextures++;
			}
		}
	}
	else
	{
		iNumTextures = pTTL->m_iNumTextures;
	}

	m_pTexturesArray = new ATexture[ iNumTextures ];
	m_pTextures      = m_pTexturesArray;
	m_iNumTextures   = iNumTextures;

	for ( TINT i = 0; i < iNumTextures; i++ )
	{
		auto pTexInfo = &pTTL->m_pTextureInfos[ i ];

		if ( !ms_bSkipLoadedTextures || !pLibList->FindTexture( pTexInfo->m_szFileName, TNULL, TNULL ) )
		{
			TASSERT( pTexInfo->m_bIsT2Texture == TRUE, "No support of other texture types" );
			m_pTextures[ i ].Name = pTexInfo->m_szFileName;

			if ( pTexInfo->m_bIsT2Texture == TRUE )
			{
				auto pTexture = new Toshi::T2Texture;

				if ( pTexture )
				{
					void* pData = TMalloc( pTexInfo->m_uiDataSize );

					Toshi::TUtil::MemCopy( pData, pTexInfo->m_pData, pTexInfo->m_uiDataSize );
					pTexture->SetData( pData, pTexInfo->m_uiDataSize );
					pTexture->Load();
				}

				m_pTextures[ i ].pTexture = pTexture;
			}
		}
	}

	return TTRUE;
}

TBOOL AMaterialLibrary::LoadTTLFile( const TCHAR* a_szFilePath )
{
	m_TRB.Load( a_szFilePath );
	LoadTTLData( m_TRB.GetSymbolAddress( "TTL" ) );
	m_TRB.Close();

	return TTRUE;
}

TBOOL AMaterialLibrary::LoadTRBFile( Toshi::TTRB* a_pTRB )
{
	void* pTTL = a_pTRB->GetSymbolAddress( "TTL" );
	TASSERT( TNULL != pTTL );

	LoadTTLData( pTTL );
	a_pTRB->Close();

	return TTRUE;
}

void AMaterialLibrary::Destroy()
{
	Node::Remove();
	DestroyTextures();
	delete this;
}

void AMaterialLibrary::DestroyTextures()
{
	for ( TINT i = 0; i < m_iNumTextures; i++ )
	{
		if ( m_pTextures[ i ].pTexture )
		{
			delete m_pTextures[ i ].pTexture;
			m_pTextures[ i ].pTexture = TNULL;
		}
	}

	m_pTextures    = TNULL;
	m_iNumTextures = 0;
	delete[] m_pTexturesArray;
}

TINT AMaterialLibrary::FindTextureIndex( const TCHAR* a_szTextureName )
{
	for ( TINT i = 0; i < m_iNumTextures; i++ )
	{
		if ( Toshi::TStringManager::String8CompareNoCase( m_pTextures[ i ].Name, a_szTextureName ) == 0 )
		{
			return i;
		}
	}

	return -1;
}
