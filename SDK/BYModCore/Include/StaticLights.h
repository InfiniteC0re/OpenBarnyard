#pragma once
#include "ModLoader.h"

#include <Render/TRenderContext.h>

using GatherStaticLightsFn = void ( * )( const Toshi::TSphere& a_rcBounds, Toshi::TLightIDList& a_rOutList );
using AddStaticLightsFn    = void ( * )( Toshi::TRenderContext* a_pContext, const Toshi::TLightIDList& a_rIDs );
using ClearStaticLightsFn  = void ( * )( Toshi::TRenderContext* a_pContext );

MODCORE_API void SetStaticLightCallbacks( GatherStaticLightsFn a_pfnGather, AddStaticLightsFn a_pfnAdd, ClearStaticLightsFn a_pfnClear );

void GatherStaticLights( const Toshi::TSphere& a_rcBounds, Toshi::TLightIDList& a_rOutList );
void AddStaticLights( Toshi::TRenderContext* a_pContext, const Toshi::TLightIDList& a_rIDs );
void ClearStaticLights( Toshi::TRenderContext* a_pContext );
