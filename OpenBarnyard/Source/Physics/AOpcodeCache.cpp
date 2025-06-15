#include "pch.h"
#include "AOpcodeCache.h"
#include "ACollisionModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AOpcodeCache::AOpcodeCache()
{
	m_uiRayColliderCache = 0xFFFFFFFF;
}

AOpcodeCache::~AOpcodeCache()
{
}

