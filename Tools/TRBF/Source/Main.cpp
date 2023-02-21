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

class TXUIBuilder
{
public:
    struct Props
    {
        uint32_t Unk1;
        char* XURName;
        char* XURData;
        uint32_t Unk2;
        uint32_t Unk3;
        uint32_t Unk4;
    };

public:
    TXUIBuilder(TLib::TRBF::TRBF& trbf) : m_TRBF(trbf)
    {
        m_Created = TFALSE;
    }

    void Create(const char* xurName)
    {
        TASSERT(m_Created == TFALSE);
        size_t xurNameLength = Toshi::T2String8::Length(xurName);

        auto pSect = m_TRBF.GetSECT();
        auto pSymb = m_TRBF.GetSYMB();

        auto txuiStack = pSect->CreateStack();
        auto xurStack = pSect->CreateStack();

        auto txui = txuiStack->Alloc<TXUIBuilder::Props>();
        pSymb->Add(txuiStack, "txui", txui.get());

        txuiStack->Alloc(&txui->XURName, xurNameLength + 1);
        Toshi::T2String8::Copy(txui->XURName, xurName, xurNameLength);

        txuiStack->SetCrossPointer(&txui->XURData, { xurStack, xurStack->GetBuffer() });

        m_XURStack = xurStack;
        m_TXUIStack = txuiStack;
        m_Created = TTRUE;
    }

    void LoadXUR(const char* filepath)
    {
        TASSERT(m_Created == TTRUE);
        Toshi::TFile* file = Toshi::TFile::Create(filepath, Toshi::TFile::FileMode_Read);
        auto fileSize = file->GetSize();

        char* xurData = new char[fileSize + 1];
        xurData[fileSize] = '\0';

        file->Read(xurData, fileSize);
        file->Destroy();

        auto xurBuffer = m_XURStack->AllocBytes(fileSize);
        Toshi::TUtil::MemCopy(xurBuffer.get(), xurData, fileSize);

        m_Created = TFALSE;
        delete[] xurData;
    }

private:
    Props m_Data;
    TLib::TRBF::TRBF& m_TRBF;
    TLib::TRBF::SECT::Stack* m_TXUIStack;
    TLib::TRBF::SECT::Stack* m_XURStack;
    bool m_Created;
};

int TMain(int argc, char** argv)
{
    const char* a_TargetXUR = "LoadingScreen.xur";
    const char* a_TRBInput = "C:\\SteamLibrary\\steamapps\\common\\de Blob\\Data\\XUI\\US\\LoadingScreen_Original.trb";
    const char* a_XURInput = "C:\\Users\\InfiniteC0re\\Desktop\\XUI_DeBlob\\Sources\\LoadingScreen_Edited.xur";
    const char* a_TRBOutput = "C:\\SteamLibrary\\steamapps\\common\\de Blob\\Data\\XUI\\US\\LoadingScreen.trb";

    TLib::TRBF::TRBF trbf;
    trbf.ReadFromFile(a_TRBInput);

    auto pSect = trbf.GetSECT();
    auto pSymb = trbf.GetSYMB();

    for (size_t i = 0; i < pSymb->GetCount(); i++)
    {
        if (pSymb->Is(i, "txui"))
        {
            auto txui = pSymb->GetByIndex<TXUIBuilder::Props>(pSect, i);
            auto xuib = pSect->GetStack(txui.stack()->GetIndex() + 1);

            if (Toshi::TStringManager::String8Compare(txui->XURName, a_TargetXUR) == 0)
            {
                // Remove the existing txui and xur data
                pSect->DeleteStack(pSymb, txui.stack());
                pSect->DeleteStack(pSymb, xuib);

                // Create txui in the trb file
                TXUIBuilder txuiBuilder(trbf);
                txuiBuilder.Create(a_TargetXUR);
                txuiBuilder.LoadXUR(a_XURInput);

                // Save the output
                trbf.WriteToFile(a_TRBOutput);
                break;
            }
        }
    }

    return 0;
}