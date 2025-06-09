#include "pch.h"
#include "ATerrainPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ATerrainPhysicsManager::ATerrainPhysicsManager()
{
}

ATerrainPhysicsManager::~ATerrainPhysicsManager()
{
}

ATerrainPhysicsManager* g_pTerrainPhysicsManager = TNULL;

void ATerrainPhysicsManager::Create()
{
	TASSERT( g_pTerrainPhysicsManager == TNULL );

	g_pTerrainPhysicsManager = new ATerrainPhysicsManager();
}
