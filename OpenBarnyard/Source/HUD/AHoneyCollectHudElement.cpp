#include "pch.h"
#include "AHoneyCollectHudElement.h"
#include "GUI/AGUI2.h"
#include "MiniGame/AHoneyCollectingGame.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUIRenderer.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMath.h>
#include <Math/TMathInline.h>
#include <Render/TRenderInterface.h>
#include <Render/TTextureFactory.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Scale of the comb (00773680, never changed)
static const TFLOAT s_fScale = 1.0f;

// Height of the honey in the full jar (00782040)
static TFLOAT s_fJarHeight;

// Height of the honey of one comb (00782044)
static TFLOAT s_fCombHeight;

// Height of the honey of one section of the comb (00782048)
static TFLOAT s_fSectionHeight;

// Angle of one section of the comb (0078204c)
static const TFLOAT s_fSectionAngle = TMath::TWO_PI * ( 1.0f / AHoneyCollectHudElement::NUM_SECTIONS );

// Colours of the pulsing corners
static constexpr TFLOAT PULSE_RED   = 0.8f; // 0077368c
static constexpr TFLOAT PULSE_GREEN = 0.2f; // 00773688
static constexpr TFLOAT PULSE_BLUE  = 0.2f; // 00773684

//-----------------------------------------------------------------------------
// AHoneyCollectHudElement::GUI2Triangle
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004531f0
AHoneyCollectHudElement::GUI2Triangle::GUI2Triangle()
{
	TTextureFactory* pTextureFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );

	// A white texture
	void* pData = TMalloc( 256 );
	TUtil::MemSet( pData, 0xFF, 256 );
	m_pTexture = pTextureFactory->CreateEx( pData, 256, 8, 8, 0, TTEXTURERESOURCEFORMAT::R8G8B8A8, 0 );
	TFree( pData );

	m_pMaterial = T2GUI::GetRenderer()->CreateMaterial( m_pTexture );
	m_pMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_MASK );
}

// $Barnyard: FUNCTION 00452cc0
// $Barnyard: FUNCTION 004532b0
AHoneyCollectHudElement::GUI2Triangle::~GUI2Triangle()
{
	T2GUI::GetRenderer()->DestroyMaterial( m_pMaterial );
	m_pTexture->DestroyResource();
}

// $Barnyard: FUNCTION 00452d30
void AHoneyCollectHudElement::GUI2Triangle::Render()
{
	// The tip is at the top, the base at the bottom of the element
	m_aVertices[ 0 ].x = m_fWidth;
	m_aVertices[ 0 ].y = m_fHeight;
	m_aVertices[ 0 ].x *= -0.5f;
	m_aVertices[ 0 ].y *= -0.5f;
	m_aVertices[ 0 ].x = m_fWidth * 0.5f + m_aVertices[ 0 ].x;

	m_aVertices[ 1 ].x = m_fWidth;
	m_aVertices[ 1 ].y = m_fHeight;
	m_aVertices[ 1 ].x *= -0.5f;
	m_aVertices[ 1 ].y *= -0.5f;
	m_aVertices[ 1 ].x += m_fWidth;
	m_aVertices[ 1 ].y += m_fHeight;

	m_aVertices[ 2 ].x = m_fWidth;
	m_aVertices[ 2 ].y = m_fHeight;
	m_aVertices[ 2 ].x *= -0.5f;
	m_aVertices[ 2 ].y *= -0.5f;
	m_aVertices[ 2 ].y += m_fHeight;

	m_aUVs[ 0 ] = TVector2( 0.5f, 0.0f );
	m_aUVs[ 1 ] = TVector2( 1.0f, 1.0f );
	m_aUVs[ 2 ] = TVector2( 0.0f, 1.0f );

	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();
	pRenderer->SetColour( m_uiColour );
	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->RenderTriStrip( m_aVertices, m_aUVs, 3 );

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 00452e30
TBOOL AHoneyCollectHudElement::GUI2Triangle::IsPointInsideTriangle( const TFLOAT& a_rfX, const TFLOAT& a_rfY )
{
	T2GUITransform oInvTransform;
	GetInvScreenTransform( oInvTransform );

	TVector2 vScreenPoint( a_rfX, a_rfY );
	TVector2 vPoint;
	oInvTransform.Transform( vPoint, vScreenPoint );

	return TMath::IsPointInTriangle2D( m_aVertices[ 0 ], m_aVertices[ 1 ], m_aVertices[ 2 ], vPoint );
}

//-----------------------------------------------------------------------------
// AHoneyCollectHudElement
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004532d0
AHoneyCollectHudElement::AHoneyCollectHudElement()
    : m_pSections( TNULL )
    , m_iHighlightedSection( -1 )
    , m_bFull( TTRUE )
    , m_iRefillSection( 0 )
    , m_bPulse( TFALSE )
    , m_iNumCollected( 0 )
    , m_iMouseSection( -1 )
    , m_bMouseInput( TFALSE )
    , m_pGame( TNULL )
{
}

// $Barnyard: FUNCTION 00452e90
// $Barnyard: FUNCTION 004533f0
AHoneyCollectHudElement::~AHoneyCollectHudElement()
{
}

// $Barnyard: FUNCTION 00453020
void AHoneyCollectHudElement::Tick( TFLOAT a_fDeltaTime )
{
	// Refill one section per frame
	if ( !m_bFull )
	{
		m_pSections->SetBit( m_iRefillSection++, 1 );

		if ( m_iRefillSection == NUM_SECTIONS )
			m_bFull = TTRUE;
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004539a0
void AHoneyCollectHudElement::Render()
{
	if ( !m_pSections )
		return;

	for ( TINT i = 0; i < NUM_CORNERS; i++ )
		m_aCorners[ i ].Show();

	m_oBackground.PreRender();
	m_oBackground.Render();
	m_oBackground.PostRender();

	m_oScreen.PreRender();
	m_oScreen.Render();
	m_oScreen.PostRender();

	TFLOAT fAngle      = 0.0f;
	TFLOAT fArrowAngle = -1.0f;

	m_oComb.PreRender();
	m_iMouseSection = -1;

	for ( TINT i = 0; i < NUM_SECTIONS; i++ )
	{
		T2GUITransform& rTransform = m_oSection.GetTransform();
		rTransform.SetIdentity();
		rTransform.Rotate( TMath::PI + fAngle );

		T2GUITransform oScale;
		oScale.m_aMatrixRows[ 0 ] = TVector2( s_fScale, 0.0f );
		oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, s_fScale );
		oScale.m_vecTranslation   = TVector2::VEC_ZERO;
		rTransform.PreMultiply( oScale );

		// The mouse sweeps the sections while its button is held
		if ( m_bMouseInput == TTRUE )
		{
			const TVector2& rCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
			TFLOAT          fCursorX   = rCursorPos.x;
			TFLOAT          fCursorY   = rCursorPos.y;

			if ( m_iMouseSection == -1 && m_oSection.IsPointInsideTriangle( fCursorX, fCursorY ) == TTRUE )
			{
				m_iMouseSection = i;
				CollectSection( i );
			}
		}

		// The empty sections are covered
		if ( !m_pSections->GetBit( i ) )
		{
			m_oSection.PreRender();
			m_oSection.Render();
			m_oSection.PostRender();
		}

		if ( i == m_iMouseSection )
			fArrowAngle = fAngle;
		else if ( i == m_iHighlightedSection )
			fArrowAngle = fAngle;

		fAngle += s_fSectionAngle;
	}

	TUINT32 uiColour;

	if ( m_bPulse == TTRUE )
	{
		TFLOAT fPulse = TFLOAT( ( sin( TMath::TWO_PI * g_oSystemManager.GetScheduler()->GetTotalTime() ) + 1.0f ) * 0.5f );
		TFLOAT fWhite = 1.0f - fPulse;

		TUINT32 uiRed   = TUINT32( TINT( ( PULSE_RED * fPulse + fWhite ) * 255.0f ) );
		TUINT32 uiGreen = TUINT32( TINT( ( PULSE_GREEN * fPulse + fWhite ) * 255.0f ) );
		TUINT32 uiBlue  = TUINT32( TINT( ( PULSE_BLUE * fPulse + fWhite ) * 255.0f ) );

		uiColour = ( ( ( uiRed | 0xFFFFFF00 ) << 8 | uiGreen ) << 8 ) | uiBlue;
	}
	else
	{
		uiColour = 0xFFFFFFFF;
	}

	for ( TINT i = 0; i < NUM_CORNERS; i++ )
		m_aCorners[ i ].SetColour( uiColour );

	// The arrow points at the current section
	if ( m_iHighlightedSection == -1 && m_iMouseSection == -1 )
	{
		m_oArrow.Hide();
	}
	else
	{
		m_oArrow.GetTransform().SetIdentity();
		m_oArrow.GetTransform().Rotate( fArrowAngle );
		m_oArrow.Show();
	}

	m_oComb.Render();
	m_oComb.PostRender();
	m_oComb.PreRender();

	m_oJar.PreRender();
	m_oJar.Render();
	m_oJar.PostRender();

	m_oJarMask.PreRender();
	m_oJarMask.Render();
	m_oJarMask.PostRender();

	m_oJarGlint.PreRender();
	m_oJarGlint.Render();
	m_oJarGlint.PostRender();

	m_oComb.PostRender();

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 00453410
TBOOL AHoneyCollectHudElement::Create( AHoneyCollectingGame* a_pGame )
{
	m_pGame = a_pGame;

	// The corners of the comb
	T2GUITextureSection* pCornerSection = T2GUITextureSectionManager::GetTextureSection( "Honey_Corner" );

	for ( TINT i = 0; i < NUM_CORNERS; i++ )
	{
		T2GUIRectangle& rCorner = m_aCorners[ i ];

		rCorner.SetTextureSection( *pCornerSection );
		rCorner.SetTransform( 0.0f, 0.0f, TFLOAT( i ) * TMath::HALF_PI );
		rCorner.SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 8 ) );

		T2GUIMaterial* pMaterial = rCorner.GetMaterial();
		pMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );
		pMaterial->SetName( "HoneyCombCorner" );

		m_oComb.AddChildTail( rCorner );
	}

	TFLOAT fHalfScreenHeight = AGUI2::GetRootElement()->GetHeight() * 0.5f;
	m_oComb.SetDimensions( m_aCorners[ 0 ].GetWidth() * 2.0f, m_aCorners[ 0 ].GetHeight() * 2.0f );

	// The background, at the top of the screen
	m_oBackground.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Hive_Background" ) );
	m_oBackground.GetMaterial()->SetName( "Hive_Background" );
	m_oBackground.SetDimensions( m_oComb.GetWidth() * 1.25f, m_oComb.GetHeight() * 1.25f );

	T2GUITransform oScale;
	oScale.m_aMatrixRows[ 0 ] = TVector2( s_fScale, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, s_fScale );
	oScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oBackground.GetTransform().PreMultiply( oScale );
	m_oBackground.GetTransform().m_vecTranslation.x = 4.0f;
	m_oBackground.GetTransform().m_vecTranslation.y = m_oBackground.GetHeight() * s_fScale * 0.5f - fHalfScreenHeight + 25.0f;
	m_oBackground.GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );

	m_oComb.GetTransform().PreMultiply( oScale );
	m_oComb.GetTransform().m_vecTranslation.x = 0.0f;
	m_oComb.GetTransform().m_vecTranslation.y = m_oBackground.GetHeight() * s_fScale * 0.5f - fHalfScreenHeight + 25.0f;

	// Darkens the screen
	TFLOAT fScreenWidth;
	TFLOAT fScreenHeight;
	AGUI2::GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );
	m_oScreen.SetDimensions( fScreenWidth, fScreenHeight );
	m_oScreen.SetColour( 0 );

	AGUI2::GetRootElement()->AddChildTail( *this );

	// The sections
	T2GUITextureSection* pPieSection = T2GUITextureSectionManager::GetTextureSection( "pie_section" );
	m_oSection.SetDimensions(
	    TMath::Abs( TFLOAT( pPieSection->GetPoint2X() - pPieSection->GetPoint1X() ) ),
	    TMath::Abs( TFLOAT( pPieSection->GetPoint2Y() - pPieSection->GetPoint1Y() ) )
	);
	m_oSection.SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 1 ) );
	m_oSection.Hide();

	if ( m_oSection.m_pMaterial )
		m_oSection.m_pMaterial->SetName( "pie_section" );

	TFLOAT fRatio = m_aCorners[ 0 ].GetHeight() / m_oSection.GetHeight();
	m_oSection.SetWidth( m_oSection.GetWidth() * ( fRatio / s_fScale ) );
	m_oSection.SetHeight( m_oSection.GetHeight() * ( fRatio / s_fScale ) );
	m_oSection.GetTransform().PreMultiply( oScale );

	// The arrow
	m_oArrow.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Honey_Arrow" ) );
	m_oArrow.SetAttachment( T2GUIATTACHMENT( 4 ), T2GUIATTACHMENT( 7 ) );
	T2GUIMaterial* pArrowMaterial = m_oArrow.GetMaterial();
	pArrowMaterial->SetName( "Honey_Arrow" );
	pArrowMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_ALPHA_ZWRITE );

	m_oComb.AddChildTail( m_oArrow );
	m_oComb.AddChildTail( m_oSection );
	AddChildTail( m_oComb );
	m_oComb.Hide();

	// The jar
	m_oJar.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Jar_Bkg" ) );
	m_oComb.AddChildTail( m_oJar );
	m_oJar.Hide();
	T2GUIMaterial* pJarMaterial = m_oJar.GetMaterial();
	pJarMaterial->SetName( "Jar_Comb" );
	pJarMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_ALPHA_ZWRITE );

	m_oJarMask.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Jar_Mask" ) );
	m_oJarMask.SetDimensions( m_oJar.GetWidth(), m_oJar.GetHeight() );
	m_oComb.AddChildTail( m_oJarMask );
	m_oJarMask.Hide();

	s_fJarHeight     = m_oJarMask.GetHeight() - 9.0f;
	s_fCombHeight    = s_fJarHeight * 0.2f;
	s_fSectionHeight = s_fCombHeight * 0.015625f;

	m_oJarHoney.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Honey_Rectangle" ) );
	m_oJarMask.AddChildTail( m_oJarHoney );
	m_oJarHoney.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	m_oJarHoney.SetWidth( m_oJarMask.GetWidth() );
	m_oJarHoney.SetHeight( 0.0f );
	m_oJarHoney.GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );

	m_oJarGlint.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Jar_Glint" ) );
	m_oComb.AddChildTail( m_oJarGlint );
	m_oJarGlint.Hide();

	m_oTransform.SetIdentity();
	SetDimensions( m_oComb.GetWidth(), m_oComb.GetHeight() );

	return TTRUE;
}

// $Barnyard: FUNCTION 00452f80
void AHoneyCollectHudElement::Destroy()
{
	for ( TINT i = 0; i < NUM_CORNERS; i++ )
		m_aCorners[ i ].RemoveSelf();

	m_oSection.RemoveSelf();
	m_oComb.RemoveSelf();
	m_oScreen.RemoveSelf();
	m_oArrow.RemoveSelf();
	m_oJar.RemoveSelf();
	m_oJarMask.RemoveSelf();
	m_pGame = TNULL;
	RemoveSelf();
}

// $Barnyard: FUNCTION 004530a0
void AHoneyCollectHudElement::SetHive( TINT a_iMaxHoney, TINT a_iHoney, TBitArray* a_pBits )
{
	s_fCombHeight = s_fJarHeight / TFLOAT( a_iMaxHoney );
	m_iHoney      = a_iHoney;
	m_fJarLevel   = TFLOAT( a_iMaxHoney - a_iHoney ) * s_fCombHeight + TFLOAT( m_iNumCollected ) * s_fSectionHeight;
	m_oJarHoney.SetHeight( s_fJarHeight - m_fJarLevel );
	m_pSections = a_pBits;
}

// $Barnyard: FUNCTION 00453110
TINT AHoneyCollectHudElement::GetNumFullSections()
{
	TBitArray* pBits = m_pSections;
	TINT       iSize = pBits->GetSize();
	TINT       iNum  = 0;

	for ( TINT i = 0; i < iSize; i++ )
	{
		if ( pBits->GetBit( i ) )
			iNum++;
	}

	return iNum;
}

// $Barnyard: FUNCTION 00453150
void AHoneyCollectHudElement::CollectSection( TINT a_iSection )
{
	if ( m_pSections->GetBit( a_iSection ) )
	{
		m_iNumCollected++;

		TFLOAT fHeight = m_oJarHoney.GetHeight() - s_fSectionHeight;
		TMath::Clip( fHeight, 0.0f, s_fJarHeight );
		m_oJarHoney.SetHeight( fHeight );

		m_pGame->AddJarHoney( s_fSectionHeight );
	}

	m_pSections->SetBit( a_iSection, 0 );
}

// $Barnyard: FUNCTION 00453060
void AHoneyCollectHudElement::Reset( TBOOL a_bFull )
{
	if ( a_bFull == TTRUE )
	{
		m_pSections->Reset();
		m_bFull               = TTRUE;
		m_iNumCollected       = 0;
		m_iHighlightedSection = -1;
	}
	else
	{
		m_bFull               = TFALSE;
		m_iRefillSection      = 0;
		m_iNumCollected       = 0;
		m_iHighlightedSection = -1;
	}
}

// $Barnyard: FUNCTION 004531d0
void AHoneyCollectHudElement::GetCentre( TFLOAT& a_rfX, TFLOAT& a_rfY )
{
	a_rfX = m_oBackground.GetTransform().m_vecTranslation.x;
	a_rfY = m_oBackground.GetTransform().m_vecTranslation.y;
}
