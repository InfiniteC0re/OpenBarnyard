#include "ToshiPCH.h"
#include "PGUITRDisplayContext.h"

#include <TGUI/TGUITextureSection.h>
#include <TGUI/TGUIFont.h>
#include <TGUI/TGUIFontGlyph.h>
#include <GUI/T2GUI.h>
#include <Render/TNullResource.h>
#include <Toshi/T2String16.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( PGUITRDisplayContext );

// The side of the square (0 right, 1 bottom, 2 left, 3 top) that the direction a_fAngle (0 to 2 PI) crosses
// NOTE: inlined in the original
static TINT GetPieSide( TFLOAT a_fAngle )
{
	if ( a_fAngle < TMath::PI * 7.0f * 0.25f && 0.25f * TMath::PI < a_fAngle )
	{
		if ( 3.0f * TMath::PI * 0.25f < a_fAngle )
			return ( 5.0f * TMath::PI * 0.25f < a_fAngle ) ? 3 : 2;

		return 1;
	}

	return 0;
}

// The point of the side a_iSide of the square -1..1 in the direction a_fAngle
// NOTE: inlined in the original, which also brings the angle back to 0..2 PI (this doesn't change the tangent)
static void GetPieSidePoint( TINT a_iSide, TFLOAT a_fAngle, TFLOAT& a_rX, TFLOAT& a_rY )
{
	if ( a_iSide == 0 )
	{
		a_rX = 1.0f;
		a_rY = TFLOAT( tan( a_fAngle ) );
	}
	else if ( a_iSide == 1 )
	{
		a_rX = TFLOAT( -tan( a_fAngle - 0.5f * TMath::PI ) );
		a_rY = 1.0f;
	}
	else if ( a_iSide == 2 )
	{
		a_rX = -1.0f;
		a_rY = TFLOAT( -tan( a_fAngle - TMath::PI ) );
	}
	else
	{
		a_rX = TFLOAT( tan( a_fAngle - 0.5f * ( 3.0f * TMath::PI ) ) );
		a_rY = -1.0f;
	}
}

// Draws the triangle of a pie from its centre to the points (a_fX1, a_fY1) and (a_fX2, a_fY2) of the square -1..1
// NOTE: inlined in the original (one call of the sprite shader with the three vertices)
static void RenderPieTriangle( T2GUIRenderer* a_pShader, TFLOAT a_fCentreX, TFLOAT a_fCentreY, TFLOAT a_fHalfWidth, TFLOAT a_fHalfHeight, TFLOAT a_fCentreU, TFLOAT a_fCentreV, TFLOAT a_fInvTextureWidth, TFLOAT a_fInvTextureHeight, TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 )
{
	TVector2 aVertices[ 3 ];
	TVector2 aUVs[ 3 ];

	aVertices[ 0 ] = TVector2( 0.0f * a_fHalfWidth + a_fCentreX, 0.0f * a_fHalfHeight + a_fCentreY );
	aVertices[ 1 ] = TVector2( a_fX1 * a_fHalfWidth + a_fCentreX, a_fY1 * a_fHalfHeight + a_fCentreY );
	aVertices[ 2 ] = TVector2( a_fX2 * a_fHalfWidth + a_fCentreX, a_fY2 * a_fHalfHeight + a_fCentreY );

	aUVs[ 0 ] = TVector2( ( 0.0f * a_fHalfWidth + a_fCentreU ) * a_fInvTextureWidth, ( 0.0f * a_fHalfHeight + a_fCentreV ) * a_fInvTextureHeight );
	aUVs[ 1 ] = TVector2( ( a_fX1 * a_fHalfWidth + a_fCentreU ) * a_fInvTextureWidth, ( a_fY1 * a_fHalfHeight + a_fCentreV ) * a_fInvTextureHeight );
	aUVs[ 2 ] = TVector2( ( a_fX2 * a_fHalfWidth + a_fCentreU ) * a_fInvTextureWidth, ( a_fY2 * a_fHalfHeight + a_fCentreV ) * a_fInvTextureHeight );

	// NOTE: the depth isn't passed (see the class comment)
	a_pShader->RenderTriStrip( aVertices, aUVs, 3 );
}

// Moves the origin of a_rMatrix to (a_fX, a_fY, a_fZ) in its own space
// NOTE: TMatrix44::Translate of the original's TOSHI
static void TranslateMatrix( TMatrix44& a_rMatrix, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ )
{
	TMatrix44::TransformVector( a_rMatrix.GetTranslation(), a_rMatrix, TVector4( a_fX, a_fY, a_fZ, 1.0f ) );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001080
PGUITRDisplayContext::PGUITRDisplayContext()
    : m_pRenderInterface( TNULL )
    , m_pTextureFactory( TNULL )
    , m_pFontFactory( TNULL )
    , m_pShader( TNULL )
    , m_pRootResource( TNULL )
    , m_fScaleX( 1.0f )
    , m_fScaleY( 1.0f )
    , m_iTransformTop( 0 )
    , m_iModulateColourTop( 0 )
    , m_iAdditiveColourTop( 0 )
    , m_iDepth( 0 )
{
	// NOTE: the colours of the two stacks are all set to white (the TGUIColour constructor does it)
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001140
// $JPOG: FUNCTION PGUIRenderer.dll+00001000
PGUITRDisplayContext::~PGUITRDisplayContext()
{
	if ( m_pTextureFactory )
		delete m_pTextureFactory;

	if ( m_pFontFactory )
		delete m_pFontFactory;

	// NOTE: the original releases its reference of the managed render interface
	m_pRenderInterface = TNULL;
}

// $JPOG: FUNCTION PGUIRenderer.dll+000011b0
void PGUITRDisplayContext::Create( TRenderInterface* a_pRenderInterface )
{
	m_pTextureFactory  = new PGUITRTextureFactory();
	m_pFontFactory     = new PGUITRFontFactory();
	m_pRenderInterface = a_pRenderInterface;

	Create();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001260
void PGUITRDisplayContext::Create( TRenderInterface* a_pRenderInterface, PGUITRTextureFactory* a_pTextureFactory, PGUITRFontFactory* a_pFontFactory )
{
	m_pTextureFactory  = a_pTextureFactory;
	m_pFontFactory     = a_pFontFactory;
	m_pRenderInterface = a_pRenderInterface;

	Create();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005920
void PGUITRDisplayContext::Create()
{
	m_pTextureFactory->Create( this );
	m_pFontFactory->Create( this );

	// NOTE: the render interface can be TNULL (tools without a renderer, they set the shader with SetShader)
	if ( m_pRenderInterface )
		m_pRootResource = m_pRenderInterface->CreateResource( &TGetClass( TNullResource ), "GUIRoot", TNULL );

	// NOTE: the original finds the "SHSPRITE" sprite shader in the shaders of the render interface
	m_pShader = T2GUI::GetContext() ? T2GUI::GetRenderer() : TNULL;
}

// $JPOG: FUNCTION PGUIRenderer.dll+000012e0
void PGUITRDisplayContext::BeginScene()
{
	// NOTE: the original sets an orthographic projection of the size m_fScaleX x m_fScaleY and an identity world view
	// matrix on the current render context. The T2GUI renderer sets its own projection in BeginScene, the GUI units are
	// mapped to it by the transform (see SetShaderTransform).
	m_aTransforms[ m_iTransformTop ]           = TMatrix44::IDENTITY;
	m_aModulateColours[ m_iModulateColourTop ] = TGUIColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );
	m_aAdditiveColours[ m_iAdditiveColourTop ] = TGUIColour( TUINT8( 0 ), TUINT8( 0 ), TUINT8( 0 ), TUINT8( 0 ) );

	m_pShader->SetMaterial( TNULL );
	m_pShader->BeginScene();

	// NOTE: a transform of the display context above the base one of the T2GUI renderer
	m_pShader->PushTransform( T2GUITransform(), TVector2::VEC_ZERO, TVector2::VEC_ZERO );
	SetShaderTransform( m_aTransforms[ m_iTransformTop ] );

	m_iDepth = -15;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005a50
void PGUITRDisplayContext::EndScene()
{
	m_pShader->PopTransform();

	// NOTE: the original also flushes the shaders of the render interface, the T2GUI renderer does it itself
	m_pShader->EndScene();
}

// NOTE: past MAX_TRANSFORMS the stack still grows, the transforms are then read after the array (same for the colour stacks)
// $JPOG: FUNCTION PGUIRenderer.dll+00001480
void PGUITRDisplayContext::PushRotation( TFLOAT a_fAngle, TINT a_iCentreX, TINT a_iCentreY )
{
	TMatrix44 oTransform = m_aTransforms[ m_iTransformTop ];

	TINT iCentreX = a_iCentreX + GetClipRect().m_iX;
	TINT iCentreY = a_iCentreY + GetClipRect().m_iY;

	TranslateMatrix( oTransform, TFLOAT( iCentreX ), TFLOAT( iCentreY ), 0.0f );
	oTransform.RotateZ( a_fAngle );
	TranslateMatrix( oTransform, TFLOAT( -iCentreX ), TFLOAT( -iCentreY ), 0.0f );

	if ( ++m_iTransformTop < MAX_TRANSFORMS )
		m_aTransforms[ m_iTransformTop ] = oTransform;

	SetShaderTransform( m_aTransforms[ m_iTransformTop ] );
}

// $JPOG: FUNCTION PGUIRenderer.dll+000016b0
void PGUITRDisplayContext::PushScale( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	TMatrix44 oTransform = m_aTransforms[ m_iTransformTop ];
	oTransform.Scale( a_fScaleX, a_fScaleY, 1.0f );

	if ( ++m_iTransformTop < MAX_TRANSFORMS )
		m_aTransforms[ m_iTransformTop ] = oTransform;

	SetShaderTransform( m_aTransforms[ m_iTransformTop ] );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001880
void PGUITRDisplayContext::PopTransform()
{
	if ( --m_iTransformTop < 0 )
		m_iTransformTop = 0;

	SetShaderTransform( m_aTransforms[ m_iTransformTop ] );
}

// $JPOG: FUNCTION PGUIRenderer.dll+000018c0
void PGUITRDisplayContext::PushModulateColour( const TGUIColour& a_rcColour )
{
	if ( ++m_iModulateColourTop < MAX_COLOURS )
		m_aModulateColours[ m_iModulateColourTop ] = a_rcColour;

	// NOTE: the original sets the modulate colour of the sprite shader (the components divided by 255)
	m_oModulateColour = a_rcColour;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001970
void PGUITRDisplayContext::PopModulateColour()
{
	if ( --m_iModulateColourTop < 0 )
		m_iModulateColourTop = 0;

	m_oModulateColour = m_aModulateColours[ m_iModulateColourTop ];
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001a10
void PGUITRDisplayContext::PushAdditiveColour( const TGUIColour& a_rcColour )
{
	if ( ++m_iAdditiveColourTop < MAX_COLOURS )
		m_aAdditiveColours[ m_iAdditiveColourTop ] = a_rcColour;

	// NOTE: the original sets the additive colour of the sprite shader, the T2GUI renderer can't add a colour
	m_oAdditiveColour = a_rcColour;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001ac0
void PGUITRDisplayContext::PopAdditiveColour()
{
	if ( --m_iAdditiveColourTop < 0 )
		m_iAdditiveColourTop = 0;

	m_oAdditiveColour = m_aAdditiveColours[ m_iAdditiveColourTop ];
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005a40
void PGUITRDisplayContext::BeginPostpone()
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005a30
void PGUITRDisplayContext::EndPostpone()
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005a20
void PGUITRDisplayContext::DrawPostpone()
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001b60
void PGUITRDisplayContext::DrawLine( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour )
{
	SetShaderColour( a_rcColour );

	const TGUIRectangle& rcClipRect = GetClipRect();
	TVector2             vecStart( TFLOAT( a_iX1 ) - 0.5f + TFLOAT( rcClipRect.m_iX ), TFLOAT( a_iY1 ) - 0.5f + TFLOAT( rcClipRect.m_iY ) );
	TVector2             vecEnd( TFLOAT( a_iX2 ) - 0.5f + TFLOAT( rcClipRect.m_iX ), -0.5f + TFLOAT( a_iY2 ) + TFLOAT( rcClipRect.m_iY ) );

	// NOTE: the depth isn't passed (see the class comment)
	m_pShader->RenderLine( vecStart, vecEnd );
	m_iDepth--;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001c30
void PGUITRDisplayContext::DrawLabel( const TGUIRectangle* a_pRect, const TGUIFont* a_pFont, const TGUIColour& a_rcColour, const TWCHAR* a_wszLabel )
{
	TINT iX = a_pRect->m_iX;
	TINT iY = a_pRect->m_iY;

	// NOTE: the result isn't used
	m_pFontFactory->GetFont( a_pFont->GetFontID() );

	TINT           iDepth        = m_iDepth - 1;
	TGUIFontGlyph* pMissingGlyph = a_pFont->GetGlyph( 0x2400 );

	// NOTE: the original takes a TWString, the label can be TNULL here
	TINT iLength = a_wszLabel ? TINT( T2String16::Length( a_wszLabel ) ) : 0;

	for ( TINT i = 0; i < iLength; i++ )
	{
		TWCHAR         wcChar = a_wszLabel[ i ];
		TGUIFontGlyph* pGlyph;

		if ( a_pFont->IsCharacterDefined( wcChar ) )
		{
			pGlyph = a_pFont->GetGlyph( wcChar );
		}
		else
		{
			if ( !pMissingGlyph || wcChar == L'\r' || wcChar == L'\n' )
				continue;

			pGlyph = pMissingGlyph;
		}

		PGUITRTextureFactory::Texture* pTexture = m_pTextureFactory->GetTexture( pGlyph->GetTexture() );

		TFLOAT fInvWidth  = 1.0f / TFLOAT( pTexture->GetWidth() );
		TFLOAT fInvHeight = 1.0f / TFLOAT( pTexture->GetHeight() );
		TINT   iGlyphX    = pGlyph->GetX();
		TINT   iGlyphY    = pGlyph->GetY();
		TINT   iWidth     = pGlyph->GetWidth();
		TINT   iHeight    = pGlyph->GetHeight();
		TFLOAT fY1        = TFLOAT( iY ) + TFLOAT( a_pFont->GetFontHeight() ) - TFLOAT( iHeight ) + TFLOAT( pGlyph->GetBaseline() );

		InsertRectIntoMesh(
		    pTexture,
		    a_rcColour,
		    TFLOAT( iX ),
		    fY1,
		    TFLOAT( iWidth ) + TFLOAT( iX ),
		    TFLOAT( iHeight ) + fY1,
		    TFLOAT( iDepth ),
		    TFLOAT( iGlyphX ) * fInvWidth,
		    TFLOAT( iGlyphY ) * fInvHeight,
		    fInvWidth * TFLOAT( iGlyphX + iWidth + 1 ),
		    fInvHeight * TFLOAT( iGlyphY + iHeight + 1 ),
		    StripDirection_Horizontal
		);

		iX += pGlyph->GetKerningRight() + pGlyph->GetKerningLeft() + a_pFont->GetSpacing() + iWidth;
	}

	m_iDepth -= 2;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00001ec0
void PGUITRDisplayContext::DrawLabel( const TGUIRectangle* a_pRect, const TGUIFont* a_pFont, const TGUIColour& a_rcColour, const TWCHAR* a_wszLabel, const TGUIRectangle* a_pOffset )
{
	TINT iX = a_pRect->m_iX;
	TINT iY = a_pRect->m_iY;

	// NOTE: the result isn't used
	m_pFontFactory->GetFont( a_pFont->GetFontID() );

	TINT           iDepth        = m_iDepth - 1;
	TINT           iOffsetX      = a_pOffset->m_iX;
	TINT           iOffsetY      = a_pFont->GetFontHeight() + a_pOffset->m_iY;
	TGUIFontGlyph* pMissingGlyph = a_pFont->GetGlyph( 0x2400 );

	// NOTE: the original takes a TLString, the label can be TNULL here
	TINT iLength = a_wszLabel ? TINT( T2String16::Length( a_wszLabel ) ) : 0;

	for ( TINT i = 0; i < iLength; i++ )
	{
		TWCHAR         wcChar = a_wszLabel[ i ];
		TGUIFontGlyph* pGlyph;

		if ( a_pFont->IsCharacterDefined( wcChar ) )
		{
			pGlyph = a_pFont->GetGlyph( wcChar );
		}
		else
		{
			if ( !pMissingGlyph || wcChar == L'\r' || wcChar == L'\n' )
				continue;

			pGlyph = pMissingGlyph;
		}

		PGUITRTextureFactory::Texture* pTexture = m_pTextureFactory->GetTexture( pGlyph->GetTexture() );

		TFLOAT fInvWidth  = 1.0f / TFLOAT( pTexture->GetWidth() );
		TFLOAT fInvHeight = 1.0f / TFLOAT( pTexture->GetHeight() );
		TINT   iGlyphX    = pGlyph->GetX();
		TINT   iGlyphY    = pGlyph->GetY();
		TINT   iWidth     = pGlyph->GetWidth();
		TINT   iHeight    = pGlyph->GetHeight();
		TFLOAT fX1        = TFLOAT( iOffsetX ) + TFLOAT( iX );
		TFLOAT fY1        = TFLOAT( iY ) + TFLOAT( iOffsetY ) - TFLOAT( iHeight ) + TFLOAT( pGlyph->GetBaseline() );

		InsertRectIntoMesh(
		    pTexture,
		    a_rcColour,
		    fX1,
		    fY1,
		    TFLOAT( a_pOffset->m_iWidth ) + TFLOAT( iWidth ) + fX1,
		    TFLOAT( iHeight ) + TFLOAT( a_pOffset->m_iHeight ) + fY1,
		    TFLOAT( iDepth ),
		    TFLOAT( iGlyphX ) * fInvWidth,
		    TFLOAT( iGlyphY ) * fInvHeight,
		    fInvWidth * TFLOAT( iGlyphX + iWidth + 1 ),
		    fInvHeight * TFLOAT( iGlyphY + iHeight + 1 ),
		    StripDirection_Horizontal
		);

		iX += pGlyph->GetKerningRight() + pGlyph->GetKerningLeft() + a_pFont->GetSpacing() + iWidth;
	}

	m_iDepth -= 2;
}

// Draws the part of a texture section from a_fStartAngle to a_fEndAngle (clockwise on the screen) around (a_iX, a_iY)
// $JPOG: FUNCTION PGUIRenderer.dll+00002190
void PGUITRDisplayContext::DrawTexturePie( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TFLOAT a_fStartAngle, TFLOAT a_fEndAngle )
{
	TMath::NormaliseAngle( a_fStartAngle );
	TMath::NormaliseAngle( a_fEndAngle );

	if ( a_fStartAngle < 0.0f )
		a_fStartAngle += 2.0f * TMath::PI;

	if ( a_fEndAngle < 0.0f )
		a_fEndAngle += 2.0f * TMath::PI;

	TINT iStartSide = GetPieSide( a_fStartAngle );
	TINT iEndSide   = GetPieSide( a_fEndAngle );

	// Side of the end before it goes round
	TINT iEndSideFirstTurn = iEndSide;

	if ( a_fStartAngle > a_fEndAngle )
		iEndSide += 4;

	PGUITRTextureFactory::Texture* pTexture = m_pTextureFactory->GetTexture( a_pTextureSection->GetTexture() );

	TUINT16 uiX, uiY, uiWidth, uiHeight;
	a_pTextureSection->GetBBox( uiX, uiY, uiWidth, uiHeight, m_fTime );

	TFLOAT fWidth          = TFLOAT( uiWidth );
	TFLOAT fHeight         = TFLOAT( uiHeight );
	TFLOAT fTextureWidth   = TFLOAT( pTexture->GetWidth() );
	TFLOAT fTextureHeight  = TFLOAT( pTexture->GetHeight() );
	TFLOAT fCentreU        = TFLOAT( uiX ) + fWidth * 0.5f;
	TFLOAT fCentreV        = TFLOAT( uiY ) + fHeight * 0.5f;
	TFLOAT fHalfWidth      = 0.5f * fWidth;
	TFLOAT fHalfHeight     = 0.5f * fHeight;
	TFLOAT fInvTexWidth    = 1.0f / fTextureWidth;
	TFLOAT fInvTexHeight   = 1.0f / fTextureHeight;

	SetShaderColour( a_rcColour );
	m_pShader->SetMaterial( pTexture->GetMaterial() );

	TFLOAT fCentreX = TFLOAT( a_iX ) - 0.5f + TFLOAT( GetClipRect().m_iX );
	TFLOAT fCentreY = TFLOAT( a_iY ) - 0.5f + TFLOAT( GetClipRect().m_iY );

	TFLOAT fX1, fY1, fX2, fY2;

	// NOTE: a pie that starts and ends on the same side with the start before the end (0.1 to 6.2 radians on the right
	// side) is drawn as the single triangle between them instead of going round
	if ( iEndSide == iStartSide )
	{
		// Both on the same side: one triangle
		GetPieSidePoint( iStartSide, a_fEndAngle, fX1, fY1 );
		GetPieSidePoint( iStartSide, a_fStartAngle, fX2, fY2 );
		RenderPieTriangle( m_pShader, fCentreX, fCentreY, fHalfWidth, fHalfHeight, fCentreU, fCentreV, fInvTexWidth, fInvTexHeight, fX1, fY1, fX2, fY2 );
	}
	else
	{
		// From the start to the corner that ends its side
		GetPieSidePoint( iStartSide, ( TFLOAT( iStartSide ) + TFLOAT( iStartSide ) + 1.0f ) * TMath::PI * 0.25f, fX1, fY1 );
		GetPieSidePoint( iStartSide, a_fStartAngle, fX2, fY2 );
		RenderPieTriangle( m_pShader, fCentreX, fCentreY, fHalfWidth, fHalfHeight, fCentreU, fCentreV, fInvTexWidth, fInvTexHeight, fX1, fY1, fX2, fY2 );

		// From the corner that starts the side of the end to the end
		GetPieSidePoint( iEndSide & 3, a_fEndAngle, fX1, fY1 );
		GetPieSidePoint( iEndSide & 3, ( -1.0f + TFLOAT( iEndSideFirstTurn ) + TFLOAT( iEndSideFirstTurn ) ) * TMath::PI * 0.25f, fX2, fY2 );
		RenderPieTriangle( m_pShader, fCentreX, fCentreY, fHalfWidth, fHalfHeight, fCentreU, fCentreV, fInvTexWidth, fInvTexHeight, fX1, fY1, fX2, fY2 );

		// The whole sides between them
		for ( TINT i = iStartSide + 1; i < iEndSide; i++ )
		{
			TINT iSide = i & 3;

			GetPieSidePoint( iSide, ( TFLOAT( iSide ) + TFLOAT( iSide ) + 3.0f ) * TMath::PI * 0.25f, fX1, fY1 );
			GetPieSidePoint( iSide, ( TFLOAT( iSide ) + 1.0f + TFLOAT( iSide ) ) * TMath::PI * 0.25f, fX2, fY2 );
			RenderPieTriangle( m_pShader, fCentreX, fCentreY, fHalfWidth, fHalfHeight, fCentreU, fCentreV, fInvTexWidth, fInvTexHeight, fX1, fY1, fX2, fY2 );
		}
	}

	m_iDepth--;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00003610
void PGUITRDisplayContext::InsertRectIntoMesh( PGUITRTextureFactory::Texture* a_pTexture, const TGUIColour& a_rcColour, TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2, StripDirection a_eDirection )
{
	SetShaderColour( a_rcColour );
	m_pShader->SetMaterial( a_pTexture->GetMaterial() );

	const TGUIRectangle& rcClipRect = GetClipRect();
	TFLOAT               fClipX     = TFLOAT( rcClipRect.m_iX );
	TFLOAT               fClipY     = TFLOAT( rcClipRect.m_iY );

	// NOTE: the depth and the direction aren't passed (see the class comment, the sprite shader ignored the direction)
	m_pShader->RenderRectangle(
	    TVector2( a_fX1 + -0.5f + fClipX, a_fY1 + -0.5f + fClipY ),
	    TVector2( a_fX2 + -0.5f + fClipX, a_fY2 + -0.5f + fClipY ),
	    TVector2( a_fU1, a_fV1 ),
	    TVector2( a_fU2, a_fV2 )
	);
}

// $JPOG: FUNCTION PGUIRenderer.dll+000036d0
void PGUITRDisplayContext::FollowRectIntoMesh( PGUITRTextureFactory::Texture* a_pTexture, const TGUIColour& a_rcColour, TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2, StripDirection a_eDirection )
{
	SetShaderColour( a_rcColour );
	m_pShader->SetMaterial( a_pTexture->GetMaterial() );

	const TGUIRectangle& rcClipRect = GetClipRect();
	TFLOAT               fClipX     = TFLOAT( rcClipRect.m_iX );
	TFLOAT               fClipY     = TFLOAT( rcClipRect.m_iY );

	// NOTE: the depth and the direction aren't passed (see the class comment, the sprite shader ignored the direction)
	m_pShader->RenderRectangle(
	    TVector2( a_fX1 + -0.5f + fClipX, a_fY1 + -0.5f + fClipY ),
	    TVector2( a_fX2 + -0.5f + fClipX, a_fY2 + -0.5f + fClipY ),
	    TVector2( a_fU1, a_fV1 ),
	    TVector2( a_fU2, a_fV2 )
	);
}

// $JPOG: FUNCTION PGUIRenderer.dll+00003790
void PGUITRDisplayContext::Imp_DrawTextureSection( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TGUIAlignment a_eAlignX, TGUIAlignment a_eAlignY )
{
	TUINT16 uiX, uiY, uiWidth, uiHeight;
	a_pTextureSection->GetBBox( uiX, uiY, uiWidth, uiHeight, m_fTime );

	TINT iSectionWidth  = uiWidth;
	TINT iSectionHeight = uiHeight;
	TINT iNumX          = a_iWidth / iSectionWidth;
	TINT iNumY          = a_iHeight / iSectionHeight;
	TINT iOffsetX       = 0;
	TINT iOffsetY       = 0;

	// Where the first whole section starts (centred: the sections are centred, -1: stretched)
	switch ( a_eAlignX )
	{
		case TGUIAlignment_Center:
			iOffsetX = ( a_iWidth % iSectionWidth ) / 2;

			if ( iNumX > 0 && ( iNumX & 1 ) == 0 )
				iOffsetX += iSectionWidth / 2;

			break;
		case TGUIAlignment_Left:
			iOffsetX = 0;
			break;
		case TGUIAlignment_Right:
			iOffsetX = a_iWidth % iSectionWidth;
			break;
		case TGUIAlignment_NUMOF:
			iOffsetX = -1;
			break;
	}

	switch ( a_eAlignY )
	{
		case TGUIAlignment_Center:
			// NOTE: a shift here, a division for x
			iOffsetY = ( a_iHeight % iSectionHeight ) >> 1;

			if ( iNumY > 0 && ( iNumY & 1 ) == 0 )
				iOffsetY += iSectionHeight / 2;

			break;
		case TGUIAlignment_Top:
			iOffsetY = 0;
			break;
		case TGUIAlignment_Bottom:
			iOffsetY = a_iHeight % iSectionHeight;
			break;
		case TGUIAlignment_NUMOF:
			iOffsetY = -1;
			break;
	}

	RenderTexture( a_pTextureSection, a_rcColour, a_iX, a_iY, a_iWidth, a_iHeight, iOffsetX, iOffsetY );
	m_iDepth--;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00003910
void PGUITRDisplayContext::Imp_DrawOutlineRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour )
{
	SetShaderColour( a_rcColour );

	const TGUIRectangle& rcClipRect = GetClipRect();
	TVector2             vecMin( TFLOAT( a_iX1 ) + -0.5f + TFLOAT( rcClipRect.m_iX ), TFLOAT( a_iY1 ) + -0.5f + TFLOAT( rcClipRect.m_iY ) );
	TVector2             vecMax( TFLOAT( a_iX2 ) + -0.5f + TFLOAT( rcClipRect.m_iX ), -0.5f + TFLOAT( a_iY2 ) + TFLOAT( rcClipRect.m_iY ) );

	// NOTE: the depth isn't passed (see the class comment)
	m_pShader->RenderOutlineRectangle( vecMin, vecMax );
	m_iDepth--;
}

// $JPOG: FUNCTION PGUIRenderer.dll+000039e0
void PGUITRDisplayContext::Imp_DrawFilledRectangle( TINT a_iX1, TINT a_iY1, TINT a_iX2, TINT a_iY2, const TGUIColour& a_rcColour )
{
	SetShaderColour( a_rcColour );

	const TGUIRectangle& rcClipRect = GetClipRect();
	TVector2             vecMin( TFLOAT( a_iX1 ) + -0.5f + TFLOAT( rcClipRect.m_iX ), TFLOAT( a_iY1 ) + -0.5f + TFLOAT( rcClipRect.m_iY ) );
	TVector2             vecMax( TFLOAT( a_iX2 ) + -0.5f + TFLOAT( rcClipRect.m_iX ), -0.5f + TFLOAT( a_iY2 ) + TFLOAT( rcClipRect.m_iY ) );

	// NOTE: the depth isn't passed (see the class comment)
	m_pShader->RenderFilledRectangle( vecMin, vecMax );
	m_iDepth--;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00003ab0
void PGUITRDisplayContext::RenderTexture( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iOffsetX, TINT a_iOffsetY )
{
	if ( !a_pTextureSection )
		return;

	TUINT16 uiX, uiY, uiWidth, uiHeight;
	a_pTextureSection->GetBBox( uiX, uiY, uiWidth, uiHeight, m_fTime );

	if ( a_iWidth % uiWidth != 0 || a_iHeight % uiHeight != 0 )
	{
		RenderComplex( a_pTextureSection, a_rcColour, a_iX, a_iY, a_iWidth, a_iHeight, a_iOffsetX, a_iOffsetY );
		return;
	}

	PGUITRTextureFactory::Texture* pTexture = m_pTextureFactory->GetTexture( a_pTextureSection->GetTexture() );

	TFLOAT fInvTexWidth  = 1.0f / TFLOAT( pTexture->GetWidth() );
	TFLOAT fInvTexHeight = 1.0f / TFLOAT( pTexture->GetHeight() );
	TINT   iTileWidth    = uiWidth;
	TINT   iTileHeight   = uiHeight;
	TINT   iNumX         = a_iWidth / uiWidth;
	TINT   iNumY         = a_iHeight / uiHeight;

	TFLOAT fU1 = TFLOAT( uiX ) * fInvTexWidth;
	TFLOAT fV1 = TFLOAT( uiY ) * fInvTexHeight;
	TFLOAT fU2 = fInvTexWidth * TFLOAT( uiWidth + uiX );
	TFLOAT fV2 = fInvTexHeight * TFLOAT( uiHeight + uiY );

	MirrorUV( fU1, fV1, fU2, fV2, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

	// Stretched
	if ( a_iOffsetX == -1 )
	{
		iNumX      = 1;
		iTileWidth = TUINT16( a_iWidth );
	}

	if ( a_iOffsetY == -1 )
	{
		iNumY       = 1;
		iTileHeight = TUINT16( a_iHeight );
	}

	TFLOAT fZ          = TFLOAT( m_iDepth );
	TFLOAT fTileWidth  = TFLOAT( iTileWidth );
	TFLOAT fTileHeight = TFLOAT( iTileHeight );

	if ( iNumX >= iNumY )
	{
		// Rows
		TFLOAT fY1 = TFLOAT( a_iY );
		TFLOAT fY2 = fTileHeight + TFLOAT( a_iY );

		for ( TINT iRow = 0; iRow < iNumY; iRow++ )
		{
			InsertRectIntoMesh( pTexture, a_rcColour, TFLOAT( a_iX ), fY1, fTileWidth + TFLOAT( a_iX ), fY2, fZ, fU1, fV1, fU2, fV2, StripDirection_Horizontal );

			TFLOAT fX1 = TFLOAT( a_iX ) + fTileWidth;
			TFLOAT fX2 = fTileWidth + TFLOAT( a_iX ) + fTileWidth;

			for ( TINT iColumn = 1; iColumn < iNumX; iColumn++ )
			{
				FollowRectIntoMesh( pTexture, a_rcColour, fX1, fY1, fX2, fY2, fZ, fU1, fV1, fU2, fV2, StripDirection_Horizontal );

				fX1 = fX1 + fTileWidth;
				fX2 = fTileWidth + fX2;
			}

			fY1 = fY1 + fTileHeight;
			fY2 = fTileHeight + fY2;
		}
	}
	else
	{
		// Columns
		TFLOAT fX1 = TFLOAT( a_iX );
		TFLOAT fX2 = fTileWidth + TFLOAT( a_iX );

		for ( TINT iColumn = 0; iColumn < iNumX; iColumn++ )
		{
			InsertRectIntoMesh( pTexture, a_rcColour, fX1, TFLOAT( a_iY ), fX2, fTileHeight + TFLOAT( a_iY ), fZ, fU1, fV1, fU2, fV2, StripDirection_Vertical );

			TFLOAT fY1 = TFLOAT( a_iY ) + fTileHeight;
			TFLOAT fY2 = fTileHeight + TFLOAT( a_iY ) + fTileHeight;

			for ( TINT iRow = 1; iRow < iNumY; iRow++ )
			{
				FollowRectIntoMesh( pTexture, a_rcColour, fX1, fY1, fX2, fY2, fZ, fU1, fV1, fU2, fV2, StripDirection_Vertical );

				fY1 = fY1 + fTileHeight;
				fY2 = fTileHeight + fY2;
			}

			fX1 = fX1 + fTileWidth;
			fX2 = fTileWidth + fX2;
		}
	}
}

// $JPOG: FUNCTION PGUIRenderer.dll+000040d0
void PGUITRDisplayContext::RenderComplex( const TGUITextureSection* a_pTextureSection, const TGUIColour& a_rcColour, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iOffsetX, TINT a_iOffsetY )
{
	PGUITRTextureFactory::Texture* pTexture = m_pTextureFactory->GetTexture( a_pTextureSection->GetTexture() );

	TFLOAT fTextureWidth  = TFLOAT( pTexture->GetWidth() );
	TFLOAT fTextureHeight = TFLOAT( pTexture->GetHeight() );

	TUINT16 uiX, uiY, uiWidth, uiHeight;
	a_pTextureSection->GetBBox( uiX, uiY, uiWidth, uiHeight, m_fTime );

	TINT iTileWidth  = TMath::Min( a_iWidth, TINT( uiWidth ) );
	TINT iTileHeight = TMath::Min( a_iHeight, TINT( uiHeight ) );

	// The number of whole sections and the size of the partial ones at the right/bottom (the left/top ones are the offset)
	TINT iNumX;
	TINT iNumY;
	TINT iRight;
	TINT iBottom;

	if ( a_iOffsetX < 0 )
	{
		iNumX      = 1;
		a_iOffsetX = 0;
		iTileWidth = a_iWidth;
		iRight     = 0;
	}
	else
	{
		iNumX  = ( a_iWidth - a_iOffsetX ) / uiWidth;
		iRight = a_iWidth + iNumX * -TINT( uiWidth ) - a_iOffsetX;
	}

	if ( a_iOffsetY < 0 )
	{
		iNumY       = 1;
		a_iOffsetY  = 0;
		iTileHeight = a_iHeight;
		iBottom     = 0;
	}
	else
	{
		iNumY   = ( a_iHeight - a_iOffsetY ) / uiHeight;
		iBottom = a_iHeight + iNumY * -TINT( uiHeight ) - a_iOffsetY;
	}

	TFLOAT fStartX = TFLOAT( a_iX + a_iOffsetX );
	TFLOAT fStartY = TFLOAT( a_iY + a_iOffsetY );
	TFLOAT fZ      = TFLOAT( m_iDepth );

	TFLOAT fInvTexWidth  = 1.0f / fTextureWidth;
	TFLOAT fInvTexHeight = 1.0f / fTextureHeight;
	TFLOAT fU1           = TFLOAT( uiX ) * fInvTexWidth;
	TFLOAT fV1           = TFLOAT( uiY ) * fInvTexHeight;
	TFLOAT fU2           = fInvTexWidth * TFLOAT( uiWidth + uiX );
	TFLOAT fV2           = fInvTexHeight * TFLOAT( uiHeight + uiY );
	MirrorUV( fU1, fV1, fU2, fV2, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

	TFLOAT fX          = TFLOAT( a_iX );
	TFLOAT fY          = TFLOAT( a_iY );
	TFLOAT fWidth      = TFLOAT( a_iWidth );
	TFLOAT fHeight     = TFLOAT( a_iHeight );
	TFLOAT fTileWidth  = TFLOAT( iTileWidth );
	TFLOAT fTileHeight = TFLOAT( iTileHeight );

	// The whole sections
	if ( iNumX >= iNumY )
	{
		TFLOAT fY1 = fStartY;
		TFLOAT fY2 = fStartY + fTileHeight;

		for ( TINT iRow = 0; iRow < iNumY; iRow++ )
		{
			InsertRectIntoMesh( pTexture, a_rcColour, fStartX, fY1, fStartX + fTileWidth, fY2, fZ, fU1, fV1, fU2, fV2, StripDirection_Horizontal );

			TFLOAT fX1 = fStartX + fTileWidth;
			TFLOAT fX2 = fX1 + fTileWidth;

			for ( TINT iColumn = 1; iColumn < iNumX; iColumn++ )
			{
				FollowRectIntoMesh( pTexture, a_rcColour, fX1, fY1, fX2, fY2, fZ, fU1, fV1, fU2, fV2, StripDirection_Horizontal );

				fX1 = fX1 + fTileWidth;
				fX2 = fTileWidth + fX2;
			}

			fY1 = fY1 + fTileHeight;
			fY2 = fTileHeight + fY2;
		}
	}
	else
	{
		TFLOAT fX1 = fStartX;
		TFLOAT fX2 = fStartX + fTileWidth;

		for ( TINT iColumn = 0; iColumn < iNumX; iColumn++ )
		{
			InsertRectIntoMesh( pTexture, a_rcColour, fX1, fStartY, fX2, fStartY + fTileHeight, fZ, fU1, fV1, fU2, fV2, StripDirection_Vertical );

			TFLOAT fY1 = fStartY + fTileHeight;
			TFLOAT fY2 = fY1 + fTileHeight;

			for ( TINT iRow = 1; iRow < iNumY; iRow++ )
			{
				FollowRectIntoMesh( pTexture, a_rcColour, fX1, fY1, fX2, fY2, fZ, fU1, fV1, fU2, fV2, StripDirection_Vertical );

				fY1 = fY1 + fTileHeight;
				fY2 = fTileHeight + fY2;
			}

			fX1 = fX1 + fTileWidth;
			fX2 = fTileWidth + fX2;
		}
	}

	// NOTE: the partial sections of the left and right columns are cut at the bottom only for the first row, those of
	// the top and bottom rows at the right only for the first column
	TFLOAT fRightX1  = fX - TFLOAT( iRight ) + fWidth;
	TFLOAT fBottomY1 = fY - TFLOAT( iBottom ) + fHeight;

	// The left column
	if ( a_iOffsetX > 0 )
	{
		TFLOAT fX2     = TFLOAT( a_iOffsetX ) + fX;
		TFLOAT fY1     = fTileHeight + fStartY;
		TFLOAT fY2     = TMath::Min( fY1, TFLOAT( a_iY + a_iHeight - iBottom ) );
		TFLOAT fSideU1 = TFLOAT( uiWidth + uiX - a_iOffsetX ) * fInvTexWidth;
		TFLOAT fSideV1 = TFLOAT( uiY ) * fInvTexHeight;
		TFLOAT fSideU2 = fInvTexWidth * TFLOAT( uiWidth + uiX );
		TFLOAT fSideV2 = fInvTexHeight * TFLOAT( uiHeight + uiY );
		MirrorUV( fSideU1, fSideV1, fSideU2, fSideV2, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

		InsertRectIntoMesh( pTexture, a_rcColour, fX, fStartY, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Vertical );
		fY2 = fY2 + fTileHeight;

		for ( TINT iRow = 1; iRow < iNumY; iRow++ )
		{
			FollowRectIntoMesh( pTexture, a_rcColour, fX, fY1, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Vertical );

			fY1 = fY1 + fTileHeight;
			fY2 = fTileHeight + fY2;
		}
	}

	// The right column
	if ( iRight > 0 )
	{
		TFLOAT fX2     = fWidth + fX;
		TFLOAT fY1     = fTileHeight + fStartY;
		TFLOAT fY2     = TMath::Min( fY1, fBottomY1 );
		TFLOAT fSideU1 = TFLOAT( uiX ) * fInvTexWidth;
		TFLOAT fSideV1 = TFLOAT( uiY ) * fInvTexHeight;
		TFLOAT fSideU2 = fInvTexWidth * TFLOAT( iRight + uiX );
		TFLOAT fSideV2 = fInvTexHeight * TFLOAT( iTileHeight + uiY );
		MirrorUV( fSideU1, fSideV1, fSideU2, fSideV2, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

		InsertRectIntoMesh( pTexture, a_rcColour, fRightX1, fStartY, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Vertical );
		fY2 = fY2 + fTileHeight;

		for ( TINT iRow = 1; iRow < iNumY; iRow++ )
		{
			FollowRectIntoMesh( pTexture, a_rcColour, fRightX1, fY1, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Vertical );

			fY1 = fY1 + fTileHeight;
			fY2 = fTileHeight + fY2;
		}
	}

	// The top row
	if ( a_iOffsetY > 0 )
	{
		TFLOAT fX1     = fTileWidth + fStartX;
		TFLOAT fX2     = TMath::Min( fX1, fRightX1 );
		TFLOAT fY2     = TFLOAT( a_iOffsetY ) + fY;
		TFLOAT fSideU1 = TFLOAT( uiX ) * fInvTexWidth;
		TFLOAT fSideV1 = TFLOAT( uiHeight + uiY - a_iOffsetY ) * fInvTexHeight;
		TFLOAT fSideU2 = fInvTexWidth * TFLOAT( uiWidth + uiX );
		TFLOAT fSideV2 = fInvTexHeight * TFLOAT( uiHeight + uiY );
		MirrorUV( fSideU1, fSideV1, fSideU2, fSideV2, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

		InsertRectIntoMesh( pTexture, a_rcColour, fStartX, fY, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Horizontal );
		fX2 = fX2 + fTileWidth;

		for ( TINT iColumn = 1; iColumn < iNumX; iColumn++ )
		{
			FollowRectIntoMesh( pTexture, a_rcColour, fX1, fY, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Horizontal );

			fX1 = fX1 + fTileWidth;
			fX2 = fTileWidth + fX2;
		}
	}

	// The bottom row
	if ( iBottom > 0 )
	{
		TFLOAT fX1     = fTileWidth + fStartX;
		TFLOAT fX2     = TMath::Min( fX1, fRightX1 );
		TFLOAT fY2     = fHeight + fY;
		TFLOAT fSideU1 = TFLOAT( uiX ) * fInvTexWidth;
		TFLOAT fSideV1 = TFLOAT( uiY ) * fInvTexHeight;
		TFLOAT fSideU2 = fInvTexWidth * TFLOAT( uiWidth + uiX );
		TFLOAT fSideV2 = fInvTexHeight * TFLOAT( iBottom + uiY );
		MirrorUV( fSideU1, fSideV1, fSideU2, fSideV2, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

		InsertRectIntoMesh( pTexture, a_rcColour, fStartX, fBottomY1, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Horizontal );
		fX2 = fX2 + fTileWidth;

		for ( TINT iColumn = 1; iColumn < iNumX; iColumn++ )
		{
			FollowRectIntoMesh( pTexture, a_rcColour, fX1, fBottomY1, fX2, fY2, fZ, fSideU1, fSideV1, fSideU2, fSideV2, StripDirection_Horizontal );

			fX1 = fX1 + fTileWidth;
			fX2 = fTileWidth + fX2;
		}
	}

	// The corners
	if ( a_iOffsetX > 0 )
	{
		TFLOAT fCornerU1 = TFLOAT( uiWidth + uiX - a_iOffsetX ) * fInvTexWidth;
		TFLOAT fCornerU2 = fInvTexWidth * TFLOAT( uiWidth + uiX );

		if ( a_iOffsetY > 0 )
		{
			// Top left
			TFLOAT fU1Corner = fCornerU1;
			TFLOAT fU2Corner = fCornerU2;
			TFLOAT fV1Corner = TFLOAT( uiHeight + uiY - a_iOffsetY ) * fInvTexHeight;
			TFLOAT fV2Corner = fInvTexHeight * TFLOAT( uiHeight + uiY );
			MirrorUV( fU1Corner, fV1Corner, fU2Corner, fV2Corner, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

			InsertRectIntoMesh( pTexture, a_rcColour, fX, fY, TFLOAT( a_iOffsetX ) + fX, TFLOAT( a_iOffsetY ) + fY, fZ, fU1Corner, fV1Corner, fU2Corner, fV2Corner, StripDirection_Horizontal );
		}

		if ( iBottom > 0 )
		{
			// Bottom left
			TFLOAT fU1Corner = fCornerU1;
			TFLOAT fU2Corner = fCornerU2;
			TFLOAT fV1Corner = TFLOAT( uiY ) * fInvTexHeight;
			TFLOAT fV2Corner = fInvTexHeight * TFLOAT( iBottom + uiY );
			MirrorUV( fU1Corner, fV1Corner, fU2Corner, fV2Corner, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

			InsertRectIntoMesh( pTexture, a_rcColour, fX, fBottomY1, TFLOAT( a_iOffsetX ) + fX, fY + fHeight, fZ, fU1Corner, fV1Corner, fU2Corner, fV2Corner, StripDirection_Horizontal );
		}
	}

	if ( iRight > 0 )
	{
		TFLOAT fCornerU1 = TFLOAT( uiX ) * fInvTexWidth;
		TFLOAT fCornerU2 = fInvTexWidth * TFLOAT( iRight + uiX );

		if ( iBottom > 0 )
		{
			// Bottom right
			TFLOAT fU1Corner = fCornerU1;
			TFLOAT fU2Corner = fCornerU2;
			TFLOAT fV1Corner = TFLOAT( uiY ) * fInvTexHeight;
			TFLOAT fV2Corner = fInvTexHeight * TFLOAT( iBottom + uiY );
			MirrorUV( fU1Corner, fV1Corner, fU2Corner, fV2Corner, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

			InsertRectIntoMesh( pTexture, a_rcColour, fRightX1, fBottomY1, fX + fWidth, fY + fHeight, fZ, fU1Corner, fV1Corner, fU2Corner, fV2Corner, StripDirection_Horizontal );
		}

		if ( a_iOffsetY > 0 )
		{
			// Top right
			TFLOAT fU1Corner = fCornerU1;
			TFLOAT fU2Corner = fCornerU2;
			TFLOAT fV1Corner = TFLOAT( uiHeight + uiY - a_iOffsetY ) * fInvTexHeight;
			TFLOAT fV2Corner = fInvTexHeight * TFLOAT( uiHeight + uiY );
			MirrorUV( fU1Corner, fV1Corner, fU2Corner, fV2Corner, a_pTextureSection->GetMirrorX(), a_pTextureSection->GetMirrorY() );

			InsertRectIntoMesh( pTexture, a_rcColour, fRightX1, fY, fX + fWidth, TFLOAT( a_iOffsetY ) + fY, fZ, fU1Corner, fV1Corner, fU2Corner, fV2Corner, StripDirection_Horizontal );
		}
	}
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005b60
void PGUITRDisplayContext::MirrorUV( TFLOAT& a_rU1, TFLOAT& a_rV1, TFLOAT& a_rU2, TFLOAT& a_rV2, TBOOL a_bMirrorX, TBOOL a_bMirrorY )
{
	if ( a_bMirrorX )
	{
		TFLOAT fTemp = a_rU1;
		a_rU1        = a_rU2;
		a_rU2        = fTemp;
	}

	if ( a_bMirrorY )
	{
		TFLOAT fTemp = a_rV1;
		a_rV1        = a_rV2;
		a_rV2        = fTemp;
	}
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005a10
TFLOAT PGUITRDisplayContext::GetBackDepth()
{
	return -15.0f;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005a00
TFLOAT PGUITRDisplayContext::GetFrontDepth()
{
	return -6551.5f;
}

// $JPOG: FUNCTION PGUIRenderer.dll+000059f0
TFLOAT PGUITRDisplayContext::GetDepthIncrement()
{
	return -1.0f;
}

// Sets the colour of the T2GUI renderer, modulated by the modulate colour
void PGUITRDisplayContext::SetShaderColour( const TGUIColour& a_rcColour )
{
	TGUIColour oColour(
	    TUINT8( ( a_rcColour.GetRed() * m_oModulateColour.GetRed() ) / 255 ),
	    TUINT8( ( a_rcColour.GetGreen() * m_oModulateColour.GetGreen() ) / 255 ),
	    TUINT8( ( a_rcColour.GetBlue() * m_oModulateColour.GetBlue() ) / 255 ),
	    TUINT8( ( a_rcColour.GetAlpha() * m_oModulateColour.GetAlpha() ) / 255 )
	);

	m_pShader->SetColour( oColour.GetARGB() );
}

// Sets the transform of the display context on the T2GUI renderer: the screen transform, the scale of the viewport and a_rcTransform
void PGUITRDisplayContext::SetShaderTransform( const TMatrix44& a_rcTransform )
{
	T2GUITransform oTransform;
	oTransform.Set(
	    TVector2( a_rcTransform.m_f11, a_rcTransform.m_f12 ),
	    TVector2( a_rcTransform.m_f21, a_rcTransform.m_f22 ),
	    TVector2( a_rcTransform.m_f41, a_rcTransform.m_f42 )
	);

	T2GUITransform oScale;
	oScale.Set( TVector2( m_fScaleX, 0.0f ), TVector2( 0.0f, m_fScaleY ), TVector2::VEC_ZERO );

	T2GUITransform oScreen;
	T2GUITransform::Multiply( oScreen, m_oScreenTransform, oScale );

	T2GUITransform oResult;
	T2GUITransform::Multiply( oResult, oScreen, oTransform );

	m_pShader->PopTransform();
	m_pShader->PushTransform( oResult, TVector2::VEC_ZERO, TVector2::VEC_ZERO );
}
