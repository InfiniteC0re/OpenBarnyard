#pragma once
#include "GameInterface/AGameState.h"
#include "Helpers/ASpline.h"
#include "Helpers/ATreasureManager.h"
#include "FSM/AFunctionTable.h"

#include <File/TTRB.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TRefCounted.h>
#include <Math/TMatrix44.h>

class AUnit;
class AUnitProxy;
class AModelInstance;
class AParticleEffectHandle;
class ACollisionObjectSimpleDummy;

//-----------------------------------------------------------------------------
// Purpose: the treasure hunt micro game. The player walks around with the
// metal detector (held at cb_rlocator) that beeps faster and sends out
// coloured pulses the closer its sensor is to a treasure of ATreasureManager
// (or, during the "Miles Raccoon Sabotage" quests, to the pipes of
// pipespline1.trb). The action buttons dig a hole: bucks, tin cans and gopher
// traps pop out of found treasures, the leaks of the sabotage quests are fixed.
// Pushed by its script function "PushState". APlayerSteer::UpdateSinking ends it.
// TClass 0078398c, vtable 00723400, size 0x7ac.
// NOTE: the name of the class is misspelled in retail ("Treasue"), it's kept
// since the class is found by name. Member names are invented.
//-----------------------------------------------------------------------------
class ATreasueHuntMicroGame
    : public AGameState
{
public:
	TDECLARE_CLASS( ATreasueHuntMicroGame, AGameState );

	static constexpr TINT MAX_NUM_HOLES  = 25;
	static constexpr TINT MAX_NUM_PULSES = 10;
	static constexpr TINT NUM_PIPES      = 2;

	// Distance (XZ) from the sensor the detector reacts to
	static constexpr TFLOAT DETECTOR_RANGE = 10.0f;

	// Distance (XZ) from the sensor a treasure can be dug up from
	static constexpr TFLOAT DIG_RADIUS = 2.8f;

	using LOADSTATE = TINT;
	enum LOADSTATE_ : LOADSTATE
	{
		LOADSTATE_START_LOADING, // shows the load screen
		LOADSTATE_LOADING,       // waits for the terrain behind the load screen
		LOADSTATE_RUNNING,
		LOADSTATE_START,  // OnInsertion: starts the game
		LOADSTATE_LOADED, // the terrain got loaded behind the load screen
	};

	// The sod that is thrown up where the player digs (smallround_sod)
	struct Hole
	{
		// Moves the sod up, then down and fades it out. Returns TFALSE once it's invisible.
		TBOOL Update( TFLOAT a_fDeltaTime );

		AModelInstance* pModel; // 0x0
		TFLOAT          fTime;  // 0x4
	};

	struct HolePool
	{
		void Create();
		void Destroy();
		void Update( TFLOAT a_fDeltaTime );

		// Places a sod on the ground under a_rcPosition (a_bBig: four times bigger) and plays the dust effect
		TBOOL Spawn( const Toshi::TVector4& a_rcPosition, TBOOL a_bBig );

		Toshi::T2Vector<Hole*, MAX_NUM_HOLES> vecActive;               // 0x000
		Hole                                  aHoles[ MAX_NUM_HOLES ]; // 0x068
		Toshi::T2Vector<Hole*, MAX_NUM_HOLES> vecFree;                 // 0x130
		TINT                                  iDustEffect;             // 0x198
	};

	// A growing ring on the ground around the sensor
	struct Pulse
	{
		AParticleEffectHandle* pEffect;     // 0x00
		Toshi::TMatrix44       mTransform;  // 0x04
		TFLOAT                 fTime;       // 0x44
		TFLOAT                 fScale;      // 0x48
		TFLOAT                 fScaleSpeed; // 0x4c
	};

	struct PulsePool
	{
		void Create();
		void Destroy();
		void Update( TFLOAT a_fDeltaTime );

		// Plays a pulse on the ground under a_rcPosition. Its colour tells how close the treasure (a_fDistance) is.
		void Spawn( const Toshi::TVector4& a_rcPosition, TINT a_iUnused, TFLOAT a_fDistance, TBOOL a_bLeak );

		Toshi::T2Vector<Pulse*, MAX_NUM_PULSES> vecActive;                 // 0x000
		Pulse                                   aPulses[ MAX_NUM_PULSES ]; // 0x02c
		Toshi::T2Vector<Pulse*, MAX_NUM_PULSES> vecFree;                   // 0x34c
		TINT                                    iGreenEffect;              // 0x378, far
		TINT                                    iBlueEffect;               // 0x37c, close to a leak
		TINT                                    iRedEffect;                // 0x380, close enough to dig
	};

public:
	ATreasueHuntMicroGame();
	~ATreasueHuntMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x68

	// Script function "PushState"
	static void PushState( TBOOL a_bUnused );

private:
	void Start();

	// The detector (the unit at the player's cb_rlocator)
	AUnit* GetDetector();

	// Position of the "sensor" bone of the detector
	Toshi::TVector4 GetSensorPosition();

	// Raises the detector when the sensor touches the ground
	void UpdateSensor( TFLOAT a_fDeltaTime );

	// Blends the uphill walk/idle animations of the detector animation set in by a_fWeight
	static void UpdateAnimations( TFLOAT a_fWeight, TFLOAT a_fDeltaTime );

	// Plays the beep, the rumble and the pulses
	void UpdateDetector( TFLOAT a_fDeltaTime, TFLOAT a_fProximity, TFLOAT a_fDistance, TBOOL a_bLeak );

	// Closest distance (XZ) from the sensor to the pipes or DETECTOR_RANGE if they are further than 3
	TFLOAT GetPipeDistance();

	// Fixes the leak the player stands at (m_pLeak). Returns TTRUE if the game is over.
	TBOOL FixLeak();

	// Digs at the sensor, returns TFALSE if the player can't dig now
	TBOOL Dig();

	void UpdateCamera( TFLOAT a_fDeltaTime );

	static TBOOL IsGopherTreasureQuestStarted();
	static TBOOL IsGopherTrapQuestStarted();

	// Spawns a pick up a_strType at a_rcPosition (a_rcDirection is unused) and rumbles
	static void SpawnTreasure( Toshi::TPString8 a_strType, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	static void FinishUphillAnimations();

private:
	static AFunctionTable ms_oFunctionTable; // 0x007839bc

private:
	TFLOAT                       m_fCameraYaw;          // 0xa4, applied to the GTA camera and cleared every frame
	TFLOAT                       m_fCameraPitch;        // 0xa8
	TUINT32                      m_aUnkAC[ 2 ];         // 0xac
	Toshi::TVector4              m_vecStartPosition;    // 0xb4
	Toshi::TVector4              m_vecStartDirection;   // 0xc4
	Toshi::TVector4              m_vecDigPosition;      // 0xd4, the player's position when digging
	Toshi::TPString8             m_strUnkE4;            // 0xe4
	Toshi::TRef<AUnitProxy>      m_oPlayer;             // 0xe8
	ACollisionObjectSimpleDummy* m_pSensorCollision;    // 0xec
	ATreasureManager::Treasure*  m_pLeak;               // 0xf0, the leak the player stands at
	TINT                         m_iBeepCue;            // 0xf4
	TFLOAT                       m_fMusicVolume;        // 0xf8, restored when the game ends
	TFLOAT                       m_fDigCooldown;        // 0xfc
	TFLOAT                       m_fStillTime;          // 0x100, the player stands still while digging
	TFLOAT                       m_fPulseTimer;         // 0x104
	TFLOAT                       m_fBeepTimer;          // 0x108
	TFLOAT                       m_fBeepFlash;          // 0x10c, 0.99 when it beeps, then fades
	HolePool                     m_oHoles;              // 0x110
	PulsePool                    m_oPulses;             // 0x2ac
	TBOOL                        m_bPulse;              // 0x630, a pulse is due
	ASpline                      m_aPipes[ NUM_PIPES ]; // 0x634
	TINT                         m_iRumbleTimer;        // 0x654, frames until the next rumble
	TBOOL                        m_bRumbling;           // 0x658
	Toshi::TTRB                  m_oTRB;                // 0x65c, unused
	LOADSTATE                    m_eLoadState;          // 0x79c
	TFLOAT                       m_fLoadTime;           // 0x7a0
	AGameState*                  m_pSimState;           // 0x7a4
	TUINT32                      m_uiSimHUDMask;        // 0x7a8, restored when the game ends
};
