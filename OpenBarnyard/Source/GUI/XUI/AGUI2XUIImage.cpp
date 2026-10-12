#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIImage.h"
#  include "AGUI2XUIResource.h"

#  include <GUI/T2GUI.h>
#  include <Render/TRenderInterface.h>
#  include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIImage );

// $BarnyardWiiJP: FUNCTION 80419c28
AGUI2XUIImage::AGUI2XUIImage()
{
	m_eSizeMode = 0;
	m_pMaterial = TNULL;
	m_pTexture  = TNULL;
}

// $BarnyardWiiJP: FUNCTION 80419c7c
// $BarnyardWiiJP: FUNCTION 8041a1f4
AGUI2XUIImage::~AGUI2XUIImage()
{
	// NOTE: the original calls OnDestroy and deletes the material itself, which is what DestroyMaterial does
	if ( m_pMaterial )
		T2GUI::GetRenderer()->DestroyMaterial( m_pMaterial );
}

// $BarnyardWiiJP: FUNCTION 80419d34
TBOOL AGUI2XUIImage::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	XURXUIImageData* pData = TSTATICCAST( XURXUIImageData, a_pData );

	AGUI2XUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();

	if ( pData->m_usImagePath != 0 )
	{
		TString8 strImagePath( a_rResource.GetString( pData->m_usImagePath ) );

		m_pTexture = pRenderer->GetTexture( strImagePath );

#  ifdef BARNYARD_COMMUNITY_PATCH
		// The textures of the XUI material libraries might not be loaded
		if ( !m_pTexture )
			m_pTexture = TRenderInterface::GetSingleton()->GetInvalidTexture();
#  endif

		m_pMaterial = pRenderer->CreateMaterial( m_pTexture );
	}
	else
	{
		// NOTE: the original takes the texture at 0x118 of the render interface
		m_pMaterial = pRenderer->CreateMaterial( TRenderInterface::GetSingleton()->GetInvalidTexture() );
	}

	m_uiColour = MakeARGB( 255, 255, 255, TINT( 255.0f * m_fOpacity ) );

	// NOTE: m_eSizeMode is never set from the data, so UpdateSize does nothing
	if ( pData->m_uiSizeMode == SIZEMODE_STRETCH )
	{
		m_vecUV1 = TVector2( 0.0f, 0.0f );
		m_vecUV2 = TVector2( 1.0f, 1.0f );
	}
	else if ( pData->m_uiSizeMode == SIZEMODE_TEXTURE )
	{
		// NOTE: the original reads the size from the Wii texture (0x98 and 0x9A)
		m_fWidth  = TFLOAT( m_pTexture->GetWidth() );
		m_vecUV1  = TVector2( 0.0f, 0.0f );
		m_fHeight = TFLOAT( m_pTexture->GetHeight() );
		m_vecUV2  = TVector2( 1.0f, 1.0f );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80419ed0
TBOOL AGUI2XUIImage::UpdateSize()
{
	if ( m_eSizeMode == SIZEMODE_STRETCH )
	{
		m_vecUV1 = TVector2( 0.0f, 0.0f );
		m_vecUV2 = TVector2( 1.0f, 1.0f );
	}
	else if ( m_eSizeMode == SIZEMODE_TEXTURE )
	{
		m_fWidth  = TFLOAT( m_pTexture->GetWidth() );
		m_vecUV1  = TVector2( 0.0f, 0.0f );
		m_fHeight = TFLOAT( m_pTexture->GetHeight() );
		m_vecUV2  = TVector2( 1.0f, 1.0f );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80419f6c
void AGUI2XUIImage::Render()
{
	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();
	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );

	TVector2 vecPos1( m_fWidth * -0.5f, m_fHeight * -0.5f );
	TVector2 vecPos2( vecPos1.x + m_fWidth, vecPos1.y + m_fHeight );

	pRenderer->RenderRectangle( vecPos1, vecPos2, m_vecUV1, m_vecUV2 );
}

// $BarnyardWiiJP: FUNCTION 8041a044
void AGUI2XUIImage::SetImage( const TWCHAR* a_wszImagePath )
{
	TString8 strImagePath( a_wszImagePath );

	m_pTexture = T2GUI::GetRenderer()->GetTexture( strImagePath );
	m_pMaterial->SetTextureResource( m_pTexture );

	// Applies the texture address mode to the new texture on the Wii
	m_pMaterial->SetTextureAddress( m_pMaterial->GetTextureAddress() );

	UpdateSize();
}

// $BarnyardWiiJP: FUNCTION 8041a0c0
void AGUI2XUIImage::UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT )
{
	if ( a_uiProp == XURPROP_IMAGE_IMAGEPATH && a_bKeyframeChanged == TTRUE )
		SetImage( a_rResource.GetString( a_uiFrom ) );

	AGUI2XUIElement::UpdateTimelinePropValue( a_rResource, a_uiProp, a_uiFrom, a_uiTo, a_iIndex, a_bKeyframeChanged, a_fT );
}

// $BarnyardWiiJP: FUNCTION 8041a164
void AGUI2XUIImage::UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT )
{
	AGUI2XUIElement::UpdateTimelinePropFloat( a_rResource, a_uiProp, a_iIndex, a_bKeyframeChanged, a_fFrom, a_fTo, a_fT );
}

#endif // BARNYARD_LATEST
