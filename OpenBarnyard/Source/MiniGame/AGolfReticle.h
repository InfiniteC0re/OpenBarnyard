#pragma once
#include "GUI/ATargetReticle.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIElement.h>

//-----------------------------------------------------------------------------
// Aim marker of the Gopher Golf minigame (AGolfMinigameState): a ring point
// ("Golf_Ring_Point") with the four ring quarters ("Golf_Ring") and the ball
// ("Putt_Ball") drawn on top of an ATargetReticle (mode 9). Every player has
// one at the landing spot of the shot, the state has another one at the flag.
// ctor 00470c40, dtor 00470e40, size 0x690.
// NOTE: the original name is unknown (it's a member of AGolfMinigameState).
//-----------------------------------------------------------------------------
class AGolfReticle
    : public ATargetReticle
{
public:
	static constexpr TINT NUM_RING_PARTS = 4;

public:
	AGolfReticle();
	~AGolfReticle();

	// Creates the rectangles and the reticle (hidden)
	void Create();

	// Shows (with SetActive) or hides the marker
	void SetVisible( TBOOL a_bVisible );

	// Sets m_bLocked, unlocking deactivates the reticle
	void SetLocked( TBOOL a_bLocked );

	// Sets the colour of the rectangles and the reticle (alpha 150 for the reticle)
	void SetMarkerColour( TUINT32 a_uiColour );

	// Changes the texture of the ball (a_szBall) and of the ring point (a_szRingPoint, can be TNULL)
	void SetTextures( const TCHAR* a_szBall, const TCHAR* a_szRingPoint );

	// Projects a_rcWorldPos to the screen and moves the marker there. When it's off the screen
	// the ring point is shown at the edge of the screen pointing at it and TTRUE is returned.
	TBOOL Update( const Toshi::TVector4& a_rcWorldPos, TFLOAT a_fDeltaTime );

	TBOOL IsVisible() const { return m_bVisible; }

public:
	Toshi::T2GUIRectangle m_oRingPoint;                   // 0x368
	Toshi::T2GUIElement   m_oRing;                        // 0x3e0
	Toshi::T2GUIRectangle m_aRingParts[ NUM_RING_PARTS ]; // 0x434
	Toshi::T2GUIRectangle m_oBall;                        // 0x614
	TBOOL                 m_bVisible;                     // 0x68c
	TBOOL                 m_bMarkerCreated;               // 0x68d
	TBOOL                 m_bLocked;                      // 0x68e
};
