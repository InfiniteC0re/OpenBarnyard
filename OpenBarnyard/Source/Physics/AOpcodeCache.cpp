#include "pch.h"
#include "AOpcodeCache.h"
#include "ACollisionModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00618ea0
AOpcodeCache::AOpcodeCache()
{
	m_uiRayColliderCache = 0xFFFFFFFF;
}

// $Barnyard: FUNCTION 00618a60
AOpcodeCache::~AOpcodeCache()
{
}

