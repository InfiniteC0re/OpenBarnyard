#pragma once
#include "AGameState.h"
#include "AGameStateControllerEvent.h"
#include "GUI/T2GUIRectangle.h"

#include <Toshi/TTask.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AFunctionTable;

class AGameStateController
    : public Toshi::TTask
    , public Toshi::TSingleton<AGameStateController>
{
public:
	TDECLARE_CLASS( AGameStateController, Toshi::TTask );

	// Script natives of the class, for the FSM
	// $Barnyard: FUNCTION 004292e0
	virtual AFunctionTable* GetFunctionTable() { return &ms_oFunctionTable; } // +0x38

	struct OverlayData
	{
		TUINT8 uiColorA;
		TUINT8 uiColorR;
		TUINT8 uiColorG;
		TUINT8 uiColorB;
	};

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_IGNORE_INPUT            = BITFLAG( 0 ),
		FLAGS_AWAITING_TRANSITION     = BITFLAG( 1 ),
		FLAGS_FADING_IN               = BITFLAG( 2 ),
		FLAGS_RETURN_TO_FRONTEND      = BITFLAG( 3 ),
		FLAGS_LEAVE_IMMEDIATELY       = BITFLAG( 4 ),
		FLAGS_START_NEW_GAME          = BITFLAG( 5 ),
		FLAGS_TRANSITION_TO_MINIGAME  = BITFLAG( 6 ),
		FLAGS_LEAVE_SCRIPTED_SEQUENCE = BITFLAG( 7 ),
		FLAGS_KEEP_OVERLAY_ALPHA      = BITFLAG( 8 ),
		FLAGS_LOAD_SAVED_GAME         = BITFLAG( 9 ),
		FLAGS_UNK10                   = BITFLAG( 10 ),
		FLAGS_REMOVE_STATE            = BITFLAG( 11 ),
		FLAGS_RESTART_MINIGAME        = BITFLAG( 12 ),
		FLAGS_FADE_SOUND              = BITFLAG( 13 ),
		// 		FLAGS_UNK14        = BITFLAG( 14 ),
		// 		FLAGS_UNK15        = BITFLAG( 15 ),
	};

public:
	AGameStateController();
	~AGameStateController();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	//-----------------------------------------------------------------------------
	// AGameState Management
	//-----------------------------------------------------------------------------
	void ReplaceState( AGameState* a_pGameState );
	void PushState( AGameState* a_pGameState );
	void PopState( AGameState* a_pGameState );

	void PopCurrentGameState();
	void ResetStack();

	// Sets the overlay of every state on the stack to AGameState::OVERLAY_1 (none)
	void ResetOverlays();

	// $Barnyard: FUNCTION 004293c0
	AGameState* GetCurrentState() { return *m_oStates.Back(); }
	TBOOL       IsCurrentState( Toshi::TClass* a_pClass ) { return GetCurrentState()->IsA( a_pClass ); }

	// Returns the first state on the stack whose class is called a_rcClassName, TNULL if there's none
	AGameState* FindGameStateOnStack( const Toshi::TPString8& a_rcClassName );

	// Creates the game state class called a_rcClassName and adds it as a child of the current state
	void AddChildStateByName( const Toshi::TPString8& a_rcClassName );

	//-----------------------------------------------------------------------------
	// Script natives "agamestatecontroller::"
	//-----------------------------------------------------------------------------

	// Loads the asset library and the game state class called a_strClassName (the gopher shop...)
	void StartGameState( Toshi::TPString8 a_strClassName );

	// Saves and goes back to the sim world
	void FinishGameState();

	// Creates the game state class called a_strClassName and pushes it
	void CreateAndPushNamedState( Toshi::TPString8 a_strClassName );

	// Creates the game state class called a_rcClassName and replaces the current state with it
	void CreateAndTransferToNamedState( const Toshi::TPString8& a_rcClassName );

	// Pops the states above the state called a_strClassName
	void ReturnToNamedState( Toshi::TPString8 a_strClassName );

	// Whether a state called a_rcClassName is on the stack
	TBOOL IsStateOnStack( const Toshi::TPString8& a_rcClassName );

	static AFunctionTable ms_oFunctionTable; // 0x0078178c

	// TRUE while the game is transitioning or fading (the items of the inventory can't be used)
	// NOTE: It's next to the FSM natives.
	static TBOOL IsBusy();

	//-----------------------------------------------------------------------------
	// Controller
	//-----------------------------------------------------------------------------
	void  SetFlags( FLAGS a_eFlags );
	void  SetIgnoreInput( TBOOL a_bIgnore ) { m_eFlags.bIgnoreInput = a_bIgnore; }
	void  SetAwaitTransition( TBOOL a_bAwait ) { m_eFlags.bAwaitingTransition = a_bAwait; }
	void  SetTransitionToMiniGame( TBOOL a_bTransition ) { m_eFlags.bTransitionToMiniGame = a_bTransition; }
	void  SetFadeSound( TBOOL a_bFadeSound ) { m_eFlags.bFadeSound = a_bFadeSound; }
	FLAGS GetFlags() const { return m_eFlagsRaw; }

	// Only sets the flags (SetFlags also resets the sound volume and the overlay)
	void AddFlags( FLAGS a_eFlags ) { m_eFlagsRaw |= a_eFlags; }

	// Overlay alpha SetFlags starts the transition with if FLAGS_KEEP_OVERLAY_ALPHA is set
	void SetOverlayGoal( TFLOAT a_fAlpha ) { m_flOverlayGoal = a_fAlpha; }

	// Pushes AStartMenuState unless a fade or a transition is in progress
	void StartMenuIfPossible();

	// Pushes the save state (or the sleep cycle chooser while quests are assigned)
	void SaveCurrentGameState( TBOOL a_bUnused, TBOOL a_bUnk, TBOOL a_bDontSave );

	// Shows (with a_fAlpha) or hides the black overlay of the current state (a_bCurrentState) or of the
	// conversation/sim/barn explore state on the stack. a_bResetHUD also hides the HUD of the current state.
#ifdef BARNYARD_LATEST
	// Wii: a_szStateClass picks the state on the stack if a_bCurrentState isn't set
	void ShowOverlay( TBOOL a_bShow, TBOOL a_bResetHUD, TFLOAT a_fAlpha, TBOOL a_bCurrentState, const TCHAR* a_szStateClass = TNULL );
#else
	void ShowOverlay( TBOOL a_bShow, TBOOL a_bResetHUD, TFLOAT a_fAlpha, TBOOL a_bCurrentState );
#endif

	// Clears the flags, resets the stack and inserts a new state of the class called a_rcClassName
	void RecreateMiniGameState( const Toshi::TPString8& a_rcClassName );

	void         SetOverlayParams( AGameState::OVERLAY a_eOverlay, const OverlayData& a_rcParams );
	OverlayData* GetOverlayParams( AGameState::OVERLAY a_eOverlay );

	// The screen overlay rectangle, other GUI elements can be attached to it
	Toshi::T2GUIRectangle& GetOverlayElement() { return m_oOverlay; }

	TBOOL UpdateTransition( TFLOAT a_flDeltaTime );

private:
	// Copies the alpha of the transition into the third overlay
	// NOTE: inlined in UpdateTransition, only while a transition or fade in is going (other fades,
	// e.g. ASSeqFade and AFadeActivity, use the same overlay)
	void UpdateOverlayAlpha()
	{
		OverlayData oData;
		oData.uiColorA = TUINT8( m_flOverlayAlpha * 255.0f );
		oData.uiColorR = 0;
		oData.uiColorG = 0;
		oData.uiColorB = 0;
		SetOverlayParams( AGameState::OVERLAY_3, oData );
	}

public:
	//-----------------------------------------------------------------------------
	// Render hooks (called by ARenderer::RenderMainScene for every state on the stack)
	//-----------------------------------------------------------------------------
	void RenderStates( TFLOAT a_fDeltaTime );
	void PostRenderStates( TFLOAT a_fDeltaTime );
	void RenderStatesOverlay( TFLOAT a_fDeltaTime );
	void UpdateScreenOverlay();

	//-----------------------------------------------------------------------------
	// IO
	//-----------------------------------------------------------------------------
	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

private:
	void InsertGameState( AGameState* a_pGameState );

public:
	// Set while a state that pauses the game is shown (instruction screen, start menu,
	// game over screen, save/load). SaveLoad saves and restores it.
	inline static TBOOL ms_bPausedByMenu; // 0x007810e5

private:
	inline static OverlayData ms_aOverlays[ AGameState::OVERLAY_NUMOF ];

private:
	// ...
	Toshi::T2Vector<AGameState*, 10> m_oStates;
	void*                            m_VectorData[ 5 ];
	Toshi::T2DynamicVector<void*>    m_UnkVector;
	// ...
	Toshi::T2GUIRectangle m_oOverlay;

	union
	{
		FLAGS m_eFlagsRaw;

		struct
		{
			TUINT16 bIgnoreInput : 1;           // FLAGS_IGNORE_INPUT
			TUINT16 bAwaitingTransition : 1;    // FLAGS_AWAITING_TRANSITION
			TUINT16 bFadingIn : 1;              // FLAGS_FADING_IN
			TUINT16 bReturnToFrontEnd : 1;      // FLAGS_RETURN_TO_FRONTEND
			TUINT16 bLeaveImmediately : 1;      // FLAGS_LEAVE_IMMEDIATELY
			TUINT16 bStartNewGame : 1;          // FLAGS_START_NEW_GAME
			TUINT16 bTransitionToMiniGame : 1;  // FLAGS_TRANSITION_TO_MINIGAME
			TUINT16 bLeaveScriptedSequence : 1; // FLAGS_LEAVE_SCRIPTED_SEQUENCE
			TUINT16 bKeepOverlayAlpha : 1;      // FLAGS_KEEP_OVERLAY_ALPHA
			TUINT16 bLoadSavedGame : 1;         // FLAGS_LOAD_SAVED_GAME
			TUINT16 UNK10 : 1;                  // FLAGS_UNK10
			TUINT16 bRemoveState : 1;           // FLAGS_REMOVE_STATE
			TUINT16 bRestartMiniGame : 1;       // FLAGS_RESTART_MINIGAME
			TUINT16 bFadeSound : 1;             // FLAGS_FADE_SOUND
		} m_eFlags;
	};

	TFLOAT m_flOverlayAlpha;
	TFLOAT m_flSoundVolume;
	TFLOAT m_flOverlayGoal;
};
