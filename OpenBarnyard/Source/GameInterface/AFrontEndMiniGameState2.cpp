#include "pch.h"
#include "AFrontEndMiniGameState2.h"
#include "MiniGame/AMiniGameManager.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "GUI/AGUI2FontManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Helpers/APlayerProgress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AFrontEndMiniGameState2 );

// $Barnyard: FUNCTION 00409ce0
AFrontEndMiniGameState2::AFrontEndMiniGameState2( TBOOL a_bHideVariantSelector )
    : m_iSelectedMiniGame( 1 )
	, m_iPrevMiniGame( 1 )
	, m_iNextMiniGame( 1 )
    , m_iHidingMiniGame( 1 )
    , m_pRectangles( TNULL )
    , m_iNumMiniGames( 0 )
    , m_eState( 0 )
    , m_bNoDebug( a_bHideVariantSelector )
    , m_bMouseDirty( TFALSE )
    , m_bMiniGameHovered( TFALSE )
    , m_bLeftArrowHovered( TFALSE )
    , m_bRightArrowHowered( TFALSE )
{
}

// $Barnyard: FUNCTION 00409de0
// $Barnyard: FUNCTION 0040a070
AFrontEndMiniGameState2::~AFrontEndMiniGameState2()
{
}

// $Barnyard: FUNCTION 0040a270
void AFrontEndMiniGameState2::OnInsertion()
{
	m_iSelectedMiniGame = 0;

	m_iNumMiniGames = ( m_bNoDebug ) ?
	    g_pMiniGameManager->GetNumVisibleMiniGames() :
	    g_pMiniGameManager->GetTotalNumMiniGames();

	m_pRectangles = new AGUI2Rectangle[ m_iNumMiniGames ];

	TFLOAT fMaxIconWidth  = 0.0f;
	TFLOAT fMaxIconHeight = 0.0f;

	for ( TINT i = 0; i < m_iNumMiniGames; i++ )
	{
		// Get correct icon for this minigame
		const TCHAR* szIconName = m_bNoDebug ?
		    g_pMiniGameManager->GetVisibleMiniGame( i )->m_szIconName :
		    g_pMiniGameManager->GetMiniGame( i )->m_szIconName;

		AGUI2TextureSection* pIcon = AGUI2TextureSectionManager::GetTextureSection( szIconName );
		TVALIDPTR( pIcon );

		TFLOAT fIconWidth = pIcon->GetWidth();
		TFLOAT fIconHeight = pIcon->GetHeight();

		// Update max size of an icon
		if ( fMaxIconWidth < fIconWidth )
			fMaxIconWidth = fIconWidth;

		if ( fMaxIconHeight < fIconHeight )
			fMaxIconHeight = fIconHeight;

		// Set correct settings for the icon
		m_pRectangles[ i ].SetDimensions( fIconWidth, fIconHeight );
		m_pRectangles[ i ].SetTextureSection( pIcon );

		m_oRoot.AddChildTail( m_pRectangles[ i ] );

		m_iPrevMiniGame = m_iSelectedMiniGame - 1;
		if ( m_iPrevMiniGame < 0 )
			m_iPrevMiniGame = m_iNumMiniGames - 1;

		m_iNextMiniGame = m_iSelectedMiniGame + 1;
		if ( m_iNextMiniGame >= m_iNumMiniGames )
			m_iNextMiniGame = 0;

		if ( i == m_iSelectedMiniGame )
		{
			// Setting up selected minigame element
			m_pRectangles[ i ].GetTransform().Scale( 1.1f, 1.1f );
			m_pRectangles[ i ].SetColour( COLOR_WHITE );

			m_pRectangles[ i ].Show();
		}
		else if ( i == m_iPrevMiniGame )
		{
			// Setting up minigame element which is previous to selected
			m_pRectangles[ i ].SetTransform( -100.0f, 0.0f, 0.0f );

			AGUI2Transform scaleTransform;
			scaleTransform.SetScale( 0.5f, 0.5f );

			m_pRectangles[ i ].GetTransform().PreMultiply( scaleTransform );
			m_pRectangles[ i ].Show();
			m_pRectangles[ i ].SetColour( COLOR_BLACK );

			m_pRectangles[ i ].Show();
		}
		else if ( i == m_iNextMiniGame )
		{
			// Setting up minigame element which is next to selected
			m_pRectangles[ i ].SetTransform( 100.0f, 0.0f, 0.0f );

			AGUI2Transform scaleTransform;
			scaleTransform.SetScale( 0.5f, 0.5f );

			m_pRectangles[ i ].GetTransform().PreMultiply( scaleTransform );
			m_pRectangles[ i ].Show();
			m_pRectangles[ i ].SetColour( COLOR_BLACK );
		}
		else
		{
			m_pRectangles[ i ].Hide();
		}
	}

	// Setup arrows
	AGUI2TextureSection* pArrowLeft = AGUI2TextureSectionManager::GetTextureSection( "left_arrow" );
	m_aArrows[ 0 ].SetDimensions( pArrowLeft->GetWidth(), pArrowLeft->GetHeight() );
	m_aArrows[ 0 ].SetTextureSection( pArrowLeft );
	m_aArrows[ 0 ].SetTransform( -150.0f, 0.0f, 0.0f );

	AGUI2TextureSection* pArrowRight = AGUI2TextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aArrows[ 1 ].SetDimensions( pArrowRight->GetWidth(), pArrowRight->GetHeight() );
	m_aArrows[ 1 ].SetTextureSection( pArrowRight );
	m_aArrows[ 1 ].SetTransform( 150.0f, 0.0f, 0.0f );

	T2_FOREACH_ARRAY_BACK( m_aArrows, it )
	{
		m_oRoot.AddChildTail( m_aArrows[ it ] );
	}

	m_oMenu.AddMenuItem( m_oRoot );

	// Setup title element
	m_oMiniGameTitle.Create( AGUI2FontManager::FindFont( "Rekord26" ), 400.0f );
	m_oMiniGameTitle.SetAttachment( AGUI2ATTACHMENT_BOTTOMCENTER, AGUI2ATTACHMENT_BOTTOMCENTER );
	m_oMiniGameTitle.SetColour( COLOR_WHITE );
	m_oMiniGameTitle.SetShadowEnabled( TTRUE );
	m_oMiniGameTitle.SetShadowColour( TCOLOR4( 0, 0, 0, 160 ) );
	m_oMiniGameTitle.SetAlpha( 0.0f );
	m_oMiniGameTitle.SetShadowAlpha( 0.0f );
	m_oMiniGameTitle.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oRoot.AddChildTail( m_oMiniGameTitle );

	// Setup description element
	m_oMiniGameDescription.Create( AGUI2FontManager::FindFont( "Rekord18" ), 400.0f );
	m_oMiniGameDescription.SetAttachment( AGUI2ATTACHMENT_TOPCENTER, AGUI2ATTACHMENT_TOPCENTER );
	m_oMiniGameDescription.SetColour( AGUI2STYLE_COLOR_PRIMARY );
	m_oMiniGameDescription.SetShadowEnabled( TTRUE );
	m_oMiniGameDescription.SetShadowColour( TCOLOR4( 0, 0, 0, 160 ) );
	m_oMiniGameDescription.SetAlpha( 0.0f );
	m_oMiniGameDescription.SetShadowAlpha( 0.0f );
	m_oMiniGameDescription.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oRoot.AddChildTail( m_oMiniGameDescription );

	// Update text about current minigame
	UpdateText();

	m_oRoot.SetDimensions( fMaxIconWidth * 3.75f, m_oMiniGameTitle.GetHeight() + m_oMiniGameDescription.GetHeight() + fMaxIconHeight + 64.0f );
	m_oMenu.SetFocus( TTRUE );
	m_oMenu.SetFocusAt( m_oRoot );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 237 ) );
	m_bFocusedElementBloated = TFALSE;

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 0040a0f0
void AFrontEndMiniGameState2::OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem )
{
	if ( m_bNoDebug && g_pPlayerProgress->IsMiniGameAvailable( m_iSelectedMiniGame ) )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
		return;
	}

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, &a_rMenuItem ) );
}

// $Barnyard: FUNCTION 0040a150
void AFrontEndMiniGameState2::UpdateText()
{
	const TWCHAR* wcsTitle;
	const TWCHAR* wcsDescription;

	if ( !m_bNoDebug )
	{
		wcsTitle       = g_pLocaleManager->GetString( g_pMiniGameManager->GetMiniGame( m_iSelectedMiniGame )->m_eLocaleTitle );
		wcsDescription = g_pLocaleManager->GetString( g_pMiniGameManager->GetMiniGame( m_iSelectedMiniGame )->m_eLocaleDescription );
	}
	else if ( !g_pPlayerProgress->IsMiniGameAvailable( m_iSelectedMiniGame ) )
	{
		wcsTitle       = g_pLocaleManager->GetString( 238 );
		wcsDescription = g_pLocaleManager->GetString( 239 );
	}
	else
	{
		wcsTitle       = g_pLocaleManager->GetString( g_pMiniGameManager->GetVisibleMiniGame( m_iSelectedMiniGame )->m_eLocaleTitle );
		wcsDescription = g_pLocaleManager->GetString( g_pMiniGameManager->GetVisibleMiniGame( m_iSelectedMiniGame )->m_eLocaleDescription );
	}

	m_oMiniGameTitle.SetText( wcsTitle );
	m_oMiniGameDescription.SetText( wcsDescription );
}
