#include "pch.h"
#include "StaticLights.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

static GatherStaticLightsFn s_pfnGather = TNULL;
static AddStaticLightsFn    s_pfnAdd    = TNULL;
static ClearStaticLightsFn  s_pfnClear  = TNULL;

void SetStaticLightCallbacks( GatherStaticLightsFn a_pfnGather, AddStaticLightsFn a_pfnAdd, ClearStaticLightsFn a_pfnClear )
{
	s_pfnGather = a_pfnGather;
	s_pfnAdd    = a_pfnAdd;
	s_pfnClear  = a_pfnClear;
}

void GatherStaticLights( const Toshi::TSphere& a_rcBounds, TINT8* a_pOutIDs )
{
	if ( s_pfnGather )
	{
		s_pfnGather( a_rcBounds, a_pOutIDs );
	}
	else
	{
		for ( TINT i = 0; i < MAX_CELL_STATIC_LIGHTS; i++ )
			a_pOutIDs[ i ] = -1;
	}
}

void AddStaticLights( Toshi::TRenderContext* a_pContext, const TINT8* a_pIDs )
{
	if ( s_pfnAdd )
		s_pfnAdd( a_pContext, a_pIDs );
}

void ClearStaticLights( Toshi::TRenderContext* a_pContext )
{
	if ( s_pfnClear )
		s_pfnClear( a_pContext );
}
