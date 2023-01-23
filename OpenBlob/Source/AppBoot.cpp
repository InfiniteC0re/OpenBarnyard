#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE 640 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// Including the entrypoint
#include <Toshi.h>

// Including everything else
#include "AppBoot.h"
#include "AAssetStreaming.h"
#include "AExampleClass.h"

#include <Toshi/Sound/TSound.h>
#include <Toshi2/T2RedBlackTree.h>
#include TOSHI_MULTIRENDER(TRender)

static Toshi::TSound m_soundSystem = Toshi::TSound();

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	return new AApplication();
}

bool AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Starting Blob...");

	ARenderer::CreateSingleton();
	AAssetStreaming::CreateSingleton();
	ALocaleManager::Create();

	m_Renderer = ARenderer::GetSingleton();
	bool interfaceCreated = m_Renderer->CreateInterface();
	
	size_t poolSize = 128 * 1024 * 1024;
	void* mempool = malloc(poolSize);

	bool bResult = m_soundSystem.Create(mempool, poolSize, -1, -1, 2);
	TASSERT(TTRUE == bResult);

	Toshi::T2RedBlackTree<int> RBT(&Toshi::T2Allocator::s_GlobalAllocator);

	Toshi::T2RedBlackTreeNode<int>* pInsertedValue;
	RBT.Insert(pInsertedValue, 8);
	RBT.Insert(pInsertedValue, 9);
	RBT.Insert(pInsertedValue, 16);
	RBT.Insert(pInsertedValue, 7);

	TApplication::OnCreate(argc, argv);
	//ALevelInformation info = ALevelInformation();
	//info.Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\Levels.trb");

	AExampleClass* exampleClass = new AExampleClass();
	exampleClass->Delete();
	
	return true;
}

bool AApplication::OnUpdate(float deltaTime)
{
	ARenderer::GetSingletonWeak()->Update(deltaTime);
	return true;
}