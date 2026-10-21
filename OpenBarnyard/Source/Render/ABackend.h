#pragma once
#include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// The render backend (HAL) of the game this build is made for, the engine one
// included. Files outside the backend folders include this header instead of
// picking Platform/<Backend>/ headers themselves, so a new backend is only
// added here.
//-----------------------------------------------------------------------------

#if defined( TRENDERINTERFACE_DX8 )
#  include "Platform/DX8/AGUI2Renderer_DX8.h"
#  include "Platform/DX8/AFXShader/AFXShader_DX8.h"
#  include "Platform/DX8/AFXShader/AFXMaterial_DX8.h"
#  include "Platform/DX8/AFXShader/AFXMesh_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassShaderHAL_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassMaterialHAL_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassMeshHAL_DX8.h"
#  include "Platform/DX8/ALineShader/ALineShaderHAL_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinShader_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinMesh_DX8.h"
#  include "Platform/DX8/AStaticInstanceShader/AStaticInstanceShaderHAL_DX8.h"
#  include "Platform/DX8/AStaticInstanceShader/AStaticInstanceMaterialHAL_DX8.h"
#  include "Platform/DX8/AStaticInstanceShader/AStaticInstanceMeshHAL_DX8.h"
#  include "Platform/DX8/ASysShader/ASysShader_DX8.h"
#  include "Platform/DX8/ASysShader/ASysMaterial_DX8.h"
#  include "Platform/DX8/ASysShader/ASysMesh_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldMaterial_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldMesh_DX8.h"
#elif defined( TRENDERINTERFACE_DX11 )
#  include "Platform/DX11/AGUI2Renderer_DX11.h"
#  include "Platform/DX11/AFXShader/AFXShader_DX11.h"
#  include "Platform/DX11/AFXShader/AFXMaterial_DX11.h"
#  include "Platform/DX11/AFXShader/AFXMesh_DX11.h"
#  include "Platform/DX11/AGrassShader/AGrassShaderHAL_DX11.h"
#  include "Platform/DX11/AGrassShader/AGrassMaterialHAL_DX11.h"
#  include "Platform/DX11/AGrassShader/AGrassMeshHAL_DX11.h"
#  include "Platform/DX11/ALineShader/ALineShaderHAL_DX11.h"
#  include "Platform/DX11/ASkinShader/ASkinShader_DX11.h"
#  include "Platform/DX11/ASkinShader/ASkinMaterial_DX11.h"
#  include "Platform/DX11/ASkinShader/ASkinMesh_DX11.h"
#  include "Platform/DX11/AStaticInstanceShader/AStaticInstanceShaderHAL_DX11.h"
#  include "Platform/DX11/AStaticInstanceShader/AStaticInstanceMaterialHAL_DX11.h"
#  include "Platform/DX11/AStaticInstanceShader/AStaticInstanceMeshHAL_DX11.h"
#  include "Platform/DX11/ASysShader/ASysShader_DX11.h"
#  include "Platform/DX11/ASysShader/ASysMaterial_DX11.h"
#  include "Platform/DX11/ASysShader/ASysMesh_DX11.h"
#  include "Platform/DX11/AWorldShader/AWorldShader_DX11.h"
#  include "Platform/DX11/AWorldShader/AWorldMaterial_DX11.h"
#  include "Platform/DX11/AWorldShader/AWorldMesh_DX11.h"
#endif
