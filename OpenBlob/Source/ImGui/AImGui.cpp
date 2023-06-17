#include "pch.h"

#ifdef TOSHI_DEBUG

#include "AImGui.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <Toshi2/T2ResourceManager.h>
#include <Platform/Windows/DX11/TRender_DX11.h>
#include <Platform/Windows/DX11/TRenderContext_DX11.h>

using namespace Toshi;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

AImGui::AImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	
	ImGui::StyleColorsDark();

	auto pRender = TRenderDX11::Interface();

	auto pWindow = pRender->GetMSWindow();

	pWindow->s_fnWndProcHandler =
		[](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	};
	
	TFIXME("Use Toshi to get input events");
	ImGui_ImplWin32_Init(pWindow->GetHWND());
	ImGui_ImplDX11_Init(pRender->m_pDevice, pRender->m_pDeviceContext);

	m_bIsInFrame = TFALSE;
}

void AImGui::Render()
{
	Begin();

	// Draw custom cursor
	auto mousePos = ImGui::GetMousePos();
	auto drawList = ImGui::GetForegroundDrawList();
	drawList->AddTriangleFilled(
		{ mousePos.x + 6, mousePos.y + 14 },
		{ mousePos.x - 0, mousePos.y - 0 },
		{ mousePos.x - 6, mousePos.y + 14 },
		// ABGR
		ImGui::IsMouseDown(ImGuiMouseButton_Left) ? 0xFFFFFFFF : 0xFF909090
	);

	auto pResourceMngr = Toshi::T2ResourceManager::GetSingleton();

	ImGui::Begin("Debug Info");
	ImGui::Text("T2ResourceManager usage: %d/%d", pResourceMngr->GetNumUsedResources(), pResourceMngr->GetMaxNumResources());
	ImGui::End();

	End();
}

void AImGui::Begin()
{
	TASSERT(TFALSE == m_bIsInFrame);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_bIsInFrame = TTRUE;
}

void AImGui::End()
{
	TASSERT(TTRUE == m_bIsInFrame);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	m_bIsInFrame = TFALSE;
}

#endif // TOSHI_DEBUG