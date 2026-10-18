#include "pch.h"

#ifdef TRENDERINTERFACE_DX8
#  include "AImGui.h"
#  include "AImGuiState.h"
#  include "Mod/AModEvents.h"
#  include "Mod/AModLoader.h"
#  include "GUI/AGUI2.h"
#  include "GameInterface/AGameStateController.h"

#  include <Input/TInputDeviceKeyboard.h>
#  include <Input/TInputDeviceMouse.h>
#  include <Platform/DX8/TRenderInterface_DX8.h>

#  include <backends/imgui_impl_dx8.h>
#  include <backends/imgui_impl_win32.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr const TCHAR* IMGUI_FONT_PATH = "Resources/Fonts/Bahnschrift.ttf";

AImGui::AImGui()
    : m_bEnabled( TFALSE ), m_iNumInputLocks( 0 )
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io                          = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = TTRUE;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// The font is optional, ImGui falls back to its own one
	if ( GetFileAttributesA( IMGUI_FONT_PATH ) != INVALID_FILE_ATTRIBUTES )
		io.Fonts->AddFontFromFileTTF( IMGUI_FONT_PATH, 17.0f );

	// Same style as BYModCore
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding     = ImVec2( 15.0f, 15.0f );
	style.WindowRounding    = 10.0f;
	style.ChildRounding     = 6.0f;
	style.FramePadding      = ImVec2( 8.0f, 7.0f );
	style.FrameRounding     = 8.0f;
	style.ItemSpacing       = ImVec2( 8.0f, 8.0f );
	style.ItemInnerSpacing  = ImVec2( 10.0f, 6.0f );
	style.IndentSpacing     = 25.0f;
	style.ScrollbarSize     = 13.0f;
	style.ScrollbarRounding = 12.0f;
	style.GrabMinSize       = 10.0f;
	style.GrabRounding      = 6.0f;
	style.PopupRounding     = 8.0f;
	style.WindowTitleAlign  = ImVec2( 0.5f, 0.5f );
	style.ButtonTextAlign   = ImVec2( 0.5f, 0.5f );

	style.Colors[ ImGuiCol_Text ]                 = ImVec4( 0.90f, 0.90f, 0.93f, 1.00f );
	style.Colors[ ImGuiCol_TextDisabled ]         = ImVec4( 0.40f, 0.40f, 0.45f, 1.00f );
	style.Colors[ ImGuiCol_WindowBg ]             = ImVec4( 0.12f, 0.12f, 0.14f, 1.00f );
	style.Colors[ ImGuiCol_ChildBg ]              = ImVec4( 0.18f, 0.20f, 0.22f, 0.30f );
	style.Colors[ ImGuiCol_PopupBg ]              = ImVec4( 0.13f, 0.13f, 0.15f, 1.00f );
	style.Colors[ ImGuiCol_Border ]               = ImVec4( 0.30f, 0.30f, 0.35f, 1.00f );
	style.Colors[ ImGuiCol_BorderShadow ]         = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
	style.Colors[ ImGuiCol_FrameBg ]              = ImVec4( 0.18f, 0.18f, 0.20f, 1.00f );
	style.Colors[ ImGuiCol_FrameBgHovered ]       = ImVec4( 0.25f, 0.25f, 0.28f, 1.00f );
	style.Colors[ ImGuiCol_FrameBgActive ]        = ImVec4( 0.30f, 0.30f, 0.34f, 1.00f );
	style.Colors[ ImGuiCol_TitleBg ]              = ImVec4( 0.15f, 0.15f, 0.17f, 1.00f );
	style.Colors[ ImGuiCol_TitleBgCollapsed ]     = ImVec4( 0.10f, 0.10f, 0.12f, 1.00f );
	style.Colors[ ImGuiCol_TitleBgActive ]        = ImVec4( 0.15f, 0.15f, 0.17f, 1.00f );
	style.Colors[ ImGuiCol_MenuBarBg ]            = ImVec4( 0.12f, 0.12f, 0.14f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarBg ]          = ImVec4( 0.12f, 0.12f, 0.14f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarGrab ]        = ImVec4( 0.30f, 0.30f, 0.35f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.40f, 0.40f, 0.45f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarGrabActive ]  = ImVec4( 0.50f, 0.50f, 0.55f, 1.00f );
	style.Colors[ ImGuiCol_CheckMark ]            = ImVec4( 0.70f, 0.70f, 0.90f, 1.00f );
	style.Colors[ ImGuiCol_SliderGrab ]           = ImVec4( 0.70f, 0.70f, 0.90f, 1.00f );
	style.Colors[ ImGuiCol_SliderGrabActive ]     = ImVec4( 0.80f, 0.80f, 0.90f, 1.00f );
	style.Colors[ ImGuiCol_Button ]               = ImVec4( 0.18f, 0.18f, 0.20f, 1.00f );
	style.Colors[ ImGuiCol_ButtonHovered ]        = ImVec4( 0.60f, 0.60f, 0.90f, 1.00f );
	style.Colors[ ImGuiCol_ButtonActive ]         = ImVec4( 0.28f, 0.56f, 0.96f, 1.00f );
	style.Colors[ ImGuiCol_Header ]               = ImVec4( 0.20f, 0.20f, 0.23f, 1.00f );
	style.Colors[ ImGuiCol_HeaderHovered ]        = ImVec4( 0.25f, 0.25f, 0.28f, 1.00f );
	style.Colors[ ImGuiCol_HeaderActive ]         = ImVec4( 0.30f, 0.30f, 0.34f, 1.00f );
	style.Colors[ ImGuiCol_Separator ]            = ImVec4( 0.40f, 0.40f, 0.45f, 1.00f );
	style.Colors[ ImGuiCol_SeparatorHovered ]     = ImVec4( 0.50f, 0.50f, 0.55f, 1.00f );
	style.Colors[ ImGuiCol_SeparatorActive ]      = ImVec4( 0.60f, 0.60f, 0.65f, 1.00f );
	style.Colors[ ImGuiCol_ResizeGrip ]           = ImVec4( 0.20f, 0.20f, 0.23f, 1.00f );
	style.Colors[ ImGuiCol_ResizeGripHovered ]    = ImVec4( 0.25f, 0.25f, 0.28f, 1.00f );
	style.Colors[ ImGuiCol_ResizeGripActive ]     = ImVec4( 0.30f, 0.30f, 0.34f, 1.00f );
	style.Colors[ ImGuiCol_PlotLines ]            = ImVec4( 0.61f, 0.61f, 0.64f, 1.00f );
	style.Colors[ ImGuiCol_PlotLinesHovered ]     = ImVec4( 0.70f, 0.70f, 0.75f, 1.00f );
	style.Colors[ ImGuiCol_PlotHistogram ]        = ImVec4( 0.61f, 0.61f, 0.64f, 1.00f );
	style.Colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 0.70f, 0.70f, 0.75f, 1.00f );
	style.Colors[ ImGuiCol_TextSelectedBg ]       = ImVec4( 0.30f, 0.30f, 0.34f, 1.00f );
	style.Colors[ ImGuiCol_ModalWindowDimBg ]     = ImVec4( 0.10f, 0.10f, 0.12f, 0.80f );
	style.Colors[ ImGuiCol_Tab ]                  = ImVec4( 0.18f, 0.20f, 0.22f, 1.00f );
	style.Colors[ ImGuiCol_TabHovered ]           = ImVec4( 0.60f, 0.60f, 0.90f, 1.00f );
	style.Colors[ ImGuiCol_TabActive ]            = ImVec4( 0.28f, 0.56f, 0.96f, 1.00f );

	TRenderD3DInterface* pRender = TRenderD3DInterface::Interface();

	ImGui_ImplDX8_Init( pRender->GetDirect3DDevice() );
	ImGui_ImplWin32_Init( pRender->GetMSWindow()->GetHWND() );

	m_oDisplayParams = *pRender->GetCurrentDisplayParams();
}

void AImGui::Subscribe()
{
	SUBSCRIBE_EVENT( ARenderer::Created, OnRendererCreated );
	SUBSCRIBE_EVENT( TRenderInterface::DeviceLost, OnDeviceLost );
	SUBSCRIBE_EVENT( TRenderInterface::DeviceFound, OnDeviceFound );
}

void AImGui::Toggle()
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( !pController->GetCurrentState()->IsExactly( &TGetClass( AImGuiState ) ) )
		pController->PushState( new AImGuiState() );
	else
		pController->PopCurrentGameState();
}

void AImGui::LockInput()
{
	m_iNumInputLocks++;
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );
}

void AImGui::UnlockInput()
{
	if ( --m_iNumInputLocks <= 0 )
	{
		m_iNumInputLocks = 0;
		AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
	}
}

void AImGui::RenderFrame()
{
	TBOOL bHasOverlay = TFALSE;

	T2_FOREACH( AModLoader::GetSingleton()->GetMods(), it )
	{
		if ( it->IsOverlayVisible() )
		{
			bHasOverlay = TTRUE;
			break;
		}
	}

	if ( !m_bEnabled && !bHasOverlay )
		return;

	BeginScene();

	if ( m_bEnabled )
		RenderSettings();

	if ( bHasOverlay )
		RenderOverlay();

	EndScene();
}

void AImGui::BeginScene()
{
	ImGui_ImplDX8_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_oDisplayParams = *TRenderD3DInterface::Interface()->GetCurrentDisplayParams();

	// The game draws its own cursor, convert its position from the AGUI2 space to pixels
	TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetMousePos();

	TFLOAT fRootWidth, fRootHeight;
	AGUI2::GetRootElement()->GetDimensions( fRootWidth, fRootHeight );

	vecCursorPos.x = ( vecCursorPos.x + fRootWidth / 2 ) / fRootWidth * m_oDisplayParams.uiWidth;
	vecCursorPos.y = ( vecCursorPos.y + fRootHeight / 2 ) / fRootHeight * m_oDisplayParams.uiHeight;

	ImGuiIO& io         = ImGui::GetIO();
	io.WantCaptureMouse = TTRUE;
	io.WantSetMousePos  = TTRUE;
	io.MousePos         = { vecCursorPos.x, vecCursorPos.y };
}

void AImGui::EndScene()
{
	ImGui::Render();
	ImGui_ImplDX8_RenderDrawData( ImGui::GetDrawData() );
}

void AImGui::RenderSettings()
{
	constexpr TFLOAT WINDOW_MAX_WIDTH  = 580;
	constexpr TFLOAT WINDOW_MAX_HEIGHT = 740;

	const TFLOAT fWidth  = TMath::Min( m_oDisplayParams.uiWidth * 0.6f, WINDOW_MAX_WIDTH );
	const TFLOAT fHeight = TMath::Min( m_oDisplayParams.uiHeight * 0.8f, WINDOW_MAX_HEIGHT );

	ImGui::SetNextWindowSize( { fWidth, fHeight } );
	ImGui::SetNextWindowPos( { m_oDisplayParams.uiWidth / 2 - fWidth / 2, m_oDisplayParams.uiHeight / 2 - fHeight / 2 } );
	ImGui::SetNextWindowBgAlpha( 0.8f );

	ImGui::Begin( "Mods", TNULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize );

	TBOOL bHasSettings = TFALSE;

	if ( ImGui::BeginTabBar( "ModSettingsTabs" ) )
	{
		T2_FOREACH( AModLoader::GetSingleton()->GetMods(), it )
		{
			if ( !it->HasSettingsUI() ) continue;

			bHasSettings = TTRUE;

			if ( ImGui::BeginTabItem( it->GetName() ) )
			{
				it->OnImGuiRender( this );
				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	if ( !bHasSettings )
		ImGui::TextDisabled( "None of the loaded mods have settings" );

	ImGui::End();
}

void AImGui::RenderOverlay()
{
	T2_FOREACH( AModLoader::GetSingleton()->GetMods(), it )
	{
		if ( it->IsOverlayVisible() )
			it->OnImGuiRenderOverlay( this );
	}
}

TBOOL AImGui::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
	TInputDevice* pSource = a_pEvent->GetSource();
	const TBOOL   bIsDown = a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN;

	// Alt + ~ toggles the settings window
	if ( bIsDown && a_pEvent->GetDoodad() == TInputDeviceKeyboard::KEY_GRAVE && pSource->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
	{
		if ( TSTATICCAST( TInputDeviceKeyboard, pSource )->IsAltDown() )
		{
			Toggle();
			return TTRUE;
		}
	}

	// Pass the input to the game unless the window is open
	if ( m_iNumInputLocks == 0 ) return TFALSE;

	ImGuiIO& io = ImGui::GetIO();

	if ( pSource->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
	{
		TInputDeviceKeyboard* pKeyboard = TSTATICCAST( TInputDeviceKeyboard, pSource );
		const TINT            iDoodad   = a_pEvent->GetDoodad();

		if ( bIsDown )
		{
			if ( const TWCHAR* wszCharacter = pKeyboard->TranslateDoodadToCharacter( iDoodad ) )
				io.AddInputCharacterUTF16( wszCharacter[ 0 ] );
		}

		switch ( iDoodad )
		{
			case TInputDeviceKeyboard::KEY_LCONTROL:
			case TInputDeviceKeyboard::KEY_RCONTROL: io.AddKeyEvent( ImGuiMod_Ctrl, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_LSHIFT:
			case TInputDeviceKeyboard::KEY_RSHIFT: io.AddKeyEvent( ImGuiMod_Shift, bIsDown ); break;
			case TInputDeviceKeyboard::VIRTKEY_ALT: io.AddKeyEvent( ImGuiMod_Alt, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_TAB: io.AddKeyEvent( ImGuiKey_Tab, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_BACK: io.AddKeyEvent( ImGuiKey_Backspace, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_DELETE: io.AddKeyEvent( ImGuiKey_Delete, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_HOME: io.AddKeyEvent( ImGuiKey_Home, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_END: io.AddKeyEvent( ImGuiKey_End, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_A: io.AddKeyEvent( ImGuiKey_A, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_C: io.AddKeyEvent( ImGuiKey_C, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_V: io.AddKeyEvent( ImGuiKey_V, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_X: io.AddKeyEvent( ImGuiKey_X, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_Y: io.AddKeyEvent( ImGuiKey_Y, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_Z: io.AddKeyEvent( ImGuiKey_Z, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_LEFT: io.AddKeyEvent( ImGuiKey_LeftArrow, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_RIGHT: io.AddKeyEvent( ImGuiKey_RightArrow, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_UP: io.AddKeyEvent( ImGuiKey_UpArrow, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_DOWN: io.AddKeyEvent( ImGuiKey_DownArrow, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_RETURN: io.AddKeyEvent( ImGuiKey_Enter, bIsDown ); break;
			case TInputDeviceKeyboard::KEY_ESCAPE: io.AddKeyEvent( ImGuiKey_Escape, bIsDown ); break;
		}
	}
	else if ( pSource->IsA( &TGetClass( TInputDeviceMouse ) ) )
	{
		switch ( a_pEvent->GetDoodad() )
		{
			case TInputDeviceMouse::BUTTON_1: io.AddMouseButtonEvent( ImGuiMouseButton_Left, bIsDown ); break;
			case TInputDeviceMouse::BUTTON_2: io.AddMouseButtonEvent( ImGuiMouseButton_Right, bIsDown ); break;
			case TInputDeviceMouse::BUTTON_3: io.AddMouseButtonEvent( ImGuiMouseButton_Middle, bIsDown ); break;
		}
	}

	// The window holds all the input while it's open
	return TTRUE;
}

void AImGui::OnRendererCreated( const ModEvent::ARenderer::Created& a_rEvent, void* a_pUserData )
{
	if ( IsSingletonCreated() ) return;

	CreateSingleton();

	// NOTE: subscribed after the mods have been loaded, so the overlay is drawn on top of what they draw
	SUBSCRIBE_EVENT( AGUI2::PostRender, OnPostRender );
}

void AImGui::OnPostRender( const ModEvent::AGUI2::PostRender& a_rEvent, void* a_pUserData )
{
	if ( IsSingletonCreated() && AModLoader::IsSingletonCreated() )
		GetSingleton()->RenderFrame();
}

void AImGui::OnDeviceLost( const ModEvent::TRenderInterface::DeviceLost& a_rEvent, void* a_pUserData )
{
	if ( IsSingletonCreated() )
		ImGui_ImplDX8_InvalidateDeviceObjects();
}

void AImGui::OnDeviceFound( const ModEvent::TRenderInterface::DeviceFound& a_rEvent, void* a_pUserData )
{
	if ( IsSingletonCreated() )
		ImGui_ImplDX8_CreateDeviceObjects();
}

#endif // TRENDERINTERFACE_DX8
