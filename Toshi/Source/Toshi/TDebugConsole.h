#pragma once
#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/TDebugConsole_Win.h"
#endif // TOSHI_SKU_WINDOWS

#ifdef TOSHI_SKU_WASM
#  include "Platform/WASM/TDebugConsole_WASM.h"
#endif
