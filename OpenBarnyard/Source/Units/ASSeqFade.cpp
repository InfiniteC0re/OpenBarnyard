#include "pch.h"
#include "ASSeqFade.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0062b200, CreateObject 0062b250, CreateObjectInPlace 0062b2d0
TDEFINE_CLASS( ASSeqFade );

// $Barnyard: FUNCTION 0062b210
ASSeqFade::ASSeqFade()
{
	m_oColour.uiColorR = 0;
	m_oColour.uiColorG = 0;
	m_oColour.uiColorB = 0;
	m_oColour.uiColorA = 255;
	m_bFadingIn        = TFALSE;
	m_fDuration        = 0.0f;
	m_fTime            = 0.0f;
}

// $Barnyard: FUNCTION 0062b350
// $Barnyard: FUNCTION 0062b240
ASSeqFade::~ASSeqFade()
{
}

// $Barnyard: FUNCTION 0062b370
void ASSeqFade::Reset( TBOOL a_bOpaque )
{
	if ( !a_bOpaque )
	{
		m_bFadingIn        = TTRUE;
		m_oColour.uiColorA = 0;
	}
	else
	{
		m_bFadingIn        = TFALSE;
		m_oColour.uiColorA = 255;
	}

	m_fDuration = 0.0f;
	m_fTime     = 0.0f;

	AGameStateController::GetSingleton()->SetOverlayParams( AGameState::OVERLAY_3, m_oColour );
	AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );
}

// $Barnyard: FUNCTION 0062b3b0
void ASSeqFade::FadeIn( TFLOAT a_fDuration )
{
	// Don't restart a fade in that is still going
	if ( !m_bFadingIn || m_fDuration <= m_fTime )
	{
		m_bFadingIn = TTRUE;
		m_fTime     = 0.0f;
		m_fDuration = a_fDuration;
		AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );
	}
}

// $Barnyard: FUNCTION 0062b3f0
void ASSeqFade::FadeOut( TFLOAT a_fDuration )
{
	// Don't restart a fade out that is still going
	if ( m_bFadingIn || m_fDuration <= m_fTime )
	{
		m_bFadingIn = TFALSE;
		m_fTime     = 0.0f;
		m_fDuration = a_fDuration;
		AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );
	}
}

// $Barnyard: FUNCTION 0062b430
void ASSeqFade::Update( TFLOAT a_fDeltaTime )
{
	if ( m_fTime >= m_fDuration )
		return;

	m_fTime += a_fDeltaTime;

	if ( m_fDuration < m_fTime )
	{
		if ( m_bFadingIn )
		{
			// Fully faded in, remove the overlay
			m_fDuration = 0.0f;
			m_fTime     = 0.0f;
			AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_1 );
			return;
		}

		m_fTime = m_fDuration;
	}

	TFLOAT fProgress = m_fTime / m_fDuration;

	if ( m_bFadingIn )
		fProgress = 1.0f - fProgress;

	m_oColour.uiColorA = TUINT8( TINT( fProgress * 255.0f ) );
	AGameStateController::GetSingleton()->SetOverlayParams( AGameState::OVERLAY_3, m_oColour );
}
