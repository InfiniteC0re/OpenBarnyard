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
	TIMPLEMENT();
	m_InputHelper.Update();

	return 0;
}

void AGameState::OnInsertion()
{
}

void AGameState::OnRemoval()
{
}

void AGameState::OnSuspend()
{
}

void AGameState::OnResume(void* unk)
{
}

void AGameState::OnActivate()
{
}

void AGameState::OnDeactivate()
{
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
