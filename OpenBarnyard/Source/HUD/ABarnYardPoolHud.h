#pragma once
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2FadingText.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// HUD of Barn Pool: a panel for each team (portrait, name and the balls of the team),
// a message text in the bottom and three fading texts at the top of the screen.
// Names from the Wii build (ABarnYardPoolHud, ABarnYardPoolHud::TeamHud).
// No vtable, embedded in ABarnYardPoolMiniGameState (+0x1394).
// ctor 004d1260, dtor 004d1300, size 0x1274.
//-----------------------------------------------------------------------------
class ABarnYardPoolHud
{
public:
	static constexpr TINT NUM_TEAMS = 2;

	//-----------------------------------------------------------------------------
	// Panel of a team: the portrait with the name on a wooden plank and the 7 balls
	// of the team plus the black ball (attached to the parent element, not the panel).
	// Plain T2GUIElement (no TClass), vtable 00720420, size 0x5c8.
	//-----------------------------------------------------------------------------
	class TeamHud
	    : public Toshi::T2GUIElement
	{
	public:
		static constexpr TINT NUM_BALLS      = 8; // the 7 balls of the team and the black one
		static constexpr TINT NUM_TEAM_BALLS = 7;
		static constexpr TINT BLACK_BALL     = 7;
		static constexpr TINT NAME_SIZE      = 32;

		// Ball colours of SetBallColour
		enum BALLCOLOUR : TINT
		{
			BALLCOLOUR_RED    = 1, // NOTE: anything but BALLCOLOUR_YELLOW is red
			BALLCOLOUR_YELLOW = 2,
		};

	public:
		TeamHud();
		~TeamHud();

		//-----------------------------------------------------------------------------
		// Toshi::T2GUIElement
		//-----------------------------------------------------------------------------
		virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

		//-----------------------------------------------------------------------------
		// Own methods
		//-----------------------------------------------------------------------------
		virtual void SetPanelVisible( TBOOL a_bVisible ); // +0x48

		// Creates the panel (the left one when a_bLeft is set) and attaches it to
		// a_pParent (TNULL is the root element)
		TBOOL Create( TBOOL a_bLeft, Toshi::T2GUIElement* a_pParent );

		// Sets the portrait (a_szTexture texture section) and the name
		void SetPlayer( const TCHAR* a_szTexture, const TWCHAR* a_wszName, TBOOL a_bLeft );

		// Gives the balls the colour a_eColour (BALLCOLOUR) and shows them, if not shown yet
		void SetBallColour( TINT a_eColour );

		// Makes the first a_iNumBalls balls opaque, the others are faded
		void SetNumLitBalls( TINT a_iNumBalls );

		// Hides the balls
		void HideBalls();

	public:
		Toshi::T2GUITextureSection* m_pYellowBall;          // 0x54
		Toshi::T2GUITextureSection* m_pRedBall;             // 0x58
		Toshi::T2GUIRectangle       m_oPortrait;            // 0x5c
		Toshi::T2GUIRectangle       m_oPlank;               // 0xd4
		AGUI2TextBox                m_oName;                // 0x14c
		Toshi::T2GUIRectangle       m_aBalls[ NUM_BALLS ];  // 0x1c4
		TWCHAR                      m_wszName[ NAME_SIZE ]; // 0x584
		TBOOL                       m_bBallsShown;          // 0x5c4
	};

public:
	ABarnYardPoolHud();
	~ABarnYardPoolHud();

	// Creates the HUD and attaches it to a_pParent (TNULL is the root element).
	// a_pUnused isn't used.
	TBOOL Create( Toshi::T2GUIElement* a_pUnused, Toshi::T2GUIElement* a_pParent );

	// Hides the balls and clears all the texts
	void Reset();

	// Updates the fading texts
	void Update( TFLOAT a_fDeltaTime );

	// Shows or hides the panels of both teams
	void SetTeamsVisible( TBOOL a_bVisible );

	// Makes the panel of the first team opaque and hides the other one (or the other way round)
	void SetFirstTeamActive( TBOOL a_bFirstTeam );

	// Sets the portrait and the name of the team a_iTeam (0 or 1)
	void SetTeamPlayer( const TCHAR* a_szTexture, const TWCHAR* a_wszName, TINT a_iTeam );

	// See TeamHud::SetBallColour
	void SetBallColour( TINT a_iTeam, TINT a_eColour );

	// See TeamHud::SetNumLitBalls
	void SetNumLitBalls( TINT a_iTeam, TINT a_iNumBalls );

	// Shows a_wszText (not copied) at the bottom of the screen, TNULL hides it
	void SetMessage( const TWCHAR* a_wszText );

	// Shows whose turn it is in the third line: the first team's player when
	// a_bOther is TFALSE, the other one's otherwise
	void ShowTurn( TBOOL a_bOther );

	// Shows a_wszText in the first line
	void SetTitleText( const TWCHAR* a_wszText );

	// Shows a_wszText in the second line
	void SetInfoText( const TWCHAR* a_wszText );

public:
	TeamHud         m_aTeams[ NUM_TEAMS ];      // 0x0
	AGUI2TextBox    m_oMessage;                 // 0xb90
	TFLOAT          m_fMessageTime;             // 0xc08
	TBOOL           m_bMessageFading;           // 0xc0c, NOTE: never set in retail, the message doesn't fade out
	AGUI2FadingText m_oTurnText;                // 0xc10, third line
	AGUI2FadingText m_oInfoText;                // 0xd84, second line
	AGUI2FadingText m_oTitleText;               // 0xef8, first line
	TWCHAR          m_wszTurnText[ 256 ];       // 0x106c
	const TWCHAR*   m_apTeamNames[ NUM_TEAMS ]; // 0x126c
};

TSTATICASSERT( sizeof( ABarnYardPoolHud::TeamHud ) == 0x5c8 );
TSTATICASSERT( sizeof( ABarnYardPoolHud ) == 0x1274 );
