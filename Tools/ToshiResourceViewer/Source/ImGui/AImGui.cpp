#include "pch.h"

#ifdef TOSHI_DEBUG

#include "AImGui.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui.h"

#ifdef TOSHI_RENDERER_DX11
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#endif

#ifdef TOSHI_RENDERER_OPENGL
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#endif

#include "Resources/AResourceViewLoader.h"

#include <Toshi/Typedefs.h>
#include <Toshi/Render/TTexture.h>
#include <Toshi2/T2ResourceManager.h>

#include <commdlg.h>

TOSHI_NAMESPACE_USING

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

AImGui::AImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Bahnschrift.ttf", 18.0f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.21f, 0.74f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	style.GrabRounding = style.FrameRounding = 2.3f;
	style.WindowPadding = { 10, 10 };
	style.FramePadding = ImVec2(12, 5);
	style.WindowBorderSize = 1.0f;

	auto pRender = TRENDER::Interface();
	auto pWindow = pRender->GetWindow();

#ifdef TOSHI_RENDERER_DX11
	TFIXME("Use Toshi to get input events");
	pWindow->s_fnWndProcHandler =
		[](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	};

	ImGui_ImplWin32_Init(pWindow->GetHWND());
	ImGui_ImplDX11_Init(pRender->m_pDevice, pRender->m_pDeviceContext);
#endif // TOSHI_RENDERER_DX11

#ifdef TOSHI_RENDERER_OPENGL
	pWindow->SetListener(this);

	ImGui_ImplSDL2_InitForOpenGL(pWindow->GetNativeWindow(), pRender->GetDeviceContext());
	ImGui_ImplOpenGL3_Init("#version 130");
#endif // TOSHI_RENDERER_OPENGL

	m_bIsInFrame = TFALSE;
}

void AImGui::Render()
{
	BeginScene();

#ifdef TOSHI_RENDERER_DX11
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
#endif

	auto pResourceLoader = AResourceViewLoader::GetSingleton();

	for (auto it = pResourceLoader->Begin(); it != pResourceLoader->End(); it++)
	{
		it->Render();
	}

	ImGui::SetNextWindowSize({ 500, 500 });

	EndScene();
}

#ifdef TOSHI_RENDERER_OPENGL
TBOOL AImGui::OnEvent(const SDL_Event& a_rEvent)
{
	return ImGui_ImplSDL2_ProcessEvent(&a_rEvent);
}
#endif // TOSHI_RENDERER_OPENGL

void AImGui::BeginScene()
{
	TASSERT(TFALSE == m_bIsInFrame);

#ifdef TOSHI_RENDERER_DX11
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
#endif

#ifdef TOSHI_RENDERER_OPENGL
	auto pRender = TRENDER::Interface();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(pRender->GetWindow()->GetNativeWindow());
#endif // TOSHI_RENDERER_OPENGL

	ImGui::NewFrame();

	m_bIsInFrame = TTRUE;
}

void AImGui::EndScene()
{
	TASSERT(TTRUE == m_bIsInFrame);

	ImGui::Render();
#ifdef TOSHI_RENDERER_DX11
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif // TOSHI_RENDERER_DX11

#ifdef TOSHI_RENDERER_OPENGL
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // TOSHI_RENDERER_OPENGL
	m_bIsInFrame = TFALSE;
}

#endif // TOSHI_DEBUG