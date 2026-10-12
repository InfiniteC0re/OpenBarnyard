#include "pch.h"
#include "AStartMiniGameHudElement.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// 00773720
static const TCHAR* s_apStageImageNames[ AStartMiniGameHudElement::STAGE_NUMOF ] = {
	"Ready",
	"Set",
	"Go"
};

// Fade speeds and target scales of the stage images.
static constexpr TFLOAT FADE_SPEED             = 5.0f;
static constexpr TFLOAT SCALE_SPEED            = 2.0f;
static constexpr TFLOAT PREV_IMAGE_END_SCALE   = 3.0f; // DAT_00773734
static constexpr TFLOAT LAST_IMAGE_END_SCALE   = 3.0f; // DAT_00773730
static constexpr TFLOAT SINGLE_FADE_SPEED      = 3.0f;
static constexpr TFLOAT SINGLE_IMAGE_END_SCALE = 5.0f; // DAT_0077372c

//-----------------------------------------------------------------------------
// NOTE: retail calls the AGUI2Rectangle::SetTextureSection overload at 006c3b00, which also
// resizes the rectangle to the texture section. T2GUIRectangle's doesn't, so the size is set here.
//-----------------------------------------------------------------------------
static void SetTextureSectionAndSize( T2GUIRectangle& a_rRectangle, T2GUITextureSection* a_pSection )
{
	if ( a_pSection )
		a_rRectangle.SetDimensions( a_pSection->GetWidth(), a_pSection->GetHeight() );

	a_rRectangle.SetTextureSection( a_pSection );
}

// Fades a_rElement out by a_fAmount and hides it once fully transparent. Returns the new alpha.
static TFLOAT FadeOut( T2GUIElement& a_rElement, TFLOAT a_fAmount )
{
	TFLOAT fAlpha = a_rElement.GetAlpha() - a_fAmount;
	if ( fAlpha < 0.0f ) fAlpha = 0.0f;

	a_rElement.SetAlpha( fAlpha );
	if ( fAlpha == 0.0f ) a_rElement.Hide();

	return fAlpha;
}

// Moves the uniform scale of a_rElement towards a_fTarget, keeping its translation.
static void ScaleTowards( T2GUIElement& a_rElement, TFLOAT a_fTarget, TFLOAT a_fProgress )
{
	TFLOAT fScale = TMath::LERP( a_rElement.GetTransform().GetScaleX(), a_fTarget, ( 1.0f < a_fProgress ) ? 1.0f : a_fProgress );
	a_rElement.GetTransform().SetScale( fScale, fScale );
}

// Resets rotation and scale of a_rElement, keeping its translation.
static void ResetScale( T2GUIElement& a_rElement )
{
	TVector2 vecTranslation = a_rElement.GetTransform().GetTranslation();
	a_rElement.GetTransform().SetIdentity();
	a_rElement.GetTransform().SetTranslation( vecTranslation );
}

// $Barnyard: FUNCTION 00459c40
AStartMiniGameHudElement::AStartMiniGameHudElement()
    : AHudElement( MASK_MINIGAME | MASK_UNK2 )
    , m_bFinished( TFALSE )
    , m_iImageIndex( 0 )
    , m_eType( 0 )
    , m_fTimer( 0.0f )
{
}

// $Barnyard: FUNCTION 0045a000
// $Barnyard: FUNCTION 00459cc0
AStartMiniGameHudElement::~AStartMiniGameHudElement()
{
}

// $Barnyard: FUNCTION 0045a020
TBOOL AStartMiniGameHudElement::OnAttach()
{
	for ( TINT i = 0; i < STAGE_NUMOF; i++ )
		m_aImages[ i ] = T2GUITextureSectionManager::GetTextureSection( s_apStageImageNames[ i ] );

	SetTextureSectionAndSize( m_oImage, m_aImages[ STAGE_READY ] );
	SetTextureSectionAndSize( m_oPrevImage, m_aImages[ STAGE_READY ] );

	m_oImage.SetTranslation( 0.0f, IMAGE_OFFSET_Y );
	m_oPrevImage.SetTranslation( 0.0f, IMAGE_OFFSET_Y );
	m_oPrevImage.Hide();

	AGUI2::GetRootElement()->AddChildTail( m_oImage );
	AGUI2::GetRootElement()->AddChildTail( m_oPrevImage );

	Hide();
	return TTRUE;
}

// $Barnyard: FUNCTION 00459fa0
void AStartMiniGameHudElement::OnDetach()
{
	m_oText.RemoveSelf();
	m_oImage.RemoveSelf();
	m_oPrevImage.RemoveSelf();
}

// $Barnyard: FUNCTION 0045a0c0
void AStartMiniGameHudElement::Show()
{
	m_oImage.Show();
	m_oText.Show();

	if ( m_iImageIndex > STAGE_READY && m_eType != TYPE_GO )
		m_oPrevImage.Show();

	if ( m_fTimer == 0.0f && m_eType != TYPE_READY_SILENT && m_eType != TYPE_PLAYER_READY )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_BEEP01 );

	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00459fe0
void AStartMiniGameHudElement::Hide()
{
	m_oImage.Hide();
	m_oText.Hide();
	m_oPrevImage.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 0045a130
void AStartMiniGameHudElement::Update( TFLOAT a_fDeltaTime )
{
	m_fTimer += a_fDeltaTime;

	switch ( m_eType )
	{
		case TYPE_COUNTDOWN:
		case TYPE_PLAYER_COUNTDOWN:
			break;
		case TYPE_READY_SILENT:
		case TYPE_PLAYER_READY:
		case TYPE_GET_READY:
		case TYPE_GO:
			// Single image: fade and grow it once its time is up
			if ( m_fTimer >= STAGE_TIME )
			{
				TFLOAT fFade  = a_fDeltaTime * SINGLE_FADE_SPEED;
				TFLOAT fAlpha = m_oImage.GetAlpha() - fFade;
				if ( fAlpha < 0.0f ) fAlpha = 0.0f;

				m_oImage.SetAlpha( fAlpha );
				m_oText.SetAlpha( fAlpha );
				ScaleTowards( m_oImage, SINGLE_IMAGE_END_SCALE, fFade );

				if ( fAlpha == 0.0f )
				{
					m_bForceHide = TTRUE;
					m_bFinished  = TTRUE;
				}
			}
			return;
		default:
			return;
	}

	// Ready / Set / Go countdown
	if ( m_oPrevImage.IsVisible() )
	{
		FadeOut( m_oPrevImage, a_fDeltaTime * FADE_SPEED );
		ScaleTowards( m_oPrevImage, PREV_IMAGE_END_SCALE, a_fDeltaTime * SCALE_SPEED );
	}

	if ( m_fTimer < STAGE_TIME ) return;

	if ( m_iImageIndex >= STAGE_GO )
	{
		TFLOAT fAlpha = FadeOut( m_oImage, a_fDeltaTime * FADE_SPEED );
		ScaleTowards( m_oImage, LAST_IMAGE_END_SCALE, a_fDeltaTime * SCALE_SPEED );

		if ( fAlpha == 0.0f )
		{
			m_bForceHide = TTRUE;
			m_bFinished  = TTRUE;
			return;
		}
	}

	if ( m_fTimer - STAGE_TIME < STAGE_GAP ) return;

	m_fTimer = 0.0f;
	if ( m_iImageIndex >= STAGE_GO ) return;

	// Next stage: the current image moves to m_oPrevImage and fades out
	if ( ++m_iImageIndex > STAGE_READY )
	{
		m_oPrevImage.Show();
		SetTextureSectionAndSize( m_oPrevImage, m_aImages[ m_iImageIndex - 1 ] );
		m_oPrevImage.SetAlpha( 1.0f );
	}

	SetTextureSectionAndSize( m_oImage, m_aImages[ m_iImageIndex ] );
	ResetScale( m_oImage );
	ResetScale( m_oPrevImage );

	if ( m_iImageIndex == STAGE_GO )
	{
		m_oImage.GetTransform().Scale( 2.0f, 2.0f );

		if ( m_eType != TYPE_READY_SILENT && m_eType != TYPE_PLAYER_READY )
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_BEEP02 );
	}
	else if ( m_eType != TYPE_READY_SILENT && m_eType != TYPE_PLAYER_READY )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_BEEP01 );
	}

	m_oPrevImage.SetAlpha( 1.0f );
}

// $Barnyard: FUNCTION 00459d40
void AStartMiniGameHudElement::Setup( TINT a_iNumPlayers, TINT a_iCurrentPlayer )
{
	for ( TINT i = 0; i < STAGE_NUMOF; i++ )
		m_aImages[ i ] = T2GUITextureSectionManager::GetTextureSection( s_apStageImageNames[ i ] );

	m_bFinished = TFALSE;

	const TWCHAR* wszText = TNULL;

	if ( a_iNumPlayers == 1 )
	{
		wszText = g_pLocaleManager->GetString( ( m_eType == TYPE_GO ) ? 989 : 987 );
	}
	else
	{
		switch ( m_eType )
		{
			case TYPE_COUNTDOWN:
			case TYPE_READY_SILENT:
				wszText = g_pLocaleManager->GetString( 987 );
				break;
			case TYPE_PLAYER_COUNTDOWN:
				ALocale::FormatTags( m_wszText, 32, 984, PPropertyValue( a_iCurrentPlayer + 1 ) );
				wszText = m_wszText;
				break;
			case TYPE_PLAYER_READY:
				ALocale::FormatTags( m_wszText, 32, 992, PPropertyValue( a_iCurrentPlayer + 1 ) );
				wszText = m_wszText;
				break;
			case TYPE_GET_READY:
				wszText = g_pLocaleManager->GetString( 991 );
				break;
			case TYPE_GO:
				wszText = g_pLocaleManager->GetString( 989 );
				break;
		}
	}

	m_iImageIndex = ( m_eType == TYPE_GO ) ? STAGE_GO : STAGE_READY;
	SetTextureSectionAndSize( m_oImage, m_aImages[ m_iImageIndex ] );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oText.Create( pFont, 0.0f );
	m_oText.SetText( wszText );
	m_oText.SetWidth( pFont->GetTextWidth( wszText, 1.0f ) * m_oText.GetScale() );

	m_oImage.SetAlpha( 1.0f );
	ResetScale( m_oImage );

	m_oPrevImage.SetAlpha( 1.0f );
	ResetScale( m_oPrevImage );

	if ( m_eType == TYPE_GO )
		m_oPrevImage.Hide();

	m_bForceHide = TFALSE;
	m_fTimer     = 0.0f;
}
