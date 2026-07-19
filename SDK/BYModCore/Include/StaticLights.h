#pragma once
#include "ModLoader.h"

#include <Render/TRenderContext.h>

inline static constexpr TINT MAX_CELL_STATIC_LIGHTS = 8;

using GatherStaticLightsFn = void ( * )( const Toshi::TSphere& a_rcBounds, TINT8* a_pOutIDs );
using AddStaticLightsFn    = void ( * )( Toshi::TRenderContext* a_pContext, const TINT8* a_pIDs );
using ClearStaticLightsFn  = void ( * )( Toshi::TRenderContext* a_pContext );

MODCORE_API void SetStaticLightCallbacks( GatherStaticLightsFn a_pfnGather, AddStaticLightsFn a_pfnAdd, ClearStaticLightsFn a_pfnClear );

void GatherStaticLights( const Toshi::TSphere& a_rcBounds, TINT8* a_pOutIDs );
void AddStaticLights( Toshi::TRenderContext* a_pContext, const TINT8* a_pIDs );
void ClearStaticLights( Toshi::TRenderContext* a_pContext );
