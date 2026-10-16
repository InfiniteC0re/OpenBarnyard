#pragma once
#include "GameInterface/AGameState.h"
#include "GameInterface/AMessagePopupState.h"
#include "HUD/ATimeLeftHudElement.h"
#include "GUI/AGUI2FadingText.h"
#include "Helpers/ASpline.h"
#include "FSM/AFunctionTable.h"

#include <File/TTRB.h>
#include <Toshi/TEvent.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: the gold rush micro game. The sim spawns a coin spawner (a stump
// "coinspawnerstump" and a lever "coinspawnerlever") at every "stumpposNN"
// goal. Pulling a lever costs a "goldrush_token": the coins ("precisioncoin")
// pop out along one of the coin trails of Data\treasure\coinspline4.trb and
// the player has a time limit to collect them.
// The interaction of the lever sets the trail, the number of coins and the
// time limit with the script natives of the class before pushing the state.
// TClass 00783b28, vtable 00723968, size 0xb64.
//-----------------------------------------------------------------------------
class ACoinPickupMicroGame
    : public AGameState
{
public:
	TDECLARE_CLASS( ACoinPickupMicroGame, AGameState );

	static constexpr TINT MAX_NUM_COINS         = 200;
	static constexpr TINT NUM_SPLINES           = 8;
	static constexpr TINT MAX_NUM_COIN_SPAWNERS = 8;

	enum STATE : TINT
	{
		STATE_PULLING_LEVER, // waits for the "coinspawner" animation of the player to end
		STATE_START,         // shows the timer and spawns the coins
		STATE_COLLECTING,
		STATE_RESULT, // shows the number of collected coins until the text fades out
		STATE_EXIT,
		STATE_NO_TOKEN, // the player has no token, shows a message
	};

	enum LOADSTATE : TINT
	{
		LOADSTATE_LOADING, // the terrain is being loaded behind the load screen
		LOADSTATE_RUNNING,
		LOADSTATE_LOADED,              // the terrain was already loaded
		LOADSTATE_LOADED_LOADINGSCREEN // the terrain got loaded behind the load screen
	};

	using PopupListener = Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, ACoinPickupMicroGame>;

public:
	ACoinPickupMicroGame();
	~ACoinPickupMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// NOTE: folded with ASleepCycleState::ProcessInput (0043fc30), it only calls AGameState::ProcessInput
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE { return AGameState::ProcessInput( a_pInputEvent ); }   // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void  OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x18
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x68

	// Spawns the coin spawners at the "stumpposNN" goals (ASimState::OnInsertion)
	static void SpawnCoinSpawners();

	// Destroys the coin spawners (ASimState::OnRemoval)
	static void DestroyCoinSpawners();

	// Script natives
	static void SetTimeLimit( TFLOAT a_fTimeLimit );
	static void SetSplineID( TINT a_iSplineID );
	static void SetNumberOfCoins( TINT a_iNumCoins );

	TINT GetNumCoins() const { return m_iNumCoins; }
	TINT GetNumCoinsLeft() const { return m_iNumCoinsLeft; }
	TINT GetScore() const { return m_iScore; }

	AUnit* GetCoin( TINT a_iIndex ) const { return m_apCoins[ a_iIndex ]; }

	// The stump of the coin spawner a_iIndex (TNULL if there's none)
	static AUnit* GetStump( TINT a_iIndex ) { return ms_apStumps ? ms_apStumps[ a_iIndex ] : TNULL; }

	STATE GetState() const { return m_eState; }

private:
	// Loads the trails, takes the token and puts the player at the lever
	void Start();

	// Spawns the coins along the trail ms_iSplineID
	void SpawnCoins();

	// Shows a_wszText with the fading text for a_fTime seconds
	void ShowText( const TWCHAR* a_wszText, TFLOAT a_fTime );

	void SetCoinsVisible( TBOOL a_bVisible );

	// Shows a_wszMessage in a popup, the state ends once it's closed
	void ShowMessage( const TWCHAR* a_wszMessage );

	// Puts a_pPlayer at the lever of the closest coin spawner and plays the animations
	static void PlacePlayerAtLever( AUnit* a_pPlayer );

	static void FindClosestCoinSpawner( const Toshi::TVector4& a_rcPosition, AUnit*& a_rpStump, AUnit*& a_rpLever );

	// Puts a_rPosition down to the lowest ground around it within a_fRadius
	static void PutOnGround( Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rcDirection, TFLOAT a_fRadius );

	static TBOOL __stdcall OnMessageResponse( ACoinPickupMicroGame* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

private:
	static AFunctionTable ms_oFunctionTable;   // 0x00783af8
	static TFLOAT         ms_fTimeLimit;       // 0x00773f64, "SetTimeLimit"
	static TINT           ms_iSplineID;        // 0x00783aec, "SetSplineID"
	static TINT           ms_iNumCoins;        // 0x00773f68, "SetNumberOfCoins"
	static AUnit**        ms_apStumps;         // 0x00783af0, MAX_NUM_COIN_SPAWNERS
	static AUnit**        ms_apLevers;         // 0x00783af4, MAX_NUM_COIN_SPAWNERS
	static TINT           ms_iLeverPlayerBone; // 0x00773f6c, "player_loc" of the lever

private:
	TINT                m_iNumCoins;                // 0xa4
	AUnit*              m_apCoins[ MAX_NUM_COINS ]; // 0xa8
	TINT                m_iNumCoinsLeft;            // 0x3c8, coins that aren't collected yet
	TFLOAT              m_fTimeLeft;                // 0x3cc
	TINT                m_iScore;                   // 0x3d0, value of the collected coins
	ATimeLeftHudElement m_oTimeLeftHud;             // 0x3d4
	ASpline             m_aSplines[ NUM_SPLINES ];  // 0x608
	Toshi::TTRB         m_oSplineTRB;               // 0x688
	STATE               m_eState;                   // 0x7c8
	PopupListener       m_oPopupListener;           // 0x7cc
	TWCHAR              m_wszText[ 256 ];           // 0x7e0
	AGUI2FadingText     m_oText;                    // 0x9e0
	TBOOL               m_bUnkB54;                  // 0xb54, cleared on start, never read
	AUnit*              m_pPlayer;                  // 0xb58
	LOADSTATE           m_eLoadState;               // 0xb5c
	TFLOAT              m_fLoadTime;                // 0xb60
};

TSTATICASSERT( sizeof( ACoinPickupMicroGame ) == 0xb64 );
