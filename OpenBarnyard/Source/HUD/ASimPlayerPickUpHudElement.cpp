#include "pch.h"
#include "ASimPlayerPickUpHudElement.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Player/APlayerProfileManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00459510, CreateObject 00459ba0, CreateObjectInPlace 00459c00
TDEFINE_CLASS( ASimPlayerPickUpHudElement );

// $Barnyard: FUNCTION 004598f0
ASimPlayerPickUpHudElement::ASimPlayerPickUpHudElement()
    : AHudElement( MASK_MINIGAME | MASK_UNK2 | MASK_UNK15 | MASK_PICK_UP | MASK_PLAYER_MAIN )
    , m_bUnk1( TTRUE )
    , m_fIconPopTime( 2.0f )
    , m_bAutoFade( TFALSE )
{
	// Wood backing in the top right corner
	// NOTE: retail uses the AGUI2Rectangle::SetTextureSection overload at 006c3b00, which also resizes the rectangle.
	T2GUITextureSection* pBacking = T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" );
	m_oBacking.SetDimensions( pBacking->GetWidth(), pBacking->GetHeight() );
	m_oBacking.SetTextureSection( pBacking );
	m_oBacking.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );

	// Buck icon on its left
	T2GUITextureSection* pIcon = T2GUITextureSectionManager::GetTextureSection( "BarnYard_Buck" );
	m_oIcon.T2GUIElement::SetDimensions( pIcon->GetWidth() * 0.75f, pIcon->GetHeight() * 0.75f );
	m_oIcon.SetTextureSection( pIcon );

	m_oIconContainer.SetDimensions( m_oIcon.GetWidth(), m_oIcon.GetHeight() );
	m_oIconContainer.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oIconContainer.GetTransform().SetTranslation( m_oBacking.GetWidth() * 0.1f, 0.0f );
	m_oIconContainer.AddChildTail( m_oIcon );
	m_oBacking.AddChildTail( m_oIconContainer );

	// The number on the right
	TStringManager::String16Format( m_wszNumber, 8, L"" );
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oTextBox.Create( pFont, 0.0f );
	m_oTextBox.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oTextBox.SetText( m_wszNumber );

	// NOTE: retail offsets the text by T2GUIFont::GetLineHeight (006c2dd0) * 0.1. The engine's T2GUIFont doesn't
	// expose the line height, so it's taken from the height of a single line minus the line margin.
	TFLOAT fLineHeight = pFont->GetTextHeightWrapped( L"0", TMath::MAXFLOAT, 1.0f ) - pFont->GetLinesMargin();
	m_oTextBox.GetTransform().SetTranslation( -20.0f, fLineHeight * 0.1f );
	m_oBacking.AddChildTail( m_oTextBox );

	TFLOAT fBackingHeight = m_oBacking.GetHeight();
	TFLOAT fIconHeight    = m_oIconContainer.GetHeight();
	m_oBacking.SetHeight( ( fIconHeight < fBackingHeight ) ? fBackingHeight : fIconHeight );

	AGUI2::GetRootElement()->AddChildTail( m_oBacking );
	m_oBacking.Show();

	if ( AGameLoader::IsMainGameLoaded() || AGameLoader::IsBarnExploreMiniGame() )
		SetNumber( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups() );
	else
		SetNumber( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetMiniGamePickups() );

	m_oBacking.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00459b80
// $Barnyard: FUNCTION 00459520
ASimPlayerPickUpHudElement::~ASimPlayerPickUpHudElement()
{
	m_oIcon.RemoveSelf();
	m_oIconContainer.RemoveSelf();
	m_oTextBox.RemoveSelf();
	m_oBacking.RemoveSelf();
}

// $Barnyard: FUNCTION 00452790
void ASimPlayerPickUpHudElement::Show()
{
	m_oBacking.Show();
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00457210
void ASimPlayerPickUpHudElement::Hide()
{
	m_oBacking.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00459630
void ASimPlayerPickUpHudElement::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bAutoFade == TTRUE )
	{
		m_fIconPopTime += a_fDeltaTime;

		if ( m_fIconPopTime > SHOW_TIME )
		{
			// Fade the counter out
			TFLOAT fAlpha = m_oBacking.GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha <= 0.0f ) fAlpha = 0.0f;

			SetAlpha( fAlpha );
			return;
		}

		TFLOAT fScale = TMath::LERP( m_oIcon.GetTransform().GetScaleX(), 1.0f, a_fDeltaTime * 5.0f );
		m_oIcon.GetTransform().SetScale( fScale, fScale );
	}
	else
	{
		TFLOAT fScale = TMath::LERPClamped( m_oIcon.GetTransform().GetScaleX(), 1.0f, a_fDeltaTime * 5.0f );
		m_oIcon.GetTransform().SetScale( fScale, fScale );
	}
}

// $Barnyard: FUNCTION 004595f0
void ASimPlayerPickUpHudElement::SetAlpha( TFLOAT a_fAlpha )
{
	m_oIcon.SetAlpha( a_fAlpha );
	m_oTextBox.SetAlpha( a_fAlpha );
	m_oBacking.SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 004597a0
void ASimPlayerPickUpHudElement::SetNumber( TINT a_iNumber )
{
	TStringManager::String16Format( m_wszNumber, 8, L"%d", a_iNumber );

	// Room for as many digits as the number has
	T2GUIFont* pFont     = T2GUIFontManager::FindFont( "Rekord26" );
	TINT       iNumChars = TStringManager::String16Length( m_wszNumber );
	m_oTextBox.Create( pFont, pFont->GetTextWidth( L"0", 1, 1.0f ) * iNumChars ); // T2GUIFont::GetCharWidth 006c3350

	m_oBacking.SetWidth( m_oIconContainer.GetWidth() * 1.25f + m_oTextBox.GetWidth() + 20.0f );

	SetAlpha( 1.0f );
	m_fIconPopTime = 0.0f;

	// Pop the icon
	if ( m_oIcon.GetTransform().GetScaleX() < 1.01f )
		m_oIcon.GetTransform().SetScale( 1.25f, 1.25f );
}

// $Barnyard: FUNCTION 004598b0
void ASimPlayerPickUpHudElement::GetIconScreenPosition( TVector2& a_rOutPosition )
{
	T2GUITransform oTransform;
	m_oIcon.GetScreenTransform( oTransform );

	a_rOutPosition = oTransform.GetTranslation();
}
