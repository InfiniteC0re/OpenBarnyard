#pragma once
#include "GameInterface/ASplitScreen.h"
#include "GameInterface/AGameState.h"
#include "Helpers/AInventoryAniEffect.h"
#include "HUD/AMiniGamePlayerFloat.h"
#include "HUD/AStartMiniGameHudElement.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "HUD/ATimeLeftHudElement.h"
#include "AMinigamePlayerAi.h"

#include <Toshi/TPString8.h>

class AMiniGameState
    : public AGameState
    , public AInventoryAniEffect::CallbackInterface
{
public:
	TDECLARE_CLASS( AMiniGameState, AGameState );

	struct PlayerInfo
	{
		// ...
	};

	struct PlayerList
	{
		PlayerInfo aInfos[ SPLITSCREEN_MAX_NUM_PLAYERS ];

		PlayerInfo& operator[](TINT a_iIndex)
		{
			TASSERT( a_iIndex < SPLITSCREEN_MAX_NUM_PLAYERS );
			return aInfos[ a_iIndex ];
		}
	};

	class S1
	{
		PlayerList       oPlayerList;
		TINT             iUnk = -1;
		TINT             iMiniGameIndex = -1;
		TINT             iMiniGameVariant = -1;
		Toshi::TPString8 strUnk;
		TBOOL            bFlag1 = TFALSE;
		TBOOL            bFlag2 = TFALSE;
		TBOOL            bFlag3 = TFALSE;
		TBOOL            bFlag4 = TTRUE;
		TBOOL            bFlag5 = TFALSE;
		TBOOL            bFlag6 = TFALSE;
	};

public:
	AMiniGameState();
	AMiniGameState( AMinigamePlayerAi* a_pPlayerAi );
	virtual ~AMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnStart() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AInventoryAniEffect::CallbackInterface
	//-----------------------------------------------------------------------------
	virtual void AInventoryAniEffect_CallbackInterface_Unknown() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void  Pause();
	virtual void  Unknown14();
	virtual void  Unknown15();
	virtual void  Unknown16();
	virtual void  Unknown17();
	virtual TBOOL Unknown18();
	virtual TBOOL Unknown19();
	virtual void  Unknown20( void* );
	virtual void  Unknown21( void* );
	virtual void  Unknown22( void* );
	virtual void  Unknown23( void* );
	virtual void  Unknown24() = 0; // UNK ARGS AND RET TYPE
	virtual void  OnDettachHUD();
	virtual void  OnAttachHUD();
	virtual void  OnSetupPlayers();

public:
	S1 m_oS1; // 0xA8
	// ...
	AMinigamePlayerAi* m_pPlayerAi;
	TINT               m_iMusicCue;
	// ...

	// Debug player numbers
	AMiniGamePlayerFloat m_aPlayerFloats[ SPLITSCREEN_MAX_NUM_PLAYERS ];
	TBOOL                m_bShowPlayerFloats;
	TBOOL                m_bIsActive;

	// HUD Elements
	AStartMiniGameHudElement   m_oStartHudElement;
	ASimPlayerPickUpHudElement m_oPickUpHudElement;
	ATimeLeftHudElement        m_oTimeLeftHudElement;

	// ...
};
