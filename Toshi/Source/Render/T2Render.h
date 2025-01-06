#pragma once

#ifdef TRENDERINTERFACE_GL

#  define TOSHI_USES_CUSTOM_RENDERER
#  include "T2RenderCommon.h"
#  include "Platform/GL/T2Render_GL.h"

TSTATICASSERT( Toshi::T2Render::API == Toshi::T2RenderAPI::OpenGL );

#endif // TRENDERINTERFACE_GL
