#pragma once
#include "Toshi/Utils/TSingleton.h"

#include <cstdint>

namespace Toshi
{
	class TXUI : public TSingleton<TXUI>
	{
	public:
		static TMemoryHeap* MemoryBlock() { return ms_pXUIMemoryBlock; }

	private:
		static TMemoryHeap* ms_pXUIMemoryBlock;
		static uint8_t ms_pXUITRBMemoryBlock;
	};
}