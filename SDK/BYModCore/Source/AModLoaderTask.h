#pragma once
#include "AModInstance.h"

#include <BYardSDK/THookedTask.h>
#include <BYardSDK/SDKHooks.h>

#include <Toshi/Core/TTask.h>

TOBJECT(AModLoaderTask, Toshi::TTask, TTRUE)
{
public:
	AModLoaderTask()
	{
		
	}

	~AModLoaderTask()
	{
		m_LoadedMods.DeleteAll();
	}

	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override
	{
		for (auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++)
		{
			TBOOL bResult = it->Update(a_fDeltaTime);
			TASSERT(bResult != TFALSE);
		}

		return TTRUE;
	}

	void AddModInstance(AModInstance* a_pModInstance)
	{
		m_LoadedMods.PushBack(a_pModInstance);
	}

private:
	Toshi::T2DList<AModInstance> m_LoadedMods;
};
