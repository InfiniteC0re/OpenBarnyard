#include "pch.h"
#include "AppBoot.h"
#include "AExampleClass.h"
#include "Locale/ALocaleManager.h"

static Toshi::TSound m_soundSystem = Toshi::TSound();

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	return new AApplication();
}

bool AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Created application with {0} arguments", argc);
	
#pragma region TClass Info
	TOSHI_INFO("===============TClass Hierarchy===============");
	Toshi::TObject::s_Class.RecurseTree(
		[](Toshi::TClass* tClass, void* data) -> bool
		{
			// this callback is called for every registered TClass
			static int s_Index = 1;

			if (tClass->GetParent())
			{
				TOSHI_INFO("{0}) {1} -> {2}", s_Index, tClass->GetParent()->GetName(), tClass->GetName());
			}
			else
			{
				TOSHI_INFO("{0}) None -> {1}", s_Index, tClass->GetName());
			}

			s_Index++;
			return true;
		},
		[](Toshi::TClass* tClass, void* data)
		{
			// this callback is called for every registered base TClass
			// called after all the derrives got into the first callback
		},
		[](Toshi::TClass* tClass, void* data)
		{
			// this callback is called for every registered base TClass
			// called before all the derrives get into the first callback
		}, nullptr);
	TOSHI_INFO("==============================================");
#pragma endregion

	m_Renderer = ARenderer::CreateSingleton();
	bool interfaceCreated = m_Renderer->CreateInterface();

	ALocaleManager::Create();
	
	size_t poolSize = 128 * 1024 * 1024;
	void* mempool = malloc(poolSize);

	bool bResult = m_soundSystem.Create(mempool, poolSize, -1, -1, 2);
	TASSERT(TTRUE == bResult, "");

<<<<<<< HEAD
	Toshi::TMSWindow window;

	ALevelInformation info;
	info.Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\Levels.trb");

	renderer.Create();
	renderer.Update();

=======
>>>>>>> b19878c8db5405a44256abff728c3dce985f73ec
	AExampleClass* exampleClass = new AExampleClass();
	exampleClass->Delete();
	
	return true;
}

bool AApplication::OnUpdate()
{
	TOSHI_INFO("AApplication::OnUpdate");

	return true;
}