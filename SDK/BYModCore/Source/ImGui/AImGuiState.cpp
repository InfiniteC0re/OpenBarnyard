#include "pch.h"
#include "AImGuiState.h"
#include "AImGUI.h"
#include "imgui.h"

#include <BYardSDK/ARootTask.h>
#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/AGUI2.h>

TDEFINE_CLASS(AImGuiState);

AImGuiState::AImGuiState()
{
	m_eOverlayColorIndex = 3;
}

TBOOL AImGuiState::OnUpdate(TFLOAT a_fDeltaTime)
{
	return TTRUE;
}

void AImGuiState::OnActivate()
{
	AImGUI::GetSingleton()->Enable(TTRUE);
	AImGUI::GetSingleton()->LockInput();
}

void AImGuiState::OnDeactivate()
{
	AImGUI::GetSingleton()->Enable(TFALSE);
	AImGUI::GetSingleton()->UnlockInput();
}

TBOOL AImGuiState::ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent)
{
	return TFALSE;
}
