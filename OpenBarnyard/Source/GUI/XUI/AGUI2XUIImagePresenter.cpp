#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIImagePresenter.h"
#  include "AGUI2XUIImage.h"
#  include "AGUI2XUIResource.h"

#  include <GUI/T2GUI.h>
#  include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIImagePresenter );

// $BarnyardWiiJP: FUNCTION 8041a294
AGUI2XUIImagePresenter::AGUI2XUIImagePresenter()
{
	m_eSizeMode = 0;
	m_pMaterial = TNULL;
}

// $BarnyardWiiJP: FUNCTION 8041a2e4
// $BarnyardWiiJP: FUNCTION 8041a648
AGUI2XUIImagePresenter::~AGUI2XUIImagePresenter()
{
}

// $BarnyardWiiJP: FUNCTION 8041a33c
void AGUI2XUIImagePresenter::SetImage( const TWCHAR* a_wszImagePath )
{
	if ( !a_wszImagePath )
		return;

	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();
	TString8       strImagePath( a_wszImagePath );

	// NOTE: the texture isn't given to the material, only the size is updated
	UpdateSize( pRenderer->GetTexture( strImagePath.GetString() ) );
}

// $BarnyardWiiJP: FUNCTION 8041a3d4
TBOOL AGUI2XUIImagePresenter::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	AGUI2XUIElement::Create( a_rResource, a_pData, a_bCreateChildren );

	m_eSizeMode = TSTATICCAST( XURXUIImagePresenterData, a_pData )->m_uiSizeMode;
	T2GUI::GetRenderer();
	m_uiColour = MakeARGB( 140, 140, 140, TINT( 255.0f * m_fOpacity ) );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041a44c
TBOOL AGUI2XUIImagePresenter::UpdateSize( TTexture* a_pTexture )
{
	if ( m_eSizeMode == AGUI2XUIImage::SIZEMODE_STRETCH )
	{
		m_vecUV1 = TVector2( 0.0f, 0.0f );
		m_vecUV2 = TVector2( 1.0f, 1.0f );
	}
	else if ( m_eSizeMode == AGUI2XUIImage::SIZEMODE_TEXTURE )
	{
		// NOTE: the original reads the size from the Wii texture (0x98 and 0x9A)
		m_fWidth  = TFLOAT( a_pTexture->GetWidth() );
		m_vecUV1  = TVector2( 0.0f, 0.0f );
		m_fHeight = TFLOAT( a_pTexture->GetHeight() );
		m_vecUV2  = TVector2( 1.0f, 1.0f );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041a4e4
void AGUI2XUIImagePresenter::Render()
{
	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();
	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );

	TVector2 vecPos1( m_fWidth * -0.5f, m_fHeight * -0.5f );
	TVector2 vecPos2( vecPos1.x + m_fWidth, vecPos1.y + m_fHeight );

	pRenderer->RenderRectangle( vecPos1, vecPos2, m_vecUV1, m_vecUV2 );
}

#endif // BARNYARD_LATEST
