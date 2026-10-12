#pragma once
#include "GUI/AGUI2Timer.h"
#include "GUI/AGUI2ScoreTicker.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/T2GUIString.h"
#include "Sound/ASoundLegacy.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIFont.h>
#include <Toshi/TPString8.h>
#include <Toshi/TString16.h>

//-----------------------------------------------------------------------------
// Keys of the Tease the Mailman / Tease Mrs Beady combos.
// Up, Left, Right and Down (Space) show the 3_Key, 1_Key, 2_Key and Space_Key icons.
//-----------------------------------------------------------------------------
enum STATUESKEY : TINT
{
	STATUESKEY_UP,    // 0, 3_Key
	STATUESKEY_LEFT,  // 1, 1_Key
	STATUESKEY_RIGHT, // 2, 2_Key
	STATUESKEY_DOWN,  // 3, Space_Key, also drops the player onto all fours
	STATUESKEY_NONE,  // 4, end of a combo / no key pressed
	STATUESKEY_NUMOF = STATUESKEY_NONE
};

//-----------------------------------------------------------------------------
// One pose of the statue (0x38 bytes). Built in AStatuesMiniGameState::OnInsertion.
// NOTE: invented name. The original has no symbol for it.
//-----------------------------------------------------------------------------
struct AStatuesPose
{
	static constexpr TINT MAX_KEYS  = 5;
	static constexpr TINT MAX_STARS = 5;

	// $Barnyard: FUNCTION 00485e10
	AStatuesPose()
	{
		for ( TINT i = 0; i < MAX_KEYS; i++ )
			aKeys[ i ] = STATUESKEY_NONE;

		for ( TINT i = 0; i < MAX_STARS; i++ )
			aScores[ i ] = 0;

		bUnk = TFALSE;
	}

	// NOTE: aKeys[ 0 ] is also the row of the pose (row r has r + 3 keys).
	STATUESKEY       aKeys[ MAX_KEYS ];    // 0x00, terminated by STATUESKEY_NONE
	Toshi::TPString8 strAnim;              // 0x14, statue animation
	TINT             aScores[ MAX_STARS ]; // 0x18, cumulative points of each star, 0 = no more stars
	TINT             iLocaleName;          // 0x2c, locale string with the name of the pose
	ASoundId         eSound;               // 0x30, cue played with the pose, ASOUNDID_INVALID = none
	TBOOL            bUnk;                 // 0x34
};

//-----------------------------------------------------------------------------
// HUD of AStatuesMiniGameState: timer, score, the three rows of key combos with
// the name of the next pose of each row, and the stars of the current pose.
// Plain helper (no TClass, no vtable), embedded in the state at 0xbd0 and
// attached to AGameState::m_GUIElement. Size 0x13a0.
// NOTE: invented name. The original has no symbol for it.
//-----------------------------------------------------------------------------
class AStatuesHud
{
public:
	static constexpr TINT NUM_ROWS  = 3;
	static constexpr TINT MAX_KEYS  = AStatuesPose::MAX_KEYS;
	static constexpr TINT MAX_STARS = AStatuesPose::MAX_STARS;

	// Key icons of one row (0x258 bytes)
	struct KeyRow
	{
		// $Barnyard: FUNCTION 00484a00
		KeyRow() {}

		// $Barnyard: FUNCTION 00484a20
		~KeyRow() {}

		Toshi::T2GUIRectangle aKeys[ MAX_KEYS ];
	};

public:
	AStatuesHud();
	~AStatuesHud();

	// Creates the elements and attaches them to a_pParent
	void Create( Toshi::T2GUIElement* a_pParent );

	// Detaches the elements and releases the fonts
	void Destroy();

	void Update( TFLOAT a_fDeltaTime );

	void Hide();
	void Show();

	// Shows a_iNumStars stars; a newly earned star pops up and shows "+a_iPoints" (if not 0)
	void SetNumStars( TINT a_iNumStars, TINT a_iPoints );

	// Shows the "%d_Star_Panel" panel, hides it for 0
	void SetStarPanel( TINT a_iNumStars );

	// Sets the icon of a key of a row, STATUESKEY_NONE hides it
	void SetKeyIcon( TINT a_iRow, TINT a_iIndex, STATUESKEY a_eKey, TBOOL a_bPressed );

	// Shows the keys and the name of the pose of a row
	void SetRowPose( TINT a_iRow, AStatuesPose* a_pPose );

	// Resets every row to its pose and clears the current combo
	void ResetRows();

	// Feeds a key into the current combo. Returns TNULL while the combo goes on,
	// the pose once it's complete or &m_oNoPose if the key didn't match (the rows reset)
	AStatuesPose* HandleKey( STATUESKEY a_eKey );

	// $Barnyard: FUNCTION 00484a60
	TINT GetNumStars() const { return m_iNumStars; }

	// $Barnyard: FUNCTION 00484a70
	void SetScore( TINT a_iScore )
	{
		m_oScoreTicker.SetScore( a_iScore );
		m_oScoreTicker.SetSpinning( TTRUE );
	}

	// $Barnyard: FUNCTION 00484a90
	void SetDuration( TFLOAT a_fDuration ) { m_oTimer.SetDuration( a_fDuration ); }

	// $Barnyard: FUNCTION 00484aa0
	void SetTime( TFLOAT a_fTime ) { m_oTimer.SetTime( a_fTime ); }

	// The input panel slides out while it's hidden and the name of the completed pose flies to it
	// $Barnyard: FUNCTION 00484ab0
	void SetInputVisible( TBOOL a_bVisible ) { m_bInputHidden = !a_bVisible; }

	AStatuesPose* GetNoPose() { return &m_oNoPose; }
	AGUI2Timer&   GetTimer() { return m_oTimer; }

private:
	AGUI2Timer            m_oTimer;                    // 0x0000
	AGUI2ScoreTicker      m_oScoreTicker;              // 0x0220, "darts_wheel_window"
	Toshi::T2GUIElement   m_oInputPanel;               // 0x031c
	Toshi::T2GUIElement   m_oScorePanel;               // 0x0370
	Toshi::T2GUIString    m_aPoseNames[ NUM_ROWS ];    // 0x03c4, name of the next pose of each row
	Toshi::T2GUIString    m_oCompletedPoseName;        // 0x04f0, flies to the input panel
	Toshi::T2GUIRectangle m_oScoreWood;                // 0x0554, "Score_Wood"
	Toshi::T2GUIRectangle m_oStarPanel;                // 0x05cc, "%d_Star_Panel"
	Toshi::T2GUIRectangle m_oInputBase;                // 0x0644, "Statue_Input_base"
	Toshi::T2GUIRectangle m_aStars[ MAX_STARS ];       // 0x06bc, "Statue_GoldStar"
	Toshi::T2GUIString    m_aStarTexts[ MAX_STARS ];   // 0x0914, "+%d"
	KeyRow                m_aKeyRows[ NUM_ROWS ];      // 0x0b08
	AStatuesPose*         m_apRowPoses[ NUM_ROWS ];    // 0x1210
	AStatuesPose          m_oNoPose;                   // 0x121c, returned by HandleKey when the key doesn't match
	AGUI2TextBox          m_oInstructions;             // 0x1254, locale 1017
	TWCHAR                m_wszInstructions[ 64 ];     // 0x12cc
	Toshi::TString16      m_aStarStrings[ MAX_STARS ]; // 0x134c
	Toshi::T2GUIFont*     m_pFont;                     // 0x1388, "rekord26"
	Toshi::T2GUIFont*     m_pFont2;                    // 0x138c, "Rekord26"
	TINT                  m_iCurrentRow;               // 0x1390, row of the combo being typed, -1 = none
	TINT                  m_iCurrentKey;               // 0x1394, index of the next key in that row
	TINT                  m_iNumStars;                 // 0x1398
	TBOOL                 m_bCreated;                  // 0x139c
	TBOOL                 m_bStarPanelVisible;         // 0x139d
	TBOOL                 m_bInputHidden;              // 0x139e
};

TSTATICASSERT( sizeof( AStatuesPose ) == 0x38 );
TSTATICASSERT( sizeof( AStatuesHud::KeyRow ) == 0x258 );
TSTATICASSERT( sizeof( AStatuesHud ) == 0x13a0 );
