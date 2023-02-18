#pragma once

#include "Toshi/Core/TFreeList.h"

namespace Toshi
{
	class TPooledCString
	{
	public:
		static TFreeList ms_oFreeList;
	};
}
