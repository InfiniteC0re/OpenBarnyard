#include "pch.h"
#include "AGUI2Dialog.h"
#include "AGUI2.h"
#include "AGUI2TextureSectionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUI2Dialog::AGUI2Dialog()
{
	m_pFocusElement = TNULL;
	m_bUseScissor   = TFALSE;
}

AGUI2Dialog::~AGUI2Dialog()
{
}

void AGUI2Dialog::Render()
{
	AGUI2Renderer* pRenderer = AGUI2::GetRenderer();

	for ( AGUI2Element* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		if ( it->IsVisible() )
		{
			it->PreRender();

			if ( m_bUseScissor && it != &m_aRectangles[ AGUI2ATTACHMENT_MIDDLECENTER ] )
			{
				TFLOAT fWidth, fHeight;
				it->GetDimensions( fWidth, fHeight );

				TFLOAT fHalfWidth  = fWidth * 0.5f;
				TFLOAT fHalfHeight = fHeight * 0.5f;

				pRenderer->SetScissor( -fHalfWidth, fHalfHeight, fHalfWidth, -fHalfHeight );
			}

			it->Render();

			if ( m_bUseScissor && it != &m_aRectangles[ AGUI2ATTACHMENT_MIDDLECENTER ] )
				pRenderer->ClearScissor();

			it->PostRender();
		}
	}
}

void AGUI2Dialog::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	SetDimensionsImpl( a_fWidth, a_fHeight );
}

void AGUI2Dialog::SetColour( TUINT32 a_uiColour )
{
	for ( TINT i = 0; i < AGUI2ATTACHMENT_NUMOF; i++ )
	{
		m_aRectangles->SetColour( a_uiColour );
	}

	AGUI2Element::SetColour( a_uiColour );
}

void AGUI2Dialog::SetAlpha( TFLOAT a_fAlpha )
{
	for ( TINT i = 0; i < AGUI2ATTACHMENT_NUMOF; i++ )
	{
		m_aRectangles->SetAlpha( a_fAlpha );
	}

	AGUI2Element::SetAlpha( a_fAlpha );
}

void AGUI2Dialog::SetFocus( TBOOL a_bFocused )
{
	if ( m_pFocusElement )
		m_pFocusElement->SetFocus( a_bFocused );

	AGUI2Element::SetFocus( a_bFocused );
}

void AGUI2Dialog::SetBorders( const TCHAR* a_szTopLeft, const TCHAR* a_szTopCenter, const TCHAR* a_szTopRight, const TCHAR* a_szMiddleLeft, const TCHAR* a_szMiddleCenter, const TCHAR* a_szMiddleRight, const TCHAR* a_szBottomLeft, const TCHAR* a_szBottomCenter, const TCHAR* a_szBottomRight )
{
	m_apTexTiles[ AGUI2ATTACHMENT_TOPLEFT ]   = AGUI2TextureSectionManager::GetTextureSection( a_szTopLeft );
	m_apTexTiles[ AGUI2ATTACHMENT_TOPCENTER ] = AGUI2TextureSectionManager::GetTextureSection( a_szTopCenter );
	m_apTexTiles[ AGUI2ATTACHMENT_TOPRIGHT ]  = AGUI2TextureSectionManager::GetTextureSection( a_szTopRight );

	m_apTexTiles[ AGUI2ATTACHMENT_MIDDLELEFT ]   = AGUI2TextureSectionManager::GetTextureSection( a_szMiddleLeft );
	m_apTexTiles[ AGUI2ATTACHMENT_MIDDLECENTER ] = AGUI2TextureSectionManager::GetTextureSection( a_szMiddleCenter );
	m_apTexTiles[ AGUI2ATTACHMENT_MIDDLERIGHT ]  = AGUI2TextureSectionManager::GetTextureSection( a_szMiddleRight );

	m_apTexTiles[ AGUI2ATTACHMENT_BOTTOMLEFT ]   = AGUI2TextureSectionManager::GetTextureSection( a_szBottomLeft );
	m_apTexTiles[ AGUI2ATTACHMENT_BOTTOMCENTER ] = AGUI2TextureSectionManager::GetTextureSection( a_szBottomCenter );
	m_apTexTiles[ AGUI2ATTACHMENT_BOTTOMRIGHT ]  = AGUI2TextureSectionManager::GetTextureSection( a_szBottomRight );

	for ( TINT i = 0; i < AGUI2ATTACHMENT_NUMOF; i++ )
	{
		m_aRectangles[ i ].SetDimensions( m_apTexTiles[ i ]->GetWidth(), m_apTexTiles[ i ]->GetHeight() );
		m_aRectangles[ i ].SetTextureSection( m_apTexTiles[ i ] );
		m_aRectangles[ i ].SetAttachment( (AGUI2ATTACHMENT)i, (AGUI2ATTACHMENT)i );

		AddChildTail( m_aRectangles[ i ] );
	}

	AddChildTail( m_aRectangles[ AGUI2ATTACHMENT_MIDDLECENTER ] );
}

void AGUI2Dialog::SetDimensionsImpl( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TFLOAT fBorderWidth  = ( m_apTexTiles[ AGUI2ATTACHMENT_MIDDLELEFT ]->GetWidth() + m_apTexTiles[ AGUI2ATTACHMENT_MIDDLERIGHT ]->GetWidth() ) * 0.5f;
	TFLOAT fBorderHeight = ( m_apTexTiles[ AGUI2ATTACHMENT_TOPCENTER ]->GetHeight() + m_apTexTiles[ AGUI2ATTACHMENT_BOTTOMCENTER ]->GetHeight() ) * 0.5f;

	TFLOAT fContentWidth  = a_fWidth - ( fBorderWidth + fBorderWidth );
	TFLOAT fContentHeight = a_fHeight - ( fBorderHeight + fBorderHeight );

	m_aRectangles[ AGUI2ATTACHMENT_TOPLEFT ].SetDimensions( fBorderWidth, fBorderHeight );
	m_aRectangles[ AGUI2ATTACHMENT_TOPCENTER ].SetDimensions( fContentWidth, fBorderHeight );
	m_aRectangles[ AGUI2ATTACHMENT_TOPRIGHT ].SetDimensions( fBorderWidth, fBorderHeight );

	m_aRectangles[ AGUI2ATTACHMENT_MIDDLELEFT ].SetDimensions( fBorderWidth, fContentHeight );
	m_aRectangles[ AGUI2ATTACHMENT_MIDDLECENTER ].SetDimensions( a_fWidth - fBorderWidth, a_fHeight - fBorderHeight );
	m_aRectangles[ AGUI2ATTACHMENT_MIDDLERIGHT ].SetDimensions( fBorderWidth, fContentHeight );

	m_aRectangles[ AGUI2ATTACHMENT_BOTTOMLEFT ].SetDimensions( fBorderWidth, fBorderHeight );
	m_aRectangles[ AGUI2ATTACHMENT_BOTTOMCENTER ].SetDimensions( fContentWidth, fBorderHeight );
	m_aRectangles[ AGUI2ATTACHMENT_BOTTOMRIGHT ].SetDimensions( fBorderWidth, fBorderHeight );

	AGUI2Element::SetDimensions( a_fWidth, a_fHeight );
	AddChildHead( m_aRectangles[ AGUI2ATTACHMENT_MIDDLECENTER ] );
}
