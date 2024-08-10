#include "ToshiPCH.h"
#include "TRegion.h"

TOSHI_NAMESPACE_START

static TREGION s_ToshiRegion;
static TBOOL s_bRegionLocked;

TREGION TRegion_GetRegion()
{
	return s_ToshiRegion;
}

void TRegion_SetRegion( TREGION a_eRegion )
{
	TASSERT( !s_bRegionLocked );
	s_ToshiRegion = a_eRegion;
}

void TRegion_LockRegion()
{
	s_bRegionLocked = TTRUE;
}

void TRegion_UnlockRegion()
{
	s_bRegionLocked = TFALSE;
}

TOSHI_NAMESPACE_END
