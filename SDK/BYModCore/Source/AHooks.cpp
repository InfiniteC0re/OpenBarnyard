#include "pch.h"
#include "AHooks.h"

#include <detours.h>

void AHooks::Initialise()
{
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	
	{
		// ASlideshow::ProcessInput
		void* pRealMethod = (void*)0x0059dac0;

		struct _methodHolder
		{
			static TBOOL __fastcall hook(
				AGUISlideshow* pThis, void*,
				Toshi::TInputInterface::InputEvent* a_pEvent
			)
			{
				for (TUINT i = 0; i < GUISlideshow::ProcessInput[HookType_Before].Size(); i++)
				{
					TBOOL bRes = GUISlideshow::ProcessInput[HookType_Before][i](pThis, a_pEvent);
					
					if (bRes)
					{
						return TTRUE;
					}
				}

				// Make all slides skippable:
				pThis->m_eFlags |= AGUISlideshow::Flags_Skippable;
				TBOOL bRes = pThis->ProcessInput(a_pEvent);

				if (!bRes)
				{
					for (TUINT i = 0; i < GUISlideshow::ProcessInput[HookType_After].Size(); i++)
					{
						TBOOL bRes = GUISlideshow::ProcessInput[HookType_After][i](pThis, a_pEvent);

						if (bRes)
						{
							return TTRUE;
						}
					}
				}

				return bRes;
			}
		};

		DetourAttach(&pRealMethod, _methodHolder::hook);
	}

	{
		// FUN_0042ab30
		// Called when new game is started
		void* pRealMethod = (void*)0x0042ab30;

		struct _methodHolder
		{
			static void __stdcall hook()
			{
				for (TUINT i = 0; i < Uncategorized::NewGameStarted[HookType_Before].Size(); i++)
				{
					Uncategorized::NewGameStarted[HookType_Before][i]();
				}

				*(TINT*)0x007817ec = 1;

				for (TUINT i = 0; i < Uncategorized::NewGameStarted[HookType_After].Size(); i++)
				{
					Uncategorized::NewGameStarted[HookType_After][i]();
				}
			}
		};

		DetourAttach(&pRealMethod, _methodHolder::hook);
	}

	DetourTransactionCommit();
}

extern "C" {

	TBOOL __declspec(dllexport) AddHookImpl(Hook a_eHook, HookType a_eHookType, void* a_pCallback)
	{
		switch (a_eHook)
		{
		case Hook_AGUISlideshow_ProcessInput:
			AHooks::GUISlideshow::ProcessInput[a_eHookType].PushBack(TSTATICCAST(AHooks::GUISlideshow::t_ProcessInput, a_pCallback));
			return TTRUE;
		case Hook_NewGameStarted:
			AHooks::Uncategorized::NewGameStarted[a_eHookType].PushBack(TSTATICCAST(AHooks::Uncategorized::t_NewGameStarted, a_pCallback));
			return TTRUE;
		}

		return TFALSE;
	}

}
