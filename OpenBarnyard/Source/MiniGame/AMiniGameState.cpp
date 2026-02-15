#include "pch.h"
#include "AMiniGameState.h"
#include "Input/AInputHandler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AMiniGameState );

// $Barnyard: FUNCTION 0046bed0
AMiniGameState::AMiniGameState()
    : m_iMusicCue( -1 )
    , m_bShowPlayerFloats( TFALSE )
    , m_bIsActive( TFALSE )
{
    // TODO: initialize other values
	m_pPlayerAi = new AMinigamePlayerAi();
}

AMiniGameState::AMiniGameState( AMinigamePlayerAi* a_pPlayerAi )
    : m_iMusicCue( -1 )
    , m_bShowPlayerFloats( TFALSE )
    , m_bIsActive( TFALSE )
{
	TVALIDPTR( a_pPlayerAi );

    // TODO: initialize other values
	m_pPlayerAi = a_pPlayerAi;
}

// $Barnyard: FUNCTION 0046c430
// $Barnyard: FUNCTION 0046c120
AMiniGameState::~AMiniGameState()
{
	if ( m_pPlayerAi ) delete m_pPlayerAi;
}

// $Barnyard: FUNCTION 0046b3f0
TBOOL AMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_Quit || a_eInputCommand == AInputCommand_Start )
	{
		if ( AInputHandler::GetSingleton()->IsValidDevice( a_pInputEvent->GetSource() ) )
		{
			TIMPLEMENT_D( "AGameStateController::StartMenuIfPossible" );
//			if ( FindChildState( &TGetClass( AAnticGameOverState ) )
// 			{
// 				AGameStateController::StartMenuIfPossible( AGameStateController::ms_pInstance );
// 			}
		}

		return TTRUE;
	}
	
	return TFALSE;
}

// $Barnyard: FUNCTION 0046bb60
void AMiniGameState::OnStart()
{
	TIMPLEMENT();
	m_bIsActive = TTRUE;
	Unknown16();
	Pause();

	// TODO: Setup music and various HUD elements
}

void AMiniGameState::AInventoryAniEffect_CallbackInterface_Unknown()
{
}

// $Barnyard: FUNCTION 0046bb60
void AMiniGameState::Pause()
{
	m_bIsActive = TFALSE;
	OnPause();
}

void AMiniGameState::Unknown14()
{
}

void AMiniGameState::Unknown15()
{
}

void AMiniGameState::Unknown16()
{
}

void AMiniGameState::Unknown17()
{
}

TBOOL AMiniGameState::Unknown18()
{
	return TTRUE;
}

TBOOL AMiniGameState::Unknown19()
{
	return TFALSE;
}

void AMiniGameState::Unknown20( void* )
{
}

void AMiniGameState::Unknown21( void* )
{
}

void AMiniGameState::Unknown22( void* )
{
}

void AMiniGameState::Unknown23( void* )
{
}

void AMiniGameState::OnDettachHUD()
{
	TIMPLEMENT();
}

void AMiniGameState::OnAttachHUD()
{
	TIMPLEMENT();
}

void AMiniGameState::OnSetupPlayers()
{
	TASSERT( TFALSE );
}
