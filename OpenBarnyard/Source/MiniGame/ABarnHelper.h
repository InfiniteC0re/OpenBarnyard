#pragma once
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

class AUnit;
class AAnimatableObject;

//-----------------------------------------------------------------------------
// Purpose: plays the activities of the animals of the barn: the pool, the
// pinball, whack-a-mole (Freddy), darts, the mechanical bull's crowd, the cards
// table and a goat walking between the games in the loft; Otis at the bar,
// Daisy and Bessy, the band and Wild Mike on the ground floor. The animals
// close to the player look at them and chat (AVOStream "chat" sets).
// Only the activities of the floor the player is on are updated.
// Created by ABarnExploreMinigameState::OnInsertion.
// NOTE: "ABarnHelper" is a class name of the Wii build, it isn't known if it's
// this one. Size 0x3c8.
//-----------------------------------------------------------------------------
class ABarnHelper
{
public:
	// The anims of the activities (m_aAnims)
	enum ANIM
	{
		ANIM_POOL_IDLE,
		ANIM_POOL_LOOK,
		ANIM_PINBALL_HAMMERPULL,
		ANIM_PINBALL_PLAY,
		ANIM_WHACK_LOOK,
		ANIM_WHACK_IDLE,
		ANIM_DARTS_IDLE,
		ANIM_DARTS_STEPUP,
		ANIM_DARTS_THROW,
		ANIM_DARTS_STEPDOWN,
		ANIM_MECHMAN_IDLE,
		ANIM_CHEER,
		ANIM_BAR_DRINK,
		ANIM_BAR_IDLE,
		ANIM_PLAY_MUSIC,
		ANIM_TALK,
		ANIM_HOLDCUE,
		ANIM_HOLDDRINK,
		ANIM_DANCE1,
		ANIM_DANCE2,
		ANIM_DANCE3,
		ANIM_IDLE1,
		ANIM_IDLE2,
		ANIM_IDLE3,
		ANIM_IDLE4,
		ANIM_CARDS_IDLE,
		ANIM_CARDS_LOOK,
		ANIM_LOOKL,
		ANIM_LOOKR,
		ANIM_IDLE_LP,
		ANIM_NUMOF
	};

	// The band members (m_aBand)
	enum BAND
	{
		BAND_DUKE,
		BAND_GOAT,
		BAND_EVERET,
		BAND_COW,
		BAND_HORSE,
		BAND_NUMOF
	};

	static constexpr TINT MAX_NUM_GOAT_SPOTS = 8;

	// An animal doing an activity
	struct Activity
	{
		TINT iState; // 0x00
		union
		{
			AUnit*             pUnit;   // 0x04
			AAnimatableObject* pObject; // 0x04, Wild Mike and his box are animatable objects
		};
		TFLOAT fTimer;       // 0x08
		TCHAR  szName[ 16 ]; // 0x0c, the type name of the unit without "nb_" (its VO set)
		TBOOL  bFlag;        // 0x1c, it's chatting, Wild Mike: the anim loops, the box: an anim plays
	};

public:
	ABarnHelper( AUnit* a_pPlayer );
	~ABarnHelper();

	void Update( TFLOAT a_fDeltaTime );

	// Clears the activities
	void Reset();

	// Adding the animals of the activities
	void AddPool( AUnit* a_pUnit );
	void AddPinball( AUnit* a_pUnit );
	void AddFreddy( AUnit* a_pUnit );
	void AddOtis( AUnit* a_pUnit );
	void AddDarts( AUnit* a_pUnit );
	void AddCheer( AUnit* a_pUnit );
	void AddBandMember( AUnit* a_pUnit );
	void AddCards( AUnit* a_pUnit );
	void AddDaisyBessy( AUnit* a_pUnit );
	void AddGoat( AUnit* a_pUnit );
	void AddWildMike( AAnimatableObject* a_pObject );
	void AddWildMikeBox( AAnimatableObject* a_pObject );

	// Shows or hides the band
	void SetBandVisible( TBOOL a_bVisible );

	// Animates Wild Mike or not
	void SetWildMikeUpdating( TBOOL a_bUpdating );

	// The cue of the pinball machines (m_iPinballCue)
	TINT GetPinballCue() const { return m_iPinballCue; }

	// The positions Wild Mike walks between on the stage ("wm_start", "wm_left", "wm_right")
	Toshi::TVector4* GetWildMikePositions() { return m_aWildMikePositions; }

	// Is one of the games the goat walks between bought
	static TBOOL HasGoatSpots();

private:
	void UpdatePool( TFLOAT a_fDeltaTime, TBOOL a_bNear );
	void UpdatePinball( TFLOAT a_fDeltaTime, Activity& a_rActivity );
	void UpdateFreddy( TFLOAT a_fDeltaTime );
	void UpdateDarts( TFLOAT a_fDeltaTime, Activity& a_rActivity, const Activity& a_rcOther );
	void UpdateCheer( TFLOAT a_fDeltaTime );
	void UpdateCards( TFLOAT a_fDeltaTime, Activity& a_rActivity, TBOOL a_bNear );
	void UpdateGoat( TFLOAT a_fDeltaTime, TBOOL a_bNear );
	void UpdateWildMikeBox( TFLOAT a_fDeltaTime );
	void UpdateOtis( TFLOAT a_fDeltaTime, TBOOL a_bNear );
	void UpdateDaisyBessy( TFLOAT a_fDeltaTime, Activity& a_rActivity, TBOOL a_bNear );
	void UpdateWildMike( TFLOAT a_fDeltaTime );

	// Is the player close (6 metres) to the unit
	TBOOL IsPlayerNear( AUnit* a_pUnit ) const;

public:
	// DAT_00782e98 and DAT_00782e30: the spots (goals of the bought games) the goat walks between and the directions it faces
	static Toshi::T2Vector<Toshi::TVector4, 6> ms_vecGoatPositions;
	static Toshi::T2Vector<Toshi::TVector4, 6> ms_vecGoatDirections;

	// DAT_00782e28, Wild Mike's anim was dance1 (it's killed before he walks)
	inline static TBOOL ms_bWildMikeKillAnims = TFALSE;

private:
	TINT                         m_iPinballCue;             // 0x000, 690
	Toshi::TVector4              m_aWildMikePositions[ 3 ]; // 0x004
	Activity                     m_oPool;                   // 0x034
	Activity                     m_oFreddy;                 // 0x054
	Activity                     m_oCheer;                  // 0x074
	Activity                     m_oOtis;                   // 0x094
	Activity                     m_oGoat;                   // 0x0b4
	Toshi::T2Vector<Activity, 2> m_vecPinball;              // 0x0d4
	Toshi::T2Vector<Activity, 2> m_vecDarts;                // 0x118
	Activity                     m_aBand[ BAND_NUMOF ];     // 0x15c
	Activity                     m_oDaisy;                  // 0x1fc
	Activity                     m_oBessy;                  // 0x21c
	Activity                     m_oWildMike;               // 0x23c
	Activity                     m_oWildMikeBox;            // 0x25c
	Toshi::T2Vector<Activity, 4> m_vecCards;                // 0x27c
	Toshi::TPString8             m_aAnims[ ANIM_NUMOF ];    // 0x300
	TINT                         m_iNumBandMembers;         // 0x378
	TBOOL                        m_bPinballCuePlaying;      // 0x37c
	TINT                         m_iGoatSpot;               // 0x380
	TBOOL                        m_bDaisyBessyChatting;     // 0x384
	AUnit*                       m_pPlayer;                 // 0x388
	Toshi::TVector4              m_vecDaisyBessyPosition;   // 0x38c, between Daisy and Bessy
	Toshi::TPString8             m_strWildMikeBoxAnim;      // 0x39c
	Toshi::TPString8             m_strWildMikeAnim;         // 0x3a0
	TINT                         m_iWildMikePosition;       // 0x3a4, index of m_aWildMikePositions
	Toshi::TVector4              m_vecWildMikePosition;     // 0x3a8
	Toshi::TVector4              m_vecWildMikeTarget;       // 0x3b8
};

TSTATICASSERT( sizeof( ABarnHelper::Activity ) == 0x20 );
TSTATICASSERT( sizeof( ABarnHelper ) == 0x3c8 );
