#include "pch.h"
#include "AGUI2Dialog.h"
#include "AGUI2.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00638020
AGUI2Dialog::AGUI2Dialog()
{
	m_pFocusElement = TNULL;
	m_bUseScissor   = TFALSE;
}

AGUI2Dialog::~AGUI2Dialog()
{
}

// $Barnyard: FUNCTION 006385f0
void AGUI2Dialog::Render()
{
	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		if ( it->IsVisible() )
		{
			it->PreRender();

			if ( m_bUseScissor && it != &m_aRectangles[ T2GUIATTACHMENT_MIDDLECENTER ] )
			{
				TFLOAT fWidth, fHeight;
				it->GetDimensions( fWidth, fHeight );

				TFLOAT fHalfWidth  = fWidth * 0.5f;
				TFLOAT fHalfHeight = fHeight * 0.5f;

				pRenderer->SetScissor( -fHalfWidth, fHalfHeight, fHalfWidth, -fHalfHeight );
			}

			it->Render();

			if ( m_bUseScissor && it != &m_aRectangles[ T2GUIATTACHMENT_MIDDLECENTER ] )
				pRenderer->ClearScissor();

			it->PostRender();
		}
	}
}

void AGUI2Dialog::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	SetDialogSize( a_fWidth, a_fHeight );
}

// $Barnyard: FUNCTION 00638550
void AGUI2Dialog::SetColour( TUINT32 a_uiColour )
{
	for ( TINT i = 0; i < T2GUIATTACHMENT_NUMOF; i++ )
	{
		m_aRectangles[ i ].SetColour( a_uiColour );
	}

	T2GUIElement::SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 00638580
void AGUI2Dialog::SetAlpha( TFLOAT a_fAlpha )
{
	for ( TINT i = 0; i < T2GUIATTACHMENT_NUMOF; i++ )
	{
		m_aRectangles[ i ].SetAlpha( a_fAlpha );
	}

	T2GUIElement::SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 006385c0
void AGUI2Dialog::SetFocus( TBOOL a_bFocused )
{
	if ( m_pFocusElement )
		m_pFocusElement->SetFocus( a_bFocused );

	T2GUIElement::SetFocus( a_bFocused );
}

// $Barnyard: FUNCTION 00638100
void AGUI2Dialog::Create( const TCHAR* a_szTopLeft, const TCHAR* a_szTopCenter, const TCHAR* a_szTopRight, const TCHAR* a_szMiddleLeft, const TCHAR* a_szMiddleCenter, const TCHAR* a_szMiddleRight, const TCHAR* a_szBottomLeft, const TCHAR* a_szBottomCenter, const TCHAR* a_szBottomRight )
{
	m_apTexTiles[ T2GUIATTACHMENT_TOPLEFT ]   = T2GUITextureSectionManager::GetTextureSection( a_szTopLeft );
	m_apTexTiles[ T2GUIATTACHMENT_TOPCENTER ] = T2GUITextureSectionManager::GetTextureSection( a_szTopCenter );
	m_apTexTiles[ T2GUIATTACHMENT_TOPRIGHT ]  = T2GUITextureSectionManager::GetTextureSection( a_szTopRight );

	m_apTexTiles[ T2GUIATTACHMENT_MIDDLELEFT ]   = T2GUITextureSectionManager::GetTextureSection( a_szMiddleLeft );
	m_apTexTiles[ T2GUIATTACHMENT_MIDDLECENTER ] = T2GUITextureSectionManager::GetTextureSection( a_szMiddleCenter );
	m_apTexTiles[ T2GUIATTACHMENT_MIDDLERIGHT ]  = T2GUITextureSectionManager::GetTextureSection( a_szMiddleRight );

	m_apTexTiles[ T2GUIATTACHMENT_BOTTOMLEFT ]   = T2GUITextureSectionManager::GetTextureSection( a_szBottomLeft );
	m_apTexTiles[ T2GUIATTACHMENT_BOTTOMCENTER ] = T2GUITextureSectionManager::GetTextureSection( a_szBottomCenter );
	m_apTexTiles[ T2GUIATTACHMENT_BOTTOMRIGHT ]  = T2GUITextureSectionManager::GetTextureSection( a_szBottomRight );

	for ( TINT i = 0; i < T2GUIATTACHMENT_NUMOF; i++ )
	{
		m_aRectangles[ i ].SetDimensions( m_apTexTiles[ i ]->GetWidth(), m_apTexTiles[ i ]->GetHeight() );
		m_aRectangles[ i ].SetTextureSection( m_apTexTiles[ i ] );
		m_aRectangles[ i ].SetAttachment( (T2GUIATTACHMENT)i, (T2GUIATTACHMENT)i );

		AddChildTail( m_aRectangles[ i ] );
	}

	AddChildTail( m_aRectangles[ T2GUIATTACHMENT_MIDDLECENTER ] );
}

// $Barnyard: FUNCTION 006383b0
void AGUI2Dialog::SetDialogSize( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TFLOAT fBorderWidth  = ( m_apTexTiles[ T2GUIATTACHMENT_MIDDLELEFT ]->GetWidth() + m_apTexTiles[ T2GUIATTACHMENT_MIDDLERIGHT ]->GetWidth() ) * 0.5f;
	TFLOAT fBorderHeight = ( m_apTexTiles[ T2GUIATTACHMENT_TOPCENTER ]->GetHeight() + m_apTexTiles[ T2GUIATTACHMENT_BOTTOMCENTER ]->GetHeight() ) * 0.5f;

	TFLOAT fContentWidth  = a_fWidth - ( fBorderWidth + fBorderWidth );
	TFLOAT fContentHeight = a_fHeight - ( fBorderHeight + fBorderHeight );

	m_aRectangles[ T2GUIATTACHMENT_TOPLEFT ].SetDimensions( fBorderWidth, fBorderHeight );
	m_aRectangles[ T2GUIATTACHMENT_TOPCENTER ].SetDimensions( fContentWidth, fBorderHeight );
	m_aRectangles[ T2GUIATTACHMENT_TOPRIGHT ].SetDimensions( fBorderWidth, fBorderHeight );

	m_aRectangles[ T2GUIATTACHMENT_MIDDLELEFT ].SetDimensions( fBorderWidth, fContentHeight );
	m_aRectangles[ T2GUIATTACHMENT_MIDDLECENTER ].SetDimensions( a_fWidth - fBorderWidth, a_fHeight - fBorderHeight );
	m_aRectangles[ T2GUIATTACHMENT_MIDDLERIGHT ].SetDimensions( fBorderWidth, fContentHeight );

	m_aRectangles[ T2GUIATTACHMENT_BOTTOMLEFT ].SetDimensions( fBorderWidth, fBorderHeight );
	m_aRectangles[ T2GUIATTACHMENT_BOTTOMCENTER ].SetDimensions( fContentWidth, fBorderHeight );
	m_aRectangles[ T2GUIATTACHMENT_BOTTOMRIGHT ].SetDimensions( fBorderWidth, fBorderHeight );

	T2GUIElement::SetDimensions( a_fWidth, a_fHeight );
	AddChildHead( m_aRectangles[ T2GUIATTACHMENT_MIDDLECENTER ] );
}
