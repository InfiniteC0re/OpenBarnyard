#include "pch.h"
#include "ALoadScreen.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "GUI/AGUI2FontManager.h"
#include "Locale/ALocaleManager.h"

#include "Render/ARenderer.h"

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
		m_aTextureSections[ i ] = AGUI2TextureSectionManager::GetTextureSection( ms_LoadTextures[ i ] );
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
		pRectangle->SetAttachment( AGUI2ATTACHMENT_BOTTOMLEFT, AGUI2ATTACHMENT_BOTTOMLEFT );
		pRectangle->SetTransform( fRectPosX, -fHeight * 0.075f );
		pRectangle->Hide();

		pRootElement->AddChildTail( *pRectangle );
		fRectPosX += ( fWidth * 0.8f ) / NUM_LOAD_INDICATORS;
	}

	auto pFont          = AGUI2FontManager::FindFont( "Rekord26" );
	auto pLoadingString = ALocaleManager::Interface()->GetString( 1123 );

	m_TextBox.Create( pFont, pFont->GetTextWidth( pLoadingString, 1.0f ) );
	m_TextBox.SetScale( 1.0f );
	m_TextBox.SetText( pLoadingString );
	m_TextBox.SetAttachment( AGUI2ATTACHMENT_BOTTOMCENTER, AGUI2ATTACHMENT_BOTTOMCENTER );
	m_TextBox.SetTransform( 0.0f, -fHeight * 0.08f );
	m_TextBox.Hide();

	pRootElement->AddChildTail( m_TextBox );
	m_oTimer.Reset();

	m_fTimer     = 0.8f;
	m_fTotalTime = 0.0f;
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

// $Barnyard: FUNCTION 0042b260
void ALoadScreen::Reset()
{
	if ( m_bFlag )
	{
		AGUISystem::GetSingleton()->SetPicture( TPString8() );
		AGUISystem::GetSingleton()->ResetPicture();
	}

	m_bFlag = TFALSE;
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
}

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
