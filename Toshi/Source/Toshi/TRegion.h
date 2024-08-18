#pragma once

TOSHI_NAMESPACE_START

enum TREGION
{
	TREGION_UK,
};

TOSHI_API TREGION TRegion_GetRegion();
TOSHI_API void    TRegion_SetRegion( TREGION a_eRegion );
TOSHI_API void    TRegion_LockRegion();
TOSHI_API void    TRegion_UnlockRegion();

TOSHI_NAMESPACE_END
