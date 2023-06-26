#include "pch.h"
#include "AGameState.h"
#include "AGameStateController.h"
#include "AAssetStreaming.h"

#include <Toshi/Input/TInputInterface.h>
#include <Toshi/Input/TInputDeviceKeyboard.h>

#include <Toshi2/T2GUI/T2GUI.h>

AGameState::AGameState() :
	m_InputHelperSimple(1, Toshi::TInputInterface::GetSingletonWeak()->GetDeviceByIndex<Toshi::TInputDeviceKeyboard>())
{
	m_InputHelperSimple.AddDoodad(Toshi::TInputDeviceKeyboard::KEY_GRAVE);
	m_State = State_Null;
}

AGameState::~AGameState()
{
	TIMPLEMENT();
}

void AGameState::Unk1()
{

}

TBOOL AGameState::Unk2()
{
	return TFALSE;
}

void AGameState::Unk3(void*, void*)
{

}

AGameState::UpdateResult AGameState::OnUpdate(float deltaTime)
{
	m_InputHelperSimple.Update();

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

TBOOL AGameState::CanActivateConsoleState()
{
	return TTRUE;
}

TBOOL AGameState::CheckForControllerRemoval()
{
	return TFALSE;
}

TBOOL AGameState::PauseOnControllerReinserted()
{
	return TFALSE;
}

#ifdef TOSHI_DEBUG
void AGameState::DEBUG_RenderImGui()
{

}
#endif // TOSHI_DEBUG

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
