#pragma once
#include "Sound/ASound.h"

struct ASoundBreakpoint
{
	TCHAR*   pszModelName;
	TCHAR*   pszAnimationName;
	TFLOAT   fTime;
	ASoundId eSoundId;
	TINT32   Unk1;
	TCHAR**  ppszBoneName; // ?
};

// Stored in TRB files
struct ASoundBreakpointsTable
{
	TUINT32 uiCount;
	ASoundBreakpoint* pBreakpoints;
};
