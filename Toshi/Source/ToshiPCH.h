#pragma once

#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>
#include <utility>
#include <cstdarg>

#include "Toshi/Toshi.h"

/* Platform specific headers */
#ifdef TOSHI_SKU_WINDOWS
#  include <WinSock2.h>
#  include <Windows.h>
#endif

#ifdef TRENDERINTERFACE_GL
#  include "Platform/GL/T2Render_GL.h"
#  include "Platform/GL/T2GLTexture_GL.h"
#  include "Platform/GL/T2FrameBuffer_GL.h"
#  include "Platform/GL/T2RenderBuffer_GL.h"
#  include "Platform/GL/T2RenderContext_GL.h"
#endif
