#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUITextureSectionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006bfb20
TBOOL T2GUITextureSectionManager::Open( const TCHAR* a_szFileName, TTRB* a_pTRB )
{
	if ( a_pTRB == TNULL )
	{
		g_providedtrb = TFALSE;
		g_trb         = new TTRB();

		if ( g_trb->Load( a_szFileName ) != TTRB::ERROR_OK )
			return TFALSE;

		g_defblock = TSTATICCAST( TRBHeader, g_trb->GetSymbolAddress( "texturesections" ) );
	}
	else
	{
		g_trb         = a_pTRB;
		g_providedtrb = TTRUE;

		auto iFilePathLength = TStringManager::String8Length( a_szFileName );
		auto iFileNamePos    = iFilePathLength - 1;

		while ( a_szFileName[ iFileNamePos ] != '\\' && a_szFileName[ iFileNamePos ] != '/' )
			iFileNamePos--;

		iFileNamePos++;
		TCHAR symbolName[ 132 ] = { '\0' };

		auto iFileNameLength = iFilePathLength - iFileNamePos - 4;
		TStringManager::String8Copy( symbolName, a_szFileName + iFileNamePos, iFileNameLength );
		symbolName[ iFileNameLength ]     = '_';
		symbolName[ iFileNameLength + 1 ] = '\0';
		TStringManager::String8ToLowerCase( symbolName );
		TStringManager::String8Copy( symbolName + iFileNameLength + 1, "texturesections" );

		g_defblock = TSTATICCAST( TRBHeader, g_trb->GetSymbolAddress( symbolName ) );
	}

	g_sections = g_defblock->m_pSections;
	CreateMaterials();

	return TTRUE;
}

// $Barnyard: FUNCTION 006bfc40
void T2GUITextureSectionManager::Close()
{
	if ( g_trb )
	{
		DestroyMaterials();
		g_sections = TNULL;
		g_defblock = TNULL;

		if ( !g_providedtrb )
			delete g_trb;

		g_providedtrb = TFALSE;
		g_trb         = TNULL;
	}
}

// $Barnyard: FUNCTION 006bf890
T2GUITextureSection* T2GUITextureSectionManager::GetTextureSection( const TCHAR* a_szSectionName )
{
	TASSERT( g_defblock && g_sections );

	for ( TUINT32 i = 0; i < g_defblock->m_uiNumSections; i++ )
	{
		auto pSection = &g_sections[ i ];

		if ( TStringManager::String8Compare( pSection->GetName(), a_szSectionName ) == 0 )
			return pSection;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006bf8e0
void T2GUITextureSectionManager::CreateMaterials()
{
	TASSERT( g_sections != TNULL );

	auto pRenderer = T2GUI::GetRenderer();

	for ( TUINT i = 0; i < g_defblock->m_uiNumSections; i++ )
	{
		auto pSection = &g_defblock->m_pSections[ i ];

		if ( pSection->GetMaterial() == TNULL )
		{
			if ( TTexture* pTexture = pRenderer->GetTexture( pSection->GetTextureFile() ) )
			{
				auto pMaterial = pRenderer->CreateMaterial( pTexture );
				pSection->SetMaterial( pMaterial );

				auto iWidth  = pRenderer->GetWidth( pMaterial );
				auto iHeight = pRenderer->GetHeight( pMaterial );

				pSection->CalculateUV( TFLOAT( iWidth ), TFLOAT( iHeight ) );
			}
		}
	}
}

// $Barnyard: FUNCTION 006bf9c0
void T2GUITextureSectionManager::DestroyMaterials()
{
	TASSERT( g_sections != TNULL );

	auto pRenderer = T2GUI::GetRenderer();

	for ( TUINT i = 0; i < g_defblock->m_uiNumSections; i++ )
	{
		auto pSection  = &g_defblock->m_pSections[ i ];
		auto pMaterial = pSection->GetMaterial();

		if ( pMaterial != TNULL )
		{
			pRenderer->DestroyMaterial( pMaterial );
			pSection->SetMaterial( TNULL );
		}
	}
}

// $Barnyard: FUNCTION 006bfa10
void T2GUITextureSectionManager::UpdateMaterials()
{
	if ( g_sections != TNULL )
	{
		auto pRenderer = T2GUI::GetRenderer();

		for ( TUINT i = 0; i < g_defblock->m_uiNumSections; i++ )
		{
			auto pSection = &g_defblock->m_pSections[ i ];
			auto pTexture = pRenderer->GetTexture( pSection->GetTextureFile() );

			if ( pSection->GetMaterial() == TNULL )
			{
				if ( pTexture )
				{
					auto pMaterial = pRenderer->CreateMaterial( pTexture );
					pSection->SetMaterial( pMaterial );

					auto iWidth  = pRenderer->GetWidth( pMaterial );
					auto iHeight = pRenderer->GetHeight( pMaterial );

					pSection->CalculateUV( TFLOAT( iWidth ), TFLOAT( iHeight ) );
				}
			}
			else if ( pTexture == TNULL )
			{
				pRenderer->DestroyMaterial( pSection->GetMaterial() );
				pSection->SetMaterial( TNULL );
			}
		}
	}
}

TOSHI_NAMESPACE_END
