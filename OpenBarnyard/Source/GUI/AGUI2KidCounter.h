#pragma once
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// NOTE: invented name (no leaked one), the textures are Tommy_on/Tommy_off/Tommy_Kid_Seeking.
// Cow Tipping counter of the remaining kids ("Tommy" icons): up to 20 icons in a
// 5x4 grid, each one an "on" image over an "off" image that both pop when they
// change, plus a text above them. Counts up/down towards the target count.
// Cow Tipping HUD element at ACowTippingMiniGameState+0x57d4.
// NOTE: retail has no own vtable (the ctor doesn't set one), so the T2GUIElement is
// probably a member there; it's a base class here.
// ctor 00636150, dtor 006361f0, size 0x1634.
//-----------------------------------------------------------------------------
class AGUI2KidCounter
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT MAX_NUM_ICONS = 20;

	enum MODE : TINT
	{
		MODE_0,       // icon size 32
		MODE_1,       // icon size 50
		MODE_2,       // icon size 31.99, the last icon pops to m_fLastIconState
		MODE_SEEKING, // a single big "Tommy_Kid_Seeking" icon
	};

	struct Icon
	{
		Icon();
		~Icon();

		// Pops the "on" image in (1.0f) or out (0.0f)
		void SetState( TFLOAT a_fState );

		// Pops the "off" image to a new size
		void SetSize( TFLOAT a_fSize )
		{
			if ( a_fSize != fSize )
			{
				fPrevSize = oOn.GetWidth();
				fSize     = a_fSize;
				fSizeTime = 0.2f;
				oOff.Show();
			}
		}

		void UpdatePosition( MODE a_eMode, TINT a_iNumIcons );
		void Update( TFLOAT a_fDeltaTime, MODE a_eMode, TINT a_iNumIcons );

		Toshi::T2GUIRectangle oOn;        // 0x00
		Toshi::T2GUIRectangle oOff;       // 0x78
		TFLOAT                fStateTime; // 0xf0
		TFLOAT                fState;     // 0xf4
		TFLOAT                fPrevState; // 0xf8
		TFLOAT                fSizeTime;  // 0xfc
		TFLOAT                fSize;      // 0x100
		TFLOAT                fPrevSize;  // 0x104
		TFLOAT                fRowY;      // 0x108
		TINT                  iIndex;     // 0x10c
	};

	TSTATICASSERT( sizeof( Icon ) == 0x110 );

public:
	AGUI2KidCounter();
	~AGUI2KidCounter();

	// Sets the count the counter goes to
	void SetTargetCount( TINT a_iCount );

	// Sets the text above the icons (scaled down for long texts). The text isn't copied.
	void SetText( const TWCHAR* a_wszText );

	// Creates the icons and attaches the counter to a_pParent
	void Create( Toshi::T2GUIElement* a_pParent );

	// Sets the displayed count right away
	void SetCount( TINT a_iCount );

	void SetIconSize( TFLOAT a_fIconSize );

	void Update( TFLOAT a_fDeltaTime );

	void SetNumIcons( TINT a_iNumIcons );

	void SetMode( TINT a_eMode, TINT a_iNumIcons );

	// Sets the mode and finishes all the animations
	void SetModeInstantly( TINT a_eMode, TINT a_iNumIcons );

public:
	AGUI2TextBox                m_oText;                   // 0x54
	Toshi::T2GUITextureSection* m_pTexOn;                  // 0xcc
	Toshi::T2GUITextureSection* m_pTexSeeking;             // 0xd0
	TBOOL                       m_bCreated;                // 0xd4
	TFLOAT                      m_fCountTime;              // 0xd8
	TFLOAT                      m_fLastIconState;          // 0xdc
	TINT                        m_iTargetCount;            // 0xe0
	TINT                        m_iCount;                  // 0xe4
	TFLOAT                      m_fIconSize;               // 0xe8
	TINT                        m_iNumIcons;               // 0xec
	MODE                        m_eMode;                   // 0xf0
	Icon                        m_aIcons[ MAX_NUM_ICONS ]; // 0xf4
};

TSTATICASSERT( sizeof( AGUI2KidCounter ) == 0x1634 );
