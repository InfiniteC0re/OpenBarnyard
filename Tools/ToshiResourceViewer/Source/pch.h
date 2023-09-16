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

/* Toshi Header Files */

#include <Toshi/Typedefs.h>
#include "Toshi/Core/STL.h"
#include <Toshi/Core/Core.h>
#include <Toshi/Core/TObject.h>
#include "Toshi/Core/TStack.h"
#include <Toshi/Core/TTask.h>
#include <Toshi/Memory/TMemory.h>
#include <Toshi/Math/Math.h>
#include <Toshi/Utils/TLog.h>
#include <Toshi/Utils/TUtil.h>
#include <Toshi/Strings/TStringManager.h>

#ifdef TOSHI_SKU_WINDOWS

#ifdef TOSHI_RENDERER_DX11
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#define TRENDER TRenderDX11
#endif // TOSHI_RENDERER_DX11

#ifdef TOSHI_RENDERER_OPENGL
#include <Platform/SDL/TRender_SDL.h>
#include <Platform/SDL/TRenderContext_SDL.h>
#define TRENDER TRenderSDL
#endif // TOSHI_RENDERER_OPENGL

#endif // TOSHI_SKU_WINDOWS
