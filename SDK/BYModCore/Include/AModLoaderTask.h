#pragma once
#include "AModInstance.h"

#include <BYardSDK/AGUI2TextBox.h>
#include <BYardSDK/THookedTask.h>
#include <BYardSDK/SDKHooks.h>

#include <Toshi/TTask.h>

class AModLoaderTask : public Toshi::TTask
{
public:
	TDECLARE_CLASS(AModLoaderTask, Toshi::TTask);

public:
	MODCORE_API AModLoaderTask();
	MODCORE_API ~AModLoaderTask();

	TBOOL OnCreate() override;
	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	void OnAGUI2Ready();
	void OnRenderInterfaceReady();
	void OnAppRendererReady();

	void LoadMods();
	void UnloadMods();

	Toshi::T2DList<AModInstance>& GetMods();

private:
	static void AGUI2MainPostRenderCallback();

private:
	TFLOAT m_fTotalTime;
	Toshi::T2DList<AModInstance> m_LoadedMods;
	TUINT m_uiNumMods;
	AGUI2TextBox* m_pTextBox;
	TBOOL m_bLoaded;
};

class MODCORE_API AGlobalModLoaderTask :
	public Toshi::TSingleton<AGlobalModLoaderTask>
{
public:
	AGlobalModLoaderTask();

	TBOOL Create();
	void Destroy();

	static AModLoaderTask* Get() { return GetSingleton()->m_pTask; }
	static AModInstance* FindMod(const char* a_szModName);

private:
	AModLoaderTask* m_pTask;
};

