#pragma once

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/THPTimer_Win.h"
#endif // TOSHI_SKU_WINDOWS

#ifdef TOSHI_SKU_WASM
#  include "Platform/WASM/THPTimer_WASM.h"
#endif
