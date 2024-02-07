#include "pch.h"
#include "AImGuiState.h"
#include "AImGUI.h"
#include "imgui.h"

#include <BYardSDK/ARootTask.h>
#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/AGUI2.h>

#include <Input/TInputDeviceKeyboard.h>

TDEFINE_CLASS(AImGuiState);

AImGuiState::AImGuiState()
{
	m_iOverlayColorIndex = 3;
}

TBOOL AImGuiState::OnUpdate(TFLOAT a_fDeltaTime)
{
	auto pRender = THookedRenderD3DInterface::GetSingleton();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	Toshi::TVector2 transformedCursorPos = AGUI2::GetSingleton()->m_oMouseCursor.m_CursorPos;

	TFLOAT fRootWidth, fRootHeight;
	AGUI2::GetRootElement()->GetDimensions(fRootWidth, fRootHeight);

	transformedCursorPos.x += fRootWidth / 2;
	transformedCursorPos.y += fRootHeight / 2;
	transformedCursorPos.x = transformedCursorPos.x / fRootWidth * pDisplayParams->uiWidth;
	transformedCursorPos.y = transformedCursorPos.y / fRootHeight * pDisplayParams->uiHeight;

	ImGuiIO& io = ImGui::GetIO();
	//io.MouseDrawCursor = TTRUE;
	io.WantCaptureMouse = TTRUE;
	io.WantSetMousePos = TTRUE;
	io.MousePos = { transformedCursorPos.x, transformedCursorPos.y };

	return TTRUE;
}

void AImGuiState::OnActivate()
{
	AGUI2::GetSingleton()->m_bShowMouseCursor = TTRUE;
	AImGUI::GetSingleton()->Enable(TTRUE);
	ARootTask::GetSingleton()->SetPaused(TTRUE);
}

void AImGuiState::OnDeactivate()
{
	AGUI2::GetSingleton()->m_bShowMouseCursor = TFALSE;
	AImGUI::GetSingleton()->Enable(TFALSE);
	ARootTask::GetSingleton()->SetPaused(TFALSE);
}

TBOOL AImGuiState::ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent)
{
	auto pEventSource = a_pInputEvent->GetSource();
	const TBOOL bIsDown = a_pInputEvent->GetEventType() == Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN;

	Toshi::TClass* pMouseClass = (Toshi::TClass*)(0x007cec68);
	Toshi::TClass* pKeyboardClass = (Toshi::TClass*)(0x007cef54);

	auto& io = ImGui::GetIO();

	if (pEventSource->IsA(pKeyboardClass))
	{
		auto pKeyboard = (Toshi::TInputDeviceKeyboard*)pEventSource;
		auto iDoodad = a_pInputEvent->GetDoodad();

		if (bIsDown)
		{
			io.AddInputCharacterUTF16(pKeyboard->TranslateDoodadToCharacter(iDoodad)[0]);
		}

		switch (a_pInputEvent->GetDoodad())
		{
		case Toshi::TInputDeviceKeyboard::KEY_LCONTROL:
		case Toshi::TInputDeviceKeyboard::KEY_RCONTROL:
			io.AddKeyEvent(ImGuiMod_Ctrl, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_LSHIFT:
		case Toshi::TInputDeviceKeyboard::KEY_RSHIFT:
			io.AddKeyEvent(ImGuiMod_Shift, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::VIRTKEY_ALT:
			io.AddKeyEvent(ImGuiMod_Alt, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_TAB:
			io.AddKeyEvent(ImGuiKey_Tab, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_BACK:
			io.AddKeyEvent(ImGuiKey_Backspace, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_A:
			io.AddKeyEvent(ImGuiKey_A, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_C:
			io.AddKeyEvent(ImGuiKey_C, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_Z:
			io.AddKeyEvent(ImGuiKey_Z, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_Y:
			io.AddKeyEvent(ImGuiKey_Y, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_LEFT:
			io.AddKeyEvent(ImGuiKey_LeftArrow, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_RIGHT:
			io.AddKeyEvent(ImGuiKey_RightArrow, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_UP:
			io.AddKeyEvent(ImGuiKey_UpArrow, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_DOWN:
			io.AddKeyEvent(ImGuiKey_DownArrow, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceKeyboard::KEY_RETURN:
			io.AddKeyEvent(ImGuiKey_Enter, bIsDown);
			return TTRUE;
		}
	}
	else if (pEventSource->IsA(pMouseClass))
	{
		switch (a_pInputEvent->GetDoodad())
		{
		case Toshi::TInputDeviceMouse::BUTTON_1:
			io.AddMouseButtonEvent(ImGuiMouseButton_Left, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceMouse::BUTTON_2:
			io.AddMouseButtonEvent(ImGuiMouseButton_Right, bIsDown);
			return TTRUE;
		case Toshi::TInputDeviceMouse::BUTTON_3:
			io.AddMouseButtonEvent(ImGuiMouseButton_Middle, bIsDown);
			return TTRUE;
		}
	}

	return TFALSE;
}
