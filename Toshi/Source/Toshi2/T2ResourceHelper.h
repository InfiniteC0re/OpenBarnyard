#pragma once
#include "Toshi2/T2ResourceManager.h"

namespace Toshi {

#ifdef TOSHI_SKU_WINDOWS
	// FIXME: TTexture is not a T2Resource since Blitworks only adapted Barnyard code for the de Blob port
	using T2TexturePtr = T2ResPtr<void>;
#else
	using T2TexturePtr = T2ResPtr<class TTexture>;
#endif

}
