#include "pch.h"
#include "AMaterialLibrary.h"
#include "AMaterialLibraryManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00615cc0
AMaterialLibrary::AMaterialLibrary()
{
	m_iNumTextures = 0;
}

// $Barnyard: FUNCTION 00615d20
TBOOL AMaterialLibrary::LoadTTLData( void* a_pTTLData )
{
	TPROFILER_SCOPE();

	TTL* pTTL = TSTATICCAST( TTL, a_pTTLData );

	auto pLibList     = AMaterialLibraryManager::List::GetSingleton();
	TINT iNumTextures = 0;

	if ( ms_bSkipLoadedTextures )
	{
		for ( TINT i = 0; i < pTTL->iNumTextures; i++ )
		{
			if ( !pLibList->FindTexture( pTTL->pTextureInfos[ i ].szFileName, TNULL, TNULL ) )
			{
				iNumTextures++;
			}
		}
	}
	else
	{
		iNumTextures = pTTL->iNumTextures;
	}

	m_pTexturesArray = new ATexture[ iNumTextures ];
	m_pTextures      = m_pTexturesArray;
	m_iNumTextures   = iNumTextures;

	for ( TINT i = 0; i < iNumTextures; i++ )
	{
		auto pTexInfo = &pTTL->pTextureInfos[ i ];

		if ( !ms_bSkipLoadedTextures || !pLibList->FindTexture( pTexInfo->szFileName, TNULL, TNULL ) )
		{
			TASSERT( pTexInfo->eFormat == TTEX_FMT_WIN_DDS, "No support of other texture types" );
			m_pTextures[ i ].Name = pTexInfo->szFileName;

			if ( pTexInfo->eFormat == TTEX_FMT_WIN_DDS )
			{
				auto pTexture = new Toshi::T2Texture;

				if ( pTexture )
				{
					void* pData = TMalloc( pTexInfo->uiDataSize );

					Toshi::TUtil::MemCopy( pData, pTexInfo->pData, pTexInfo->uiDataSize );
					pTexture->SetData( pData, pTexInfo->uiDataSize );
					pTexture->Load();
				}

				m_pTextures[ i ].pTexture = pTexture;
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00616000
TBOOL AMaterialLibrary::LoadTTLFile( const TCHAR* a_szFilePath )
{
	m_TRB.Load( a_szFilePath );
	LoadTTLData( m_TRB.GetSymbolAddress( "TTL" ) );
	m_TRB.Close();

	return TTRUE;
}

// $Barnyard: FUNCTION 00616040
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

// $Barnyard: FUNCTION 00615f60
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
	m_pTexturesArray = TNULL;
}

// $Barnyard: FUNCTION 00615f10
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
