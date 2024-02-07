#include "pch.h"
#include <AImGUI.h>
#include <ModLoader.h>
#include <AModLoaderTask.h>
#include <AHooks.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/THPTimer.h>
#include <File/TFile.h>

#include "ABYTexturePacks.h"
#include "ATextureUpdater.h"

#include "Platform/DX8/TTextureResourceHAL_DX8.h"

TOSHI_NAMESPACE_USING

void ABYTexturePacks::SetTexturePack(const char* a_szName)
{
	auto pModLoader = AGlobalModLoaderTask::Get();
	TStringManager::String8Copy(g_szTexturePackName, a_szName);
	UpdateTexturePackPath();
}

void ABYTexturePacks::UpdateTexturePackPath()
{
	if (m_pTexWatcher)
	{
		m_pTexWatcher->Destroy();
		m_pTexWatcher = TNULL;
	}

	TStringManager::String8Format(
		g_szTexturePackPath,
		sizeof(g_szTexturePackPath),
		"Mods\\TexturePacks\\%s\\",
		g_szTexturePackName
	);

	m_pTexWatcher = new ATextureUpdateWatcher();
	m_pTexWatcher->Init(g_szTexturePackPath);
	m_pTexWatcher->Create(32, TThread::THREAD_PRIORITY_LOWEST, 0);

	ATextureUpdater::ReloadAllTextures();
}

TBOOL ABYTexturePacks::OnLoad()
{
	SetTexturePack("Default");
	ATextureUpdater::CreateSingleton();

	return TTRUE;
}

TBOOL ABYTexturePacks::OnUpdate(TFLOAT a_fDeltaTime)
{
	return TTRUE;
}

void ABYTexturePacks::OnUnload()
{

}

void ABYTexturePacks::OnRenderInterfaceReady(Toshi::TRenderD3DInterface* a_pRenderInterface)
{
	TRenderInterface::SetSingletonExplicit(
		THookedRenderD3DInterface::GetSingleton()
	);
}

void ABYTexturePacks::OnImGuiRender()
{
	static char s_InputTexturePackName[64];

	ImGui::InputText("##TexturePackName", s_InputTexturePackName, 64);
	ImGui::SameLine();
	
	if (ImGui::Button("Load"))
	{
		if (s_InputTexturePackName[0] == '\0')
		{
			SetTexturePack("Default");
		}
		else
		{
			SetTexturePack(s_InputTexturePackName);
		}
	}

	ImGui::Checkbox("Dump Textures", &g_bDumpTextures);
	ImGui::Checkbox("Auto Reload", &g_bAutoReload);

	if (ImGui::Button("Reload Textures"))
	{
		ATextureUpdater::ReloadAllTextures();
	}
}

TBOOL ABYTexturePacks::HasSettingsUI()
{
	return TTRUE;
}

const char* ABYTexturePacks::GetName()
{
	return "BYTexturePacks";
}

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance()
	{
		TMemory::Initialise(64 * 1024 * 1024, 0);
		TFileManager::Create();
		TLog::Create("ABYTexturePacks");

		g_LoadMutex.Create();

		return new ABYTexturePacks();
	}
}
