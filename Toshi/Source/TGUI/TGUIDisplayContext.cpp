#include "ToshiPCH.h"
#include "TGUIDisplayContext.h"
#include "TGUIFormattedText.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: abstract, CreateObject returns TNULL in Nicktoons
TDEFINE_CLASS_NORUNTIME( TGUIDisplayContext );

// $NicktoonsPS2: FUNCTION 003639c8
void TGUIDisplayContext::ClipStack::Push( const TGUIRectangle& a_rcRect )
{
	iTop++;

	if ( iTop < MAX_CLIP_DEPTH )
		aRects[ iTop ] = a_rcRect;
}

// Pushes a copy of the top rectangle
// $NicktoonsPS2: FUNCTION 00363a18
void TGUIDisplayContext::ClipStack::Push()
{
	Push( Top() );
}

// $NicktoonsPS2: FUNCTION 00363a48
const TGUIRectangle& TGUIDisplayContext::ClipStack::Pop()
{
	iTop--;

	if ( iTop < 0 )
		iTop = 0;

	return aRects[ iTop ];
}

// $NicktoonsPS2: FUNCTION 00130ce0
TGUIRectangle& TGUIDisplayContext::ClipStack::Top()
{
	return aRects[ iTop ];
}

// $NicktoonsPS2: FUNCTION 00363688
// $JPOG: FUNCTION TGuiInterface.dll+0000a380
TGUIDisplayContext::TGUIDisplayContext()
    : m_fCurrentTime( 0.0f )
    , m_fTime( 0.0f )
{
	m_oClipStack.iTop = 0;

	for ( TINT i = 0; i < MAX_CLIP_DEPTH; i++ )
		m_oClipStack.aRects[ i ].Set( 0, 0, 0, 0 );
}

// $NicktoonsPS2: FUNCTION 003636f0
// $JPOG: FUNCTION TGuiInterface.dll+0000a370
TGUIDisplayContext::~TGUIDisplayContext()
{
}

// $NicktoonsPS2: FUNCTION 00363718
// $JPOG: FUNCTION TGuiInterface.dll+0000a100
void TGUIDisplayContext::PushClip( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
{
	m_oClipStack.Push();

	TGUIRectangle& rTop = m_oClipStack.Top();
	rTop.m_iX += a_iX;
	rTop.m_iY += a_iY;
	rTop.m_iWidth  = a_iWidth;
	rTop.m_iHeight = a_iHeight;
}

// $NicktoonsPS2: FUNCTION 003637b0
// $JPOG: FUNCTION TGuiInterface.dll+0000a350
void TGUIDisplayContext::PopClip()
{
	m_oClipStack.Pop();
}

// $NicktoonsPS2: FUNCTION 003637d0
// $JPOG: FUNCTION TGuiInterface.dll+0000a180
void TGUIDisplayContext::PushRotation( TFLOAT a_fAngle )
{
	PushRotation( a_fAngle, m_oClipStack.Top().m_iWidth / 2, m_oClipStack.Top().m_iHeight / 2 );
}

// $NicktoonsPS2: FUNCTION 00363860
// $JPOG: FUNCTION TGuiInterface.dll+0000a1c0
void TGUIDisplayContext::DrawFormattedText( TGUIFormattedText& a_rText, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
{
	TGUIRectangle oRect( 0, a_iY, 0, 0 );

	for ( auto it = a_rText.Begin(); it != a_rText.End(); it++ )
	{
		if ( it->eAlignment == TGUIAlignment_Left )
			oRect.m_iX = a_iX;
		else if ( it->eAlignment == TGUIAlignment_Center )
			oRect.m_iX = a_iX + ( a_iWidth - it->uiWidth ) / 2;
		else if ( it->eAlignment == TGUIAlignment_Right )
			oRect.m_iX = a_iX + a_iWidth - it->uiWidth;

		oRect.m_iWidth  = it->uiWidth;
		oRect.m_iHeight = it->uiHeight;

		// NOTE: JPOG also passes IsTemporaryString() of the text (its DrawLabel takes a TLString)
		DrawLabel( &oRect, it->pFont, it->oColour, it->strText.GetString() );

		// NOTE: Nicktoons' font adds its line spacing
		oRect.m_iY += it->uiHeight;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a600
void TGUIDisplayContext::DrawTextureSection( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, const TGUIRectangle* a_pRect, TGUIAlignment a_eAlignX, TGUIAlignment a_eAlignY )
{
	Imp_DrawTextureSection( a_pTextureSection, a_rcColour, a_pRect->m_iX, a_pRect->m_iY, a_pRect->m_iWidth, a_pRect->m_iHeight, a_eAlignX, a_eAlignY );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a640
void TGUIDisplayContext::DrawTextureSection( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TGUIAlignment a_eAlignX, TGUIAlignment a_eAlignY )
{
	Imp_DrawTextureSection( a_pTextureSection, a_rcColour, a_iX, a_iY, a_iWidth, a_iHeight, a_eAlignX, a_eAlignY );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a590
void TGUIDisplayContext::DrawFilledRectangle( const TGUIRectangle& a_rcRect, const TGUIColour& a_rcColour )
{
	Imp_DrawFilledRectangle( a_rcRect.m_iX, a_rcRect.m_iY, a_rcRect.GetRight(), a_rcRect.GetBottom(), a_rcColour );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a5d0
void TGUIDisplayContext::DrawFilledRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour )
{
	Imp_DrawFilledRectangle( a_iX1, a_iY1, a_iX2, a_iY2, a_rcColour );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a520
void TGUIDisplayContext::DrawOutlineRectangle( const TGUIRectangle& a_rcRect, const TGUIColour& a_rcColour )
{
	Imp_DrawOutlineRectangle( a_rcRect.m_iX, a_rcRect.m_iY, a_rcRect.GetRight(), a_rcRect.GetBottom(), a_rcColour );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a560
void TGUIDisplayContext::DrawOutlineRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour )
{
	Imp_DrawOutlineRectangle( a_iX1, a_iY1, a_iX2, a_iY2, a_rcColour );
}

TOSHI_NAMESPACE_END
