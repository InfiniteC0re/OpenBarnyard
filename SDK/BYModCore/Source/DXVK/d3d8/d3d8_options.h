#pragma once

#include "d3d8_include.h"
#include "../d3d9/d3d9_bridge.h"

namespace dxvk {

  struct D3D8Options {

    /// Override application vertex shader declarations.
    std::vector<std::pair<D3DVSDE_REGISTER, D3DVSDT_TYPE>> forceVsDecl;

    /// Enable/disable the drawcall batcher.
    bool batching;

    /// Place all P8 textures in D3DPOOL_SCRATCH.
    bool placeP8InScratch;

    /// Ignore D3DLOCK_DISCARD for everything except D3DUSAGE_DYNAMIC + D3DUSAGE_WRITEONLY buffers.
    bool forceLegacyDiscard;

    /// Force D3DTTFF_PROJECTED for the necessary stages when a depth texture is bound to slot 0.
    bool shadowPerspectiveDivide;

  };

}
