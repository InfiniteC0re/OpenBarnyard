#pragma once
#include "AModInstance.h"

#include <BYardSDK/AGUI2TextBox.h>
#include <BYardSDK/THookedTask.h>
#include <BYardSDK/SDKHooks.h>

#include <Toshi/Core/TTask.h>

TOBJECT(AModLoaderTask, Toshi::TTask, TTRUE),
	public Toshi::TSingleton<AModLoaderTask>
{
public:
	AModLoaderTask();
	~AModLoaderTask();

	TBOOL OnCreate() override;
	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	void OnAGUI2Ready();
	void OnRenderInterfaceReady();

	void LoadMods();

	Toshi::T2DList<AModInstance>& GetMods()
	{
		return m_LoadedMods;
	}

private:
	void AddModInstance(AModInstance* a_pModInstance)
	{
		m_LoadedMods.PushBack(a_pModInstance);
		m_uiNumMods += 1;
	}

private:
	static void AGUI2MainPostRenderCallback();

private:
	TFLOAT m_fTotalTime;
	Toshi::T2DList<AModInstance> m_LoadedMods;
	TUINT m_uiNumMods;
	AGUI2TextBox* m_pTextBox;
	TBOOL m_bLoaded;
};
