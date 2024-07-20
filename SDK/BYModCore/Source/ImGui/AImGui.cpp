#include "pch.h"
#include "AHooks.h"
#include "AImGui.h"
#include "AImGuiState.h"
#include "imgui_impl_dx8.h"
#include "imgui_impl_win32.h"

#include "AModLoaderTask.h"

#include <BYardSDK/AGUI2.h>
#include <BYardSDK/AGameState.h>
#include <BYardSDK/AGameStateController.h>
#include <BYardSDK/AGUI2MouseCursor.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/TPString8.h>

void AImGUI_RenderCallback()
{
	if (AImGUI::IsSingletonCreated() && AImGUI::GetSingleton()->IsEnabled())
	{
		AImGUI::GetSingleton()->Render();
	}
}

AImGUI::AImGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = TTRUE;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	io.Fonts->AddFontFromFileTTF("Bahnschrift.ttf", 17.0f);

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
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
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
	style.WindowBorderSize = 0.0f;

	auto pRender = THookedRenderD3DInterface::GetSingleton();
	ImGui_ImplDX8_Init(pRender->GetDirect3DDevice());
	ImGui_ImplWin32_Init(pRender->GetWindow()->GetHWND());
	m_DisplayParams = *pRender->GetCurrentDisplayParams();

	AHooks::AddHook(Hook_AGUI2_MainPostRenderCallback, HookType_Before, AImGUI_RenderCallback);
}

void AImGUI::BeginScene()
{
	ImGui_ImplDX8_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void AImGUI::EndScene()
{
	ImGui::Render();
	ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
}

void AImGUI::Toggle()
{
	auto pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

	if (!pCurrentState->IsExactly(&TGetClass(AImGuiState)))
	{
		AGameStateController::GetSingleton()->PushState(new AImGuiState());
	}
	else
	{
		AGameStateController::GetSingleton()->PopCurrentGameState();
	}
}

void AImGUI::Render()
{
	BeginScene();

	constexpr TFLOAT WINDOW_MAX_WIDTH = 580;
	constexpr TFLOAT WINDOW_MAX_HEIGHT = 740;

	TFLOAT fWidth = m_DisplayParams.uiWidth * 0.6f;
	TFLOAT fHeight = m_DisplayParams.uiHeight * 0.8f;

	fWidth = Toshi::TMath::Min(fWidth, WINDOW_MAX_WIDTH);
	fHeight = Toshi::TMath::Min(fHeight, WINDOW_MAX_HEIGHT);

	ImGui::SetNextWindowSize({ fWidth, fHeight });
	ImGui::SetNextWindowPos({
		TFLOAT(m_DisplayParams.uiWidth / 2 - fWidth / 2),
		TFLOAT(m_DisplayParams.uiHeight / 2 - fHeight / 2)
		});

	ImGui::SetNextWindowBgAlpha(0.8f);
	ImGui::Begin(
		"BYModCore",
		TNULL,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
	);

	if (ImGui::BeginTabBar("BYModCoreSettingsTabs"))
	{
		auto pModLoader = AGlobalModLoaderTask::Get();
		auto pMods = &pModLoader->GetMods();

		if (ImGui::BeginTabItem("ModCore"))
		{
			ImGui::TextColored(ImColor(200, 200, 200, 255), "AGUI2");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

			ImGui::Checkbox("Show FPS", &AGUI2::GetSingleton()->m_bShowFPSInfo);
			ImGui::Checkbox("Show Player Info", &AGUI2::GetSingleton()->m_bShowPlayerInfo);
			ImGui::Checkbox("Show Textures Info", &AGUI2::GetSingleton()->m_bShowTexturesInfo);
			ImGui::Checkbox("Show Memory Info", &AGUI2::GetSingleton()->m_bShowMemStatsInfo);

			/*extern TBOOL g_bNoCullingInRadiusOfObject;
			extern TFLOAT g_fNoCullingAdditionalRadius;

			ImGui::TextColored(ImColor(200, 200, 200, 255), "Culling Settings");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

			ImGui::Checkbox("Disable culling in radius", &g_bNoCullingInRadiusOfObject);
			ImGui::DragFloat("Additional radius", &g_fNoCullingAdditionalRadius);*/

			ImGui::TextColored(ImColor(200, 200, 200, 255), "AGameStateController");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

			if (ImGui::Button("Open SSeq Debug Menu"))
			{
				Toggle();

				auto pDebugSSeqGameState = TSTATICCAST(AGameState, TMalloc(15300));
				CALL_THIS(0x00407f70, AGameState*, AGameState*, pDebugSSeqGameState);

				AGameStateController::GetSingleton()->PushState(pDebugSSeqGameState);
			}

			if (ImGui::Button("Hide GUI"))
			{
				Toggle();
				CALL(0x00603210, void);
			}

			//if (ImGui::Button("Start New Game (Experimental)"))
			//{
			//	Toggle();
			//	//AGameStateController::GetSingleton()->ReplaceState(new AGameState);
			//	*(TUINT*)(0x007817ec) = 1;
			//	CALL_THIS(0x00651a90, void*, void, *(void**)0x007b5ea8);
			//	AGameStateController::GetSingleton()->m_eFlags |= 0x20;
			//	CALL_THIS(0x00429580, AGameStateController*, void, AGameStateController::GetSingleton(), TINT, 0x2002);
			//	//AGameStateController::GetSingleton()->GetCurrentState()->Remove();
			//}

			//auto pPrevState = AGameStateController::GetSingleton()->GetPreviousState();

			//if (pPrevState->IsExactly((Toshi::TClass*)0x00781b0c))
			//{
			//	static const char* MINIGAME_LIST[] = {
			//		"AChickenFireMinigameState",
			//		"AGolfMiniGameState",
			//		"AStatuesMiniGameState",
			//		"ACarChaseMiniGameState",
			//		"AGateCrashMiniGameState",
			//		"ABarnYardPoolMiniGameState",
			//		"ABikeRaceMicroGame",
			//		"AChasingChicksMiniGame",
			//		"AVeggiePatchDefenderGame",
			//		"ACowTippingMiniGameState",
			//		"ATapperMiniGameState",
			//		"ADartMiniGame",
			//		"AWhackARaccMiniGameState",
			//		"ATeaseMrsBeadyGame",
			//		"AHoneyCollectingGame",
			//		"APrecisionSquirt",
			//		"AChickenCoopDefender"
			//	};

			//	static TUINT NUM_MINIGAMES = sizeof(MINIGAME_LIST) / sizeof(*MINIGAME_LIST);

			//	static TINT s_iSelectedMiniGame = 0;

			//	if (ImGui::BeginCombo("##AMiniGame", MINIGAME_LIST[s_iSelectedMiniGame]))
			//	{
			//		for (TUINT i = 0; i < NUM_MINIGAMES; i++)
			//		{
			//			TBOOL bIsSelected = s_iSelectedMiniGame == i;
			//			ImGui::Selectable(MINIGAME_LIST[i], &bIsSelected);

			//			if (bIsSelected)
			//			{
			//				s_iSelectedMiniGame = i;
			//			}
			//		}

			//		ImGui::EndCombo();
			//	}

			//	ImGui::SameLine();
			//	if (ImGui::Button("Load MiniGame"))
			//	{
			//		Toggle();

			//		class AMiniGameManager;
			//		auto pMiniGameManager = *(AMiniGameManager**)0x0078266c;

			//		CALL_THIS(0x00469890, AMiniGameManager*, void, pMiniGameManager, const Toshi::TPString8&, MINIGAME_LIST[s_iSelectedMiniGame]);
			//	}
			//}

			ImGui::EndTabItem();
		}

		for (auto it = pMods->Begin(); it != pMods->End(); it++)
		{
			if (it->HasSettingsUI())
			{
				if (ImGui::BeginTabItem(it->GetName()))
				{
					it->OnImGuiRender();
					ImGui::EndTabItem();
				}
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();

	EndScene();
}

void AImGUI::OnD3DDeviceLost()
{
	ImGui_ImplDX8_InvalidateDeviceObjects();
}

void AImGUI::OnD3DDeviceFound()
{
	ImGui_ImplDX8_CreateDeviceObjects();
}
