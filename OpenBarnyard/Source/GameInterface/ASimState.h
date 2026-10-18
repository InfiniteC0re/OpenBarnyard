#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2DayOfWeek.h"
#include "HUD/ASimHudGroup.h"
#include "HUD/ASimMiniMap.h"
#include "HUD/AItemCountHudElement.h"
#include "Helpers/AMilkBottlePickups.h"
#include "GUI/AMilkReticleManager.h"
#include "Physics/ACollisionObjectMesh.h"
#include "Helpers/AWindmillHelper.h"
#include "Helpers/AGameTimeManager.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TRefCounted.h>
#include <Toshi/TEvent.h>

class AModelInstance;
class ALadderBeansGame;

//-----------------------------------------------------------------------------
// Purpose: the sim, the main game world (EnvMain) the player walks around in.
// Created and pushed by AGameLoader's LoadMainGameImpl (00425bc0).
// NOTE: classes of the members that aren't reconstructed yet are kept as
// placeholder byte arrays of their retail size.
//-----------------------------------------------------------------------------
class ASimState : public AGameState
{
public:
	TDECLARE_CLASS( ASimState, AGameState );

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_FARMER_SEQUENCE = BITFLAG( 0 ), // a sequence of the farmer was started, AFarmerTruck::SetSequenceMode shows the characters again on OnActivate
		FLAGS_FARMER_UNK1     = BITFLAG( 1 ), // passed to AFarmerTruck::SetSequenceMode on OnActivate, never set by ASimState
		FLAGS_FARMER_ARRIVED  = BITFLAG( 2 ), // set with FLAGS_FARMER_SEQUENCE when the farmer arrives at night
		FLAGS_CLOSE_BARN_GATE = BITFLAG( 5 ), // the barn gate has to be closed when the sim is activated again
		FLAGS_DAILY_EVENTS    = BITFLAG( 6 ), // the day or night has started, UpdateDailyEvents has to run
		FLAGS_FARMER_TRUCK    = BITFLAG( 7 ), // the day or night has started, UpdateFarmerTruck has to run
		FLAGS_RESTART_MUSIC   = BITFLAG( 8 ), // the music has to be started again once the sim is activated
		FLAGS_SHOW_WEEKDAY    = BITFLAG( 9 ), // a new day has started, the weekday banner has to be shown
	};

public:
	ASimState();
	~ASimState();

	// The music has to be started again once the sim is the current state (after a farmer sequence)
	TBOOL IsRestartingMusic() const { return HASANYFLAG( m_eFlags, FLAGS_RESTART_MUSIC ); }

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// RETURNS_FALSE (006ce000)
	virtual TBOOL    ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE { return TFALSE; }                                      // +0x0c
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void     OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x18
	virtual void     OnLoaded() OVERRIDE;                                                                                                                     // +0x24
	virtual ASoundId GetSound() OVERRIDE;                                                                                                                     // +0x2c
	virtual void     OnCollisionModelLoaded( void* a_pCollisionModel, void* a_pGroupId ) OVERRIDE;                                                            // +0x40
	virtual TBOOL    OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void     OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void     OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void     OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void     OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Updates the parts of the sim the subclasses (ABarnExploreMinigameState) share: the HUD, the milk and the player's launcher
	virtual void UpdateSim( TFLOAT a_fDeltaTime ); // +0x68

	// $Barnyard: FUNCTION 0043ce90
	ASimHudGroup* GetSimHudGroup() { return &m_oSimHudGroup; }
	ASimMiniMap&  GetMiniMap() { return m_oMiniMap; }

	FLAGS GetFlags() const { return m_eFlags; }

	// NOTE: inlined, ACoinPickupMicroGame sets it while the sim runs under it
	void SetActive( TBOOL a_bActive ) { m_bIsActive = a_bActive; }

	AMilkReticleManager& GetMilkReticles() { return m_oMilkReticles; }
	ALadderBeansGame*    GetLadderBeansGame() const { return m_pLadderBeansGame; }

#ifdef BARNYARD_LATEST
	// Wii: the farmer comes home to Daisy's birthday party (SSBFF), started by AQuestManager::SetQuestTier
	void StartBirthdayCakeSequence();
	void StartTomatoLauncherSequence();
#endif

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// Development: the game time runs a_fScale times faster while the sim is the current state ("-devtimescale <N>",
	// the debug menu)
	static void   Dev_SetTimeScale( TFLOAT a_fScale );
	static TFLOAT Dev_GetTimeScale();
#endif

private:
	void  RefreshPlayerProxy();
	void  UpdateFarmerTruck();
	void  UpdateWeekdayBanner();
	void  UpdateDailyEvents();
	void  OnNewDay();
	TBOOL OnDateChange( AGameTimeManager::DateChangeEvent* a_pEvent );

	static TBOOL __stdcall OnDateChangeCallback( ASimState* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent );

protected:
	Toshi::TRef<AUnitProxy>                                                          m_pPlayerProxy;             // 0xa4
	ASimHudGroup                                                                     m_oSimHudGroup;             // 0xa8
	ASimMiniMap                                                                      m_oMiniMap;                 // 0xbc
	AGUI2DayOfWeek                                                                   m_oDayOfWeek;               // 0xa064
	AItemCountHudElement                                                             m_oItemCountHud;            // 0xa138
	TBOOL                                                                            m_bPendingAutoSave;         // 0xa328
	AMilkReticleManager                                                              m_oMilkReticles;            // 0xa32c, target reticles of the milk squirt
	TFLOAT                                                                           m_fUnkFD50;                 // 0xfd50
	Toshi::TListener<AGameTimeManager, AGameTimeManager::DateChangeEvent, ASimState> m_oDateChangeListener;      // 0xfd54
	TBOOL                                                                            m_bUnkFD68;                 // 0xfd68
	AModelInstance*                                                                  m_pGateModel;               // 0xfd6c, "data/models/fncveggateopen"
	ACollisionObjectMesh                                                             m_oGateCollision;           // 0xfd70
	AModelInstance*                                                                  m_pScarecrowDayModel;       // 0xfdd8
	AModelInstance*                                                                  m_pScarecrowNightModel;     // 0xfddc
	ACollisionObjectMesh                                                             m_oScarecrowDayCollision;   // 0xfde0
	ACollisionObjectMesh                                                             m_oScarecrowNightCollision; // 0xfe48
	Toshi::TRef<AUnitProxy>                                                          m_pUnitToDestroy;           // 0xfeb0, destroyed by OnRemoval
	AWindmillHelper                                                                  m_oWindmillHelper;          // 0xfeb4
	ALadderBeansGame*                                                                m_pLadderBeansGame;         // 0xff4c
	AMilkBottlePickups                                                               m_oMilkBottles;             // 0xff50
	TFLOAT                                                                           m_fSavedNear;               // 0xfffc
	TFLOAT                                                                           m_fSavedFar;                // 0x10000
	TINT                                                                             m_iCachedMinute;            // 0x10004
	FLAGS                                                                            m_eFlags;                   // 0x10008
	TBOOL                                                                            m_bIsActive;                // 0x1000c
	TBOOL                                                                            m_bSpawnedPip;              // 0x1000d
	AUnit*                                                                           m_pPip;                     // 0x10010
	AUnit*                                                                           m_pNbBull;                  // 0x10014
	Toshi::TPString8                                                                 m_strBarnExitTrigger;       // 0x10018
	TINT                                                                             m_iNightAmbienceCue;        // 0x1001c
};

TSTATICASSERT( sizeof( ASimState ) == 0x10020 );
