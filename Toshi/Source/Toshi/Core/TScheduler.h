#pragma once
#include "TKernelInterface.h"

namespace Toshi
{
	class TScheduler : public TObject
	{
		TOSHI_CLASS_DEFINE(TScheduler)

	public:
		inline TKernelInterface* GetKernelInterface() { return m_KernelInterface; }
		
		inline uint32_t TaskCount() { return m_TaskCount; }
		inline void TaskCount(uint32_t count) { m_TaskCount = count; }

	private:
		uint32_t m_TaskCount;					// 0x04 (just a guess)
		TKernelInterface* m_KernelInterface;	// 0x08
	};
}