#include "pch.h"
#include "AGameState.h"
#include "Toshi/Input/TInputInterface.h"

AGameState::AGameState() :
	m_InputHelper(1, Toshi::TInputInterface::GetSingletonWeak()->GetKeyboardByIndex(0))
{
	m_InputHelper.AddDoodad(0x2002d);
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

int AGameState::OnUpdate(float deltaTime)
{
	TTODO("Something with AAssetStreaming");
	m_InputHelper.Update();

	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnUpdate(deltaTime);
	}

	return 0;
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

void AGameState::OnResume(void* unk)
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
