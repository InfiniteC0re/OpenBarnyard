#include "pch.h"
#include "Helpers/ABikeManager.h"
#include "ASimPlayerMainHudElement.h"
#include "ALoadScreen.h"
#include "GUI/AGUI2.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "Units/APlayerControlComponent.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00458a00, CreateObject 00458cd0, CreateObjectInPlace 00458d30
TDEFINE_CLASS( ASimPlayerMainHudElement );

static TUINT32 s_aStaminaBarColours[ ASimPlayerMainHudElement::NUM_STAMINA_BARS ] = { 0xFF3A96CB, 0xFF3A96CB, 0xFF3A96CB, 0xFF3A96CB, 0xFF3A96CB }; // 0x007821d8
static TUINT32 s_uiStaminaFlashColour                                             = 0xFFFF0000;                                                     // 0x007821d4

// Splits an ARGB colour into floats in the [0; 1] range
// $Barnyard: FUNCTION 00445e40
static TFLOAT* __stdcall ColourToFloats( TUINT32 a_uiColour, TFLOAT* a_pA, TFLOAT* a_pR, TFLOAT* a_pG, TFLOAT* a_pB )
{
	*a_pB = TUINT8( a_uiColour ) * ( 1.0f / 255.0f );
	*a_pG = TUINT8( a_uiColour >> 8 ) * ( 1.0f / 255.0f );
	*a_pR = TUINT8( a_uiColour >> 16 ) * ( 1.0f / 255.0f );
	*a_pA = TUINT8( a_uiColour >> 24 ) * ( 1.0f / 255.0f );
	return a_pA;
}

// $Barnyard: FUNCTION 00458a10
ASimPlayerMainHudElement::ASimPlayerMainHudElement()
    : AHudElement( MASK_UNK2 | MASK_PLAYER_MAIN )
    , m_fStamina( 1.0f )
    , m_fFlashTime( -1.0f )
{
}

// $Barnyard: FUNCTION 00458ac0
// $Barnyard: FUNCTION 00458d70
ASimPlayerMainHudElement::~ASimPlayerMainHudElement()
{
}

// $Barnyard: FUNCTION 00458d90
TBOOL ASimPlayerMainHudElement::OnAttach()
{
	// The milk bottles
	m_oMilkAmmoBar.Create( 32.5f, 1.625f, 0.3f );
	m_oMilkAmmoBar.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oMilkAmmoBar.SetTransform( 45.5f, 6.5f );
	m_oBacking.AddChildTail( m_oMilkAmmoBar );

	TFLOAT fWidth = m_oMilkAmmoBar.GetWidth();

	// The stamina bars below them
	m_pStaminaBarSection = T2GUITextureSectionManager::GetTextureSection( "Stamina_Bar" );

	TFLOAT fBarWidth = m_pStaminaBarSection->GetWidth() / m_pStaminaBarSection->GetHeight() * 11.7f;
	TFLOAT fBarStep  = fBarWidth - 1.0f;
	TFLOAT fX        = 45.5f;

	for ( TINT i = 0; i < NUM_STAMINA_BARS; )
	{
		T2GUIRectangle& rBar = m_aStaminaBars[ i ];
		rBar.SetDimensions( fBarWidth, 11.7f );
		rBar.SetTextureSection( m_pStaminaBarSection );
		rBar.SetColour( s_aStaminaBarColours[ i++ ] );

		// Every other bar is a bit lower
		rBar.SetTransform( fX, ( i % 2 ) * 1.3f + 44.85f );
		fX += fBarStep;

		rBar.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_oBacking.AddChildTail( rBar );
	}

	fWidth = TMath::Max( fWidth, fX );

	// The wooden backing
	m_oBacking.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Score_Wood" ) );
	m_oBacking.SetDimensions( fWidth + 26.0f, 65.0f );
	m_oBacking.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oBacking.SetTransform( 20.0f, 0.0f );
	AGUI2::GetRootElement()->AddChildTail( m_oBacking );

	// The message icon in the bottom right corner
	T2GUITextureSection* pMessage = T2GUITextureSectionManager::GetTextureSection( "MSG" );
	m_oMessageIcon.SetDimensions( pMessage->GetWidth(), pMessage->GetHeight() );
	m_oMessageIcon.SetTextureSection( pMessage );
	m_oMessageIcon.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oMessageIcon.T2GUIElement::Hide();
	AGUI2::GetRootElement()->AddChildTail( m_oMessageIcon );

	if ( ( m_eMask & AGameStateController::GetSingleton()->GetCurrentState()->GetHUDMask() ) == 0 )
		Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 00458b50
void ASimPlayerMainHudElement::OnDetach()
{
	m_oMilkAmmoBar.RemoveSelf();

	for ( TINT i = 0; i < NUM_STAMINA_BARS; i++ )
		m_aStaminaBars[ i ].RemoveSelf();

	m_oBacking.RemoveSelf();
	m_oMessageIcon.RemoveSelf();
}

// $Barnyard: FUNCTION 00458bb0
void ASimPlayerMainHudElement::Show()
{
	m_oBacking.T2GUIElement::Show();
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00458bd0
void ASimPlayerMainHudElement::Hide()
{
	m_oBacking.T2GUIElement::Hide();
	m_oMessageIcon.T2GUIElement::Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00458fe0
void ASimPlayerMainHudElement::Update( TFLOAT a_fDeltaTime )
{

	AUnit*                   pPlayer      = APlayerManager::GetSingleton()->GetPlayer( 0 );
	APlayerControlComponent* pControlComp = pPlayer ? TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) : TNULL;
	TBOOL                    bBikeRace    = TFALSE;

	bBikeRace = ABikeManager::GetSingleton()->IsPlayerOnBike() != TFALSE;

	// The milk can't be used while riding or while the hands are busy
	m_oMilkAmmoBar.SetDisabled( bBikeRace || ( pControlComp && !pControlComp->CanUseHand() ) );

	if ( m_fFlashTime >= 0.0f )
	{
		// Flash the stamina bars red, one after another
		m_fFlashTime += a_fDeltaTime;

		for ( TINT i = 0; i < NUM_STAMINA_BARS; i++ )
		{
			TFLOAT fProgress = ( m_fFlashTime - i * 0.1f ) * 1.3333334f;
			if ( fProgress < 0.0f || fProgress > 1.0f ) fProgress = 1.0f;

			TVector4 vecColour;
			TVector4 vecFlashColour;
			ColourToFloats( s_aStaminaBarColours[ i ], &vecColour.w, &vecColour.x, &vecColour.y, &vecColour.z );
			ColourToFloats( s_uiStaminaFlashColour, &vecFlashColour.w, &vecFlashColour.x, &vecFlashColour.y, &vecFlashColour.z );
			vecFlashColour.Lerp4( vecFlashColour, vecColour, fProgress );

			T2GUIRectangle& rBar = m_aStaminaBars[ i ];
			rBar.SetColour(
			    ( TUINT32( vecFlashColour.w * 255.0f ) << 24 ) |
			    ( TUINT32( vecFlashColour.x * 255.0f ) << 16 ) |
			    ( TUINT32( vecFlashColour.y * 255.0f ) << 8 ) |
			    TUINT32( vecFlashColour.z * 255.0f )
			);

			// Pop the bar
			TFLOAT fScale = ( 1.0f - fProgress ) * 0.2f + 1.0f;
			rBar.GetTransform().SetScale( fScale, fScale );
		}

		if ( m_fFlashTime > 1.25f )
			m_fFlashTime = -1.0f;
	}

	if ( m_fStamina > 0.1f )
	{
		// Fade the filled bars in and the empty ones out
		TINT iNumFilled = TMath::CeilToInt( m_fStamina * NUM_STAMINA_BARS );

		for ( TINT i = 0; i < iNumFilled; i++ )
		{
			TFLOAT fAlpha = m_aStaminaBars[ i ].GetAlpha() + ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha > 1.0f ) fAlpha = 1.0f;

			m_aStaminaBars[ i ].SetAlpha( fAlpha );
		}

		for ( TINT i = iNumFilled; i < NUM_STAMINA_BARS; i++ )
		{
			TFLOAT fAlpha = m_aStaminaBars[ i ].GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha < 0.3f ) fAlpha = 0.3f;

			m_aStaminaBars[ i ].SetAlpha( fAlpha );
		}
	}
	else
	{
		for ( TINT i = 0; i < NUM_STAMINA_BARS; i++ )
		{
			TFLOAT fAlpha = m_aStaminaBars[ i ].GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha < 0.3f ) fAlpha = 0.3f;

			m_aStaminaBars[ i ].SetAlpha( fAlpha );
		}
	}

	if ( m_oMessageIcon.IsVisible() )
	{
		m_fMessageTime -= a_fDeltaTime;

		if ( m_fMessageTime <= 0.0f )
		{
			// Fade the icon out
			TFLOAT fAlpha = m_oMessageIcon.GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );

			if ( fAlpha <= 0.0f )
			{
				fAlpha = 0.0f;
				m_oMessageIcon.T2GUIElement::Hide();
			}

			m_oMessageIcon.SetAlpha( fAlpha );
		}

		TFLOAT fScale = TMath::LERPClamped( m_oMessageIcon.GetTransform().GetScaleX(), 1.0f, a_fDeltaTime * 10.0f );
		m_oMessageIcon.GetTransform().SetFromScale( fScale, fScale );
	}
}

// $Barnyard: FUNCTION 00458bf0
void ASimPlayerMainHudElement::SetStamina( TFLOAT a_fStamina )
{
	m_fStamina = a_fStamina;
	TMath::Clip( m_fStamina, 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 00458c10
void ASimPlayerMainHudElement::UpdateMilkAmmo( AProjectileLauncher* a_pLauncher )
{
	ALaunchAmmo* pAmmo = a_pLauncher->GetAmmo();
	m_oMilkAmmoBar.SetAmmo( TINT( pAmmo->GetAmmo() ), pAmmo->GetMaxAmmo() );
}

// $Barnyard: FUNCTION 00458c50
void ASimPlayerMainHudElement::RefreshStaminaBars()
{
	TINT iNumFilled = TMath::CeilToInt( m_fStamina * NUM_STAMINA_BARS );

	for ( TINT i = 0; i < iNumFilled; i++ )
		m_aStaminaBars[ i ].SetAlpha( 1.0f );

	for ( TINT i = iNumFilled; i < NUM_STAMINA_BARS; i++ )
		m_aStaminaBars[ i ].SetAlpha( 0.0f );
}

// $Barnyard: FUNCTION 004594a0
void ASimPlayerMainHudElement::ShowMessageIcon()
{
	if ( !m_bForceHide && !ASSeqState::IsPlaying() && !g_oLoadScreen.IsLoadingScreen() )
	{
		m_oMessageIcon.SetAlpha( 1.0f );
		m_oMessageIcon.T2GUIElement::Show();
		m_oMessageIcon.GetTransform().Scale( 1.25f, 1.25f );
		m_fMessageTime = 3.0f;

		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_SMS );
	}
}
