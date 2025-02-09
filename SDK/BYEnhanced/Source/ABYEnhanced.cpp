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
#include <File/TFile.h>
#include <Render/TIndexPoolResource.h>
#include <Render/TVertexPoolResource.h>
#include <Platform/DX8/TRenderContext_DX8.h>

TOSHI_NAMESPACE_USING

class ARenderer
{};

MEMBER_HOOK( 0x006154c0, ARenderer, ARenderer_CreateTRender, TBOOL )
{
	AEnhancedRenderer::CreateSingleton()->Create();

	return CallOriginal();
}

class ABYEnhanced : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		InstallHook<ARenderer_CreateTRender>();
		
		return TTRUE;
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
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

	virtual void OnImGuiRender() override
	{
		ImGui::DragFloat3( "Sun Direction", (TFLOAT*)&enhRender::g_DirectionalLightDir );
		ImGui::ColorEdit3( "Fog Color", (TFLOAT*)&enhRender::g_FogColor );
		/*ImGui::Checkbox( "Render GUI", &g_bRenderGUI );
		ImGui::Checkbox( "Wireframe World", &g_bRenderWorldWireframe );
		ImGui::Checkbox( "Wireframe Skin", &g_bRenderSkinWireframe );*/
	}

	virtual TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}

	const char* GetName() override
	{
		return "BYGLRender";
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
}
