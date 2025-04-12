#pragma once

#include "d3d8_include.h"
#include "d3d8_wrapped_object.h"

#include <array>
#include <vector>
#include <type_traits>
#include <unordered_map>

namespace dxvk {

  class D3D8Interface;

  struct D3D8VertexShaderInfo;

  using D3D8DeviceBase = D3D8WrappedObject<d3d9::IDirect3DDevice9, IDirect3DDevice8>;
  class D3D8Device final : public D3D8DeviceBase {

    friend class D3D8StateBlock;

  private:

  };

}
