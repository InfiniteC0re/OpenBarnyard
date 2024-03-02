#include "pch.h"
#include "AGameSystemManager.h"
#include "Assets/AAssetStreaming.h"
#include "Render/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Terrain/ATerrain.h"

#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS(AGameSystemManager);

TOSHI_NAMESPACE_USING

TBOOL AGameSystemManager::OnCreate()
{
	TIMPLEMENT();

	TGetClass(AModelRepos).CreateObject();

	auto pScheduler = g_oSystemManager.GetScheduler();
	pScheduler->CreateTask<ACameraManager>(this)->Create();

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
