#pragma once
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: A text label that follows a world position on the screen, with a pointer
// ("Putt_Red_Point", "Putt_Yel_Point" when the position is off screen) on the side
// facing the position. Used by ASimpleProjectileManager (distance of the tracked
// projectile) and by ACarChaseMiniGameState.
// NOTE: not a T2GUIElement itself (no vtable), the name is invented.
//-----------------------------------------------------------------------------
class AGUI2TargetIndicator
{
public:
	// Corner/side of the label the pointer is on (see SetPointerSide)
	using SIDE = TINT;
	enum SIDE_ : SIDE
	{
		SIDE_TOPRIGHT    = 0,
		SIDE_TOP         = 1,
		SIDE_TOPLEFT     = 2,
		SIDE_LEFT        = 3,
		SIDE_BOTTOMLEFT  = 4,
		SIDE_BOTTOM      = 5,
		SIDE_BOTTOMRIGHT = 6,
		SIDE_RIGHT       = 7,
		SIDE_DEFAULT     = 8, // same as SIDE_TOPLEFT, set by Create
		SIDE_NONE        = 9,
	};

	static constexpr TINT MAX_TEXT_LENGTH = 31;

public:
	AGUI2TargetIndicator();
	~AGUI2TargetIndicator();

	// Creates the elements and adds them to a_pParent (the root element of AGUI2 if TNULL)
	TBOOL Create( Toshi::T2GUIElement* a_pParent );

	// Projects the target position on the screen, places the label and turns the pointer to it
	void Update( TFLOAT a_fDeltaTime );

	// Shows or hides the label
	void SetVisible( TBOOL a_bVisible );

	// Sets the text (a_wszText isn't copied) and the target position, resizes the label
	void SetText( const TWCHAR* a_wszText, const Toshi::TVector4& a_rcPosition );

	// Sets the text to the distance a_fDistance (with the unit) and the target position
	void SetDistance( const Toshi::TVector4& a_rcPosition, TFLOAT a_fDistance );

	// Moves the pointer to a_eSide of the label
	void SetPointerSide( SIDE a_eSide );

	TBOOL IsVisible() const { return m_bVisible; }

public:
	inline static Toshi::T2GUITextureSection* ms_pRedPoint;    // 0x007b50c0, "Putt_Red_Point"
	inline static Toshi::T2GUITextureSection* ms_pYellowPoint; // 0x007b50c4, "Putt_Yel_Point"

private:
	Toshi::T2GUIRectangle m_oRoot;                      // 0x000, the label
	Toshi::T2GUIRectangle m_oPointerContainer;          // 0x078
	Toshi::T2GUIRectangle m_oPointer;                   // 0x0f0
	AGUI2TextBox          m_oText;                      // 0x168
	TWCHAR                m_wszText[ MAX_TEXT_LENGTH ]; // 0x1e0
	Toshi::TVector4       m_vScreenPosition;            // 0x220
	Toshi::TVector4       m_vPosition;                  // 0x230, target position in the world
	TFLOAT                m_fWidth;                     // 0x240
	TFLOAT                m_fHeight;                    // 0x244
	SIDE                  m_eSide;                      // 0x248
	TBOOL                 m_bVisible;                   // 0x24c
};

TSTATICASSERT( sizeof( AGUI2TargetIndicator ) == 0x250 );
