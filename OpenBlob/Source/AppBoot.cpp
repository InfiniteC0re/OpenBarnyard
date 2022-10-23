#include "pch.h"
#include "AppBoot.h"
#include "Locale/ALocaleManager.h"

static Toshi::TSound m_soundSystem = Toshi::TSound();

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	// allocate 64 mbytes for the Toshi memory block
	Toshi::TMemory memorySettings;
	memorySettings.Init();

	auto application = new AApplication();
	TOSHI_INFO("Created application with {0} arguments", argc);

	return application;
}

bool AApplication::OnCreate()
{
	TOSHI_INFO("The application has been created");
	TTODO("Initialize static here");
	
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

#pragma region TMemory Test
	// force chunks to defragment
	TFree(TMalloc(32));

	TOSHI_INFO("TMemory Check:");
	TOSHI_INFO("1) The second allocation should have the same address the first one had");
	TOSHI_INFO("2) Sizes of allocated chunks should be aligned to 4 bytes up");
	TOSHI_INFO("3) The third allocation should have it's own address");
	
	void* ptr1 = TMalloc(5);
	TOSHI_INFO("Allocated 5 ({0}) bytes at: 0x{1:X}", TMath::AlignNumUp(5), (size_t)ptr1);
	
	TFree(ptr1);
	TOSHI_INFO("Freed memory at: 0x{0:X}", (size_t)ptr1);
	
	void* ptr2 = TMalloc(5);
	TOSHI_INFO("Allocated 9 ({0}) bytes at: 0x{1:X}", TMath::AlignNumUp(9), (size_t)ptr2);

	void* ptr3 = TMalloc(sizeof(float));
	TOSHI_INFO("Allocated {0} ({1}) bytes at: 0x{2:X}", sizeof(float), TMath::AlignNumUp(sizeof(float)), (size_t)ptr3);
	
	if (ptr1 == ptr2 && ptr1 != ptr3)
	{
		TOSHI_INFO("TMemory check succeeded");
	}
	else
	{
		TOSHI_CRITICAL("TMemory check failed");
	}
#pragma endregion

	Toshi::TRenderInterface renderer;

	
	void* mempool = malloc(0x8000000);

	bool bResult = m_soundSystem.Create(mempool, 0x8000000, -1, -1, 2);
	TASSERT(TTRUE == bResult, "");
	
	return true;
}

bool AApplication::OnUpdate()
{
	TOSHI_INFO("AApplication::OnUpdate");

	AExampleClass* exampleClass = new AExampleClass();
	exampleClass->Delete();

	return true;
}