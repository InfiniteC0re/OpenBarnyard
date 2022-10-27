#include "pch.h"
#include "AppBoot.h"
#include "Locale/ALocaleManager.h"
#include <Toshi/Core/TMSWindow.h>

static Toshi::TSound m_soundSystem = Toshi::TSound();

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	return new AApplication();
}

bool AApplication::OnCreate(int argc, char** argv)
{
	TOSHI_INFO("Created application with {0} arguments", argc);

	ALocaleManager::Create();
	
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

	Toshi::TRenderInterface renderer;
	
	size_t poolSize = 128 * 1024 * 1024;
	void* mempool = malloc(poolSize);

	bool bResult = m_soundSystem.Create(mempool, poolSize, -1, -1, 2);
	TASSERT(TTRUE == bResult, "");

	Toshi::TMSWindow window;

	window.Create(&renderer, "le Blob");

	AExampleClass* exampleClass = new AExampleClass();
	exampleClass->Delete();
	
	return true;
}

bool AApplication::OnUpdate()
{
	TOSHI_INFO("AApplication::OnUpdate");

	return true;
}