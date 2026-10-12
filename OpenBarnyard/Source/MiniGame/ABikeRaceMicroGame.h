#pragma once
#include "AMiniGameState.h"
#include "HUD/ARacePlacingHudElement.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "HUD/ASimMiniMap.h"
#include "GUI/AGUI2AnythingPopup.h"
#include "GUI/AGUI2TextBox.h"
#include "Player/APlayerSet.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class AModelInstance;
class AInteriorNetwork;
class ACollisionObjectMesh;
class AUnit;
class AFade;
class ABikeSteer;
struct ATerrainLocator;

//-----------------------------------------------------------------------------
// Purpose: The Bike Race micro-game ("Bike_Race", 1 player): the player rides
// a bike against computer controlled rivals on a lap course in the main world.
// TClass 007838fc, vtable 00723028, size 0xe450.
//
// - The course (variant 0..5 = race1..race6, see ms_aTracks) is the interior
//   network "bikerace<n>" (start goals "player<n>start", waypoints "wp_000"...
//   when the terrain has no "wp_" locators) and the model "race<n>_collision".
// - Every 10 waypoints form a segment, a lap counts once the last segment is
//   reached and the "bikefinishline" locator is crossed.
// - Rivals are AI driven bikes seeking the next waypoint, with rubber banding
//   (they get teleported out of sight when far behind, or slowed down when far
//   ahead), getting unstuck and jumping on the "bikejump"/"emojump" locators.
// - Barnbucks ("coin" locators) and chilliberries ("coin_04", restore the
//   stamina, respawn after 16 seconds) can be picked up on the way.
// - The race starts with the SSBR_Main sequence, the result is the place of the
//   player (won if no rival finished first).
//-----------------------------------------------------------------------------
class ABikeRaceMicroGame
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( ABikeRaceMicroGame, AMiniGameState );

	static constexpr TINT MAX_RIVALS      = 3;
	static constexpr TINT MAX_BIKERS      = MAX_RIVALS + 1;
	static constexpr TINT MAX_WAYPOINTS   = 256;
	static constexpr TINT MAX_COINS       = 240;
	static constexpr TINT MAX_JUMPS       = 32;
	static constexpr TINT MAX_SPECTATORS  = 50;
	static constexpr TINT MAX_BERRIES     = 16;
	static constexpr TINT SEGMENT_SIZE    = 10; // waypoints per segment
	static constexpr TINT NUM_TRACKS      = 6;
	static constexpr TINT HIGHSCORE_TABLE = 70; // + variant

	// Locale strings
	static constexpr TINT LOCALE_LAP       = 526; // "Lap %d/%d"
	static constexpr TINT LOCALE_WRONG_WAY = 1044;

	// Setup of a race (0x00722eb0, 0x28 bytes each)
	struct TrackInfo
	{
		const TCHAR* szName;                     // 0x00, "race1", models are "<name>_collision", skeleton "Data\Terrain\<name>\<name>_skel.trb"
		const TCHAR* szNetwork;                  // 0x04, "bikerace1", the interior network with the start goals
		TINT         iNumLaps;                   // 0x08
		TINT         iNumRivals;                 // 0x0c
		TINT         aiRivalSkins[ MAX_RIVALS ]; // 0x10, passed to AUnitTransform
		TINT         aiRivalBikes[ MAX_RIVALS ]; // 0x1c, ABikeHelpers::AttachBikeModel
	};

	// A bike in the race, 0 is the player
	struct Biker
	{
		APlayerSlot* pSlot;     // 0x00
		TINT         iSegment;  // 0x04, groups of SEGMENT_SIZE waypoints reached in this lap
		TINT         iWaypoint; // 0x08, next waypoint
		TINT         iLap;      // 0x0c
	};

	enum RIVALSTATE : TINT
	{
		RIVALSTATE_RACING,     // 0
		RIVALSTATE_STUCK,      // 1, stopped, waits before reversing
		RIVALSTATE_BRAKING,    // 2, a sharp turn is ahead
		RIVALSTATE_SLOWED,     // 3, too far ahead of the others
		RIVALSTATE_REVERSING,  // 4
		RIVALSTATE_RECOVERING, // 5, seeks back to the track after reversing
		RIVALSTATE_UNK6,       // 6, never set
		RIVALSTATE_FINISHED,   // 7
	};

	// AI of a rival (index 0 is the biker 1)
	struct Rival
	{
		RIVALSTATE eState;      // 0x00
		TFLOAT     fTimer;      // 0x04
		TFLOAT     fTauntTimer; // 0x08, delay of the taunt after overtaking the player
		TBOOL      bAhead;      // 0x0c, ahead of the player in the last update
	};

	struct Coin
	{
		AAnimatableObject* pObject; // 0x00
		TINT               iState;  // 0x04, COINSTATE
	};

	enum COINSTATE : TINT
	{
		COINSTATE_NOT_GROUNDED, // 0, put on the ground once close
		COINSTATE_GROUNDED,     // 1
		COINSTATE_AIR,          // 2, "aircoin", floats where it was placed
		COINSTATE_COLLECTED,    // 3
	};

	struct Berry
	{
		AAnimatableObject* pObject;   // 0x00
		Toshi::TVector4    vPosition; // 0x04
		TFLOAT             fTimer;    // 0x14, -1 shown, > -1 growing, -2 collected, > 0 waiting to respawn
	};

public:
	ABikeRaceMicroGame();
	~ABikeRaceMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnPause() OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void Pause() OVERRIDE;            // +0x68, plays the intro sequence
	virtual void PlayLoseSequence() OVERRIDE; // +0x6c, plays the defeat sequence
	virtual void PlayWinSequence() OVERRIDE;  // +0x70, plays the victory sequence
	virtual void OnPreStart() OVERRIDE;       // +0x74
	// $Barnyard: FUNCTION 00527370
	virtual TBOOL IsGameOver() OVERRIDE { return m_bGameOver; }   // +0x80
	virtual void  UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE;   // +0x84
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;     // +0x88
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x8c
	virtual void  UpdatePreStart( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x90
	virtual S1*   GetResults() OVERRIDE;                          // +0x94
	virtual void  OnStartHudFinished() OVERRIDE;                  // +0x9c

	// Number of rivals (used by ABikePhysicsCameraHelper)
	// $Barnyard: FUNCTION 00527380
	TINT GetNumRivals() const { return m_pTrack->iNumRivals; }

	// Unit of the rival a_iRival (0 is the first rival)
	AUnit* GetRivalUnit( TINT a_iRival );

private:
	AUnit*      GetBikerUnit( TINT a_iBiker ) const;
	ABikeSteer* GetBikerSteer( TINT a_iBiker ) const;

	// Shows or hides all bikers
	void SetBikersVisible( TBOOL a_bVisible );

	//-----------------------------------------------------------------------------
	// Track (ABikeRaceMicroGame_Track.cpp)
	//-----------------------------------------------------------------------------

	// Loads the skeleton of the course, the gate model, the waypoints, places the bikers and creates the pick ups
	void SetupTrack();

	// Reads the waypoints and finds where the course crosses itself
	void LoadWaypoints();

	// TRUE if the biker a_iBiker is ahead of the biker a_iOther
	TBOOL IsAhead( TINT a_iBiker, TINT a_iOther );

	// TRUE if a_rcPosition is just across the finish line
	static TBOOL HasCrossedFinishLine( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW );

	// Counts waypoints, segments and laps of all the bikers
	void UpdateProgress();

	// Point the rival a_iBiker seeks to
	Toshi::TVector4 GetSeekTarget( TINT a_iBiker );

	// Index of the waypoint closest to a_rcPosition
	TINT FindClosestWaypoint( const Toshi::TVector4& a_rcPosition );

	// Position and direction to put a bike back on the course at the waypoint a_iWaypoint
	// (-1: the one before the biker of a_pUnit, or the closest one if it's not on the terrain)
	TBOOL GetRespawnPoint( AUnit* a_pUnit, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection, TINT a_iWaypoint );

	// Placing, lap text and the wrong way popup
	void UpdateHUD( TFLOAT a_fDeltaTime );

	//-----------------------------------------------------------------------------
	// AI and riding (ABikeRaceMicroGame_AI.cpp)
	//-----------------------------------------------------------------------------

	// Rubber banding of the rival a_iBiker relative to the biker a_iOther (a_iOffset waypoints apart)
	void UpdateRubberBand( TINT a_iBiker, TINT a_iOther, TINT a_iOffset );

	// The player has been thrown off the bike: fades out and puts the player back on the bike
	void UpdatePlayerFall( TFLOAT a_fDeltaTime );

	// Puts the bike of a_pUnit back on the course after it sank. a_bPlayer fades the screen.
	void UpdateSunkBike( AUnit* a_pUnit, TBOOL a_bPlayer );
	void UpdateSunkBikes();

	void UpdateRivals( TFLOAT a_fDeltaTime );
	void UpdatePickups( TFLOAT a_fDeltaTime );
	void UpdateSpectators();

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" rides the bike of the player along the waypoints
	void Dev_UpdateAutoPlay();
#endif

public:
	static const TrackInfo ms_aTracks[ NUM_TRACKS ];

private:
	Toshi::TVector4          m_aWaypoints[ MAX_WAYPOINTS ];    // 0x0bac, the y of the waypoints is 0
	Toshi::TVector4          m_aWaypointDirs[ MAX_WAYPOINTS ]; // 0x1bac, waypoint - previous waypoint
	TINT                     m_aiCrossings[ MAX_WAYPOINTS ];   // 0x2bac, segment crossed by the segment ending at the waypoint, -1 if none
	Coin                     m_aCoins[ MAX_COINS ];            // 0x2fac
	ATerrainLocator*         m_apJumps[ MAX_JUMPS ];           // 0x372c, "bikejump1" and "emojump_01" locators
	AAnimatableObject*       m_apSpectators[ MAX_SPECTATORS ]; // 0x37ac, never created by the retail game
	Berry                    m_aBerries[ MAX_BERRIES ];        // 0x3874
	Biker                    m_aBikers[ MAX_BIKERS ];          // 0x39f4
	Rival                    m_aRivals[ MAX_RIVALS ];          // 0x3a34
	ARacePlacingHudElement   m_oPlacingHud;                    // 0x3a64
	ASimPlayerMainHudElement m_oMainHud;                       // 0x3be4
	Toshi::T2GUIRectangle    m_oPortrait;                      // 0x4208
	ASimMiniMap              m_oMiniMap;                       // 0x4280
	AGUI2AnythingPopup       m_oWrongWayPopup;                 // 0xe228
	AGUI2TextBox             m_oLapText;                       // 0xe2e8
	TWCHAR                   m_wszLapText[ 16 ];               // 0xe360
	ACollisionObjectMesh*    m_pTrackCollision;                // 0xe380
	AModelInstance*          m_pTrackModel;                    // 0xe384, "race<n>_collision"
	AModelInstance*          m_pBarriersModel;                 // 0xe388, "overpassbarriers" (race5 and race6)
	ACollisionObjectMesh*    m_pGateCollision;                 // 0xe38c
	AModelInstance*          m_pGateModel;                     // 0xe390, "fncveggate"
	AInteriorNetwork*        m_pNetwork;                       // 0xe394
	const TrackInfo*         m_pTrack;                         // 0xe398
	AFade*                   m_pFade;                          // 0xe39c
	const TCHAR*             m_szOldSkeleton;                  // 0xe3a0, skeleton of the terrain before the race
	TCHAR                    m_szSkeleton[ 64 ];               // 0xe3a4
	TFLOAT                   m_fWrongWayTime;                  // 0xe3e4
	TFLOAT                   m_fGameOverTime;                  // 0xe3e8, also the time since the player fell off
	TFLOAT                   m_fPickupTime;                    // 0xe3ec, spins the pick ups
	TINT                     m_iNumWaypoints;                  // 0xe3f0
	TINT                     m_iNumSegments;                   // 0xe3f4
	TINT                     m_iNumCoins;                      // 0xe3f8
	TINT                     m_iNumJumps;                      // 0xe3fc
	TINT                     m_iNumSpectators;                 // 0xe400
	TINT                     m_iNumBerries;                    // 0xe404
	TINT                     m_iTauntCue;                      // 0xe408
	TBOOL                    m_bGameOver;                      // 0xe40c
	TBOOL                    m_bWon;                           // 0xe40d
	TBOOL                    m_bFallStarted;                   // 0xe40e
	Toshi::TMatrix44         m_matFinishLine;                  // 0xe410
};

TSTATICASSERT( sizeof( ABikeRaceMicroGame::TrackInfo ) == 0x28 );
TSTATICASSERT( sizeof( ABikeRaceMicroGame::Berry ) == 0x18 );
TSTATICASSERT( sizeof( ABikeRaceMicroGame ) == 0xe450 );
