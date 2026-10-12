#include "pch.h"
#include "T2GUIString.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006cd8a0
T2GUIString::T2GUIString()
    : m_wszText( TNULL )
    , m_pFont( TNULL )
    , m_fScale( 1.0f )
{
}

// $Barnyard: FUNCTION 006cd9c0
T2GUIString::~T2GUIString()
{
}

// $Barnyard: FUNCTION 006cd8d0
void T2GUIString::SetFont( T2GUIFont* a_pFont )
{
	m_pFont            = a_pFont;
	m_bDimensionsDirty = TTRUE;
}

// $Barnyard: FUNCTION 006cd940
void T2GUIString::Render()
{
	if ( m_pFont && m_wszText )
	{
		TVector2 vecPos = m_oTransform.GetTranslation();

		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions( fWidth, fHeight );

		vecPos.x -= fWidth * 0.5f;
		vecPos.y -= fHeight * 0.5f;

		// Inlined T2GUIFont 006c33c0: draws a single line if the text isn't empty
		if ( m_wszText[ 0 ] != L'\0' )
			m_pFont->DrawTextSingleLine( m_wszText, TStringManager::String16Length( m_wszText ), vecPos.x, vecPos.y, m_uiColour, m_fScale );
	}
}

// $Barnyard: FUNCTION 006cd8e0
void T2GUIString::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	if ( m_bDimensionsDirty )
	{
		if ( m_pFont && m_wszText )
		{
			m_fWidth = m_pFont->GetTextWidth( m_wszText, m_fScale );

			// NOTE: retail uses T2GUIFont::GetLineHeight (006c2dd0) * m_fScale. The engine's
			// T2GUIFont doesn't expose the line height, so it's taken from the height of
			// the (single line) text minus the line margin. Empty text gets no height.
			m_fHeight = ( m_wszText[ 0 ] != L'\0' ) ? m_pFont->GetTextHeightWrapped( m_wszText, TMath::MAXFLOAT, m_fScale ) - m_pFont->GetLinesMargin() * m_fScale : 0.0f;
		}
		else
		{
			m_fWidth  = 0.0f;
			m_fHeight = 0.0f;
		}

		m_bDimensionsDirty = TFALSE;
	}

	a_rWidth  = m_fWidth;
	a_rHeight = m_fHeight;
}

TOSHI_NAMESPACE_END
