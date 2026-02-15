#include "pch.h"
#include "AGUI2.h"
#include "SDK_T2GUITextureSectionManager.h"

TOSHI_NAMESPACE_USING

TBOOL SDK_T2GUITextureSectionManager::Open( const char* a_szFileName, Toshi::TTRB* a_pTRB )
{
	if ( a_pTRB == TNULL )
	{
		*g_providedtrb = TFALSE;
		*g_trb         = new TTRB();

		if ( ( *g_trb )->Load( a_szFileName ) != TTRB::ERROR_OK )
			return TFALSE;

		*g_defblock = TSTATICCAST( TRBHeader, ( *g_trb )->GetSymbolAddress( "texturesections" ) );
	}
	else
	{
		*g_trb         = a_pTRB;
		*g_providedtrb = TTRUE;

		auto iFilePathLength = TStringManager::String8Length( a_szFileName );
		auto iFileNamePos    = iFilePathLength - 1;

		while ( a_szFileName[ iFileNamePos ] != '\\' && a_szFileName[ iFileNamePos ] != '/' )
			iFileNamePos--;

		iFileNamePos++;
		char symbolName[ 132 ] = { '\0' };

		auto iFileNameLength = iFilePathLength - iFileNamePos - 4;
		TStringManager::String8Copy( symbolName, a_szFileName + iFileNamePos, iFileNameLength );
		symbolName[ iFileNameLength ]     = '_';
		symbolName[ iFileNameLength + 1 ] = '\0';
		TStringManager::String8ToLowerCase( symbolName );
		TStringManager::String8Copy( symbolName + iFileNameLength + 1, "texturesections" );

		*g_defblock = TSTATICCAST( TRBHeader, ( *g_trb )->GetSymbolAddress( symbolName ) );
	}

	*g_sections = ( *g_defblock )->m_pSections;
	CreateMaterials();

	return TTRUE;
}

void SDK_T2GUITextureSectionManager::Close()
{
	if ( *g_trb )
	{
		DestroyMaterials();
		g_sections = TNULL;
		g_defblock = TNULL;

		if ( !g_providedtrb )
			delete *g_trb;

		*g_providedtrb = TFALSE;
		*g_trb         = TNULL;
	}
}

Toshi::T2GUITextureSection* SDK_T2GUITextureSectionManager::GetTextureSection( const char* a_szSectionName )
{
	TASSERT( ( *g_defblock ) && ( *g_sections ) );

	for ( TUINT32 i = 0; i < ( *g_defblock )->m_uiNumSections; i++ )
	{
		auto pSection = &( *g_sections[ i ] );

		if ( TStringManager::String8Compare( pSection->GetName(), a_szSectionName ) == 0 )
			return pSection;
	}

	return TNULL;
}

void SDK_T2GUITextureSectionManager::CreateMaterials()
{
	TASSERT( g_sections != TNULL );

	auto pRenderer = AGUI2::GetRenderer();

	for ( TUINT i = 0; i < ( *g_defblock )->m_uiNumSections; i++ )
	{
		auto pSection = &( *g_defblock )->m_pSections[ i ];

		if ( pSection->GetMaterial() == TNULL )
		{
			auto pTexture = pRenderer->GetTexture( pSection->GetTextureFile() );

			if ( pTexture )
			{
				auto pMaterial = pRenderer->CreateMaterial( pTexture );
				pSection->SetMaterial( (T2GUIMaterial*)pMaterial );

				auto iWidth  = pRenderer->GetWidth( pMaterial );
				auto iHeight = pRenderer->GetHeight( pMaterial );

				pSection->CalculateUV( TFLOAT( iWidth ), TFLOAT( iHeight ) );
			}
		}
	}
}

void SDK_T2GUITextureSectionManager::DestroyMaterials()
{
	TASSERT( g_sections != TNULL );

	auto pRenderer = AGUI2::GetRenderer();

	for ( TUINT i = 0; i < ( *g_defblock )->m_uiNumSections; i++ )
	{
		auto pSection  = &( *g_defblock )->m_pSections[ i ];
		auto pMaterial = (SDK_T2GUIMaterial*)pSection->GetMaterial();

		if ( pMaterial != TNULL )
		{
			pRenderer->DestroyMaterial( pMaterial );
			pSection->SetMaterial( TNULL );
		}
	}
}

void SDK_T2GUITextureSectionManager::UpdateMaterials()
{
	if ( g_sections != TNULL )
	{
		auto pRenderer = AGUI2::GetRenderer();

		for ( TUINT i = 0; i < ( *g_defblock )->m_uiNumSections; i++ )
		{
			auto pSection = &( *g_defblock )->m_pSections[ i ];
			auto pTexture = pRenderer->GetTexture( pSection->GetTextureFile() );

			if ( pSection->GetMaterial() == TNULL )
			{
				if ( pTexture )
				{
					auto pMaterial = pRenderer->CreateMaterial( pTexture );
					pSection->SetMaterial( (T2GUIMaterial*)pMaterial );

					auto iWidth  = pRenderer->GetWidth( pMaterial );
					auto iHeight = pRenderer->GetHeight( pMaterial );

					pSection->CalculateUV( TFLOAT( iWidth ), TFLOAT( iHeight ) );
				}
			}
			else if ( pTexture == TNULL )
			{
				pRenderer->DestroyMaterial( (SDK_T2GUIMaterial*)pSection->GetMaterial() );
				pSection->SetMaterial( TNULL );
			}
		}
	}
}
