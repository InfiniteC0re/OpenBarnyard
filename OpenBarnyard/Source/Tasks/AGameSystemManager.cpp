#include "pch.h"
#include "AGameSystemManager.h"
#include "Assets/AAssetStreaming.h"
#include "Terrain/ATerrain.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS(AGameSystemManager);

TBOOL AGameSystemManager::OnCreate()
{
	TIMPLEMENT();
	AAssetStreaming::CreateSingleton()->SetFlag(TFALSE);

	return TTRUE;
}

TBOOL AGameSystemManager::OnUpdate(TFLOAT a_fDeltaTime)
{
	TIMPLEMENT();

	AAssetStreaming::GetSingleton()->Update();

	if (ATerrain::IsSingletonCreated())
	{
		ATerrain::GetSingleton()->Update();
	}

	return TTRUE;
}
