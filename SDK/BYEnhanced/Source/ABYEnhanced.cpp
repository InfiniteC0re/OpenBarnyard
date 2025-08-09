#include "pch.h"
#include "Render/AEnhancedRenderer.h"

#include <AImGUI.h>
#include <ModLoader.h>
#include <AModLoaderTask.h>
#include <AHooks.h>
#include <HookHelpers.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/THPTimer.h>
#include <Input/TInputDeviceMouse.h>
#include <Input/TInputInterface.h>
#include <File/TFile.h>
#include <Render/TIndexPoolResource.h>
#include <Render/TVertexPoolResource.h>
#include <Platform/DX8/TRenderContext_DX8.h>

TOSHI_NAMESPACE_USING

class ARenderer
{};

struct MouseEventState
{

	TBOOL bNewEvent = TFALSE;
	TBOOL bDown     = TTRUE;
	TINT  iButton   = 0;

} g_oMouseEventState;

MEMBER_HOOK( 0x006154c0, ARenderer, ARenderer_CreateTRender, TBOOL )
{
	AEnhancedRenderer::CreateSingleton()->Create();

	class SDLEventListener : public T2Window::EventListener
	{
	public:
		virtual TBOOL OnEvent( const SDL_Event& a_rEvent )
		{
			switch ( a_rEvent.type )
			{
				case SDL_MOUSEBUTTONDOWN:
					g_oMouseEventState.bNewEvent = TTRUE;
					g_oMouseEventState.bDown     = TTRUE;
					g_oMouseEventState.iButton   = a_rEvent.button.button;
					break;
				case SDL_MOUSEBUTTONUP:
					g_oMouseEventState.bNewEvent = TTRUE;
					g_oMouseEventState.bDown     = TFALSE;
					g_oMouseEventState.iButton   = a_rEvent.button.button;
					break;
			}
			return TTRUE;
		}
	};

	T2Window* pWindow = T2Render::GetSingleton()->GetWindow();
	pWindow->SetListener( new SDLEventListener );

	return CallOriginal();
}

MEMBER_HOOK( 0x006d4200, TInputDeviceMouse, TInputDXDeviceMouse_ProcessEvents, TINT, TInputDevice::EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	if ( g_oMouseEventState.bNewEvent )
	{
		a_rEmitter.Throw( TInputInterface::InputEvent( this, TInputDeviceMouse::BUTTON_1 + g_oMouseEventState.iButton - 1, !g_oMouseEventState.bDown ? TInputInterface::EVENT_TYPE_GONE_DOWN : TInputInterface::EVENT_TYPE_GONE_UP ) );
		g_oMouseEventState.bNewEvent = TFALSE;
	}

	return CallOriginal( a_rEmitter, a_fDeltaTime );
}

class ABYEnhanced : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		InstallHook<ARenderer_CreateTRender>();
		InstallHook<TInputDXDeviceMouse_ProcessEvents>();
		
		return TTRUE;
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
		g_oSystemManager.Update();

		return TTRUE;
	}

	void OnUnload() override
	{
	}

	void OnRenderInterfaceReady( Toshi::TRenderD3DInterface* a_pRenderInterface ) override
	{
		
	}

	void OnAppRendererReady() override
	{
	}

	virtual void OnImGuiRender( AImGUI* a_pImGui ) override
	{
		ImGui::SliderFloat( "Shadow Bias (Min)", &enhRender::g_ShadowBiasMin, 0.0f, 1.0f, "%.8f" );
		ImGui::SliderFloat( "Shadow Bias (Max)", &enhRender::g_ShadowBiasMax, 0.0f, 1.0f, "%.8f" );
		ImGui::SliderFloat( "Shadow Strength", &enhRender::g_ShadowStrength, 0.0f, 1.0f, "%.3f" );

		ImGui::DragFloat3( "Sun Direction", (TFLOAT*)&enhRender::g_DirectionalLightDir );
		ImGui::ColorEdit3( "Fog Color", (TFLOAT*)&enhRender::g_FogColor );
		ImGui::ColorEdit3( "Specular Color", (TFLOAT*)&enhRender::g_SpecularColor );

		ImGui::SliderFloat( "Gamma", &enhRender::g_Gamma, 0.0f, 10.0f, "%.1f" );
		ImGui::SliderFloat( "Exposure", &enhRender::g_Exposure, 0.0f, 20.0f, "%.1f" );
		/*ImGui::Checkbox( "Render GUI", &g_bRenderGUI );
		ImGui::Checkbox( "Wireframe World", &g_bRenderWorldWireframe );
		ImGui::Checkbox( "Wireframe Skin", &g_bRenderSkinWireframe );*/
	}

	virtual TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance( const T2CommandLine* a_pCommandLine )
	{
		TMemory::Initialise( 2 * 1024 * 1024, 0 );

		TUtil::TOSHIParams toshiParams;
		toshiParams.szCommandLine = "";
		toshiParams.szLogFileName = "enhanced";
		toshiParams.szLogAppName  = "BYEnhanced";

		TUtil::ToshiCreate( toshiParams );
		TUtil::SetTPStringPool( new TPString8Pool( 1024, 0, Toshi::GetGlobalAllocator(), TNULL ) );

		return new ABYEnhanced();
	}

	MODLOADER_EXPORT const TCHAR* GetModName()
	{
		return "Enhanced";
	}

	MODLOADER_EXPORT TUINT32 GetModVersion()
	{
		return TVERSION( 1, 0 );
	}
}
