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
#include <filesystem>

struct TXUI
{
    uint32_t Unk1;
    char* XURName;
    char* XURData;
    uint32_t Unk2;
    uint32_t Unk3;
    uint32_t Unk4;
};

int TMain(int argc, char** argv)
{
    TLib::TRBF::TRBF trbf;
    trbf.ReadFromFile("C:\\SteamLibrary\\steamapps\\common\\de Blob\\Data\\XUI\\US\\LoadingScreen_Original.trb");

    auto pSect = trbf.GetSECT();
    auto pSymb = trbf.GetSYMB();

    auto txuiIndex = pSymb->FindIndex(pSect, "txui");
    TASSERT(txuiIndex != -1);

    auto txui = pSymb->GetByIndex<TXUI>(pSect, txuiIndex);
    auto xuib = pSect->GetStack(txui.stack()->GetIndex() + 1);

    auto oldXurName = Toshi::TStringManager::GetTempString8();
    size_t xurNameLength = Toshi::T2String8::Length(txui->XURName);

    Toshi::T2String8::Copy(oldXurName, txui->XURName);

    // Remove the existing txui and xur file
    pSect->DeleteStack(pSymb, txui.stack());
    pSect->DeleteStack(pSymb, xuib);
    
    // Create stack for txui
    auto pTxuiStack = pSect->CreateStack();
    auto newTxui = pTxuiStack->Alloc<TXUI>();
    pSymb->Add(pTxuiStack, "txui", newTxui.get());

    auto xurName = pTxuiStack->Alloc(&newTxui->XURName, xurNameLength + 1);
    Toshi::T2String8::Copy(xurName.get(), oldXurName, xurNameLength);

    // Read xur file
    Toshi::TFile* file = Toshi::TFile::Create("C:\\Users\\InfiniteC0re\\Desktop\\XUI_DeBlob\\Sources\\test.xur", Toshi::TFile::FileMode_Read);
    int size = file->GetSize();

    char* buffer = new char[size + 1];
    buffer[size] = '\0';

    file->Read(buffer, size);
    file->Destroy();
    
    // Create stack for xur file
    auto pXurStack = pSect->CreateStack();
    auto xurFileData = pXurStack->AllocBytes(size);
    Toshi::TUtil::MemCopy(xurFileData.get(), buffer, size);

    // Link xur data to txui
    pTxuiStack->SetCrossPointer(&newTxui->XURData, { xurFileData.stack(), xurFileData.get()});

    trbf.WriteToFile("C:\\SteamLibrary\\steamapps\\common\\de Blob\\Data\\XUI\\US\\LoadingScreen.trb");

    return 0;
}