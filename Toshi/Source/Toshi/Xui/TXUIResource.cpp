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
#include "TXUINineGrid.h"
#include "XURReader.h"

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

    TBOOL TXUIResource::ReadHeader(uint8_t* buffer)
    {
        XURReader reader(buffer);

        m_oHeader.m_uiFileID = reader.ReadUInt32LE();
        TASSERT(m_oHeader.m_uiFileID == IDXUR, "Not a .xur file!");

        m_oHeader.m_uiVersion = reader.ReadUInt32();
        m_oHeader.m_uiFlags = reader.ReadUInt32();
        m_oHeader.m_uiXuiVersion = reader.ReadUInt16();
        m_oHeader.m_usBinSize = reader.ReadUInt32();
        m_oHeader.m_usNumSections = reader.ReadUInt16();

        TASSERT(m_oHeader.m_usNumSections > 0, "There must be one or more Sections");
        m_oHeader.m_apSections = new (TXUI::MemoryBlock()) Section[m_oHeader.m_usNumSections];

        uint32_t sectionID = PARSEDWORD(buffer);

        if (sectionID != IDXURSTRING && sectionID != IDXURVEC && sectionID != IDXURQUAT && sectionID != IDXURCUST)
        {
            buffer += 0x28;
        }

        for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
        {
            m_oHeader.m_apSections[i].m_uiSectionID = reader.ReadUInt32LE();

            TASSERT(
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST ||
                m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA,
                "Invalid Section ID"
            );

            m_oHeader.m_apSections[i].m_uiOffset = reader.ReadUInt32();
            m_oHeader.m_apSections[i].m_uiSize = reader.ReadUInt32();
        }

        return TTRUE;
    }


    void TXUIResource::Load(TBOOL loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, TBOOL loadTrb, void* unk3)
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
                uint8_t* buffer = new uint8_t[size];
                file->Read(buffer, size);
                file->Destroy();
                TBOOL bRes = ReadHeader(buffer);

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

    TBOOL TXUIResource::Load(uint8_t* buffer)
    {
        Destroy();
        ReadHeader(buffer);

        int totalSize = m_oHeader.m_usNumSections * sizeof(Section) + 4;

        for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
        {
            auto& section = m_oHeader.m_apSections[i];
            uint8_t* currentSectionBuffer = buffer + section.m_uiOffset;

            if (section.m_uiSectionID == IDXURQUAT)
            {
                totalSize += (section.m_uiSize / sizeof(TQuaternion)) * 4 + 4;
            }
            else if (section.m_uiSectionID == IDXURCUST)
            {
                ReadCustSection(currentSectionBuffer, section.m_uiSize);
            }
            else if (section.m_uiSectionID == IDXURDATA)
            {
                ReadDataSection(currentSectionBuffer, section.m_uiSize);
            }
            else if (section.m_uiSectionID == IDXURSTRING)
            {
                ReadStringSection(currentSectionBuffer, section.m_uiSize);
                totalSize += GetStringTableSize(currentSectionBuffer, section.m_uiSize);
            }
            else if (section.m_uiSectionID == IDXURVEC)
            {
                totalSize += (section.m_uiSize / sizeof(TVector3)) * 8 + 0xC;
            }
        }

        return TTRUE;
    }

    int TXUIResource::ReadDataSection(uint8_t* buffer, uint32_t size)
    {
        XURReader reader(buffer);

        uint16_t rootNameId = reader.ReadUInt16();
        uint8_t opcode = reader.ReadUInt8();

        TASSERT(0 == TStringManager::String16Compare(GetString(rootNameId), _TS16("XuiCanvas")), "First Element is not XuiCanvas!");
        m_root = CreateObjectData(*this, rootNameId);
        m_root->Load(*this, buffer);

        if ((opcode & 2) != 0)
        {
            m_root->LoadChildren(*this, buffer);
        }

        if ((opcode & 4) != 0 && m_root->LoadNamedFrames(*this, buffer) && (opcode & 2) != 0)
        {
            m_root->LoadTimelines(*this, buffer);
        }

        return TTRUE;
    }

    TBOOL TXUIResource::ReadStringSection(uint8_t* buffer, uint32_t size)
    {
        TASSERT(TNULL == m_asStringTable, "StringTable must not be initialized");
        XURReader reader(buffer);

        uint8_t* pStart = buffer;
        uint8_t* pEnd = buffer + size;
        m_uiStringTableCount = 1;

        while (reader.GetPos() < pEnd)
        {
            uint16_t stringLength = reader.ReadUInt16();
            reader.SeekFromCur(stringLength * sizeof(wchar_t));
            m_uiStringTableCount++;
        }

        reader.SetPos(pStart);

        m_asStringTable = new (TXUI::MemoryBlock()) wchar_t* [m_uiStringTableCount];
        m_asStringTable[0] = new (TXUI::MemoryBlock()) wchar_t[1];
        m_asStringTable[0][0] = L'\0';

        for (size_t i = 1; i < m_uiStringTableCount; i++)
        {
            TASSERT(buffer < pEnd, "Pointer overflow");

            uint16_t stringLength = reader.ReadUInt16();
            m_asStringTable[i] = new (TXUI::MemoryBlock()) wchar_t[stringLength + 1];
            m_asStringTable[i][stringLength] = L'\0';

            for (size_t j = 0; j < stringLength; j++)
            {
                m_asStringTable[i][j] = static_cast<wchar_t>(reader.ReadUInt16());
            }
        }

        return TTRUE;
    }

    int TXUIResource::ReadCustSection(uint8_t* buffer, uint32_t size)
    {
        m_pCust = static_cast<uint8_t*>(TMemoryHeap::Malloc(TXUI::MemoryBlock(), size));
        TUtil::MemCopy(m_pCust, buffer, size);
        m_uiCustDataSize = size;
        return 1;
    }

    int TXUIResource::GetStringTableSize(uint8_t* pPtr, uint32_t size)
    {
        TIMPLEMENT_D("TXUIResource::GetStringTableSize");
        return 0;
    }

    void TXUIResource::PushID(const wchar_t* a_wsID)
    {
        m_oIDComparisonStack.Push(a_wsID);
        if (m_uIDStrLength >> 8 != 0)
        {
            m_uIDStr->Concat(_TS16("."), -1);
        }
        m_uIDStr->Concat(a_wsID, -1);
    }

    void TXUIResource::PopID()
    {
        const wchar_t* top = m_oIDComparisonStack.Top();
        m_oIDComparisonStack.Pop();
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
            return new (TXUI::MemoryBlock()) XURXUINineGridData();
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