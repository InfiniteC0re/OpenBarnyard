#include "pch.h"
#include "AGUIMatlibPicture.h"
#include "AGUI2.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"

#include <Render/TRenderInterface.h>
#include <Render/TTextureFactory.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0059cc50, CreateObject 0059cc90 and CreateObjectInPlace 0059cd00 of the TClass
// $Barnyard: FUNCTION 0059cc90
// $Barnyard: FUNCTION 0059cd00
TDEFINE_CLASS( AGUIMatlibPicture );

// $Barnyard: FUNCTION 0059cc60
AGUIMatlibPicture::AGUIMatlibPicture()
{
	m_pMaterialLibrary = TNULL;
}

// $Barnyard: FUNCTION 0059d210
// $Barnyard: FUNCTION 0059d260
AGUIMatlibPicture::~AGUIMatlibPicture()
{
	Flush();
}

// $Barnyard: FUNCTION 0059d140
void AGUIMatlibPicture::Flush()
{
	if ( m_iNumRects1 != 0 )
	{
		if ( m_pImages != TNULL )
		{
			for ( TINT i = 0; i < m_iNumRects2; i++ )
			{
				if ( m_pImages[ i ].pTexture )
				{
					m_pImages[ i ].pTexture->DestroyResource();
					m_pImages[ i ].pTexture = TNULL;
				}

				AGUI2::GetRenderer()->DestroyMaterial( m_pImages[ i ].pMaterial );
				m_pImages[ i ].pMaterial = TNULL;
			}

			delete[] m_pImages;
			m_pImages = TNULL;
		}

		m_iNumRects2 = 0;
		delete[] m_pRectangles;
		m_pRectangles = TNULL;

		m_iNumRects1 = 0;
		UnloadMatLib();
		m_bLoaded = TFALSE;
	}
}

// $Barnyard: FUNCTION 0059ce40
void AGUIMatlibPicture::Cache()
{
	if ( m_iNumRects1 > 0 || m_Name.IsEmpty() )
		return;

	m_iNumRects1  = GetNumImages();
	m_pRectangles = new Rectangle[ m_iNumRects1 ];

	m_iNumRects2 = GetNumImages();
	m_pImages    = new Image[ m_iNumRects2 ];

	LoadMatLib();

	TString8         strTexName;
	TTextureFactory* pTexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );

	for ( TINT i = 0; i < m_iNumRects2; i++ )
	{
		strTexName.Format( "%s_%d.tga", m_Name.GetString8().GetString(), i + 1 );

		TINT      iTexIndex = m_pMaterialLibrary->FindTextureIndex( strTexName );
		TTexture* pTexture  = pTexFactory->CreateFromT2Texture( m_pMaterialLibrary->GetTexture( iTexIndex )->pTexture );

		m_pImages[ i ].pTexture  = pTexture;
		m_pImages[ i ].pMaterial = AGUI2::GetRenderer()->CreateMaterial( pTexture );
		m_pImages[ i ].pMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_OPAQUE );
		m_pImages[ i ].pMaterial->SetTextureAddress( T2GUIMaterial::TEXTUREADDRESS_CLAMP );
	}

	for ( TINT i = 0; i < m_iNumRects1; i++ )
	{
#ifdef BARNYARD_LATEST
		const SplitRule* pRule = GetLayoutSplitRule( i );
#else
		const SplitRule* pRule = GetSplitRule( i );
#endif
		Rectangle* pRect = &m_pRectangles[ i ];

		pRect->ppMaterial = &m_pImages[ pRule->iImageIndex ].pMaterial;
		pRect->Pos1.x     = pRule->Pos1.x;
		pRect->Pos1.y     = pRule->Pos2.x;
		pRect->Pos2.x     = pRule->Pos1.y;
		pRect->Pos2.y     = pRule->Pos2.y;
		pRect->UV1.x      = pRule->UV1.x;
		pRect->UV1.y      = pRule->UV2.x;
		pRect->UV2.x      = pRule->UV1.y;
		pRect->UV2.y      = pRule->UV2.y;
	}

	m_bLoaded = TTRUE;
}

// $Barnyard: FUNCTION 0059d090
void AGUIMatlibPicture::Create( const TPString8& a_rName )
{
	if ( m_Name != a_rName ) Flush();
	m_Name = a_rName;
}

// $Barnyard: FUNCTION 0059cd70
void AGUIMatlibPicture::LoadMatLib()
{
	// NOTE: unused in the original
	TString8 strUnused( "16" );

	TCHAR szPath[ 256 ];
	TStringManager::String8Format( szPath, sizeof( szPath ), "data/matlibs/%s.ttl", m_Name.GetString8().GetString() );

	m_pMaterialLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    szPath,
	    AAssetLoader::GetAssetTRB( AAssetType_AssetPack )
	);
}

// $Barnyard: FUNCTION 0059d0d0
void AGUIMatlibPicture::UnloadMatLib()
{
	AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pMaterialLibrary );
	m_pMaterialLibrary = TNULL;
	m_Name             = TPString8();
}
