#include "pch.h"
#include "AGameState.h"
#include "AGameStateController.h"
#include "Toshi/Input/TInputInterface.h"
#include "Toshi2/T2GUI/T2GUI.h"
#include "AAssetStreaming.h"

AGameState::AGameState() :
	m_InputHelper(1, Toshi::TInputInterface::GetSingletonWeak()->GetKeyboardByIndex(0))
{
	m_InputHelper.AddDoodad(0x2002d);
	m_State = State_Null;
}

AGameState::~AGameState()
{
	
}

void AGameState::Unk1()
{

}

bool AGameState::Unk2()
{
	return false;
}

void AGameState::Unk3(void*, void*)
{

}

AGameState::UpdateResult AGameState::OnUpdate(float deltaTime)
{
	m_InputHelper.Update();

	if (!AAssetStreaming::GetSingletonWeak()->HasActiveJobs())
	{
		sm_pLoadIconRect->SetVisible(TFALSE);
	}

	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnUpdate(deltaTime);
	}
	
	return UpdateResult_OK;
}

void AGameState::OnInsertion()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnInsertion();
	}
}

void AGameState::OnRemoval()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnRemoval();
	}

	m_GameStates.DeleteAll();
}

void AGameState::OnSuspend()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnDeactivate();
	}
}

void AGameState::OnResume(AGameState* pOldState)
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnActivate();
	}
}

void AGameState::OnActivate()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnActivate();
	}
}

void AGameState::OnDeactivate()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnDeactivate();
	}
}

bool AGameState::CanActivateConsoleState()
{
	return true;
}

bool AGameState::CheckForControllerRemoval()
{
	return false;
}

bool AGameState::PauseOnControllerReinserted()
{
	return false;
}

void AGameState::SetupLoadIcon()
{
	TASSERT(TNULL == sm_pLoadIconRect);
	sm_pLoadIconRect = new Toshi::T2GUIRectangle();
	sm_pLoadIconRect->Create(50.0f, 50.0f);
	sm_pLoadIconRect->SetColour(-1);
	sm_pLoadIconMat = Toshi::T2GUI::GetSingletonWeak()->CreateMaterial("loadicon.tga");
	sm_pLoadIconRect->SetMaterial(sm_pLoadIconMat);
	sm_pLoadIconRect->SetAnchor(Toshi::T2GUIElement::Anchor::BottomRight);
	sm_pLoadIconRect->SetPivot(Toshi::T2GUIElement::Pivot::BottomRight);
}

void AGameState::DestroyLoadIcon()
{
}

void AGameState::RemoveSelf()
{
	TASSERT(AGameStateController::GetSingletonWeak()->GetCurrentGameState() == this);
	AGameStateController::GetSingletonWeak()->PopCurrentState();
}
