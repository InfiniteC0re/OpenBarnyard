#pragma once
#include "TGUIRectangle.h"
#include "TGUIColour.h"
#include "TGUIAlignment.h"

TOSHI_NAMESPACE_START

class TGUITextureSection;
class TGUIFont;
class TGUIFormattedText;

//-----------------------------------------------------------------------------
// Purpose: where the GUI paints: a stack of clip rectangles, the time of the
// animations of the components, and the drawing functions the renderer of
// the platform implements (PGUITRDisplayContext in JPOG, AGUIDisplayContext
// in Nicktoons).
//
// NOTE: the vtable follows Nicktoons, which swaps the two PushRotation and
// has two DrawLabel taking the text as a wide string (JPOG has three taking a
// TLString/TWString) and an unknown slot after DrawFormattedText.
//-----------------------------------------------------------------------------
class TGUIDisplayContext : public TObject
{
public:
	TDECLARE_CLASS( TGUIDisplayContext, TObject );

	static constexpr TINT MAX_CLIP_DEPTH = 32;

	// Stack of the clip rectangles (positions are absolute)
	// NOTE: a template in the original (TStack?), its functions are separate in Nicktoons
	struct ClipStack
	{
		void                 Push( const TGUIRectangle& a_rcRect );
		void                 Push();
		const TGUIRectangle& Pop();
		TGUIRectangle&       Top();

		TINT          iTop;                     // 0x00
		TGUIRectangle aRects[ MAX_CLIP_DEPTH ]; // 0x04
	};

public:
	TGUIDisplayContext();
	~TGUIDisplayContext();

	// +0x14: Pushes the clip rectangle at a_iX, a_iY in the current one
	virtual void PushClip( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight );

	// +0x18
	virtual void PopClip();

	// +0x1C: Rotation around the centre of the clip rectangle
	virtual void PushRotation( TFLOAT a_fAngle );

	// +0x20
	virtual void PushRotation( TFLOAT a_fAngle, TINT a_iCentreX, TINT a_iCentreY ) = 0;
	virtual void PushScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY )                   = 0;
	virtual void PopTransform()                                                    = 0;
	virtual void PushModulateColour( const TGUIColour& a_rcColour )                = 0;
	virtual void PopModulateColour()                                               = 0;
	virtual void PushAdditiveColour( const TGUIColour& a_rcColour )                = 0;
	virtual void PopAdditiveColour()                                               = 0;
	virtual void BeginScene()                                                      = 0;
	virtual void EndScene()                                                        = 0;
	virtual void BeginPostpone()                                                   = 0;
	virtual void EndPostpone()                                                     = 0;
	virtual void DrawPostpone()                                                    = 0;

	// +0x50
	virtual void DrawLine( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour ) = 0;

	// +0x54
	virtual void DrawTexturePie( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TFLOAT a_fStartAngle, TFLOAT a_fEndAngle ) = 0;

	// +0x58
	virtual void DrawLabel( const TGUIRectangle* a_pRect, const TGUIFont* a_pFont, const TGUIColour& a_rcColour, const TWCHAR* a_wszLabel ) = 0;

	// +0x5C
	virtual void DrawLabel( const TGUIRectangle* a_pRect, const TGUIFont* a_pFont, const TGUIColour& a_rcColour, const TWCHAR* a_wszLabel, const TGUIRectangle* a_pClipRect ) = 0;

	// +0x60: Draws the segments of a formatted text in the rectangle
	virtual void DrawFormattedText( TGUIFormattedText& a_rText, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight );

	// +0x64
	// NOTE: unknown, does nothing in Nicktoons' AGUIDisplayContext
	virtual void Unknown_0x64() = 0;

	// +0x68
	virtual void Imp_DrawTextureSection( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TGUIAlignment a_eAlignX, TGUIAlignment a_eAlignY ) = 0;

	// +0x6C
	virtual void Imp_DrawFilledRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour ) = 0;

	// +0x70
	virtual void Imp_DrawOutlineRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour ) = 0;

	// a_eAlignX/a_eAlignY give how the texture section is repeated in the rectangle, TGUIAlignment_NUMOF stretches it
	void DrawTextureSection( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, const TGUIRectangle* a_pRect, TGUIAlignment a_eAlignX, TGUIAlignment a_eAlignY );
	void DrawTextureSection( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TGUIAlignment a_eAlignX, TGUIAlignment a_eAlignY );

	void DrawFilledRectangle( const TGUIRectangle& a_rcRect, const TGUIColour& a_rcColour );
	void DrawFilledRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour );
	void DrawOutlineRectangle( const TGUIRectangle& a_rcRect, const TGUIColour& a_rcColour );
	void DrawOutlineRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour );

	// $JPOG: FUNCTION TGuiInterface.dll+0000a4e0
	TFLOAT GetCurrentTime() const { return m_fTime; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000a510
	void SetCurrentTime( TFLOAT a_fTime ) { m_fCurrentTime = a_fTime; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000a4f0
	void SetTimeOffset( TFLOAT a_fOffset ) { m_fTime = m_fCurrentTime + a_fOffset; }

	const TGUIRectangle& GetClipRect() { return m_oClipStack.Top(); }

protected:
	ClipStack m_oClipStack;   // 0x004
	TFLOAT    m_fCurrentTime; // 0x208
	TFLOAT    m_fTime;        // 0x20C current time with the offset of the widget (the components use it)
};

TOSHI_NAMESPACE_END
