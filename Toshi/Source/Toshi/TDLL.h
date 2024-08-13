#pragma once

#ifdef TOSHI_SKU_WINDOWS
#	include "Platform/Windows/TDLL_Win.h"
#else  // TOSHI_SKU_WINDOWS
class TDLL;
#endif // TOSHI_SKU_WINDOWS
