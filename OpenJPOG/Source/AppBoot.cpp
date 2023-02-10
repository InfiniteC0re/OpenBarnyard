#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE 640 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// Including the entrypoint
#include <Toshi.h>

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include <Platform/Windows/TSound_Win.h>
#include TOSHI_MULTIRENDER(TRender)

AApplication AApplication::g_oTheApp;

bool AApplication::OnCreate(int argc, char** argv)
{
	return true;
}

bool AApplication::OnUpdate(float deltaTime)
{
	return true;
}