#include "pch.h"
#include "AGameTimeHudElement.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameTimeManager.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00452520, CreateObject 004527f0, CreateObjectInPlace 00452850
TDEFINE_CLASS( AGameTimeHudElement );

// $Barnyard: FUNCTION 00452590
AGameTimeHudElement::AGameTimeHudElement()
    : AHudElement( MASK_UNK2 | MASK_GAME_TIME )
    , m_bUpdateHands( TTRUE )
{
	m_pAMSection         = T2GUITextureSectionManager::GetTextureSection( "am" );
	m_pPMSection         = T2GUITextureSectionManager::GetTextureSection( "pm" );
	m_pClockFaceSection  = T2GUITextureSectionManager::GetTextureSection( "clockface" );
	m_pHourHandSection   = T2GUITextureSectionManager::GetTextureSection( "clockarm_small" );
	m_pMinuteHandSection = T2GUITextureSectionManager::GetTextureSection( "clockarm_big" );
}

// $Barnyard: FUNCTION 004526b0
// $Barnyard: FUNCTION 004528b0
AGameTimeHudElement::~AGameTimeHudElement()
{
}

// $Barnyard: FUNCTION 004527b0
void AGameTimeHudElement::GetDimensions( TINT& a_rWidth, TINT& a_rHeight )
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	m_oClockFace.GetDimensions( fWidth, fHeight );

	a_rWidth  = TINT( fWidth );
	a_rHeight = TINT( fHeight );
}

// $Barnyard: FUNCTION 004528d0
TBOOL AGameTimeHudElement::OnAttach()
{
	// The clock face in the top left corner
	m_oClockFace.SetDimensions( m_pClockFaceSection->GetWidth(), m_pClockFaceSection->GetHeight() );
	m_oClockFace.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oClockFace.SetTextureSection( m_pClockFaceSection );
	AGUI2::GetRootElement()->AddChildTail( m_oClockFace );

	// The day/night sign
	m_oDayNightSign.SetDimensions( m_pAMSection->GetWidth(), m_pAMSection->GetHeight() );
	m_oDayNightSign.SetTextureSection( m_pAMSection );
	m_oClockFace.AddChildTail( m_oDayNightSign );

	// The hands rotate around their bottom
	m_oHourHand.SetDimensions( m_pHourHandSection->GetWidth(), m_pHourHandSection->GetHeight() );
	m_oHourHand.m_oPivotTransform.SetTranslation( 0.0f, m_oHourHand.GetHeight() * -0.5f );
	m_oHourHand.SetTextureSection( m_pHourHandSection );
	m_oClockFace.AddChildTail( m_oHourHand );

	m_oMinuteHand.SetDimensions( m_pMinuteHandSection->GetWidth(), m_pMinuteHandSection->GetHeight() );
	m_oMinuteHand.m_oPivotTransform.SetTranslation( 0.0f, m_oMinuteHand.GetHeight() * -0.5f );
	m_oMinuteHand.SetTextureSection( m_pMinuteHandSection );
	m_oClockFace.AddChildTail( m_oMinuteHand );

	return TTRUE;
}

// $Barnyard: FUNCTION 00452740
void AGameTimeHudElement::OnDetach()
{
	m_oMinuteHand.RemoveSelf();
	m_oDayNightSign.RemoveSelf();
	m_oHourHand.RemoveSelf();
	m_oClockFace.RemoveSelf();
}

// NOTE: folded with ASimPlayerPickUpHudElement::Show (00452790)
void AGameTimeHudElement::Show()
{
	m_oClockFace.T2GUIElement::Show();
	m_bCurrentlyVisible = TTRUE;
}

// NOTE: folded with ASimPlayerPickUpHudElement::Hide (00457210)
void AGameTimeHudElement::Hide()
{
	m_oClockFace.T2GUIElement::Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00452a80
void AGameTimeHudElement::Update( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fSecondsPerDay  = TFLOAT( AGameTimeManager::SECONDS_PER_DAY );  // 0x00782008
	static TFLOAT s_fSecondsPerHour = TFLOAT( AGameTimeManager::SECONDS_PER_HOUR ); // 0x00782004
	static TFLOAT s_fSecondsPerHalf = s_fSecondsPerDay * 0.5f;                      // 0x00782000

	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();

	if ( m_bUpdateHands == TTRUE )
	{
		// The hour hand does a turn every 12 hours
		TFLOAT fSeconds = AGameTimeManager::GAME_TIME_TO_REAL_TIME * pGameTimeManager->GetDayTime();
		if ( fSeconds > TFLOAT( AGameTimeManager::SECONDS_PER_DAY ) )
			fSeconds -= TFLOAT( AGameTimeManager::SECONDS_PER_DAY );

		TFLOAT fHourAngle = ( ( fSeconds <= s_fSecondsPerHalf ) ? fSeconds / s_fSecondsPerHalf : ( fSeconds - s_fSecondsPerHalf ) / s_fSecondsPerHalf ) * TMath::TWO_PI;

		// The minute hand does a turn every hour
		TFLOAT fSeconds2 = AGameTimeManager::GAME_TIME_TO_REAL_TIME * pGameTimeManager->GetDayTime();
		if ( fSeconds2 > TFLOAT( AGameTimeManager::SECONDS_PER_DAY ) )
			fSeconds2 -= TFLOAT( AGameTimeManager::SECONDS_PER_DAY );

		TFLOAT fOneOverSixty = 1.0f / 60.0f;
		TFLOAT fMinuteAngle  = ( fSeconds - TFLOAT( TUINT( fSeconds2 ) / AGameTimeManager::SECONDS_PER_HOUR ) * s_fSecondsPerHour ) * fOneOverSixty * fOneOverSixty * TMath::TWO_PI;

		T2GUITransform& rHourTransform = m_oHourHand.GetTransform();
		TVector2        vecHourPos     = rHourTransform.GetTranslation();
		rHourTransform.SetIdentity();
		rHourTransform.Rotate( fHourAngle );
		rHourTransform.SetTranslation( vecHourPos );

		T2GUITransform& rMinuteTransform = m_oMinuteHand.GetTransform();
		TVector2        vecMinutePos     = rMinuteTransform.GetTranslation();
		rMinuteTransform.SetIdentity();
		rMinuteTransform.Rotate( fMinuteAngle );
		rMinuteTransform.SetTranslation( vecMinutePos );
	}

	// The sign shows whether it's day or night
	AGameTimeManager::DAYPHASE eDayPhase = pGameTimeManager->GetCurrentDayPhase();

	if ( eDayPhase == AGameTimeManager::DAYPHASE_DAWN || eDayPhase == AGameTimeManager::DAYPHASE_MORNING || eDayPhase == AGameTimeManager::DAYPHASE_AFTERNOON )
		m_oDayNightSign.SetTextureSection( m_pAMSection );
	else
		m_oDayNightSign.SetTextureSection( m_pPMSection );
}

// $Barnyard: FUNCTION 00452540
void AGameTimeHudElement::ClockHand::PreRender()
{
	T2GUIElement::PreRender();
	T2GUI::GetRenderer()->PushTransform( m_oPivotTransform, TVector2::VEC_ZERO, TVector2::VEC_ZERO );
}

// $Barnyard: FUNCTION 00452570
void AGameTimeHudElement::ClockHand::PostRender()
{
	T2GUI::GetRenderer()->PopTransform();
	T2GUIElement::PostRender();
}
