#include "ToshiPCH.h"
#include "TXUIResource.h"
#include "TXUIElement.h"
#include "TXUICanvas.h"
#include "Toshi/Xui/TXUIScene.h"

namespace Toshi
{
    TQuaternion* TXUIResource::GetQuat(int a_iIndex)
    {
        TASSERT(a_iIndex >= -1, "");
        return a_iIndex == -1 ? (TQuaternion*)&TQuaternion::IDENTITY : &m_pQuat[a_iIndex];
    }

    bool TXUIResource::ReadHeader(unsigned char* buffer)
    {
        m_oHeader.m_uiFileID = PARSEDWORD(buffer);

        TASSERT(m_oHeader.m_uiFileID == IDXUR, "Not a .xur file!");

        m_oHeader.m_uiUnk = PARSEDWORD_BIG(buffer + 0x4);
        m_oHeader.m_uiUnk2 = PARSEDWORD_BIG(buffer + 0xA);
        m_oHeader.m_uiSize = PARSEDWORD_BIG(buffer + 0xE);
        m_oHeader.m_usUnk4 = PARSEDWORD_BIG(buffer + 0x8);
        m_oHeader.m_usNumSections = PARSEWORD_BIG(buffer + 0x12);

        TASSERT(m_oHeader.m_usNumSections > 0, "There must be one or more Sections");

        m_oHeader.m_apSections = new (TXUI::MemoryBlock()) Section[m_oHeader.m_usNumSections];

        uint32_t sectionID = PARSEDWORD(buffer + 0x14);

        buffer += 0x14;

        if (sectionID != IDXURSTRING && sectionID != IDXURVEC && sectionID != IDXURQUAT && sectionID != IDXURCUST)
        {
            buffer += 0x3C;
        }

        for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
        {
            m_oHeader.m_apSections[i].m_uiSectionID = PARSEDWORD(buffer);

            TASSERT(
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA,
                "Invalid Section ID"
            );

            m_oHeader.m_apSections[i].m_uiOffset = PARSEDWORD_BIG(buffer + 4);
            m_oHeader.m_apSections[i].m_uiSize = PARSEDWORD_BIG(buffer + 8);

            buffer += sizeof(Section);
        }

        return true;
    }


    void TXUIResource::Load(bool loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, bool loadTrb, void* unk3)
    {
        Destroy();

        if (loadStringTables && fileNameStringTable != TNULL)
        {
            TTODO("Load Stringtables");
        }

        const char* extension = loadTrb ? ".trb" : ".xur";
        const char* fullFileName = strstr(filenameXUIB, extension);

        if (!loadTrb)
        {
            auto file = TFile::Create(fullFileName, TFile::OpenFlags_Read);

            if (file != TNULL)
            {
                int size = file->GetSize();
                unsigned char* buffer = (unsigned char*)TMalloc(size);
                file->Read(buffer, size);
                file->Destroy();
                bool bRes = ReadHeader(buffer);

                if (bRes)
                {
                    for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
                    {
                        TTODO("Finish LoadXUIB2");
                    }
                }
            }
        }
    }

    int TXUIResource::Load(unsigned char* buffer)
    {
        Destroy();
        ReadHeader(buffer);

        int totalSize = m_oHeader.m_usNumSections * sizeof(Section) + 4;

        for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
        {
            unsigned char* currentSectionBuffer = buffer + m_oHeader.m_apSections[i].m_uiOffset;

            if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT)
            {
                totalSize += (m_oHeader.m_apSections[i].m_uiSize / sizeof(TQuaternion)) * 4 + 4;
            }
            else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST)
            {
                ReadCustSection(currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            }
            else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA)
            {
                ReadDataSection(currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            }
            else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING)
            {
                ReadStringSection((wchar_t*)currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
                totalSize += GetStringTableSize(currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            }
            else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC)
            {
                totalSize += (m_oHeader.m_apSections[i].m_uiSize / sizeof(TVector3)) * 8 + 0xC;
            }
        }

        return totalSize;
    }

    int TXUIResource::ReadDataSection(unsigned char* buffer, uint32_t size)
    {
        uint16_t uiType = PARSEWORD_BIG(buffer);
        buffer += 2;

        TASSERT(0 == TStringManager::String16Compare(GetString(uiType), _TS16(XuiCanvas)), "");

        m_root = CreateObjectData(*this, uiType);

        uint8_t opcode = *buffer++;

        m_root->Load(*this, buffer);

        if ((opcode & 2) != 0)
        {
            m_root->LoadChildren(*this, buffer);
        }
        if ((opcode & 4) != 0 && m_root->LoadNamedFrames(*this, buffer) && (opcode & 2) != 0)
        {
            m_root->LoadTimelines(*this, buffer);
        }
        return true;
    }

    bool TXUIResource::ReadStringSection(wchar_t* pPtr, uint32_t size)
    {
        TASSERT(TNULL == m_asStringTable, "StringTable must not be initialized");

        wchar_t* pStart = pPtr;
        wchar_t* pEnd = pPtr + (size / 2);
        m_uiStringTableCount = 1;

        while (pStart < pEnd)
        {
            wchar_t stringLength = PARSEWORD_BIG((uint8_t*)pStart);
            m_uiStringTableCount++;
            pStart += stringLength + 1;
        }

        m_asStringTable = new (TXUI::MemoryBlock()) wchar_t* [m_uiStringTableCount];

        m_asStringTable[0] = new (TXUI::MemoryBlock()) wchar_t[1];

        m_asStringTable[0][0] = L'\0';

        for (size_t i = 1; i < m_uiStringTableCount; i++)
        {
            TASSERT(pPtr < pEnd, "Pointer overflow");

            uint16_t stringLength = PARSEWORD_BIG((uint8_t*)pPtr);
            uint16_t size = stringLength * sizeof(wchar_t) + sizeof(wchar_t);

            m_asStringTable[i] = new (TXUI::MemoryBlock()) wchar_t[size];

            // Blob would call FUN_006eba70 but MemCopy seems better tbh
            TUtil::MemCopy(m_asStringTable[i], pPtr + 1, size - sizeof(wchar_t));

            pPtr += stringLength + 1;
        }

        return true;
    }

    int TXUIResource::ReadCustSection(unsigned char* buffer, uint32_t size)
    {
        m_pCust = static_cast<uint8_t*>(TMemoryHeap::Malloc(TXUI::MemoryBlock(), size));
        TUtil::MemCopy(m_pCust, buffer, size);
        m_uiCustDataSize = size;
        return 1;
    }

    int TXUIResource::GetStringTableSize(unsigned char* pPtr, uint32_t size)
    {
        TIMPLEMENT_D("TXUIResource::GetStringTableSize");
        return 0;
    }

    void TXUIResource::PushID(const wchar_t* a_wsID)
    {
        m_uIDs.Push(a_wsID);
        //if (m_pQuat >> 8 != 0)
    }

    XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, uint16_t index)
    {
        if (index == 0) return TNULL;
        return CreateObjectData(a_rResource, a_rResource.m_pData[index]);
    }

    XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, const wchar_t* objectName)
    {
        if (TStringManager::String16Compare(objectName, _TS16("XuiCanvas"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUICanvasData();
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiScene"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUISceneData();
        }
        return TNULL;
    }

}