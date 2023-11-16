#include "pch.h"
#include "AGameState.h"
#include "Input/AInputHandler.h"
#include "Tasks/ARootTask.h"
#include "Cameras/ACamera.h"
#include "GUI/AGUI2.h"
#include "AppBoot.h"
#include "AGameStateController.h"

#include <Toshi/Input/TInputDeviceKeyboard.h>

#include TOSHI_MULTIRENDER(TRenderInterface)

TOSHI_NAMESPACE_USING

AGameState::AGameState()
{
	m_pInputMap = TNULL;
	m_pOwnerState = TNULL;
	m_bWasInserted = TFALSE;
	m_bIsActivated = TFALSE;
	m_fFOV = Toshi::TMath::DegToRad(60.0f);

	auto pRootElement = AGUI2::GetRootElement();
	m_GUIElement.SetDimensions(pRootElement->GetWidth(), pRootElement->GetHeight());
	m_GUIElement.Hide();

	pRootElement->AddChildHead(&m_GUIElement);
}

TBOOL AGameState::ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent)
{
	if (ExecuteForOneChildState(&AGameState::ChildState_ProcessInput, 0, a_pInputEvent))
	{
		return TTRUE;
	}

	return TFALSE == ARootTask::GetSingleton()->IsGameSystemCreated();
}

TBOOL AGameState::ProcessCommand(InputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents)
{
	return TFALSE;
}

TBOOL AGameState::Unknown1(void* a_pUnk1, void* a_pUnk2)
{
	return ExecuteForOneChildState(&AGameState::ChildState_Unknown1, 0, a_pUnk1, a_pUnk2);
}

TFLOAT AGameState::GetFOV()
{
	return ACamera::s_fCurrentFOV;
}

TBOOL AGameState::OnUpdate(TFLOAT a_fDeltaTime)
{
	ExecuteForAllChildStates(&AGameState::ChildState_OnUpdate, 0, a_fDeltaTime);
	return TTRUE;
}

void AGameState::OnInsertion()
{
	m_bWasInserted = TTRUE;
	ExecuteForAllChildStates(&AGameState::ChildState_OnInsertion, 0);
}

void AGameState::OnRemoval()
{
	ExecuteForAllChildStates(&AGameState::ChildState_OnRemoval, 0);
}

void AGameState::OnSuspend()
{
	ExecuteForAllChildStates(&AGameState::ChildState_OnSuspend, 0);
}

void AGameState::OnResume(AGameState* a_pPreviousState)
{
	ExecuteForAllChildStates(&AGameState::ChildState_OnResume, 0, a_pPreviousState);
}

void AGameState::OnActivate()
{
	m_bIsActivated = TTRUE;
	ExecuteForAllChildStates(&AGameState::ChildState_OnActivate, 0);
}

void AGameState::OnDeactivate()
{
	m_bIsActivated = TFALSE;
	ExecuteForAllChildStates(&AGameState::ChildState_OnDeactivate, 0);
}

void AGameState::Destroy()
{
	auto pRender = TSTATICCAST(Toshi::TRenderD3DInterface*, Toshi::TRenderInterface::GetSingleton());
	pRender->BeginEndScene();

	Deactivate();

	OnRemoval();
	delete this;
}

void AGameState::Destroy(TBOOL a_bDeactivate)
{
	auto pRender = TSTATICCAST(Toshi::TRenderD3DInterface*, Toshi::TRenderInterface::GetSingleton());
	pRender->BeginEndScene();

	if (a_bDeactivate)
	{
		Deactivate();
	}

	OnRemoval();
	delete this;
}

TBOOL AGameState::SendInputCommands(const Toshi::TInputInterface::InputEvent* a_pEvent)
{
	auto pInputHandler = AInputHandler::GetSingleton();
	auto pInputManager = AInputMapManager::GetSingleton();

	pInputManager->PushInputMap(m_pInputMap);

	AInputCommandArray commands;
	pInputManager->GetEventCommands(a_pEvent, commands);

	if (a_pEvent->GetDoodad() == TInputDeviceKeyboard::KEY_F4 && pInputHandler->GetKeyboardDevice()->IsAltDown())
	{
		// Alt + F4
		TTODO("FUN_00425980");
		g_oTheApp.Destroy();
	}
	else
	{
		TBOOL bProcessedCommand = TFALSE;

		for (TINT i = 0; i < commands.iNumCommands; i++)
		{
			auto eCommand = commands.aCommands[i];
			TBOOL bStopEvents = TTRUE;

			if (ProcessCommand(eCommand, a_pEvent, bStopEvents))
			{
				bProcessedCommand = TTRUE;

				if (bStopEvents)
				{
					pInputManager->PopInputMap();

					for (TINT k = 0; k < commands.iNumCommands; k++)
					{
						commands.aCommands[k] = 0;
					}
					
					return TTRUE;
				}
			}
		}

		pInputManager->PopInputMap();
		commands.Clear();

		if (!bProcessedCommand)
		{
			return ExecuteForOneChildState(&AGameState::SendInputCommands, 0, a_pEvent);
		}
	}

	return TTRUE;
}

void AGameState::Remove()
{
	if (m_pOwnerState == TNULL)
	{
		AGameStateController::GetSingleton()->PopCurrentGameState();
	}
	else
	{
		Node::Remove();

		if (m_bIsActivated)
		{
			OnDeactivate();
		}

		OnRemoval();
		delete this;
	}
}
