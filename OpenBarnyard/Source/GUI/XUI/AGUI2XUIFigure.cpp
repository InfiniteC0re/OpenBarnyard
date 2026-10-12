#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIFigure.h"
#  include "AGUI2XUIResource.h"

#  include <GUI/T2GUI.h>
#  include <Render/TRenderInterface.h>
#  include <Render/TTextureFactory.h>
#  include <Math/TMathInline.h>
#  include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIFigure );

// $BarnyardWiiJP: FUNCTION 804181b0
AGUI2XUIFigure::AGUI2XUIFigure()
{
	m_bClosed             = TFALSE;
	m_uiDirtyFlags        = DIRTY_NONE;
	m_pUVs                = TNULL;
	m_pMaterial           = TNULL;
	m_pGradientTexture    = TNULL;
	m_fStrokeWidth        = 0.0f;
	m_uiNumGradientStops  = 0;
	m_pGradientColours    = TNULL;
	m_pGradientPositions  = TNULL;
	m_bRadialGradient     = TFALSE;
	m_vecOriginalSize     = TVector2::VEC_ZERO;
	m_vecScale            = TVector2( 1.0f, 1.0f );
	m_pShapeKey           = TNULL;
	m_pShape              = TNULL;
	m_pStrokeVertices     = TNULL;
	m_uiNumStrokeVertices = 0;
	m_pGradientPixels     = TNULL;
	m_uiDirtyFlags |= DIRTY_UNK1;
	m_oFillTransform.Set( TVector2( 1.0f, 0.0f ), TVector2( 0.0f, 1.0f ), TVector2( 0.0f, 0.0f ) );
}

// $BarnyardWiiJP: FUNCTION 80418298
// $BarnyardWiiJP: FUNCTION 804197cc
AGUI2XUIFigure::~AGUI2XUIFigure()
{
	if ( m_pGradientTexture )
		m_pGradientTexture->DestroyResource();

	// NOTE: the original calls OnDestroy and deletes the material itself, which is what DestroyMaterial does
	if ( m_pMaterial )
		T2GUI::GetRenderer()->DestroyMaterial( m_pMaterial );

	delete[] m_pUVs;
	delete[] m_pGradientColours;
	delete[] m_pGradientPositions;
	delete[] m_pStrokeVertices;
	TFree( m_pGradientPixels );
	AGUI2XUIShapeCache::GetSingleton()->ReleaseShape( m_pShape );
	TFree( m_pShapeKey );
}

// $BarnyardWiiJP: FUNCTION 804183b8
TBOOL AGUI2XUIFigure::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	XURXUIFigureData* pData = TSTATICCAST( XURXUIFigureData, a_pData );

	AGUI2XUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	m_bClosed = pData->m_bClosed == TTRUE;

	if ( pData->m_pFill )
		SetupFill( a_rResource, pData->m_pFill );
	else
		m_uiColour = 0xFF0F0F80;

	if ( pData->m_pStroke )
	{
		m_fStrokeWidth   = pData->m_pStroke->m_fStrokeWidth;
		m_uiStrokeColour = pData->m_pStroke->m_uiStrokeColor;
	}

	// Apply the opacity
	TINT iA, iR, iG, iB;
	SplitARGB( m_uiColour, iA, iR, iG, iB );
	iA         = TINT( TFLOAT( iA ) * m_fOpacity );
	m_uiColour = MakeARGB( iR, iG, iB, iA );

	m_pShapeKey              = TSTATICCAST( AGUI2XUIShapeCache::ShapeKey, TMalloc( sizeof( AGUI2XUIShapeCache::ShapeKey ) ) );
	m_pShapeKey->pPoints     = pData->m_pPoints;
	m_pShapeKey->uiNumPoints = pData->m_uiNumPoints;
	m_pShapeKey->vecSize.x   = m_fWidth;
	m_pShapeKey->vecSize.y   = m_fHeight;
	m_pShape                 = AGUI2XUIShapeCache::GetSingleton()->GetShape( m_pShapeKey );

	if ( m_fStrokeWidth > 0.0f )
	{
		m_uiNumStrokeVertices = 2 * m_pShape->m_uiNumVertices + 2;
		m_pStrokeVertices     = new TVector2[ m_uiNumStrokeVertices ];
		m_pShape->BuildStroke( &m_pStrokeVertices, m_vecScale, m_fStrokeWidth );
	}

	UpdateUVs();
	m_vecOriginalSize = TVector2( m_fWidth, m_fHeight );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041859c
void AGUI2XUIFigure::PreRender()
{
	if ( m_uiDirtyFlags & DIRTY_GRADIENT )
		GenerateGradient( m_bRadialGradient );

	if ( m_uiDirtyFlags & DIRTY_STROKE )
	{
		m_pShape->BuildStroke( &m_pStrokeVertices, m_vecScale, m_fStrokeWidth );
		m_uiDirtyFlags &= ~DIRTY_STROKE;
	}

	if ( m_uiDirtyFlags & DIRTY_UVS )
	{
		UpdateUVs();
		m_uiDirtyFlags &= ~DIRTY_UVS;
	}

	AGUI2XUIElement::PreRender();
}

// $BarnyardWiiJP: FUNCTION 8041863c
void AGUI2XUIFigure::Render()
{
	if ( !m_pShape->m_pVertices )
	{
		T2GUIElement::Render();
		return;
	}

	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();
	pRenderer->SetColour( m_uiColour );
	pRenderer->SetMaterial( m_pMaterial );

	if ( m_pShape->m_pIndices )
	{
		pRenderer->SetMaterial( m_pMaterial );

		pRenderer->RenderIndexedTriList( m_pShape->m_pVertices, m_pUVs, m_pShape->m_pIndices, m_pShape->m_uiNumIndices, m_pShape->m_uiNumVertices, m_vecScale.x, m_vecScale.y );
	}

	pRenderer->SetMaterial( TNULL );

	if ( m_pStrokeVertices )
	{
		pRenderer->SetColour( m_uiStrokeColour );

		// NOTE: the Wii version also passes a 1.0, 1.0 scale
		if ( m_pStrokeVertices )
			pRenderer->RenderTriStrip( m_pStrokeVertices, TNULL, m_uiNumStrokeVertices );
	}

	if ( ms_bDrawOutlines )
	{
		pRenderer->SetColour( 0xFFFFFFFF );

		// The outline
		TUINT32 i = 0;
		for ( ; i < m_pShape->m_uiNumVertices - 1; i++ )
			pRenderer->RenderLine( m_pShape->m_pVertices[ i ], m_pShape->m_pVertices[ i + 1 ] );

		pRenderer->RenderLine( m_pShape->m_pVertices[ m_pShape->m_uiNumVertices - 1 ], m_pShape->m_pVertices[ 0 ] );

		// The triangles
		for ( TUINT32 k = 0; k < m_pShape->m_uiNumTriangles; k++ )
			pRenderer->RenderLine( m_pShape->m_pVertices[ m_pShape->m_pIndices[ k * 3 ] ], m_pShape->m_pVertices[ m_pShape->m_pIndices[ k * 3 + 2 ] ] );

		// The stroke
		if ( m_pStrokeVertices )
		{
			TUINT32 k = 0;
			for ( ; k < m_uiNumStrokeVertices - 1; k++ )
				pRenderer->RenderLine( m_pStrokeVertices[ k ], m_pStrokeVertices[ k + 1 ] );

			pRenderer->RenderLine( m_pStrokeVertices[ m_uiNumStrokeVertices - 1 ], m_pStrokeVertices[ 0 ] );
		}
	}

	pRenderer->SetColour( m_uiColour );
	T2GUIElement::Render();
}

// $BarnyardWiiJP: FUNCTION 8041891c
void AGUI2XUIFigure::PostRender()
{
	AGUI2XUIElement::PostRender();
}

// $BarnyardWiiJP: FUNCTION 80418920
TUINT32 AGUI2XUIFigure::LerpColour( TFLOAT a_fT, TUINT32 a_uiFrom, TUINT32 a_uiTo )
{
	TINT aFrom[ 4 ];
	TINT aTo[ 4 ];
	SplitARGB( a_uiFrom, aFrom[ 0 ], aFrom[ 1 ], aFrom[ 2 ], aFrom[ 3 ] );
	SplitARGB( a_uiTo, aTo[ 0 ], aTo[ 1 ], aTo[ 2 ], aTo[ 3 ] );

	for ( TINT i = 0; i < 4; i++ )
		aFrom[ i ] = TINT( TMath::LERP( TFLOAT( aFrom[ i ] ), TFLOAT( aTo[ i ] ), a_fT ) );

	return MakeARGB( aFrom[ 1 ], aFrom[ 2 ], aFrom[ 3 ], aFrom[ 0 ] );
}

// $BarnyardWiiJP: FUNCTION 80418a38
void AGUI2XUIFigure::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	AGUI2XUIElement::SetDimensions( a_fWidth, a_fHeight );
}

// $BarnyardWiiJP: FUNCTION 80418a3c
void AGUI2XUIFigure::GenerateGradient( TBOOL a_bRadial )
{
	TINT iNumRows = ( a_bRadial == TTRUE ) ? 64 : 32;

	if ( !m_pGradientPixels )
		m_pGradientPixels = TSTATICCAST( TUINT32, TMemalign( 32, iNumRows << 8, TNULL ) );

	// A 64 x iNumRows RGBA texture
	if ( !m_pGradientTexture )
	{
		TTextureFactory* pTextureFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );
		m_pGradientTexture               = pTextureFactory->CreateEx( m_pGradientPixels, iNumRows << 8, 64, iNumRows, 1, TTEXTURERESOURCEFORMAT::R8G8B8A8, 0 );
	}

	if ( !m_pMaterial )
		m_pMaterial = T2GUI::GetRenderer()->CreateMaterial( m_pGradientTexture );

	TUINT32* pPixel = m_pGradientPixels;

	// Colour at a_fPos of the gradient
	auto GetColour = [ this ]( TFLOAT a_fPos ) {
		TUINT32 uiStop = 0;

		while ( a_fPos > m_pGradientPositions[ uiStop ] )
			uiStop++;

		TINT iPrevStop = uiStop - 1;

		if ( iPrevStop < 0 )
			iPrevStop = 0;

		return LerpColour(
		    ( a_fPos - m_pGradientPositions[ iPrevStop ] ) / ( m_pGradientPositions[ uiStop ] - m_pGradientPositions[ iPrevStop ] ),
		    m_pGradientColours[ iPrevStop ],
		    m_pGradientColours[ uiStop ]
		);
	};

	if ( a_bRadial == TTRUE )
	{
		TFLOAT fY = 0.0f;

		for ( TINT i = 0; i < iNumRows; i++ )
		{
			TFLOAT fDistY = TMath::Abs( fY - 31.5f );
			fDistY *= fDistY;

			TFLOAT fX = 0.0f;

			for ( TINT k = 0; k < 64; k++ )
			{
				TFLOAT fDistX = TMath::Abs( fX - 31.5f );
				TFLOAT fPos   = TMath::Sqrt( fDistX * fDistX + fDistY ) / 31.5f;

				if ( fPos >= 1.0f || fPos >= m_pGradientPositions[ m_uiNumGradientStops - 1 ] )
					*pPixel++ = m_pGradientColours[ m_uiNumGradientStops - 1 ];
				else if ( fPos <= m_pGradientPositions[ 0 ] )
					*pPixel++ = m_pGradientColours[ 0 ];
				else
					*pPixel++ = GetColour( fPos );

				fX += 1.0f;
			}

			fY += 1.0f;
		}
	}
	else
	{
		for ( TINT i = 0; i < iNumRows; i++ )
		{
			for ( TINT k = 0; k < 64; k++ )
			{
				TFLOAT fPos = TFLOAT( k ) / 63.0f;

				if ( fPos > m_pGradientPositions[ 0 ] )
				{
					if ( fPos < m_pGradientPositions[ m_uiNumGradientStops - 1 ] )
						*pPixel++ = GetColour( fPos );
					else
						*pPixel++ = m_pGradientColours[ m_uiNumGradientStops - 1 ];
				}
				else
				{
					*pPixel++ = m_pGradientColours[ 0 ];
				}
			}
		}
	}

	// NOTE: the Wii build can also save the first gradient to c:	est.tga (a debug flag that's off)

	// Upload the pixels
	TTexture::LOCKSTATE oLockState;

	if ( m_pGradientTexture && m_pGradientTexture->Lock( oLockState ) )
	{
		// NOTE: the Wii texture has no row padding, the D3D one can have
		for ( TINT i = 0; i < iNumRows; i++ )
			TUtil::MemCopy( TSTATICCAST( TUINT8, oLockState.pBits ) + i * oLockState.Pitch, m_pGradientPixels + i * 64, 64 * sizeof( TUINT32 ) );

		m_pGradientTexture->Unlock();
	}

	m_uiDirtyFlags &= ~DIRTY_GRADIENT;
}

// $BarnyardWiiJP: FUNCTION 80418e5c
void AGUI2XUIFigure::SetGradientStopPos( TUINT32 a_uiIndex, TFLOAT a_fPos )
{
	if ( a_uiIndex < m_uiNumGradientStops && a_fPos != m_pGradientPositions[ a_uiIndex ] )
	{
		m_pGradientPositions[ a_uiIndex ] = a_fPos;
		m_uiDirtyFlags |= DIRTY_GRADIENT;
	}
}

// $BarnyardWiiJP: FUNCTION 80418e90
void AGUI2XUIFigure::SetGradientStopColour( TUINT32 a_uiIndex, TUINT32 a_uiColour )
{
	if ( a_uiIndex < m_uiNumGradientStops && a_uiColour != m_pGradientColours[ a_uiIndex ] )
	{
		m_pGradientColours[ a_uiIndex ] = a_uiColour;
		m_uiDirtyFlags |= DIRTY_GRADIENT;
	}
}

// $BarnyardWiiJP: FUNCTION 80418ec4
TBOOL AGUI2XUIFigure::SetupFill( AGUI2XUIResource& a_rResource, XURXUIFillData* a_pFill )
{
	switch ( a_pFill->m_uiFillType )
	{
		case FILLTYPE_NONE:
			m_uiColour = 0;
			break;
		case FILLTYPE_SOLID:
			m_uiColour = a_pFill->m_uiFillColor;
			break;
		case FILLTYPE_LINEARGRADIENT:
			m_uiColour = 0xFFFFFFFF;
			SetGradient( a_pFill->m_pGradient, TFALSE );
			break;
		case FILLTYPE_RADIALGRADIENT:
			m_uiColour = 0xFFFFFFFF;
			SetGradient( a_pFill->m_pGradient, TTRUE );
			break;
		case FILLTYPE_TEXTURE:
		{
			m_uiColour = 0xFFFFFFFF;

			TString8       strTexture( a_rResource.GetString( a_pFill->m_usTextureFileName ) );
			T2GUIRenderer* pRenderer = T2GUI::GetRenderer();
			m_pMaterial              = pRenderer->CreateMaterial( pRenderer->GetTexture( strTexture ) );
			break;
		}
		default:
			return TFALSE;
	}

	m_oFillTransform.Set( TVector2( 1.0f, 0.0f ), TVector2( 0.0f, 1.0f ), TVector2( 0.0f, 0.0f ) );
	m_oFillTransform.Rotate( TMath::DegToRad( a_pFill->m_fRotation ) );

	if ( a_pFill->m_iScale >= 0 )
	{
		const TVector4& rcScale = a_rResource.GetVector( a_pFill->m_iScale );

		T2GUITransform oScale;
		oScale.Set( TVector2( rcScale.x, 0.0f ), TVector2( 0.0f, rcScale.y ), TVector2( 0.0f, 0.0f ) );
		m_oFillTransform.PreMultiply( oScale );
	}

	if ( a_pFill->m_iTranslation >= 0 )
	{
		const TVector4& rcTranslation = a_rResource.GetVector( a_pFill->m_iTranslation );
		m_oFillTransform.SetTranslation( rcTranslation.x, rcTranslation.y );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041911c
TBOOL AGUI2XUIFigure::SetGradient( XURXUIGradientData* a_pGradient, TBOOL a_bRadial )
{
	m_bRadialGradient    = a_bRadial;
	m_uiColour           = 0xFFFFFFFF;
	m_uiNumGradientStops = a_pGradient->m_uiNumStops;
	m_pGradientColours   = new TUINT32[ m_uiNumGradientStops ];
	m_pGradientPositions = new TFLOAT[ m_uiNumGradientStops ];
	TUtil::MemCopy( m_pGradientColours, a_pGradient->m_pStopColors, m_uiNumGradientStops * sizeof( TUINT32 ) );
	TUtil::MemCopy( m_pGradientPositions, a_pGradient->m_pStopPos, m_uiNumGradientStops * sizeof( TFLOAT ) );

	GenerateGradient( m_bRadialGradient );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804191d0
void AGUI2XUIFigure::SetWidth( TFLOAT a_fWidth )
{
	TBOOL bHasStroke = m_fStrokeWidth > 0.0f;
	m_vecScale.x     = a_fWidth / m_vecOriginalSize.x;

	if ( bHasStroke )
		m_uiDirtyFlags |= DIRTY_STROKE;

	m_fWidth = a_fWidth;
	m_uiDirtyFlags |= DIRTY_UVS;
}

// $BarnyardWiiJP: FUNCTION 8041920c
void AGUI2XUIFigure::SetHeight( TFLOAT a_fHeight )
{
	TBOOL bHasStroke = m_fStrokeWidth > 0.0f;
	m_vecScale.y     = a_fHeight / m_vecOriginalSize.y;

	if ( bHasStroke )
		m_uiDirtyFlags |= DIRTY_STROKE;

	m_fHeight = a_fHeight;
	m_uiDirtyFlags |= DIRTY_UVS;
}

// $BarnyardWiiJP: FUNCTION 80419248
TBOOL AGUI2XUIFigure::UpdateUVs()
{
	if ( m_pUVs )
	{
		delete[] m_pUVs;
		m_pUVs = TNULL;
	}

	if ( !m_pMaterial )
		return TTRUE;

	if ( !m_pUVs )
		m_pUVs = new TVector2[ m_pShape->m_uiNumVertices ];

	TFLOAT fMaxU, fMaxV;

	if ( m_pGradientTexture )
	{
		fMaxU = 1.0f;
		fMaxV = 1.0f;
	}
	else
	{
		// The texture repeats
		TTexture* pTexture = m_pMaterial->GetTextureResource();
		fMaxU              = m_fWidth / TFLOAT( pTexture->GetWidth() );
		fMaxV              = m_fHeight / TFLOAT( pTexture->GetHeight() );
		m_pMaterial->SetTextureAddress( T2GUIMaterial::TEXTUREADDRESS_WRAP );
	}

	TVector2 vecMin( m_fWidth * -0.5f, m_fHeight * -0.5f );
	TVector2 vecMax( vecMin.x + m_fWidth, vecMin.y + m_fHeight );
	TVector2 vecRange( vecMax.x - vecMin.x, vecMax.y - vecMin.y );

	for ( TUINT32 i = 0; i < m_pShape->m_uiNumVertices; i++ )
	{
		TVector2& rUV = m_pUVs[ i ];

		rUV.x = ( ( m_pShape->m_pVertices[ i ].x * m_vecScale.x - vecMin.x ) / vecRange.x ) * fMaxU;
		rUV.y = ( ( m_pShape->m_pVertices[ i ].y * m_vecScale.y - vecMin.y ) / vecRange.y ) * fMaxV;
		TMath::Clip( rUV.x, 0.0f, fMaxU );
		TMath::Clip( rUV.y, 0.0f, fMaxV );

		// Apply the fill transform around the centre
		rUV.x -= 0.5f;
		rUV.y -= 0.5f;

		TVector2 vecTransformed;
		m_oFillTransform.Transform( vecTransformed, rUV );
		rUV = vecTransformed;

		rUV.x += 0.5f;
		rUV.y += 0.5f;
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80419598
void AGUI2XUIFigure::UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT )
{
	if ( a_uiProp == XURPROP_FIGURE_STROKE_STROKECOLOR )
		m_uiStrokeColour = InterpColour( a_uiFrom, a_uiTo, a_fT );
	else if ( a_uiProp == XURPROP_FIGURE_FILL_GRADIENT_STOPCOLOR )
		SetGradientStopColour( a_iIndex, InterpColour( a_uiFrom, a_uiTo, a_fT ) );

	AGUI2XUIElement::UpdateTimelinePropValue( a_rResource, a_uiProp, a_uiFrom, a_uiTo, a_iIndex, a_bKeyframeChanged, a_fT );
}

// $BarnyardWiiJP: FUNCTION 80419694
void AGUI2XUIFigure::UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT )
{
	if ( a_uiProp == XURPROP_FIGURE_FILL_GRADIENT_STOPPOS )
		SetGradientStopPos( a_iIndex, TMath::LERP( a_fFrom, a_fTo, a_fT ) );

	AGUI2XUIElement::UpdateTimelinePropFloat( a_rResource, a_uiProp, a_iIndex, a_bKeyframeChanged, a_fFrom, a_fTo, a_fT );
}

#endif // BARNYARD_LATEST
