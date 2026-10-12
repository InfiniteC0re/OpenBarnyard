#pragma once
#include "GameInterface/AGameStateController.h"

#include <Toshi/TObject.h>

// Screen fades of the scripted sequences (SSeq), owned by ASSeqController (+0x6c).
// Uses the third overlay of AGameStateController (AGameState::OVERLAY_3).
//
// TClass 0x007b4bc0 (size 0x14, runtime creatable), vtable 0x00739350.
class ASSeqFade : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ASSeqFade, Toshi::TObject );

public:
	ASSeqFade();
	~ASSeqFade();

	// Shows the overlay right away (a_bOpaque) or prepares fading in from a clear screen
	void Reset( TBOOL a_bOpaque );

	// Fades the overlay out of the screen within a_fDuration
	void FadeIn( TFLOAT a_fDuration );

	// Fades the overlay over the screen within a_fDuration
	void FadeOut( TFLOAT a_fDuration );

	void Update( TFLOAT a_fDeltaTime );

	// TTRUE once the current fade is over
	TBOOL IsOver() const { return m_fTime >= m_fDuration; }

private:
	AGameStateController::OverlayData m_oColour;   // 0x04
	TBOOL                             m_bFadingIn; // 0x08, the overlay is disappearing
	TFLOAT                            m_fDuration; // 0x0c
	TFLOAT                            m_fTime;     // 0x10
};

TSTATICASSERT( sizeof( ASSeqFade ) == 0x14 );
