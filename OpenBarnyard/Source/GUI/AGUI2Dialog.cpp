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

			if ( m_bUseScissor && it != &m_aRectangles[ WT_MIDDLECENTER ] )
			{
				TFLOAT fWidth, fHeight;
				it->GetDimensions( fWidth, fHeight );

				TFLOAT fHalfWidth  = fWidth * 0.5f;
				TFLOAT fHalfHeight = fHeight * 0.5f;

				pRenderer->SetScissor( -fHalfWidth, fHalfHeight, fHalfWidth, -fHalfHeight );
			}

			it->Render();

			if ( m_bUseScissor && it != &m_aRectangles[ WT_MIDDLECENTER ] )
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
	for ( TINT i = 0; i < WT_NUMOF; i++ )
	{
		m_aRectangles->SetColour( a_uiColour );
	}

	AGUI2Element::SetColour( a_uiColour );
}

void AGUI2Dialog::SetAlpha( TFLOAT a_fAlpha )
{
	for ( TINT i = 0; i < WT_NUMOF; i++ )
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
	m_apTexTiles[ WT_TOPLEFT ]   = AGUI2TextureSectionManager::GetTextureSection( a_szTopLeft );
	m_apTexTiles[ WT_TOPCENTER ] = AGUI2TextureSectionManager::GetTextureSection( a_szTopCenter );
	m_apTexTiles[ WT_TOPRIGHT ]  = AGUI2TextureSectionManager::GetTextureSection( a_szTopRight );

	m_apTexTiles[ WT_MIDDLELEFT ]   = AGUI2TextureSectionManager::GetTextureSection( a_szMiddleLeft );
	m_apTexTiles[ WT_MIDDLECENTER ] = AGUI2TextureSectionManager::GetTextureSection( a_szMiddleCenter );
	m_apTexTiles[ WT_MIDDLERIGHT ]  = AGUI2TextureSectionManager::GetTextureSection( a_szMiddleRight );

	m_apTexTiles[ WT_BOTTOMLEFT ]   = AGUI2TextureSectionManager::GetTextureSection( a_szBottomLeft );
	m_apTexTiles[ WT_BOTTOMCENTER ] = AGUI2TextureSectionManager::GetTextureSection( a_szBottomCenter );
	m_apTexTiles[ WT_BOTTOMRIGHT ]  = AGUI2TextureSectionManager::GetTextureSection( a_szBottomRight );

	for ( TINT i = 0; i < WT_NUMOF; i++ )
	{
		m_aRectangles[ i ].SetDimensions( m_apTexTiles[ i ]->GetWidth(), m_apTexTiles[ i ]->GetHeight() );
		m_aRectangles[ i ].SetTextureSection( m_apTexTiles[ i ] );
		m_aRectangles[ i ].SetAnchor( i );
		m_aRectangles[ i ].SetPivot( i );

		AddChildTail( m_aRectangles[ i ] );
	}

	AddChildTail( m_aRectangles[ WT_MIDDLECENTER ] );
}

void AGUI2Dialog::SetDimensionsImpl( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TFLOAT fBorderWidth  = ( m_apTexTiles[ WT_MIDDLELEFT ]->GetWidth() + m_apTexTiles[ WT_MIDDLERIGHT ]->GetWidth() ) * 0.5f;
	TFLOAT fBorderHeight = ( m_apTexTiles[ WT_TOPCENTER ]->GetHeight() + m_apTexTiles[ WT_BOTTOMCENTER ]->GetHeight() ) * 0.5f;

	TFLOAT fContentWidth  = a_fWidth - ( fBorderWidth + fBorderWidth );
	TFLOAT fContentHeight = a_fHeight - ( fBorderHeight + fBorderHeight );

	m_aRectangles[ WT_TOPLEFT ].SetDimensions( fBorderWidth, fBorderHeight );
	m_aRectangles[ WT_TOPCENTER ].SetDimensions( fContentWidth, fBorderHeight );
	m_aRectangles[ WT_TOPRIGHT ].SetDimensions( fBorderWidth, fBorderHeight );

	m_aRectangles[ WT_MIDDLELEFT ].SetDimensions( fBorderWidth, fContentHeight );
	m_aRectangles[ WT_MIDDLECENTER ].SetDimensions( a_fWidth - fBorderWidth, a_fHeight - fBorderHeight );
	m_aRectangles[ WT_MIDDLERIGHT ].SetDimensions( fBorderWidth, fContentHeight );

	m_aRectangles[ WT_BOTTOMLEFT ].SetDimensions( fBorderWidth, fBorderHeight );
	m_aRectangles[ WT_BOTTOMCENTER ].SetDimensions( fContentWidth, fBorderHeight );
	m_aRectangles[ WT_BOTTOMRIGHT ].SetDimensions( fBorderWidth, fBorderHeight );

	AGUI2Element::SetDimensions( a_fWidth, a_fHeight );
	AddChildHead( m_aRectangles[ WT_MIDDLECENTER ] );
}
