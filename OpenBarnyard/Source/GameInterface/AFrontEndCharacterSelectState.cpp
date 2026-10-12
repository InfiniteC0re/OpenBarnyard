#include "pch.h"
#include "AFrontEndCharacterSelectState.h"
#include "AFrontEndMiniGameState2.h"
#include "AFrontEndDebugSSeqState.h"
#include "AGameStateController.h"
#include "Helpers/AGameLoader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AFrontEndCharacterSelectState );

// $Barnyard: FUNCTION 00407c50
AFrontEndCharacterSelectState::AFrontEndCharacterSelectState( TBOOL a_bAntics, TBOOL a_bStartMainGame, TBOOL a_bDebugSSeq )
    : m_oUI( a_bAntics, TTRUE )
{
	m_bStartMainGame = a_bStartMainGame;
	m_bAntics        = a_bAntics;
	m_bDebugSSeq     = a_bDebugSSeq;
}

// $Barnyard: FUNCTION 00407cd0
// $Barnyard: FUNCTION 00407db0
AFrontEndCharacterSelectState::~AFrontEndCharacterSelectState()
{
}

// $Barnyard: FUNCTION 00407da0
TBOOL AFrontEndCharacterSelectState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	return m_oUI.ProcessInputEvent( a_pInputEvent );
}

// $Barnyard: FUNCTION 00407d50
TBOOL AFrontEndCharacterSelectState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_oUI.IsActive() == TTRUE )
		return m_oUI.ProcessInputCommand( a_eInputCommand, a_pInputEvent );

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00407dd0
TBOOL AFrontEndCharacterSelectState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_oUI.IsActive() )
	{
		m_oUI.Update( a_fDeltaTime );
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	if ( m_oUI.GetPage() == ASSeqUI::PAGE_GENDER )
	{
		// Cancelled
		Remove();
		return TTRUE;
	}

	// NOTE: the original also clears a global here (DAT_007810e9) that is never read

	if ( m_bStartMainGame == TTRUE )
	{
		AGameLoader::SetTransitionToMainGame( TTRUE );
		return TTRUE;
	}

	AGameState* pNextState;

	if ( m_bDebugSSeq )
	{
		// NOTE: nothing creates this state with a_bDebugSSeq in retail
		pNextState = new AFrontEndDebugSSeqState();
	}
	else
	{
		pNextState = new AFrontEndMiniGameState2( m_bAntics );
	}

	AGameStateController::GetSingleton()->PushState( pNextState );
	return TTRUE;
}

// $Barnyard: FUNCTION 00407d30
void AFrontEndCharacterSelectState::OnInsertion()
{
	AGameState::OnInsertion();
	m_HUDParams.SetFlags( AHudElement::MASK_MENU_NAV );
}

// $Barnyard: FUNCTION 00407d80
void AFrontEndCharacterSelectState::OnActivate()
{
	m_oUI.Show( TNULL );
	AGameState::OnActivate();
}
