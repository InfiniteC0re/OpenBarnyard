#include "Toshi.h"
#include "AExampleClass.h"
#include "AApplication.h"
#include "Toshi/Core/Core.h"
#include "Toshi/Math/TMathInline.h"

Toshi::TApplication* Toshi::CreateApplication(int argc, char** argv)
{
	// allocate 64 mbytes for the Toshi memory block
	Toshi::TMemory::TMemory::Initialize(64 * 1024 * 1024);

	auto application = new AApplication;
	TOSHI_INFO("Created application with {0} arguments", argc);

	return application;
}

bool AApplication::OnCreate()
{
	TOSHI_INFO("The application has been created");

	TTODO("Initialize static here");
	Toshi::TClass* tObjectClass = Toshi::TObject::s_Class;
		
#pragma region TClass Info
	TOSHI_INFO("---------------TClass Hierarchy---------------");

	tObjectClass->RecurseTree(
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
	TOSHI_INFO("----------------------------------------------");
#pragma endregion

#pragma region Simple TMemory Test
	bool status = false;
	TOSHI_INFO("TMemory Check:");
	TOSHI_INFO("1) The second allocation should have the same address the first one had");
	TOSHI_INFO("2) Sizes of allocated chunks should be aligned to 4 bytes up");
	TOSHI_INFO("3) The third allocation should have it's own address");
	
	void* ptr1 = Toshi::tmalloc(5);
	TOSHI_INFO("Allocated 5 ({0}) bytes at: 0x{1:X}", TMath::AlignNum((uint32_t)5 + 3), (size_t)ptr1);
	
	status = Toshi::tfree(ptr1);
	TOSHI_INFO("Freed memory at: 0x{0:X} (Status: {1})", (size_t)ptr1, status);
	
	void* ptr2 = Toshi::tmalloc(9);
	TOSHI_INFO("Allocated 9 ({0}) bytes at: 0x{1:X}", TMath::AlignNum((uint32_t)9 + 3), (size_t)ptr2);

	void* ptr3 = Toshi::tmalloc(sizeof(float));
	TOSHI_INFO("Allocated {0} ({1}) bytes at: 0x{2:X}", sizeof(float), TMath::AlignNum((uint32_t)sizeof(float) + 3), (size_t)ptr3);
	
	if (ptr1 == ptr2 && ptr1 != ptr3 && status)
	{
		TOSHI_INFO("TMemory check succeeded");
	}
	else
	{
		TOSHI_CRITICAL("TMemory check failed");
	}
#pragma endregion

	return true;
}

bool AApplication::OnUpdate()
{
	TOSHI_INFO("AApplication::OnUpdate");

	AExampleClass* exampleClass = new AExampleClass();
	exampleClass->Delete();

	return true;
}