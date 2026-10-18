#pragma once
#include <TGUI/TGUIDisplayContext.h>
#include <Math/TMatrix44.h>
#include <GUI/T2GUIRenderer.h>
#include <GUI/T2GUITransform.h>
#include <Render/TRenderInterface.h>

#include "PGUITRTextureFactory.h"
#include "PGUITRFontFactory.h"

//-----------------------------------------------------------------------------
// Purpose: TGUI display context of the TRender GUI renderer: draws the
// texture sections (tiled, see RenderTexture), labels, pies, lines and
// rectangles as textured quads and triangles. The clip rectangles only
// offset the drawing, nothing is cut. Every draw moves the depth towards the
// front (the first drawn is at the back).
//
// NOTE: the original drew with the TRender sprite shader ("SHSPRITE"), here
// it is the T2GUI renderer (AGUI2's on Barnyard). The calls of the sprite
// shader that the T2GUI renderer can't make are emulated or dropped:
// - the modulate colour multiplies the colours, there is no additive colour;
// - the T2GUI renderer has its own depth, the depths aren't passed;
// - the projection the original set on the render context (the GUI units
//   scaled by m_fScaleX/m_fScaleY) is part of the transform, see
//   SetScreenTransform.
//-----------------------------------------------------------------------------
class PGUITRDisplayContext : public Toshi::TGUIDisplayContext
{
public:
	TDECLARE_CLASS( PGUITRDisplayContext, Toshi::TGUIDisplayContext );

	static constexpr TINT MAX_TRANSFORMS = 32;
	static constexpr TINT MAX_COLOURS    = 16;

	// How the rectangles of a tiled texture follow each other
	enum StripDirection : TINT
	{
		StripDirection_Horizontal = 0,
		StripDirection_Vertical   = 1,
	};

public:
	PGUITRDisplayContext();
	~PGUITRDisplayContext();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIDisplayContext
	//-----------------------------------------------------------------------------
	virtual void PushRotation( TFLOAT a_fAngle, TINT a_iCentreX, TINT a_iCentreY ) override;
	virtual void PushScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY ) override;
	virtual void PopTransform() override;
	virtual void PushModulateColour( const Toshi::TGUIColour& a_rcColour ) override;
	virtual void PopModulateColour() override;
	virtual void PushAdditiveColour( const Toshi::TGUIColour& a_rcColour ) override;
	virtual void PopAdditiveColour() override;
	virtual void BeginScene() override;
	virtual void EndScene() override;
	virtual void BeginPostpone() override;
	virtual void EndPostpone() override;
	virtual void DrawPostpone() override;
	virtual void DrawLine( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const Toshi::TGUIColour& a_rcColour ) override;
	virtual void DrawTexturePie( const Toshi::TGUITextureSection* a_pTextureSection, const Toshi::TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TFLOAT a_fStartAngle, TFLOAT a_fEndAngle ) override;
	virtual void DrawLabel( const Toshi::TGUIRectangle* a_pRect, const Toshi::TGUIFont* a_pFont, const Toshi::TGUIColour& a_rcColour, const TWCHAR* a_wszLabel ) override;
	virtual void DrawLabel( const Toshi::TGUIRectangle* a_pRect, const Toshi::TGUIFont* a_pFont, const Toshi::TGUIColour& a_rcColour, const TWCHAR* a_wszLabel, const Toshi::TGUIRectangle* a_pOffset ) override;
	virtual void Unknown_0x64() override {} // DOES_NOTHING

protected:
	virtual void Imp_DrawTextureSection( const Toshi::TGUITextureSection* a_pTextureSection, const Toshi::TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, Toshi::TGUIAlignment a_eAlignX, Toshi::TGUIAlignment a_eAlignY ) override;
	virtual void Imp_DrawFilledRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const Toshi::TGUIColour& a_rcColour ) override;
	virtual void Imp_DrawOutlineRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const Toshi::TGUIColour& a_rcColour ) override;

public:
	//-----------------------------------------------------------------------------
	// PGUITRDisplayContext
	//-----------------------------------------------------------------------------
	virtual TFLOAT GetBackDepth();
	virtual TFLOAT GetFrontDepth();
	virtual TFLOAT GetDepthIncrement();

	// Creates its own texture and font factories
	void Create( Toshi::TRenderInterface* a_pRenderInterface );
	void Create( Toshi::TRenderInterface* a_pRenderInterface, PGUITRTextureFactory* a_pTextureFactory, PGUITRFontFactory* a_pFontFactory );

	// Draws a rectangle of a texture, positions relative to the clip rectangle
	void InsertRectIntoMesh( PGUITRTextureFactory::Texture* a_pTexture, const Toshi::TGUIColour& a_rcColour, TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2, StripDirection a_eDirection );

	// Same as InsertRectIntoMesh, for the next rectangles of a strip
	void FollowRectIntoMesh( PGUITRTextureFactory::Texture* a_pTexture, const Toshi::TGUIColour& a_rcColour, TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2, StripDirection a_eDirection );

	// Sets the renderer the display context draws with
	// NOTE: added, the original found the sprite shader in Create
	void SetShader( Toshi::T2GUIRenderer* a_pRenderer ) { m_pShader = a_pRenderer; }

	// Maps the GUI units (once scaled by m_fScaleX/m_fScaleY) to the units of the T2GUI renderer, identity by default
	// NOTE: added, the original's projection maps them to the viewport
	void SetScreenTransform( const Toshi::T2GUITransform& a_rcTransform ) { m_oScreenTransform = a_rcTransform; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005bb0
	Toshi::T2GUIRenderer* GetShader() const { return m_pShader; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005bc0
	PGUITRFontFactory* GetFontFactory() const { return m_pFontFactory; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005bd0
	PGUITRTextureFactory* GetTextureFactory() const { return m_pTextureFactory; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005be0
	TFLOAT GetScaleY() const { return m_fScaleY; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005bf0
	TFLOAT GetScaleX() const { return m_fScaleX; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005c00
	void SetScaleY( TFLOAT a_fScaleY ) { m_fScaleY = a_fScaleY; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005c10
	void SetScaleX( TFLOAT a_fScaleX ) { m_fScaleX = a_fScaleX; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005c20
	Toshi::TRenderInterface* GetRenderInterface() const { return m_pRenderInterface; }

protected:
	// Draws a texture section repeated in the rectangle, starting a_iOffsetX/a_iOffsetY in it (-1: stretched)
	void RenderTexture( const Toshi::TGUITextureSection* a_pTextureSection, const Toshi::TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iOffsetX, TINT a_iOffsetY );

	// RenderTexture when the size isn't a multiple of the texture section size: the partial sections at the borders are cut
	void RenderComplex( const Toshi::TGUITextureSection* a_pTextureSection, const Toshi::TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iOffsetX, TINT a_iOffsetY );

	void MirrorUV( TFLOAT& a_rU1, TFLOAT& a_rV1, TFLOAT& a_rU2, TFLOAT& a_rV2, TBOOL a_bMirrorX, TBOOL a_bMirrorY );

private:
	void Create();

	// NOTE: the TSpriteShader state the T2GUI renderer doesn't have (see the class comment)
	void SetShaderColour( const Toshi::TGUIColour& a_rcColour );
	void SetShaderTransform( const Toshi::TMatrix44& a_rcTransform );

private:
	Toshi::TRenderInterface* m_pRenderInterface;                    // 0x210 NOTE: a TManagedPtr in the original
	PGUITRTextureFactory*    m_pTextureFactory;                     // 0x214
	PGUITRFontFactory*       m_pFontFactory;                        // 0x218
	Toshi::T2GUIRenderer*    m_pShader;                             // 0x21C
	Toshi::TResource*        m_pRootResource;                       // 0x220 "GUIRoot"
	TFLOAT                   m_fScaleX;                             // 0x224 viewport size / screen size
	TFLOAT                   m_fScaleY;                             // 0x228
	TINT                     m_iTransformTop;                       // 0x22C
	Toshi::TMatrix44         m_aTransforms[ MAX_TRANSFORMS ];       // 0x230
	TINT                     m_iModulateColourTop;                  // 0xA30
	Toshi::TGUIColour        m_aModulateColours[ MAX_COLOURS ];     // 0xA34
	TINT                     m_iAdditiveColourTop;                  // 0xA74
	Toshi::TGUIColour        m_aAdditiveColours[ MAX_COLOURS ];     // 0xA78
	TINT                     m_iDepth;                              // 0xAB8 decreases with each draw

	// NOTE: not in the original (the state of its sprite shader and its projection)
	Toshi::TGUIColour     m_oModulateColour;
	Toshi::TGUIColour     m_oAdditiveColour;
	Toshi::T2GUITransform m_oScreenTransform;
};
