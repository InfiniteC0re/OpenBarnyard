#pragma once
#include "THPTimer.h"

namespace Toshi
{
	class TKernelInterface : public TObject
	{
		TOSHI_CLASS_DEFINE(TKernelInterface)

	public:
		TKernelInterface();

	public:
		inline THPTimer& GetSystemTimer() { return m_SystemTimer; }
		inline bool ShouldSkipTasks() { return m_SkipTasks; }

	private:
		THPTimer m_SystemTimer;          // 0x08
		bool m_SkipTasks;                // 0x54 (not sure)
	};
}