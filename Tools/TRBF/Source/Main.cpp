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
	L"String 1",
	L"String 2",
	L"String 3",
	L"String 4",
	L"String 5",
	L"Abcdefg",
	L"1234567",
};

int TMain(int argc, char** argv)
{
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
	trbf.WriteToFile("D:\\TestLocaleFile.trb", false);
	trbf.WriteToFile("D:\\TestLocaleFile_Compressed.trb", true);

	trbf.ReadFromFile("D:\\TestLocaleFile_Compressed.trb");
	auto pLocaleStrings = pSymb->Find<LocaleStrings>(pSect, "LocaleStrings");

	TOSHI_INFO("String count: {0}", pLocaleStrings->Count);
	TOSHI_INFO(L"First string: {0}", pLocaleStrings->Strings[0]);

	return 0;

#if 0
	// BTEC test
	auto file = Toshi::TFile::Create("F:\\BTEC_ORIGINAL", Toshi::TFile::FileMode_CreateNew);
	pStack->Unlink();
	file->Write(pStack->GetBuffer(), pStack->GetUsedSize());
	file->Destroy();

	file = Toshi::TFile::Create("F:\\BTEC_TEST", Toshi::TFile::FileMode_CreateNew);
	Toshi::TCompress_Compress::Compress(file, pStack->GetBuffer(), pStack->GetUsedSize(), 0, false);
	file->Destroy();

	file = Toshi::TFile::Create("F:\\BTEC_TEST", Toshi::TFile::FileMode_Read);
	auto file2 = Toshi::TFile::Create("F:\\BTEC_DECOMPRESSED", Toshi::TFile::FileMode_CreateNew);

	Toshi::TCompress::Header header;
	Toshi::TCompress_Decompress::GetHeader(file, header);
	char* buffer = new char[header.Size];

	Toshi::TCompress_Decompress::Decompress(file, &header, buffer, header.Size);
	file2->Write(buffer, header.Size);

	file->Destroy();
	file2->Destroy();

	return 0;
#endif
}