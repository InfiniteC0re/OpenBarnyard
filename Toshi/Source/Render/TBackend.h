#pragma once

//-----------------------------------------------------------------------------
// The render backend (HAL) of the engine this build is made for.
// Files outside the backend folders include this header instead of picking
// Platform/<Backend>/ headers themselves, so a new backend is only added here.
//-----------------------------------------------------------------------------

#if defined( TRENDERINTERFACE_GL )
#  define TRenderInterfaceHAL TRenderGLInterface
#  define TRenderContextHAL   TRenderContextGL
#  define TRenderAdapterHAL   TGLAdapter
#  define TWindowHAL          TGLWindow
#  define TDebugFontHAL       TGLFont
#else
#  define TRenderInterfaceHAL TRenderD3DInterface
#  define TRenderContextHAL   TRenderContextD3D
#  define TRenderAdapterHAL   TD3DAdapter
#  define TWindowHAL          TMSWindow
#  define TDebugFontHAL       CD3DFont
#endif

#if defined( TRENDERINTERFACE_DX8 )
#  include "Platform/DX8/TRenderInterface_DX8.h"
#  include "Platform/DX8/TRenderAdapter_DX8.h"
#  include "Platform/DX8/TRenderContext_DX8.h"
#  include "Platform/DX8/TRenderCapture_DX8.h"
#  include "Platform/DX8/TDebugText_DX8.h"
#  include "Platform/DX8/TViewport_DX8.h"
#  include "Platform/DX8/TMSWindow.h"
#  include "Platform/DX8/TMouse_D3D8_Win.h"
#  include "Platform/DX8/TModel_DX8.h"
#  include "Platform/DX8/T2Texture_DX8.h"
#  include "Platform/DX8/TTextureResourceHAL_DX8.h"
#  include "Platform/DX8/TTextureFactoryHAL_DX8.h"
#  include "Platform/DX8/TVertexBlockResource_DX8.h"
#  include "Platform/DX8/TVertexPoolResource_DX8.h"
#  include "Platform/DX8/TVertexFactoryResource_DX8.h"
#  include "Platform/DX8/TIndexBlockResource_DX8.h"
#  include "Platform/DX8/TIndexPoolResource_DX8.h"
#  include "Platform/DX8/TIndexFactoryResource_DX8.h"
#elif defined( TRENDERINTERFACE_DX11 )
#  include "Platform/DX11/TRenderInterface_DX11.h"
#  include "Platform/DX11/TRenderAdapter_DX11.h"
#  include "Platform/DX11/TRenderContext_DX11.h"
#  include "Platform/DX11/TRenderCapture_DX11.h"
#  include "Platform/DX11/TDebugText_DX11.h"
#  include "Platform/DX11/TViewport_DX11.h"
#  include "Platform/DX11/TMSWindow.h"
#  include "Platform/DX11/TMouse_D3D8_Win.h"
#  include "Platform/DX11/TModel_DX11.h"
#  include "Platform/DX11/T2Texture_DX11.h"
#  include "Platform/DX11/TTextureResourceHAL_DX11.h"
#  include "Platform/DX11/TTextureFactoryHAL_DX11.h"
#  include "Platform/DX11/TVertexBlockResource_DX11.h"
#  include "Platform/DX11/TVertexPoolResource_DX11.h"
#  include "Platform/DX11/TVertexFactoryResource_DX11.h"
#  include "Platform/DX11/TIndexBlockResource_DX11.h"
#  include "Platform/DX11/TIndexPoolResource_DX11.h"
#  include "Platform/DX11/TIndexFactoryResource_DX11.h"
#elif defined( TRENDERINTERFACE_GL )
#  include "Platform/GL/TRenderInterface_GL.h"
#  include "Platform/GL/TRenderAdapter_GL.h"
#  include "Platform/GL/TRenderContext_GL.h"
#  include "Platform/GL/TRenderCapture_GL.h"
#  include "Platform/GL/TDebugText_GL.h"
#  include "Platform/GL/TViewport_GL.h"
#  ifdef TOSHI_SKU_WINDOWS
#	include "Platform/GL/TMouse_GL.h"
#  endif
#  include "Platform/GL/TModel_GL.h"
#  include "Platform/GL/T2Texture_GL.h"
#  include "Platform/GL/TTextureResourceHAL_GL.h"
#  include "Platform/GL/TTextureFactoryHAL_GL.h"
#  include "Platform/GL/TVertexBlockResource_GL.h"
#  include "Platform/GL/TVertexPoolResource_GL.h"
#  include "Platform/GL/TVertexFactoryResource_GL.h"
#  include "Platform/GL/TIndexBlockResource_GL.h"
#  include "Platform/GL/TIndexPoolResource_GL.h"
#  include "Platform/GL/TIndexFactoryResource_GL.h"
#endif
