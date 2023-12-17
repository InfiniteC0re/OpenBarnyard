#include "pch.h"
#include "AHooks.h"
#include "AImGUI.h"
#include "HookHelpers/HookHelpers.h"

#include <BYardSDK/AGameStateController.h>
#include <BYardSDK/AGUI2.h>

#include <Toshi/Input/TInputDeviceKeyboard.h>
#include <Platform/Windows/TMSWindow.h>

MEMBER_HOOK(0x006b4a20, Toshi::TMemory, TMemory_Free, TBOOL, void* a_pMem)
{
	return this->Free(a_pMem);
}

MEMBER_HOOK(0x006b5230, Toshi::TMemory, TMemory_Alloc, void*, TUINT a_uiSize, TINT a_uiAlignment, Toshi::TMemory::MemBlock* a_pMemBlock, const char* a_szUnused1, TINT a_iUnused2)
{
	return this->Alloc(a_uiSize, a_uiAlignment, a_pMemBlock, a_szUnused1, a_iUnused2);
}

HOOK(0x006b4ba0, TMemory_GetMemInfo, void, Toshi::TMemory::MemInfo& a_rMemInfo, Toshi::TMemory::MemBlock* a_pBlock)
{
	auto pBlock = (*(Toshi::TMemory**)0x007ce1d4)->GetGlobalBlock();
	Toshi::TMemory::GetMemInfo(a_rMemInfo, pBlock);
}

MEMBER_HOOK(0x006da4d0, Toshi::TMSWindow, TMSWindow_SetPosition, void, TUINT x, TUINT y, TUINT width, TUINT height)
{
	if (IsWindowed())
	{
		// Fix window size when in windowed mode
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = width;
		rect.bottom = height;

		if (TRUE == AdjustWindowRectEx(
			&rect,
			GetWindowLongA(GetHWND(), GWL_STYLE),
			FALSE,
			GetWindowLongA(GetHWND(), GWL_EXSTYLE)
		))
		{
			x = rect.left;
			y = rect.top;
			width = rect.right;
			height = rect.bottom;
		}
	}
	
	CallOriginal(x, y, width, height);
}

MEMBER_HOOK(0x0059dac0, AGUISlideshow, AGUISlideshow_ProcessInput, TBOOL, Toshi::TInputInterface::InputEvent* a_pEvent)
{
	for (TUINT i = 0; i < AHooks::GUISlideshow::ProcessInput[HookType_Before].Size(); i++)
	{
		if (AHooks::GUISlideshow::ProcessInput[HookType_Before][i](this, a_pEvent))
		{
			return TTRUE;
		}
	}

	// Make all slides skippable:
	m_eFlags |= AGUISlideshow::Flags_Skippable;

	if (!ProcessInput(a_pEvent))
	{
		for (TUINT i = 0; i < AHooks::GUISlideshow::ProcessInput[HookType_After].Size(); i++)
		{
			if (AHooks::GUISlideshow::ProcessInput[HookType_After][i](this, a_pEvent))
			{
				return TTRUE;
			}
		}

		return TFALSE;
	}

	return TTRUE;
}

HOOK(0x0042ab30, FUN_0042ab30, void)
{
	for (TUINT i = 0; i < AHooks::Uncategorized::NewGameStarted[HookType_Before].Size(); i++)
	{
		AHooks::Uncategorized::NewGameStarted[HookType_Before][i]();
	}

	CallOriginal();

	for (TUINT i = 0; i < AHooks::Uncategorized::NewGameStarted[HookType_After].Size(); i++)
	{
		AHooks::Uncategorized::NewGameStarted[HookType_After][i]();
	}
}

HOOK(0x00635410, AGUI2_MainPostRenderCallback, void)
{
	for (TUINT i = 0; i < AHooks::GUI2::MainPostRenderCallback[HookType_Before].Size(); i++)
	{
		AHooks::GUI2::MainPostRenderCallback[HookType_Before][i]();
	}

	CallOriginal();

	for (TUINT i = 0; i < AHooks::GUI2::MainPostRenderCallback[HookType_After].Size(); i++)
	{
		AHooks::GUI2::MainPostRenderCallback[HookType_After][i]();
	}
}

MEMBER_HOOK(0x00635440, AGUI2, AGUI2_Constructor, AGUI2*)
{
	CallOriginal();
	return this;
}

MEMBER_HOOK(0x006357d0, AGUI2, AGUI2_OnUpdate, TBOOL, TFLOAT a_fDeltaTime)
{
	if (m_bShowMemStatsInfo) m_oMemStats.Show();
	else m_oMemStats.Hide();

	if (m_bShowFPSInfo) m_oFPS.Show();
	else m_oFPS.Hide();

	if (m_bShowPlayerInfo) m_oPlayerInfo.Show();
	else m_oPlayerInfo.Hide();

	if (m_bShowTexturesInfo) m_oTexturesInfo.Show();
	else m_oTexturesInfo.Hide();

	return CallOriginal(a_fDeltaTime);
}

MEMBER_HOOK(0x004293d0, AGameStateController, AGameStateController_ProcessInput, TBOOL, Toshi::TInputInterface::InputEvent* a_pInputEvent)
{
	if (a_pInputEvent->GetEventType() == Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN &&
		a_pInputEvent->GetDoodad() == Toshi::TInputDeviceKeyboard::KEY_GRAVE)
	{
		AImGUI::GetSingleton()->Toggle();
	}

	return CallOriginal(a_pInputEvent);
}

MEMBER_HOOK(0x005ea8b0, ATerrain, ATerrain_Render, void)
{
	for (TUINT i = 0; i < AHooks::Terrain::Render[HookType_Before].Size(); i++)
	{
		AHooks::Terrain::Render[HookType_Before][i](this);
	}
	
	CallOriginal();

	for (TUINT i = 0; i < AHooks::Terrain::Render[HookType_After].Size(); i++)
	{
		AHooks::Terrain::Render[HookType_After][i](this);
	}
}

HOOK(0x006114d0, AModelLoader_AModelLoaderLoadTRBCallback, TBOOL, Toshi::TModel* a_pModel)
{
	TBOOL bRes;

	for (TUINT i = 0; i < AHooks::ModelLoader::LoadTRBCallback[HookType_Before].Size(); i++)
	{
		bRes |= AHooks::ModelLoader::LoadTRBCallback[HookType_Before][i](a_pModel);
	}

	bRes |= CallOriginal(a_pModel);

	for (TUINT i = 0; i < AHooks::ModelLoader::LoadTRBCallback[HookType_After].Size(); i++)
	{
		bRes |= AHooks::ModelLoader::LoadTRBCallback[HookType_After][i](a_pModel);
	}

	return bRes;
}

void AHooks::Initialise()
{
	InstallHook<TMemory_Free>();
	InstallHook<TMemory_Alloc>();
	InstallHook<TMemory_GetMemInfo>();
	InstallHook<TMSWindow_SetPosition>();
	InstallHook<AGUISlideshow_ProcessInput>();
	InstallHook<FUN_0042ab30>();
	InstallHook<AGUI2_MainPostRenderCallback>();
	InstallHook<AGUI2_Constructor>();
	InstallHook<AGUI2_OnUpdate>();
	InstallHook<AGameStateController_ProcessInput>();
	InstallHook<ATerrain_Render>();
	InstallHook<AModelLoader_AModelLoaderLoadTRBCallback>();
}

TBOOL AHooks::AddHook(Hook a_eHook, HookType a_eHookType, void* a_pCallback)
{
	switch (a_eHook)
	{
	case Hook_AGUI2_MainPostRenderCallback:
		AHooks::GUI2::MainPostRenderCallback[a_eHookType].PushBack(TSTATICCAST(AHooks::GUI2::t_MainPostRenderCallback, a_pCallback));
		return TTRUE;
	case Hook_AGUISlideshow_ProcessInput:
		AHooks::GUISlideshow::ProcessInput[a_eHookType].PushBack(TSTATICCAST(AHooks::GUISlideshow::t_ProcessInput, a_pCallback));
		return TTRUE;
	case Hook_NewGameStarted:
		AHooks::Uncategorized::NewGameStarted[a_eHookType].PushBack(TSTATICCAST(AHooks::Uncategorized::t_NewGameStarted, a_pCallback));
		return TTRUE;
	case Hook_ATerrain_Render:
		AHooks::Terrain::Render[a_eHookType].PushBack(TSTATICCAST(AHooks::Terrain::t_Render, a_pCallback));
		return TTRUE;
	case Hook_AModelLoader_LoadTRBCallback:
		AHooks::ModelLoader::LoadTRBCallback[a_eHookType].PushBack(TSTATICCAST(AHooks::ModelLoader::t_LoadTRBCallback, a_pCallback));
		return TTRUE;
	}

	return TFALSE;
}
