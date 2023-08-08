#pragma once

#include "Toshi/Core/TFreeList.h"

namespace Toshi
{
#ifdef TOSHI_ENABLE_DEPRECATED
	class TPooledCString
	{
	public:
		static TFreeList ms_oFreeList;
	};
#endif TOSHI_ENABLE_DEPRECATED
}
