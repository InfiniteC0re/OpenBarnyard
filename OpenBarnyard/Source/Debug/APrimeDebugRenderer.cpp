#include "pch.h"
#include "APrimeDebugRenderer.h"

#ifdef PRIMEDEBUG_DEV
#  include "Render/ARenderer.h"
#  include "Cameras/ACameraManager.h"
#  include "GUI/AGUI2.h"

#  include <GUI/T2GUIFont.h>
#  include <GUI/T2GUIFontManager.h>
#  include <GUI/T2GUIRenderer.h>

#  include "Render/ALineShader/ALineShader.h"

#  if defined( TRENDERINTERFACE_DX8 )
#	include <Platform/DX8/CD3DFont.h>
#  elif defined( TRENDERINTERFACE_DX11 )
#	include <Platform/DX11/CD3DFont.h>
#  elif defined( TRENDERINTERFACE_GL )
#	include <Platform/GL/TGLFont.h>
#  endif

#  include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

APrimeDebugRenderer::APrimeDebugRenderer()
    : m_pFont( TNULL )
    , m_bIn2D( TFALSE )
    , m_fRootWidth( 0.0f )
    , m_fRootHeight( 0.0f )
{
#  ifdef APRIMEDEBUG_D3DFONT
	m_iNumFonts = 0;
#  endif
}

APrimeDebugRenderer::~APrimeDebugRenderer()
{
#  ifdef APRIMEDEBUG_D3DFONT
	for ( TINT i = 0; i < m_iNumFonts; i++ )
	{
		m_aFonts[ i ].pFont->InvalidateDeviceObjects();
		m_aFonts[ i ].pFont->DeleteDeviceObjects();
		delete m_aFonts[ i ].pFont;
	}

	m_iNumFonts = 0;
	m_oGlyphs.Free();
#  endif

	// NOTE: the GUI font isn't released: the renderer goes after the GUI, which destroyed its fonts already
	m_pFont = TNULL;
}

TBOOL APrimeDebugRenderer::Begin2D()
{
	if ( !AGUI2::IsSingletonCreated() || !AGUI2::GetRootElement() || !AGUI2::GetRenderer() )
		return TFALSE;

#  ifndef APRIMEDEBUG_D3DFONT
	if ( !m_pFont )
		m_pFont = T2GUIFontManager::FindFont( "Rekord18" );
#  endif

	AGUI2::GetRootElement()->GetDimensions( m_fRootWidth, m_fRootHeight );

	AGUI2::GetRenderer()->BeginScene();
	m_bIn2D = TTRUE;
	return TTRUE;
}

void APrimeDebugRenderer::End2D()
{
	if ( !m_bIn2D )
		return;

	AGUI2::GetRenderer()->EndScene();
	m_bIn2D = TFALSE;

#  ifdef APRIMEDEBUG_D3DFONT
	// The font of TOSHI's debug text (CD3DFont), fixed width, the characters centred in their cells
	// NOTE: the fonts aren't restored after a lost device (development only)
	TFLOAT fScreenWidth  = TFLOAT( GetScreenWidth() );
	TFLOAT fScreenHeight = TFLOAT( GetScreenHeight() );

	for ( TINT i = 0; i < m_oGlyphs.Size(); i++ )
	{
		const PDebugGlyph& rcGlyph = m_oGlyphs[ i ];

		if ( rcGlyph.cChar == ' ' || TUINT8( rcGlyph.cChar ) < ' ' || TUINT8( rcGlyph.cChar ) > '~' )
			continue;

		TINT iPixels = TINT( rcGlyph.fHalfHeight * 2.0f * fScreenHeight * 0.8f + 0.5f );

		if ( iPixels < 4 )
			continue;

		TFLOAT    fCharWidth;
		TDebugFontHAL* pFont = GetFont( iPixels, fCharWidth );

		TCHAR  szChar[ 2 ] = { rcGlyph.cChar, '\0' };
		TFLOAT fX          = TFLOAT( TINT( rcGlyph.fX * fScreenWidth - fCharWidth * 0.5f ) );
		TFLOAT fY          = TFLOAT( TINT( rcGlyph.fY * fScreenHeight - TFLOAT( iPixels ) * 0.6f ) );
		pFont->DrawText( fX, fY, rcGlyph.uiColour, szChar, 0 );
	}

	m_oGlyphs.SetSize( 0 );
#  endif
}

void APrimeDebugRenderer::ToGUI( TFLOAT a_fX, TFLOAT a_fY, TFLOAT& a_rfX, TFLOAT& a_rfY )
{
	// The AGUI2 root element is centred on the screen, y down
	a_rfX = ( a_fX - 0.5f ) * m_fRootWidth;
	a_rfY = ( a_fY - 0.5f ) * m_fRootHeight;
}

void APrimeDebugRenderer::DrawRect( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour )
{
	if ( !m_bIn2D || a_fWidth <= 0.0f || a_fHeight <= 0.0f )
		return;

	TVector2 vMin, vMax;
	ToGUI( a_fX, a_fY, vMin.x, vMin.y );
	ToGUI( a_fX + a_fWidth, a_fY + a_fHeight, vMax.x, vMax.y );

	T2GUIRenderer* pRenderer = AGUI2::GetRenderer();
	pRenderer->SetColour( a_uiColour );
	pRenderer->RenderFilledRectangle( vMin, vMax );
}

#  ifdef APRIMEDEBUG_D3DFONT

TDebugFontHAL* APrimeDebugRenderer::GetFont( TINT a_iPixels, TFLOAT& a_rfCharWidth )
{
	for ( TINT i = 0; i < m_iNumFonts; i++ )
	{
		if ( m_aFonts[ i ].iPixels == a_iPixels )
		{
			a_rfCharWidth = m_aFonts[ i ].fCharWidth;
			return m_aFonts[ i ].pFont;
		}
	}

	// The oldest font goes when all are used
	if ( m_iNumFonts == MAX_FONTS )
	{
		m_aFonts[ 0 ].pFont->InvalidateDeviceObjects();
		m_aFonts[ 0 ].pFont->DeleteDeviceObjects();
		delete m_aFonts[ 0 ].pFont;

		for ( TINT i = 1; i < MAX_FONTS; i++ )
			m_aFonts[ i - 1 ] = m_aFonts[ i ];

		m_iNumFonts--;
	}

	// CD3DFont's height is in points
	HDC  hDC  = GetDC( NULL );
	TINT iDPI = GetDeviceCaps( hDC, LOGPIXELSY );
	ReleaseDC( NULL, hDC );

	if ( iDPI <= 0 )
		iDPI = 96;

	TINT iPoints = TMath::Max( 4, ( a_iPixels * 72 + iDPI / 2 ) / iDPI );

	TDebugFontHAL* pFont = new TDebugFontHAL( "Lucida Console", DWORD( iPoints ), 0 );
#	ifdef TRENDERINTERFACE_DX8
	pFont->InitDeviceObjects( TRenderInterfaceHAL::Interface()->GetDirect3DDevice() );
#	else
	pFont->InitDeviceObjects( TRenderInterfaceHAL::Interface() );
#	endif
	pFont->RestoreDeviceObjects();

#	ifdef TRENDERINTERFACE_GL
	TINT iCharWidth = 0, iCharHeight = 0;
	pFont->GetTextExtent( "W", &iCharWidth, &iCharHeight );
#	else
	SIZE oSize = {};
	pFont->GetTextExtent( "W", &oSize );
	const TINT iCharWidth = oSize.cx;
#	endif

	Font& rFont      = m_aFonts[ m_iNumFonts++ ];
	rFont.iPixels    = a_iPixels;
	rFont.fCharWidth = TFLOAT( iCharWidth );
	rFont.pFont      = pFont;

	a_rfCharWidth = rFont.fCharWidth;
	return pFont;
}

#  endif

void APrimeDebugRenderer::DrawGlyphs( const PDebugGlyph* a_pGlyphs, TINT a_iNumGlyphs )
{
	if ( !m_bIn2D )
		return;

#  ifdef APRIMEDEBUG_D3DFONT
	// Drawn after the AGUI2 scene (its renderer may batch the rectangles)
	for ( TINT i = 0; i < a_iNumGlyphs; i++ )
		m_oGlyphs.PushBack( a_pGlyphs[ i ] );
#  else

	if ( !m_pFont || m_pFont->GetLineHeight() <= 0.0f )
		return;

	TFLOAT fLineHeight = m_pFont->GetLineHeight();
	TFLOAT fWideWidth  = m_pFont->GetCharWidth( L'W', 1.0f );

	for ( TINT i = 0; i < a_iNumGlyphs; i++ )
	{
		const PDebugGlyph& rcGlyph = a_pGlyphs[ i ];

		if ( rcGlyph.cChar == ' ' || TUINT8( rcGlyph.cChar ) < ' ' )
			continue;

		// The character fills the height of its cell, unless the widest ones wouldn't fit its width
		TFLOAT fCellWidth  = rcGlyph.fHalfWidth * 2.0f * m_fRootWidth;
		TFLOAT fCellHeight = rcGlyph.fHalfHeight * 2.0f * m_fRootHeight;
		TFLOAT fScale      = fCellHeight / fLineHeight;

		if ( fWideWidth > 0.0f && fWideWidth * fScale > fCellWidth * 1.15f )
			fScale = fCellWidth * 1.15f / fWideWidth;

		TWCHAR wszChar[ 2 ] = { TWCHAR( TUINT8( rcGlyph.cChar ) ), L'\0' };
		TFLOAT fCharWidth   = m_pFont->GetCharWidth( wszChar[ 0 ], fScale );

		TFLOAT fX, fY;
		ToGUI( rcGlyph.fX, rcGlyph.fY, fX, fY );

		m_pFont->DrawTextSingleLine( wszChar, fX - fCharWidth * 0.5f, fY - fLineHeight * fScale * 0.5f, rcGlyph.uiColour, fScale );
	}
#  endif
}

void APrimeDebugRenderer::DrawLines2D( const PDebugVertex* a_pVertices, TINT a_iNumVertices )
{
	if ( !m_bIn2D )
		return;

	T2GUIRenderer* pRenderer = AGUI2::GetRenderer();
	pRenderer->SetMaterial( TNULL );

	for ( TINT i = 0; i + 1 < a_iNumVertices; i += 2 )
	{
		TFLOAT fX1, fY1, fX2, fY2;
		ToGUI( a_pVertices[ i ].fX, a_pVertices[ i ].fY, fX1, fY1 );
		ToGUI( a_pVertices[ i + 1 ].fX, a_pVertices[ i + 1 ].fY, fX2, fY2 );

		pRenderer->SetColour( a_pVertices[ i ].uiColour );
		pRenderer->RenderLine( fX1, fY1, fX2, fY2 );
	}
}

void APrimeDebugRenderer::DrawLines3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth, TBOOL a_bOverSolid )
{
	Draw3D( a_pVertices, a_iNumVertices, a_bDepth, TTRUE, a_bOverSolid );
}

void APrimeDebugRenderer::DrawTriangles3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth )
{
	Draw3D( a_pVertices, a_iNumVertices, a_bDepth, TFALSE, TFALSE );
}

void APrimeDebugRenderer::Draw3D( const PDebugVertex* a_pVertices, TINT a_iNumVertices, TBOOL a_bDepth, TBOOL a_bLines, TBOOL a_bOverSolid )
{
#  if defined( TRENDERINTERFACE_DX8 ) || defined( TRENDERINTERFACE_DX11 ) || defined( TRENDERINTERFACE_GL )
	if ( a_iNumVertices <= 0 || !ALineShader::IsSingletonCreated() )
		return;

	ALineShader* pLineShader = ALineShader::GetSingleton();

	if ( a_bLines )
	{
		// The wire of a solid shape is moved a little towards the camera, so it shows over the shape's front and
		// hides behind it like Prime's (which moves it in the depth)
		TVector3 vEye( 0.0f, 0.0f, 0.0f );
		TFLOAT   fNudge = 0.0f;

		if ( a_bOverSolid && ACameraManager::IsSingletonCreated() && ACameraManager::GetSingleton()->GetCurrentCamera() )
		{
			const TVector4& rcEye = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
			vEye.Set( rcEye.x, rcEye.y, rcEye.z );
			fNudge = WIRE_NUDGE;
		}

		auto fnNudge = [ & ]( const PDebugVertex& a_rcVertex ) {
			return TVector3( a_rcVertex.fX + ( vEye.x - a_rcVertex.fX ) * fNudge, a_rcVertex.fY + ( vEye.y - a_rcVertex.fY ) * fNudge, a_rcVertex.fZ + ( vEye.z - a_rcVertex.fZ ) * fNudge );
		};

		for ( TINT i = 0; i + 1 < a_iNumVertices; i += 2 )
		{
			const PDebugVertex& rcA = a_pVertices[ i ];
			const PDebugVertex& rcB = a_pVertices[ i + 1 ];
			pLineShader->AddLineDirect( fnNudge( rcA ), fnNudge( rcB ), rcA.uiColour, rcB.uiColour );
		}
	}
	else
	{
		for ( TINT i = 0; i + 2 < a_iNumVertices; i += 3 )
		{
			TVector3 aVertices[ 3 ] = {
				TVector3( a_pVertices[ i ].fX, a_pVertices[ i ].fY, a_pVertices[ i ].fZ ),
				TVector3( a_pVertices[ i + 1 ].fX, a_pVertices[ i + 1 ].fY, a_pVertices[ i + 1 ].fZ ),
				TVector3( a_pVertices[ i + 2 ].fX, a_pVertices[ i + 2 ].fY, a_pVertices[ i + 2 ].fZ ),
			};

			pLineShader->AddTriangle( aVertices, a_pVertices[ i ].uiColour );
		}
	}

	// As AWorldDebugDraw: in the space of the world view matrix of the main viewport's context
	TRenderInterfaceHAL* pRender    = TRenderInterfaceHAL::Interface();
	TRenderContext*      pContext   = pRender->GetCurrentContext();
	TMatrix44            oOldMatrix = pContext->GetModelViewMatrix();

	pContext->SetModelViewMatrix( pContext->GetWorldViewMatrix() );

#	ifdef TRENDERINTERFACE_GL
	TRenderInterfaceHAL::CULLMODE eCull = pRender->GetCullMode();
	pRender->SetZEnable( a_bDepth );
	pRender->SetZWriteEnable( a_bDepth && !a_bLines );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetSrcBlend( GL_SRC_ALPHA );
	pRender->SetDestBlend( GL_ONE_MINUS_SRC_ALPHA );
	pRender->SetCullMode( TRenderInterfaceHAL::CULL_NONE );

	pLineShader->FlushLines();

	pRender->SetZEnable( TTRUE );
	pRender->SetZWriteEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TFALSE );
	pRender->SetCullMode( eCull );
#	elif defined( TRENDERINTERFACE_DX11 )
	// The states are cached by the renderer: the main pass's ones are set back
	D3D11_CULL_MODE eCull = pRender->GetCullMode();
	// The tested solids write the depth, so the wires drawn after them (a little nearer) hide behind them
	pRender->SetZEnable( a_bDepth );
	pRender->SetZWriteEnable( a_bDepth && !a_bLines );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
	pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
	pRender->SetCullMode( D3D11_CULL_NONE );

	pLineShader->FlushLines();

	pRender->SetZEnable( TTRUE );
	pRender->SetZWriteEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TFALSE );
	pRender->SetCullMode( eCull );
#	else
	IDirect3DDevice8* pDevice = pRender->GetDirect3DDevice();

	DWORD uiLighting, uiFog, uiZEnable, uiZWrite, uiAlphaBlend, uiAlphaTest, uiSrcBlend, uiDestBlend, uiCull, uiZFunc;
	pDevice->GetRenderState( D3DRS_LIGHTING, &uiLighting );
	pDevice->GetRenderState( D3DRS_FOGENABLE, &uiFog );
	pDevice->GetRenderState( D3DRS_ZENABLE, &uiZEnable );
	pDevice->GetRenderState( D3DRS_ZWRITEENABLE, &uiZWrite );
	pDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &uiAlphaBlend );
	pDevice->GetRenderState( D3DRS_ALPHATESTENABLE, &uiAlphaTest );
	pDevice->GetRenderState( D3DRS_SRCBLEND, &uiSrcBlend );
	pDevice->GetRenderState( D3DRS_DESTBLEND, &uiDestBlend );
	pDevice->GetRenderState( D3DRS_CULLMODE, &uiCull );
	pDevice->GetRenderState( D3DRS_ZFUNC, &uiZFunc );

	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZENABLE, a_bDepth ? D3DZB_TRUE : D3DZB_FALSE );
	// The tested solids write the depth, so the wires drawn after them (a little nearer) hide behind them
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, ( a_bDepth && !a_bLines ) ? TRUE : FALSE );
	pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	pLineShader->FlushLines();

	pDevice->SetRenderState( D3DRS_LIGHTING, uiLighting );
	pDevice->SetRenderState( D3DRS_FOGENABLE, uiFog );
	pDevice->SetRenderState( D3DRS_ZENABLE, uiZEnable );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, uiZWrite );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, uiAlphaBlend );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, uiAlphaTest );
	pDevice->SetRenderState( D3DRS_SRCBLEND, uiSrcBlend );
	pDevice->SetRenderState( D3DRS_DESTBLEND, uiDestBlend );
	pDevice->SetRenderState( D3DRS_CULLMODE, uiCull );
	pDevice->SetRenderState( D3DRS_ZFUNC, uiZFunc );
#	endif

	pContext->SetModelViewMatrix( oOldMatrix );
#  endif
}

TINT APrimeDebugRenderer::GetScreenWidth()
{
	return TINT( TRenderInterface::GetSingleton()->GetCurrentDisplayParams()->uiWidth );
}

TINT APrimeDebugRenderer::GetScreenHeight()
{
	return TINT( TRenderInterface::GetSingleton()->GetCurrentDisplayParams()->uiHeight );
}

TBOOL APrimeDebugRenderer::ProjectToScreen( const TVector4& a_rcPosition, TFLOAT& a_rfX, TFLOAT& a_rfY )
{
	ACamera* pCamera = ACameraManager::IsSingletonCreated() ? ACameraManager::GetSingleton()->GetCurrentCamera() : TNULL;

	if ( !pCamera || !ARenderer::IsSingletonCreated() )
		return TFALSE;

	TVector2 vScreen;

	if ( !ARenderer::GetSingleton()->ProjectToScreen( pCamera, a_rcPosition, vScreen ) )
		return TFALSE;

	a_rfX = vScreen.x / TFLOAT( GetScreenWidth() );
	a_rfY = vScreen.y / TFLOAT( GetScreenHeight() );
	return TTRUE;
}

#endif // PRIMEDEBUG_DEV
