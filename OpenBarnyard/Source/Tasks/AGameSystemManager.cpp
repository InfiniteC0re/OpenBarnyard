#include "pch.h"
#include "AGameSystemManager.h"
#include "Assets/AAssetStreaming.h"

TBOOL AGameSystemManager::OnCreate()
{
	TIMPLEMENT();
	AAssetStreaming::CreateSingleton()->SetFlag(TFALSE);

	return TTRUE;
}
