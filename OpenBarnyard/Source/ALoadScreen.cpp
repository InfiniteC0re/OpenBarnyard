#include "pch.h"
#include "ALoadScreen.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"

#include "Render/ARenderer.h"

#ifdef BARNYARD_LATEST
#  include "GUI/XUI/AGUI2XUIScene.h"
#  include "GUI/XUI/AGUI2XUIGroup.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Global definition
ALoadScreen g_oLoadScreen;

// $Barnyard: FUNCTION 0042ac30
ALoadScreen::ALoadScreen()
{
	m_bIsLoadingScreen = TFALSE;
	m_bFlag            = TFALSE;
	m_iNumCowIcons     = 1;
	m_fTotalTime       = 0.0f;
	m_fTimer           = 0.8f;

#ifdef BARNYARD_LATEST
	m_pXUIScene = TNULL;
#endif
}

// $Barnyard: FUNCTION 0042acd0
// $Barnyard: FUNCTION 0042ada0
ALoadScreen::~ALoadScreen()
{
}

// $Barnyard: FUNCTION 0042adc0
void ALoadScreen::Create()
{
	T2_FOREACH_ARRAY( ms_LoadTextures, i )
	{
		TASSERT( m_aTextureSections[ i ] == TNULL, "ALoadScreen is already created" );
		m_aTextureSections[ i ] = T2GUITextureSectionManager::GetTextureSection( ms_LoadTextures[ i ] );
		TASSERT( m_aTextureSections[ i ] != TNULL, "Unable to find texture section" );
	}

	auto pRootElement = AGUI2::GetRootElement();

	TFLOAT fWidth;
	TFLOAT fHeight;
	pRootElement->GetDimensions( fWidth, fHeight );

	m_CanvasElement.SetDimensions( fWidth, fHeight );
	m_CanvasElement.SetColour( TCOLOR( 0, 0, 0 ) );
	m_CanvasElement.Hide();

	pRootElement->AddChildTail( m_CanvasElement );

	TFLOAT fRectPosX = fWidth * 0.1f;

	for ( TUINT i = 0; i < NUM_LOAD_INDICATORS; i++ )
	{
		auto pTexSection = m_aTextureSections[ i % NUM_LOAD_TEXTURES ];
		auto pRectangle  = &m_aRectangles[ i ];

		pRectangle->SetDimensions( pTexSection->GetWidth(), pTexSection->GetHeight() );
		pRectangle->SetTextureSection( pTexSection );
		pRectangle->SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
		pRectangle->SetTransform( fRectPosX, -fHeight * 0.075f );
		pRectangle->Hide();

		pRootElement->AddChildTail( *pRectangle );
		fRectPosX += ( fWidth * 0.8f ) / NUM_LOAD_INDICATORS;
	}

	auto pFont          = T2GUIFontManager::FindFont( "Rekord26" );
	auto pLoadingString = ALocaleManager::Interface()->GetString( 1123 );

	m_TextBox.Create( pFont, pFont->GetTextWidth( pLoadingString, 1.0f ) );
	m_TextBox.SetScale( 1.0f );
	m_TextBox.SetText( pLoadingString );
	m_TextBox.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_TextBox.SetTransform( 0.0f, -fHeight * 0.08f );
	m_TextBox.Hide();

	pRootElement->AddChildTail( m_TextBox );
	m_oTimer.Reset();

	m_fTimer     = 0.8f;
	m_fTotalTime = 0.0f;
}

#ifdef BARNYARD_COMMUNITY_PATCH

// $BarnyardPS2EU: FUNCTION 00141200
void ALoadScreen::UpdateLoadingText()
{
	m_TextBox.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 1123 ) );
}

#endif // BARNYARD_COMMUNITY_PATCH

// $Barnyard: FUNCTION 0042ad50
void ALoadScreen::RemoveElements()
{
	for ( TUINT i = 0; i < NUM_LOAD_INDICATORS; i++ )
		m_aRectangles[ i ].RemoveSelf();

	m_CanvasElement.RemoveSelf();
	m_TextBox.RemoveSelf();
}

// $Barnyard: FUNCTION 0042b160
void ALoadScreen::StartLoading( TINT a_iUnused, TBOOL a_bRender )
{
	m_bIsLoadingScreen = TTRUE;

	if ( !m_bFlag )
	{
		m_CanvasElement.Show();
	}

	m_TextBox.Show();
	m_iNumCowIcons = 1;
	m_iCowPos      = 0;
	m_bFlag        = TFALSE;

	m_oTimer.Reset();
	m_fTimer     = 0.0f;
	m_fTotalTime = 0.0f;

	for ( TUINT i = 0; i < NUM_LOAD_INDICATORS; i++ )
	{
		m_aRectangles[ i ].Show();
	}

	m_TextBox.Show();
	UpdateUI( a_bRender );
}

// $Barnyard: FUNCTION 0042b100
void ALoadScreen::SetLoadingState( TBOOL a_bIsLoading, TBOOL a_bUpdateUI )
{
	m_bIsLoadingScreen = a_bIsLoading;

	if ( !m_bFlag )
	{
		if ( a_bIsLoading ) m_CanvasElement.Show();
		else
			m_CanvasElement.Hide();
	}

	if ( !a_bIsLoading )
	{
		m_iNumCowIcons = 0;
	}

	if ( a_bIsLoading ) m_TextBox.Show();
	else
		m_TextBox.Hide();

	if ( a_bUpdateUI ) UpdateUI( a_bIsLoading );
}

// $Barnyard: FUNCTION 0042b1e0
void ALoadScreen::Update( TFLOAT a_fDeltaTime, TBOOL a_bRender )
{
	if ( m_bIsLoadingScreen )
	{
		m_oTimer.Update();
		m_fTimer += m_oTimer.GetDelta();
		m_fTotalTime += m_oTimer.GetDelta();

		if ( m_fTimer >= ICON_SWITCH_INTERVAL )
		{
			m_fTimer = 0.0f;

			if ( m_iNumCowIcons < 3 ) m_iNumCowIcons++;
			m_iCowPos = ( m_iCowPos + 1 ) % NUM_LOAD_INDICATORS;
			UpdateUI( a_bRender );
		}
	}
}

// $Barnyard: FUNCTION 0042b080
void ALoadScreen::ResetIndicators( TBOOL a_bRender )
{
	if ( m_bIsLoadingScreen )
	{
		m_iNumCowIcons = 0;
		m_iCowPos      = 0;

		UpdateUI( a_bRender );
	}
}

// $Barnyard: FUNCTION 0042b0a0
// $BarnyardWiiJP: FUNCTION 80133ab4
void ALoadScreen::SetPicture( const TPString8& a_rcPicture, TBOOL a_bRender )
{
	if ( a_rcPicture.GetPooledString() && !a_rcPicture.GetPooledString()->GetString8().IsEmpty() )
	{
#ifdef BARNYARD_LATEST
		g_pGUISystem->ShowBackgroundPicture( TFALSE );
#endif

		g_pGUISystem->SetPicture( a_rcPicture );
		m_bFlag = TTRUE;
		m_CanvasElement.Hide();

		if ( a_bRender )
			ARenderer::GetSingleton()->ForceUpdate30FPS();
	}
}

// $Barnyard: FUNCTION 0042b260
// $BarnyardWiiJP: FUNCTION 801338e0
void ALoadScreen::Reset()
{
	if ( m_bFlag )
	{
		g_pGUISystem->SetPicture( TPString8() );
		g_pGUISystem->ResetPicture();
	}

	m_bFlag = TFALSE;

#ifdef BARNYARD_LATEST
	g_pGUISystem->ShowBackgroundPicture( TFALSE );
#endif

	m_oTimer.Reset();
	m_fTimer = 0.8f;

	for ( TUINT i = 0; i < NUM_LOAD_INDICATORS; i++ )
	{
		auto pRectangle = &m_aRectangles[ i ];
		pRectangle->Hide();
	}

	m_CanvasElement.Hide();
	m_TextBox.Hide();

	m_bIsLoadingScreen = TFALSE;

#ifdef BARNYARD_LATEST
	DeleteXUIScene();
#endif
}

#ifdef BARNYARD_LATEST

// NOTE: not in the Wii build. The XUI load screens of the minigames show the minigame's title,
// its goal and its artwork next to the Wii Remote and Nunchuk controls. The controls don't apply
// on PC, so only the elements listed here are kept. The scene stays until the player dismisses
// AMiniGameInstructionState, whose prompt (the PC keys) is drawn over it.
static constexpr const TCHAR* s_aXUILoadScreenElements[][ 5 ] = {
	{ "BarnyardDarts_LoadScreen", "XuiText2", "XuiText1", "XuiImage3", TNULL },
	{ "BarnyardPool_LoadScreen", "XuiText2", "XuiText1", "XuiImage3", TNULL },
	{ "Bike_Race_LoadScreen", "XuiText2", "Title1", TNULL, TNULL },
	{ "ChasingChicks_LoadScreen", "XuiText2", "XuiText1", TNULL, TNULL },
	{ "ChickenCoopDefender_LoadScreen", "XuiText2", "XuiText1", "XuiImage2", TNULL },
	{ "ChickenLaunch_LoadScreen", "XuiText2", "XuiText1", TNULL, TNULL },
	{ "CowTipping_LoadScreen", "XuiText2", "XuiText1", "XuiImage2", TNULL },
	{ "CoyoteShowdown_LoadScreen", "XuiText2", "XuiText1", "XuiImage2", "XuiImage4" },
	{ "Gophergolf_LoadScreen", "XuiText2", TNULL, TNULL, TNULL },
	{ "HoneyCollecting_LoadScreen", "XuiText2", TNULL, TNULL, TNULL },
	{ "JoyRide_LoadScreen", "XuiText2", "XuiText1", "XuiImage3", TNULL },
	{ "MilkBar_LoadScreen", "XuiText2", "XuiText1", TNULL, TNULL },
	{ "MudJumpers_LoadScreen", "XuiText2", "XuiText1", "XuiImage3", TNULL },
	{ "SharpSquirter_LoadScreen", "XuiText2", "XuiText1", "XuiImage4", TNULL },
	{ "Teasemrsbeady_LoadScreen", "XuiText2", "XuiText18", "XuiImage2", TNULL },
	{ "TeaseTheMailman_LoadScreen", "XuiText2", "XuiText1", "XuiImage4", TNULL },
	{ "VegetablePatchDefender_LoadScreen", "XuiText2", "XuiText1", "XuiImage2", TNULL },
	{ "WhackARac_LoadScreen", "XuiText2", "XuiText1", "XuiImage2", TNULL },
};

// Hides the elements of a_pElement (groups are walked) whose id isn't in a_ppKeep
static void HideXUIControls( AGUI2XUIElement* a_pElement, const TCHAR* const* a_ppKeep, TINT a_iNumKeep )
{
	for ( T2GUIElement* it = a_pElement->ChildRBegin(); it != TNULL; it = a_pElement->GetPrevChild( it ) )
	{
		if ( !it->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) )
			continue;

		AGUI2XUIElement* pChild = TSTATICCAST( AGUI2XUIElement, it );

		if ( pChild->IsA( &TGetClass( AGUI2XUIGroup ) ) )
		{
			HideXUIControls( pChild, a_ppKeep, a_iNumKeep );
			continue;
		}

		TCHAR szId[ 64 ] = "";
		if ( pChild->GetId() )
			TStringManager::StringUnicodeToChar( szId, pChild->GetId(), sizeof( szId ) - 1 );

		TBOOL bKeep = TFALSE;
		for ( TINT i = 0; i < a_iNumKeep && a_ppKeep[ i ]; i++ )
		{
			if ( TStringManager::String8Compare( szId, a_ppKeep[ i ] ) == 0 )
				bKeep = TTRUE;
		}

		if ( !bKeep )
			pChild->Hide();
	}
}

// $BarnyardWiiJP: FUNCTION 80133c48
AGUI2XUIScene* ALoadScreen::SetXUIScene( const TCHAR* a_szName, TUINT32 a_uiIndex )
{
	if ( !m_pXUIScene )
	{
		AGUI2XUIResource* pResource = AGUI2::GetSingleton()->GetXUI().FindResource( a_szName );

		if ( pResource )
		{
			m_pXUIScene = AGUI2::GetSingleton()->CreateXUIScene( pResource, a_uiIndex );

			if ( m_pXUIScene )
			{
				for ( TINT i = 0; i < TARRAYSIZE( s_aXUILoadScreenElements ); i++ )
				{
					if ( TStringManager::String8CompareNoCase( a_szName, s_aXUILoadScreenElements[ i ][ 0 ] ) == 0 )
						HideXUIControls( m_pXUIScene, &s_aXUILoadScreenElements[ i ][ 1 ], TARRAYSIZE( s_aXUILoadScreenElements[ i ] ) - 1 );
				}
			}
		}
	}

#  ifdef BARNYARD_COMMUNITY_PATCH
	// The Wii data is optional on PC
	if ( !m_pXUIScene )
		return TNULL;
#  endif

	m_pXUIScene->Show();
	return m_pXUIScene;
}

// $BarnyardWiiJP: FUNCTION 80133a30
void ALoadScreen::ShowXUIBackground( TBOOL a_bShow, TBOOL a_bRender )
{
	g_pGUISystem->ShowBackgroundPicture( a_bShow );

	if ( a_bShow )
		m_CanvasElement.Hide();
	else
		m_CanvasElement.Show();

	if ( a_bRender )
		ARenderer::GetSingleton()->ForceUpdate30FPS();
}

// $BarnyardWiiJP: FUNCTION 80133cc0
void ALoadScreen::DeleteXUIScene()
{
	if ( m_pXUIScene )
	{
		AGUI2::GetSingleton()->RemoveXUIScene( m_pXUIScene );
		m_pXUIScene = TNULL;
	}
}

#endif // BARNYARD_LATEST

// $Barnyard: FUNCTION 0042afc0
void ALoadScreen::UpdateUI( TBOOL a_bRender )
{
	for ( TUINT i = 0; i < NUM_LOAD_INDICATORS; i++ )
	{
		auto pRectangle = &m_aRectangles[ i ];

		if ( m_iNumCowIcons == 3 && ( ( i + 2 ) % NUM_LOAD_INDICATORS == m_iCowPos ) )
		{
			pRectangle->SetAlpha( 0.2f );
			pRectangle->Show();
		}
		else if ( m_iNumCowIcons >= 2 && ( ( i + 1 ) % NUM_LOAD_INDICATORS == m_iCowPos ) )
		{
			pRectangle->SetAlpha( 0.5f );
			pRectangle->Show();
		}
		else if ( m_iNumCowIcons >= 1 && i == m_iCowPos )
		{
			pRectangle->SetAlpha( 1.0f );
			pRectangle->Show();
		}
		else
		{
			pRectangle->Hide();
		}
	}

	if ( a_bRender )
	{
		ARenderer::GetSingleton()->ForceUpdate30FPS();
	}
}
