#pragma once

#include "Toshi/Utils/TSingleton.h"
#include <cstdint>

namespace Toshi
{
	class TXUI : TSingleton<TXUI>
	{
		static uint8_t ms_pXUIMemoryBlock;
		static uint8_t ms_pXUITRBMemoryBlock;
	};
}