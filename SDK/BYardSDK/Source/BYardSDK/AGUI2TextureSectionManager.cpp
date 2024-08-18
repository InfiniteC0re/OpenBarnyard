#include "pch.h"
#include "AGUI2.h"
#include "AGUI2TextureSectionManager.h"

TOSHI_NAMESPACE_USING

TBOOL AGUI2TextureSectionManager::Open( const char* a_szFileName, Toshi::TTRB* a_pTRB )
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

void AGUI2TextureSectionManager::Close()
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

AGUI2TextureSection* AGUI2TextureSectionManager::GetTextureSection( const char* a_szSectionName )
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

void AGUI2TextureSectionManager::CreateMaterials()
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
				pSection->SetMaterial( pMaterial );

				auto iWidth  = pRenderer->GetWidth( pMaterial );
				auto iHeight = pRenderer->GetHeight( pMaterial );

				auto& uv1 = pSection->GetUVPoint1();
				uv1.x     = TFLOAT( pSection->GetPoint1X() ) / TFLOAT( iWidth );
				uv1.y     = TFLOAT( pSection->GetPoint1Y() ) / TFLOAT( iHeight );
				uv1.z     = TFLOAT( pSection->GetPoint2X() ) / TFLOAT( iWidth );
				uv1.w     = TFLOAT( pSection->GetPoint1Y() ) / TFLOAT( iHeight );

				auto& uv2 = pSection->GetUVPoint2();
				uv2.x     = TFLOAT( pSection->GetPoint2X() ) / TFLOAT( iWidth );
				uv2.y     = TFLOAT( pSection->GetPoint2Y() ) / TFLOAT( iHeight );
				uv2.z     = TFLOAT( pSection->GetPoint1X() ) / TFLOAT( iWidth );
				uv2.w     = TFLOAT( pSection->GetPoint2Y() ) / TFLOAT( iHeight );
			}
		}
	}
}

void AGUI2TextureSectionManager::DestroyMaterials()
{
	TASSERT( g_sections != TNULL );

	auto pRenderer = AGUI2::GetRenderer();

	for ( TUINT i = 0; i < ( *g_defblock )->m_uiNumSections; i++ )
	{
		auto pSection  = &( *g_defblock )->m_pSections[ i ];
		auto pMaterial = pSection->GetMaterial();

		if ( pMaterial != TNULL )
		{
			pRenderer->DestroyMaterial( pMaterial );
			pSection->SetMaterial( TNULL );
		}
	}
}

void AGUI2TextureSectionManager::UpdateMaterials()
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
					pSection->SetMaterial( pMaterial );

					auto iWidth  = pRenderer->GetWidth( pMaterial );
					auto iHeight = pRenderer->GetHeight( pMaterial );

					auto& uv1 = pSection->GetUVPoint1();
					uv1.x     = TFLOAT( pSection->GetPoint1X() ) * TFLOAT( iWidth );
					uv1.y     = TFLOAT( pSection->GetPoint1Y() ) * ( 1.0f / TFLOAT( iHeight ) );
					uv1.z     = TFLOAT( pSection->GetPoint2X() ) * TFLOAT( iWidth );
					uv1.w     = TFLOAT( pSection->GetPoint1Y() ) * ( 1.0f / TFLOAT( iHeight ) );

					auto& uv2 = pSection->GetUVPoint2();
					uv2.x     = TFLOAT( pSection->GetPoint2X() ) * TFLOAT( iWidth );
					uv2.y     = TFLOAT( pSection->GetPoint2Y() ) * ( 1.0f / TFLOAT( iHeight ) );
					uv2.z     = TFLOAT( pSection->GetPoint1X() ) * TFLOAT( iWidth );
					uv2.w     = TFLOAT( pSection->GetPoint2Y() ) * ( 1.0f / TFLOAT( iHeight ) );
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
