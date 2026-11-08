#pragma once

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/T2Atomic_Win.h"
#endif // TOSHI_SKU_WINDOWS

#ifdef TOSHI_SKU_WASM
#  include "Platform/WASM/T2Atomic_WASM.h"
#endif
