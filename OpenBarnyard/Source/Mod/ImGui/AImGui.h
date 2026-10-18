#pragma once
#include <Mod/ModAPI.h>
#include "Mod/AModEvents.h"
#include <Toshi/TSingleton.h>
#include <Input/TInputInterface.h>
#include <Render/TRenderInterface.h>

#include <imgui.h>

//-----------------------------------------------------------------------------
// ImGui overlay of the mod loader (DX8 only). Alt + ~ opens a window with a
// tab for each mod that has settings, see AModInstance::HasSettingsUI.
// Mods can also draw on top of the game with AModInstance::IsOverlayVisible.
//-----------------------------------------------------------------------------
class MODAPI AImGui
    : public Toshi::TSingleton<AImGui>
{
public:
	AImGui();
	~AImGui();

	// Subscribes to the events that create and drive the overlay
	static void Subscribe();

	// Opens or closes the settings window
	void  Toggle();
	TBOOL IsEnabled() const { return m_bEnabled; }

	// Called by AImGuiState
	void Enable( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }
	void LockInput();
	void UnlockInput();

	// Toggles the window on Alt + ~ and takes the input while the window is open.
	// Called by AGameStateController::ProcessInput, returns TTRUE if the input is used.
	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

private:
	void RenderFrame();
	void BeginScene();
	void EndScene();
	void RenderSettings();
	void RenderOverlay();

	static void OnRendererCreated( const ModEvent::ARenderer::Created& a_rEvent, void* a_pUserData );
	static void OnPostRender( const ModEvent::AGUI2::PostRender& a_rEvent, void* a_pUserData );
	static void OnDeviceLost( const ModEvent::TRenderInterface::DeviceLost& a_rEvent, void* a_pUserData );
	static void OnDeviceFound( const ModEvent::TRenderInterface::DeviceFound& a_rEvent, void* a_pUserData );

private:
	Toshi::TRenderInterface::DISPLAYPARAMS m_oDisplayParams;
	TBOOL                                  m_bEnabled;
	TINT                                   m_iNumInputLocks;
};
