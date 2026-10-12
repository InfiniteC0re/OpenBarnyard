#pragma once
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: Rating of the distance of the tracked projectile (see
// ASimpleProjectileManager): up to five gold stars ("Statue_GoldStar") fly in
// from the right side of the screen onto a wooden plank ("Wood_Thin") and a label
// under them names the rating (locale strings 700-703 and 326 for a new record).
// NOTE: not a T2GUIElement itself (no vtable), the name is invented.
//-----------------------------------------------------------------------------
class AGUI2StarRating
{
public:
	static constexpr TINT NUM_STARS = 5;

public:
	AGUI2StarRating();
	~AGUI2StarRating();

	// Creates the elements. a_fX and a_fY are the position of the middle star as a
	// fraction of the screen size, a_fStarSize the size of a star as a fraction of
	// the smaller screen dimension.
	TBOOL Create( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fStarSize );

	// Moves the stars towards their positions
	TBOOL Update( TFLOAT a_fDeltaTime );

	void SetVisible( TBOOL a_bVisible );

	// Sets the number of stars (0-5) and the label
	TBOOL SetNumStars( TINT a_iNumStars );

	// Rates a_fDistance, returns TTRUE if it's a new record (stored in a_rfRecord)
	TBOOL SetDistance( TFLOAT a_fDistance, TFLOAT& a_rfRecord );

	// $Barnyard: FUNCTION 006375c0
	TINT GetNumStars() const { return m_iNumStars; }

private:
	Toshi::T2GUIRectangle m_oPlank;                      // 0x000
	Toshi::T2GUIRectangle m_aStars[ NUM_STARS ];         // 0x078
	Toshi::T2GUIRectangle m_oLabelRoot;                  // 0x2d0
	AGUI2TextBox          m_oLabel;                      // 0x348
	Toshi::TVector2       m_vPosition;                   // 0x3c0, the middle star
	Toshi::TVector2       m_aStarTargets[ NUM_STARS ];   // 0x3c8
	Toshi::TVector2       m_aStarPositions[ NUM_STARS ]; // 0x3f0
	Toshi::TVector2       m_vStarStart;                  // 0x418, off screen on the right
	TINT                  m_iNumStars;                   // 0x420
	TBOOL                 m_bVisible;                    // 0x424
	TBOOL                 m_bMoving;                     // 0x425, a visible star isn't at its position yet
	const TWCHAR*         m_apLabels[ NUM_STARS ];       // 0x428
};

TSTATICASSERT( sizeof( AGUI2StarRating ) == 0x43c );
