#pragma once
#include "FSM/AFunctionTable.h"
#include "GameInterface/AHackedQuickSave.h"
#include "Helpers/AGameTimeManager.h"

#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AModelInstance;
class AParticleEffectHandle;
class AGameState;

//-----------------------------------------------------------------------------
// Purpose: A minigame that can be replayed from the farm once it has been
// played in the story (its quest tier has been passed). Every replay has a
// star in the world (at the locator of its name) that starts the minigame.
// The variants (m_abVariants) are the levels of the minigame that can be
// picked (AReplayVariantSelectState), the minigame gets the index + 1.
// Base vtable 0x00724328.
//-----------------------------------------------------------------------------
class AReplayableGame
{
public:
	// Where the star of the replay is (AReplayManager::CreateStars)
	using LOCATION = TINT;
	enum LOCATION_ : LOCATION
	{
		LOCATION_FARM = 1,
		LOCATION_BARN = 2,
		LOCATION_ALL  = 4,
	};

	using STARSTATE = TINT;
	enum STARSTATE_ : STARSTATE
	{
		STARSTATE_NONE        = 0,
		STARSTATE_AVAILABLE   = 1, // m_pStar
		STARSTATE_COMPLETED   = 2, // m_pStarCompleted, the human star of the high score has been earned
		STARSTATE_UNAVAILABLE = 3, // m_pStarUnavailable
		STARSTATE_HIDDEN      = 4,
	};

	static constexpr TINT MAX_NUM_VARIANTS = 12;

public:
	AReplayableGame();
	virtual ~AReplayableGame();

	virtual void  Create( Toshi::TPString8 a_strLocator, const Toshi::TPString8& a_rcMiniGame, LOCATION a_eLocation, Toshi::TPString8 a_strName, Toshi::TPString8 a_strLocator2 ); // +0x04
	virtual TBOOL IsAvailable() = 0;                                                                                                                                               // +0x08, also updates m_abVariants
	virtual void  Save( AHackedQuickSave::Buffer* a_pBuffer ) {}                                                                                                                   // +0x0c, DOES_NOTHING
	virtual void  Load( AHackedQuickSave::Buffer* a_pBuffer ) {}                                                                                                                   // +0x10, DOES_NOTHING
	virtual void  Reset() {}                                                                                                                                                       // +0x14, DOES_NOTHING

	// TTRUE if the story is past the quest tier of the replay
	TBOOL IsTierReached() const;

	void GetStarPosition( Toshi::TVector4& a_rOutPosition ) const;

	TINT  GetNumVariants() const;
	TBOOL HasMultipleVariants() const;

public:
	Toshi::TPString8       m_strLocator;                     // 0x04, the star is placed at this locator (or interior area)
	Toshi::TPString8       m_strLocator2;                    // 0x08
	TINT                   m_iMiniGame;                      // 0x0c, AMiniGameManager index
	TINT                   m_iAct;                           // 0x10
	TINT                   m_iTier;                          // 0x14
	TINT                   m_iHighScoreRecord;               // 0x18, AHighScoreManager record, -1 if none
	TINT                   m_iMapIcon;                       // 0x1c, ASimMiniMap icon
	Toshi::TPString8       m_strMiniGame;                    // 0x20, class name of the minigame state
	Toshi::TPString8       m_strName;                        // 0x24
	LOCATION               m_eLocation;                      // 0x28
	STARSTATE              m_eStarState;                     // 0x2c
	AModelInstance*        m_pStar;                          // 0x30
	AModelInstance*        m_pStarUnavailable;               // 0x34
	AModelInstance*        m_pStarCompleted;                 // 0x38
	AParticleEffectHandle* m_pEffect;                        // 0x3c, "Gamezone"
	AReplayableGame*       m_pNext;                          // 0x40
	TBOOL                  m_abVariants[ MAX_NUM_VARIANTS ]; // 0x44
	TBOOL                  m_bDisabled;                      // 0x50, never set in retail
	TINT                   m_iNight;                         // 0x54, AGenericReplayGame: 1 if the replay is played at night
};

TSTATICASSERT( sizeof( AReplayableGame ) == 0x58 );

// Available once the tier is reached, at the time of day of m_iNight (vtable 0x00724388)
class AGenericReplayGame : public AReplayableGame
{
public:
	virtual TBOOL IsAvailable() override;
};

// The replays with their own rules (names of the Wii build)
#define AREPLAY_DECLARE( CLASS )              \
	class CLASS : public AReplayableGame      \
	{                                         \
	public:                                   \
		virtual TBOOL IsAvailable() override; \
	}

AREPLAY_DECLARE( AReplayVeggiePatch );   // vtable 0x007243a0
AREPLAY_DECLARE( AReplayChickDefender ); // vtable 0x007243b8
AREPLAY_DECLARE( AReplayDarts );         // vtable 0x007243d0
AREPLAY_DECLARE( AReplayBarnPool );      // vtable 0x007243e8
AREPLAY_DECLARE( AReplayTeaseMailMan );  // vtable 0x00724400
AREPLAY_DECLARE( AReplayTeaseBeady );    // vtable 0x00724418
AREPLAY_DECLARE( AReplayGolfBackNine );  // vtable 0x00724430
AREPLAY_DECLARE( AReplayGolfFrontNine ); // vtable 0x00724448
AREPLAY_DECLARE( AReplayBarntender );    // vtable 0x00724460
AREPLAY_DECLARE( AReplayCowTipping );    // vtable 0x00724478
AREPLAY_DECLARE( AReplayWhack );         // vtable 0x00724490

#undef AREPLAY_DECLARE

// The honey of the three hives is saved (vtable 0x00724340)
class AReplayHoneyCollecting : public AReplayableGame
{
public:
	static constexpr TINT NUM_HIVES = 3;

public:
	AReplayHoneyCollecting();

	virtual TBOOL IsAvailable() override;
	virtual void  Save( AHackedQuickSave::Buffer* a_pBuffer ) override;
	virtual void  Load( AHackedQuickSave::Buffer* a_pBuffer ) override;
	virtual void  Reset() override;

	// Fills the hives again
	void RefillHives();

public:
	TINT m_aiHoney[ NUM_HIVES ]; // 0x58
};

TSTATICASSERT( sizeof( AReplayHoneyCollecting ) == 0x64 );

//-----------------------------------------------------------------------------
// Purpose: The replays of the minigames (the stars in the world, the map and
// the "ReplayMiniGame" FSM native).
// TClass 0x00783df8 (version 1.0, size 0x924), singleton at 0x00783e28,
// vtable 0x00724658.
//-----------------------------------------------------------------------------
class AReplayManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AReplayManager>
{
public:
	TDECLARE_CLASS( AReplayManager, Toshi::TObject );

public:
	AReplayManager();
	~AReplayManager();

	virtual AFunctionTable* GetFunctionTable(); // +0x0c

	TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer );

	// Resets the replays (AGameLoader::ReturnToFrontEnd)
	void Reset();
	void ResetAllUnlocked() { m_bAllUnlocked = TFALSE; }

	// Places the stars and updates their state
	void UpdateReplays();

	// Called when the sim is entered / the barn is explored
	void OnEnterFarm();
	void OnEnterBarn();

	void OnDateChange( AGameTimeManager::DAYPHASE a_eDayPhase );

	void CreateStars( AReplayableGame::LOCATION a_eLocation );
	void DestroyStars();

	// Spins the stars
	void RotateStars( TFLOAT a_fDeltaTime );

	void Render( TBOOL a_bOnTop );

	AReplayableGame* GetFirstReplay() const { return m_pReplays; }
	AReplayableGame* FindReplay( Toshi::TPString8 a_strName );

	// a_bCheckStar: the star of the replay has to be visible
	TBOOL IsReplayUnlocked( Toshi::TPString8 a_strName, TBOOL a_bCheckStar );

	// Between "Daisy Flowers For Ben" and "Twink Round Up Chicks" the replays are blocked
	TBOOL IsInBlockedTier() const;

	// $Barnyard: FUNCTION 00547070
	void SetReplaysDisabled( TBOOL a_bDisabled ) { m_bReplaysDisabled = a_bDisabled; }

	// FSM natives
	void ReplayMiniGame( Toshi::TPString8 a_strName );

private:
	void CreateReplays();
	void FindBlockedTiers();

	// The stars become unavailable while a quest is assigned (or the replays are disabled)
	TBOOL DisableStars();

	void UpdateStar( AReplayableGame* a_pReplay );

	TBOOL IsInCoyoteTier() const;

	static void PlaceStar( AReplayableGame* a_pReplay );
	static void PlayStarEffect( AReplayableGame* a_pReplay );
	static void UpdateStarEffect( AReplayableGame* a_pReplay );

public:
	// DAT_00783dc8
	static AFunctionTable ms_oFunctionTable;

	// The ABarnExploreMinigameState on the stack (DAT_00783dc4)
	inline static AGameState* ms_pBarnExploreState;

	// Set by the honey collecting game, the honey of the hives is restored from the profile (DAT_007810ea)
	inline static TBOOL ms_bRestoreHoneyCollecting;

	// Never set in retail (DAT_00783e2c)
	inline static TBOOL ms_bRenderOnTop;

private:
	AReplayableGame*       m_pReplays;           // 0x004, linked list of the replays
	AGenericReplayGame     m_oChickenLaunch1;    // 0x008
	AGenericReplayGame     m_oChickenLaunch2;    // 0x060
	AGenericReplayGame     m_oChickenLaunch3;    // 0x0b8
	AGenericReplayGame     m_oBeadySquirt;       // 0x110
	AGenericReplayGame     m_oGolfSquirt;        // 0x168
	AGenericReplayGame     m_oBeadyCarJoyride;   // 0x1c0
	AGenericReplayGame     m_oFreddyDream;       // 0x218
	AGenericReplayGame     m_oIzzyPrecisionGolf; // 0x270
	AGenericReplayGame     m_oPrecisionBackNine; // 0x2c8
	AReplayVeggiePatch     m_oVeggiePatch;       // 0x320
	AReplayChickDefender   m_oChickCoop;         // 0x378
	AGenericReplayGame     m_oChaseChick;        // 0x3d0
	AGenericReplayGame     m_oChaseChickTwo;     // 0x428
	AReplayDarts           m_oDarts;             // 0x480
	AReplayBarnPool        m_oBarnPool;          // 0x4d8
	AReplayTeaseMailMan    m_oTeaseMailMan;      // 0x530
	AReplayTeaseBeady      m_oTeaseBeady;        // 0x588
	AReplayGolfBackNine    m_oGolfBackNine;      // 0x5e0
	AReplayGolfFrontNine   m_oGolfFrontNine;     // 0x638
	AGenericReplayGame     m_oMudJumpers;        // 0x690
	AReplayBarntender      m_oBarntender;        // 0x6e8
	AReplayCowTipping      m_oCowTipping;        // 0x740
	AReplayHoneyCollecting m_oHoneyCollecting;   // 0x798
	AReplayWhack           m_oWhack;             // 0x7fc
	AGenericReplayGame     m_oBarnyardSquirt;    // 0x854
	AGenericReplayGame     m_oBenshillSquirt;    // 0x8ac
	TBOOL                  m_bAllUnlocked;       // 0x904, only changed by the save
	TBOOL                  m_bReplaysDisabled;   // 0x905, set by the coin pickup microgame
	TINT                   m_iBlockStartTier;    // 0x908
	TINT                   m_iBlockStartAct;     // 0x90c
	TINT                   m_iBlockEndTier;      // 0x910
	TINT                   m_iBlockEndAct;       // 0x914
	TINT                   m_iCoyoteAct;         // 0x918
	TINT                   m_iCoyoteTier;        // 0x91c
	TBOOL                  m_bStarsCreated;      // 0x920
};
