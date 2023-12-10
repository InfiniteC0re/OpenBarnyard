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

	{
		// AGUI2::MainPostRenderCallback
		using t_OriginalMethod = void(*)();
		static t_OriginalMethod OriginalMethod = (t_OriginalMethod)(0x00635410);

		struct _methodHolder
		{
			static void __stdcall hook()
			{
				for (TUINT i = 0; i < GUI2::MainPostRenderCallback[HookType_Before].Size(); i++)
				{
					GUI2::MainPostRenderCallback[HookType_Before][i]();
				}

				OriginalMethod();

				for (TUINT i = 0; i < GUI2::MainPostRenderCallback[HookType_After].Size(); i++)
				{
					GUI2::MainPostRenderCallback[HookType_After][i]();
				}
			}
		};

		DetourAttach((PVOID*)&OriginalMethod, _methodHolder::hook);
	}

	static Toshi::TMutex s_MemoryMutex;
	s_MemoryMutex.Create();

	{
		// TMemory::Free
		using t_OriginalMethod = TBOOL(__fastcall*)(Toshi::TMemory*, void*, void*);
		static t_OriginalMethod OriginalMethod = (t_OriginalMethod)(0x006b4a20);

#define MEM_TO_HOLE(PTR) (((Toshi::TMemory::Hole*)(((TUINT)PTR) + 4)) - 1)

		struct _methodHolder
		{
			static TBOOL __fastcall hook(
				Toshi::TMemory* pThis, void* _EDX,
				void* a_pMem
			)
			{
				return pThis->Free(a_pMem);
			}
		};

		DetourAttach((PVOID*)&OriginalMethod, _methodHolder::hook);
	}

	{
		// TMemory::Alloc
		using t_OriginalMethod = void(*)();
		static t_OriginalMethod OriginalMethod = (t_OriginalMethod)(0x006b5230);

		struct _methodHolder
		{
			static void* __fastcall hook(
				Toshi::TMemory* pThis, void*,
				TUINT a_uiSize, TINT a_uiAlignment, Toshi::TMemory::MemBlock* a_pMemBlock, const char* a_szUnused1, TINT a_iUnused2
			)
			{
				return pThis->Alloc(a_uiSize, a_uiAlignment, a_pMemBlock, a_szUnused1, a_iUnused2);
			}
		};

		DetourAttach((PVOID*)&OriginalMethod, _methodHolder::hook);
	}

	DetourTransactionCommit();
}

extern "C" {

	TBOOL __declspec(dllexport) AddHook(Hook a_eHook, HookType a_eHookType, void* a_pCallback)
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
		}

		return TFALSE;
	}

}
