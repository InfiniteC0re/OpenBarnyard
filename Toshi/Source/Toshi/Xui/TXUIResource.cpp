#include "ToshiPCH.h"
#include "TXUIResource.h"
#include "TXUIElement.h"
#include "TXUICanvas.h"
#include "Toshi/Xui/TXUIScene.h"
#include "Toshi/Xui/TXUI.h"
#include "TXUIGroup.h"
#include "TXUIBackButton.h"
#include "TXUIText.h"
#include "TXUIFigure.h"

namespace Toshi
{
    TQuaternion* TXUIResource::GetQuat(int a_iIndex)
    {
        TASSERT(a_iIndex >= -1);
        return a_iIndex == -1 ? (TQuaternion*)&TQuaternion::IDENTITY : &m_pQuat[a_iIndex];
    }

    TVector4* TXUIResource::GetVector(int a_iIndex)
    {
        TASSERT(a_iIndex >= -1);
        return a_iIndex == -1 ? (TVector4*)&TVector4::VEC_ZERO : &m_pVect[a_iIndex];
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
            TFile* file = TFile::Create(fullFileName);

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

    bool TXUIResource::Load(unsigned char* buffer)
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

        return true;
    }

    int TXUIResource::ReadDataSection(unsigned char* buffer, uint32_t size)
    {
        uint16_t uiType = PARSEWORD_BIG(buffer);
        buffer += 2;

        TASSERT(0 == TStringManager::String16Compare(GetString(uiType), _TS16("XuiCanvas")), "First Element is not XuiCanvas!");

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

            wchar_t* pPtr2 = pPtr;
            pPtr2++;

            m_asStringTable[i] = new (TXUI::MemoryBlock()) wchar_t[size];

            for (size_t j = 0; j < stringLength; j++)
            {
                m_asStringTable[i][j] = PARSEWORD_BIG((uint8_t*)pPtr2++);
            }

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
        if (m_uIDStrLength >> 8 != 0)
        {
            m_uIDStr->Concat(_TS16("."), -1);
        }
        m_uIDStr->Concat(a_wsID, -1);
    }

    void TXUIResource::PopID()
    {
        const wchar_t* top = m_uIDs.Top();
        m_uIDs.Pop();
        *m_uIDStr = m_uIDStr->Left(m_uIDStrLength - TStringManager::String16Length(top));
    }

    TXUIStringTable& TXUIResource::LookupStringTable()
    {
        if (m_uiStringTableCount != 0)
        {
            //TXUIStringTable::Lookup()
        }
        auto lal = TXUIStringTable();

        return lal;
        // TODO: insert return statement here
    }

    XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, uint16_t index)
    {
        if (index == 0) return TNULL;
        return CreateObjectData(a_rResource, a_rResource.m_asStringTable[index]);
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
        else if (TStringManager::String16Compare(objectName, _TS16("XuiTabScene"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiFigure"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUIFigureData();
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiVisual"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiImage"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiText"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUITextData();
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiButton"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUIButtonData();
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiGroup"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUIGroupData();
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiImagePresenter"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiTextPresenter"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiNineGrid"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiSoundXAudio"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiSoundXAct"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiLabel"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiEdit"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiMessageBoxButton"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiNavButton"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiBackButton"), -1) == 0)
        {
            return new (TXUI::MemoryBlock()) XURXUIBackButtonData();
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiRadioButton"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiCheckbox"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiProgressbar"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiSlider"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiList"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiCommonList"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiRadioGroup"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiScrollEnd"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiCaret"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiListItem"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlPresenter"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlElement"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlControl"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiControl"), -1) == 0)
        {
            return TNULL;
        }
        else if (TStringManager::String16Compare(objectName, _TS16("XuiVideo"), -1) == 0)
        {
            return TNULL;
        }
        return TNULL;
    }

}