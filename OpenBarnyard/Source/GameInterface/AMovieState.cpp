#include "pch.h"
#include "AMovieState.h"
#include "AGameStateController.h"
#include "GUI/AFadeManager.h"
#include "Movie/AMoviePlayer.h"
#include "Locale/ALocaleManager.h"

#include <Input/TInputDeviceMouse.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME(AMovieState);

AMovieState::AMovieState(const TCHAR* a_szName, TBOOL a_bFlag1, AGameState* a_pNextGameState, TBOOL a_bFlag2)
{
	m_pNextGameState = a_pNextGameState;
	m_bFlag1 = a_bFlag1;
	m_bFlag2 = a_bFlag2;
	m_HUDParams.SetFlags(0);
	m_FileName = a_szName;
}

TBOOL AMovieState::ProcessInput(const TInputInterface::InputEvent* a_pInputEvent)
{
	if (AFadeManager::GetSingleton()->HasAnyFadesInProgress(TTRUE) || m_FadeOverListener.IsLinked())
	{
		return TFALSE;
	}

	if (a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN &&
		a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1)
	{
		SkipMovie(TFALSE);
		return TTRUE;
	}

	return AGameState::ProcessInput(a_pInputEvent);
}

TBOOL AMovieState::ProcessCommand(AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents)
{
	if (AFadeManager::GetSingleton()->HasAnyFadesInProgress(TTRUE) || m_FadeOverListener.IsLinked())
	{
		return TFALSE;
	}

	if (a_eInputCommand == AInputCommand_OK ||
		a_eInputCommand == AInputCommand_Cancel ||
		a_eInputCommand == AInputCommand_Start)
	{
		SkipMovie(TFALSE);
		return TTRUE;
	}

	return TFALSE;
}

TBOOL AMovieState::OnUpdate(TFLOAT a_fDeltaTime)
{
	if (AMoviePlayer::GetSingleton()->IsMoviePlaying())
	{
		return AGameState::OnUpdate(a_fDeltaTime);
	}
	else
	{
		return EndMovieImmediately();
	}
}

TPSTRING8_DECLARE(DialogInputMap);

void AMovieState::OnInsertion()
{
	SetInputMap(TPS8(DialogInputMap));
	AGameState::OnInsertion();

	if (m_FileName == TPString8("Attract"))
	{
		switch (ALocaleManager::Interface()->GetLangId())
		{
		case ALocaleManager::Lang_German:
			m_FileName = "AttractD";
			break;
		case ALocaleManager::Lang_Italian:
			m_FileName = "AttractI";
			break;
		case ALocaleManager::Lang_Spanish:
			m_FileName = "AttractE";
			break;
		case ALocaleManager::Lang_French:
			m_FileName = "AttractF";
			break;
		case ALocaleManager::Lang_Netherlandic:
			m_FileName = "AttractN";
			break;
		}
	}

	AMoviePlayer::GetSingleton()->PlayMovie(m_FileName, -(m_bFlag2 != TFALSE) & 2);
}

void AMovieState::OnRemoval()
{
	m_Listener1.Disconnect();
	AMoviePlayer::GetSingleton()->StopMovie();
	AGameState::OnRemoval();
}

void AMovieState::SkipMovie(TBOOL a_bFastSkip)
{
	if (!a_bFastSkip)
	{
		if (!m_FadeOverListener.IsLinked())
		{
			AFade::Color fadeFrom(0, 0, 0, 0);
			AFade::Color fadeTo(255, 0, 0, 0);

			auto pFade = AFadeManager::GetSingleton()->StartFade(fadeFrom, fadeTo, 0.5f);
			m_FadeOverListener.Connect(
				pFade->GetFadeOverEmitter(),
				this,
				[](AMovieState* a_pMovieState, AFade* a_pFade, TBOOL*)
				{
					return a_pMovieState->EndMovieImmediately();
				},
				0
			);
		}
	}
	else
	{
		EndMovieImmediately();
	}
}

TBOOL AMovieState::EndMovieImmediately()
{
	AFade::Color fadeTo(0, 0, 0, 0);
	AFade::Color fadeFrom(0, 0, 0, 0);

	if (m_bFlag1)
	{
		m_FadeOverListener.Disconnect();
		fadeFrom.A = 255;
		fadeTo.A = 0;
	}
	else
	{
		if (!m_pNextGameState)
		{
			m_FadeOverListener.Disconnect();
			fadeFrom.A = 255;
			fadeTo.A = 0;

			AFadeManager::GetSingleton()->StartFade(fadeFrom, fadeTo, 0.75f);
			AGameStateController::GetSingleton()->PopState(this);
			return TTRUE;
		}

		m_FadeOverListener.Disconnect();
		fadeFrom.A = 255;
		fadeTo.A = 0;
	}

	AFadeManager::GetSingleton()->StartFade(fadeFrom, fadeTo, 0.75f);
	AGameStateController::GetSingleton()->ReplaceState(m_pNextGameState);
	return TTRUE;
}
