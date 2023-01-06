#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"
#include "TRBF/TRBF.h"

struct LocaleStrings
{
	uint32_t Count;
	wchar_t** Strings;
};

const wchar_t* g_Strings[] = {
	L"String 1 2 3 4 5",
	L"String 2",
	L"String 3",
	L"String 4",
	L"String 5",
	L"Abcdefg",
	L"1234567",
};

static uint8_t g_Func1[] = {
	0xc7, 0x01, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0x41, 0x0c, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0x41, 0x14, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0x81, 0x48, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xc7, 0x81, 0x4C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xc3
};

typedef void(__thiscall *t_Func1)(void* pThis);

int TMain(int argc, char** argv)
{
	/*DWORD oldProtect;
	VirtualProtect(g_Func1, sizeof(g_Func1), PAGE_EXECUTE_READWRITE, &oldProtect);
	t_Func1 func1 = (t_Func1)(&g_Func1[0]);

	char a[1104];
	func1((void*)a);

	return 0;*/

	TLib::TRBF::TRBF trbf;

	auto pSect = trbf.GetSECT();
	auto pSymb = trbf.GetSYMB();
	auto pStack = pSect->CreateSection();

	auto pLocale = pStack->Alloc<LocaleStrings>();
	pLocale->Count = sizeof(g_Strings) / sizeof(*g_Strings);
	pLocale->Strings = TNULL;

	pStack->Alloc<wchar_t*>(&pLocale->Strings, pLocale->Count);

	for (size_t i = 0; i < pLocale->Count; i++)
	{
		pStack->Alloc<wchar_t>(&pLocale->Strings[i], wcslen(g_Strings[i]) + 1);
		wcscpy(pLocale->Strings[i], g_Strings[i]);
	}

	pSymb->Add(pStack, "LocaleStrings", pLocale.get());

	// BTEC test
	auto file = Toshi::TFile::Create("D:\\BTEC_ORIGINAL", Toshi::TFile::FileMode_CreateNew);
	pStack->Unlink();
	file->Write(pStack->GetBuffer(), pStack->GetUsedSize());
	file->Destroy();

	file = Toshi::TFile::Create("D:\\BTEC_TEST", Toshi::TFile::FileMode_CreateNew);
	Toshi::TCompress_Compress::Compress(file, pStack->GetBuffer(), pStack->GetUsedSize(), 0, false);
	file->Destroy();

	file = Toshi::TFile::Create("D:\\BTEC_TEST", Toshi::TFile::FileMode_Read);
	auto file2 = Toshi::TFile::Create("D:\\BTEC_DECOMPRESSED", Toshi::TFile::FileMode_CreateNew);

	Toshi::TCompress::Header header;
	Toshi::TCompress_Decompress::GetHeader(file, header);
	char* buffer = new char[header.Size];

	Toshi::TCompress_Decompress::Decompress(file, &header, buffer, header.Size);
	file2->Write(buffer, header.Size);

	file->Destroy();
	file2->Destroy();

	return 0;
}