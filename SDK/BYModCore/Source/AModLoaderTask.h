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

	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;

	void AddModInstance(AModInstance* a_pModInstance) { m_LoadedMods.PushBack(a_pModInstance); }

	AGUI2TextBox* GetTextBox() { return m_pTextBox; }

private:
	static void AGUI2MainPostRenderCallback();

private:
	TFLOAT m_fTotalTime;
	Toshi::T2DList<AModInstance> m_LoadedMods;
	AGUI2TextBox* m_pTextBox;
};
