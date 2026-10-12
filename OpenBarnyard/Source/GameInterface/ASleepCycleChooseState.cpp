#include "pch.h"
#include "ASleepCycleChooseState.h"
#include "ASleepCycleState.h"
#include "AGameStateController.h"
#include "Locale/ALocaleManager.h"
#include "Quests/AQuestManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 00440490, CreateObjectInPlace 004404f0
TDEFINE_CLASS( ASleepCycleChooseState );

// Locale strings
static constexpr TINT LOCALE_CANNOT_SAVE_TITLE = 1111;
static constexpr TINT LOCALE_CANNOT_SAVE       = 3357;
static constexpr TINT LOCALE_SLEEP_TITLE       = 1064;
static constexpr TINT LOCALE_SLEEP_QUESTS      = 185; // quests are assigned, sleep anyway?

// $Barnyard: FUNCTION 00440360
ASleepCycleChooseState::ASleepCycleChooseState()
    : m_bStarted( TFALSE )
    , m_bCannotSave( TFALSE )
{
}

// $Barnyard: FUNCTION 0043fbd0
// $Barnyard: FUNCTION 004403c0
ASleepCycleChooseState::~ASleepCycleChooseState()
{
}

// $Barnyard: FUNCTION 0043fc50
TBOOL ASleepCycleChooseState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_Quit )
		return TTRUE;

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0043fc80
void ASleepCycleChooseState::OnInsertion()
{
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00440ba0
TBOOL ASleepCycleChooseState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bStarted )
		return TTRUE;

	AGameStateController* pController = AGameStateController::GetSingleton();
	AMessagePopupState*   pPopup;

	if ( m_bCannotSave )
	{
		pPopup = new AMessagePopupState(
		    g_pLocaleManager->GetString( LOCALE_CANNOT_SAVE_TITLE ),
		    g_pLocaleManager->GetString( LOCALE_CANNOT_SAVE ),
		    TPString8(),
		    1,
		    TFALSE,
		    AInputCommand_OK,
		    AInputCommand_Empty,
		    AInputCommand_Empty,
		    AInputCommand_Empty,
		    TFALSE,
		    60.0f,
		    0
		);
	}
	else if ( AQuestManager::GetSingleton()->GetNumAssignedQuests() > 0 )
	{
		pPopup = new AMessagePopupState(
		    g_pLocaleManager->GetString( LOCALE_SLEEP_TITLE ),
		    g_pLocaleManager->GetString( LOCALE_SLEEP_QUESTS ),
		    TPString8(),
		    1,
		    TFALSE,
		    AInputCommand_OK,
		    AInputCommand_Cancel,
		    AInputCommand_Empty,
		    AInputCommand_Empty,
		    TFALSE,
		    60.0f,
		    0
		);
	}
	else
	{
		// Nothing to ask, go to sleep
		m_bStarted = TTRUE;
		Remove();
		AGameStateController::GetSingleton()->PushState( new ASleepCycleState() );
		return TTRUE;
	}

	m_oPopupListener.Connect( pPopup->GetResponseEmitter(), this, OnPopupResponse, 0 );
	pController->PushState( pPopup );
	m_bStarted = TTRUE;

	return TTRUE;
}

// $Barnyard: FUNCTION 004403e0
TBOOL __stdcall ASleepCycleChooseState::OnPopupResponse( ASleepCycleChooseState* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	if ( a_pPopup )
		a_pPopup->Remove();

	AGameStateController* pController = AGameStateController::GetSingleton();
	TBOOL                 bCannotSave = a_pState->m_bCannotSave;

	// NOTE: this destroys a_pState
	pController->PopState( a_pState );

	if ( !bCannotSave && a_pEvent->eButton == AInputCommand_OK )
	{
		AQuestManager::GetSingleton()->SetBannerState( AQuestManager::BANNERSTATE_NONE );
		pController->PushState( new ASleepCycleState() );
	}

	return TFALSE;
}
