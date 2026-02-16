#pragma once
#include "AGameState.h"
#include "AGameStateControllerEvent.h"
#include "GUI/T2GUIRectangle.h"

#include <Toshi/TTask.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AGameStateController
    : public Toshi::TTask
    , public Toshi::TSingleton<AGameStateController>
{
public:
	TDECLARE_CLASS( AGameStateController, Toshi::TTask );

	struct OverlayData
	{
		TUINT8 uiColorA;
		TUINT8 uiColorR;
		TUINT8 uiColorG;
		TUINT8 uiColorB;
	};

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                   = 0,
		FLAGS_IGNORE_INPUT           = BITFLAG( 0 ),
		FLAGS_AWAITING_TRANSITION    = BITFLAG( 1 ),
		FLAGS_FADING_IN              = BITFLAG( 2 ),
		FLAGS_UNK3                   = BITFLAG( 3 ),
		FLAGS_UNK4                   = BITFLAG( 4 ),
		FLAGS_UNK5                   = BITFLAG( 5 ),
		FLAGS_TRANSITION_TO_MINIGAME = BITFLAG( 6 ),
		FLAGS_UNK7                   = BITFLAG( 7 ),
		FLAGS_UNK8                   = BITFLAG( 8 ),
		FLAGS_UNK9                   = BITFLAG( 9 ),
		FLAGS_UNK10                  = BITFLAG( 10 ),
		FLAGS_UNK11                  = BITFLAG( 11 ),
		FLAGS_UNK12                  = BITFLAG( 12 ),
		FLAGS_MUTING                 = BITFLAG( 13 ),
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

	AGameState* GetCurrentState() { return *m_oStates.Back(); }
	TBOOL       IsCurrentState( Toshi::TClass* a_pClass ) { return GetCurrentState()->IsA( a_pClass ); }

	//-----------------------------------------------------------------------------
	// Controller
	//-----------------------------------------------------------------------------
	void SetFlags( TUINT16 a_eFlags );

	void         SetOverlayParams( AGameState::OVERLAY a_eOverlay, const OverlayData& a_rcParams );
	OverlayData* GetOverlayParams( AGameState::OVERLAY a_eOverlay );

	TBOOL UpdateTransition( TFLOAT a_flDeltaTime );
	void  UpdateScreenOverlay();

	//-----------------------------------------------------------------------------
	// IO
	//-----------------------------------------------------------------------------
	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

public:
	static void StartMiniGame( TINT a_iMiniGame, TBOOL a_bRightNow );

private:
	void InsertGameState( AGameState* a_pGameState );

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
		TUINT16 m_eFlagsRaw;

		struct
		{
			TUINT16 bIgnoreInput : 1;          // FLAGS_IGNORE_INPUT
			TUINT16 bAwaitingTransition : 1;   // FLAGS_AWAITING_TRANSITION
			TUINT16 bFadingIt : 1;             // FLAGS_FADING_IN
			TUINT16 UNK3 : 1;                  // FLAGS_UNK3
			TUINT16 UNK4 : 1;                  // FLAGS_UNK4
			TUINT16 UNK5 : 1;                  // FLAGS_UNK5
			TUINT16 bTransitionToMiniGame : 1; // FLAGS_TRANSITION_TO_MINIGAME
			TUINT16 UNK7 : 1;                  // FLAGS_UNK7
			TUINT16 UNK8 : 1;                  // FLAGS_UNK8
			TUINT16 UNK9 : 1;                  // FLAGS_UNK9
			TUINT16 UNK10 : 1;                 // FLAGS_UNK10
		} m_eFlags;
	};

	TFLOAT m_flOverlayAlpha;
	TFLOAT m_flSoundVolume;
	TFLOAT m_flOverlayGoal;
};
