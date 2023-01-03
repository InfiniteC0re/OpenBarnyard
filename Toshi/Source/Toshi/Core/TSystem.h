#pragma once
#include "TEvent.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/THPTimer.h"

namespace Toshi
{
	class TSystemManager : public TSingleton<TSystemManager>
	{
	public:
		TSystemManager();

		bool SetPaused(bool paused)
		{
			bool oldState = m_PauseManagers;
			TTODO("FUN_00663190(true)");

			return oldState;
		}

		static bool Create();

	private:
		TGenericEmitter m_Emitter; // 0x00
		uint32_t m_Unk1;
		bool m_PauseManagers;
		uint32_t m_Unk2;
		THPTimer m_SomeTimer;
		uint32_t m_Unk3;
		uint32_t m_Unk4;
		uint32_t m_Unk5;
		uint32_t m_Unk6;
		bool m_Unk7;
	};
}


