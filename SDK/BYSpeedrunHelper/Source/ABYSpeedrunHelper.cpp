#include "pch.h"
#include "ARunTimer.h"
#include "ACollisionInspector.h"
#include "ASplitsServer.h"

#include <AImGUI.h>
#include <ModLoader.h>
#include <AHooks.h>

#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/THPTimer.h>
#include <File/TFile.h>

TOSHI_NAMESPACE_USING

ARunTimer g_Timer;

void NewGameStarted()
{
	g_Timer.Start();
}

void AGUI2_MainPostRenderCallback()
{
	g_Timer.Render();
}

class ABYSpeedrunHelper : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		if ( AHooks::AddHook( Hook_NewGameStarted, HookType_Before, NewGameStarted ) &&
			 AHooks::AddHook( Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2_MainPostRenderCallback ) )
		{
			ACollisionInspector::CreateSingleton();
			ASplitsServer::CreateSingleton();

			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
		g_Timer.Update();
		return TTRUE;
	}

	void OnUnload() override
	{

	}

	void OnRenderInterfaceReady( Toshi::TRenderD3DInterface* a_pRenderInterface ) override
	{
		TRenderInterface::SetSingletonExplicit(
			THookedRenderD3DInterface::GetSingleton()
		);
	}

	void OnAGUI2Ready() override
	{
		g_Timer.Create();
	}

	void OnImGuiRender() override
	{
		ImGui::Checkbox( "Show Timer", &g_Timer.GetUIElement().IsVisible() );
		ImGui::Checkbox( "Show Collision", &ACollisionInspector::GetSingleton()->IsCollisionVisible() );

		if ( ImGui::Button( "Restart Timer" ) ) g_Timer.Start();
		ImGui::SameLine();
		if ( ImGui::Button( "Resume Timer" ) ) g_Timer.Resume();
		ImGui::SameLine();
		if ( ImGui::Button( "Pause Timer" ) ) g_Timer.Pause();
		ImGui::SameLine();
		if ( ImGui::Button( "Reset Timer" ) ) g_Timer.Reset();
	}

	TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}

	const char* GetName() override
	{
		return "BYSpeedrunHelper";
	}
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance()
	{
		TMemory::Initialise( 8 * 1024 * 1024, 0 );

		TUtil::TOSHIParams toshiParams;
		toshiParams.szCommandLine = "";
		toshiParams.szLogFileName = "speedrunhelper";
		toshiParams.szLogAppName = "BYSpeedrunHelper";

		TUtil::ToshiCreate( toshiParams );

		return new ABYSpeedrunHelper();
	}
}
