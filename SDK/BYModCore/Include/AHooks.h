#pragma once
#include "BYardSDK/AGUISlideshow.h"

#include <Toshi2/T2Vector.h>

extern "C" {

	enum Hook
	{
		Hook_AGUISlideshow_ProcessInput,
		Hook_NewGameStarted,
		Hook_NUMOF,
	};

	enum HookType
	{
		HookType_Before,
		HookType_After,
		HookType_NUMOF
	};

	class AHooks
	{
	public:
		constexpr static TUINT MAX_NUM_CALLBACKS = 16;

		struct GUISlideshow
		{
			using t_ProcessInput = TBOOL(*)(AGUISlideshow* a_pSlideshow, Toshi::TInputInterface::InputEvent* a_pEvent);
			inline static Toshi::T2Vector<t_ProcessInput, MAX_NUM_CALLBACKS> ProcessInput[HookType_NUMOF];
		};

		struct Uncategorized
		{
			using t_NewGameStarted = void(*)();
			inline static Toshi::T2Vector<t_NewGameStarted, MAX_NUM_CALLBACKS> NewGameStarted[HookType_NUMOF];
		};

	public:
		static void Initialise();
	};

	TBOOL __declspec(dllexport) AddHookImpl(Hook a_eHook, HookType a_eHookType, void* a_pCallback);
}
