#include "ToshiPCH.h"
#include "TXUIResource.h"

bool Toshi::TXUIResource::ReadHeader(unsigned char* buffer)
{
    m_oHeader.m_uiFileID = PARSEDWORD(buffer);

    TASSERT(m_oHeader.m_uiFileID == IDXUR, "Not a .xur file!");

    m_oHeader.m_uiUnk = PARSEDWORD_BIG(buffer + 0x4);
    m_oHeader.m_uiUnk2 = PARSEDWORD_BIG(buffer + 0xA);
    m_oHeader.m_uiSize = PARSEDWORD_BIG(buffer + 0xE);
    m_oHeader.m_usUnk4 = PARSEDWORD_BIG(buffer + 0x8);
    m_oHeader.m_usNumSections = PARSEWORD_BIG(buffer + 0x12);

    TASSERT(m_oHeader.m_usNumSections > 0, "There must be one or more Sections");

    uint8_t* memBlock = static_cast<uint8_t*>(tmalloc(m_oHeader.m_usNumSections * sizeof(Section)));

    ms_pXUIMemoryBlock = memBlock;
    m_oHeader.m_apSections = reinterpret_cast<Section*>(memBlock);

    uint32_t sectionID = PARSEDWORD(buffer + 0x14);

    buffer += 0x14;

    if (sectionID != IDXURSTRING && sectionID != IDXURVEC && sectionID != IDXURQUAT && sectionID != IDXURCUST)
    {
        buffer += 0x3C;
    }

    for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
    {
        m_oHeader.m_apSections[i].m_uiSectionID = PARSEDWORD(buffer);

        TASSERT(m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA, "Invalid Section ID");

        m_oHeader.m_apSections[i].m_uiOffset = PARSEDWORD_BIG(buffer + 4);
        m_oHeader.m_apSections[i].m_uiSize = PARSEDWORD_BIG(buffer + 8);

        buffer += sizeof(Section);
    }

    return true;
}


void Toshi::TXUIResource::LoadXUIB2(bool loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, bool loadTrb, void* unk3)
{
    for (size_t i = 0; i < m_uiStringTableCount; i++)
    {
        tdelete(m_asStringTable[i]);
    }
    tdelete(m_asStringTable);
    // tdelete(this+0x20);
    // tdelete(this+0x24);
    // this+2C != TNULL
    // this+0x2C(1) probably function pointer parameter=int (value 1 in this case)

    if (loadStringTables && fileNameStringTable != TNULL)
    {
        TTODO("Load Stringtables");
    }

    const char* extension = loadTrb ? ".trb" : ".xur";
    const char* fullFileName = strstr(filenameXUIB, extension);
    
    if (!loadTrb)
    {
        auto file = TFile::Create(fullFileName, Toshi::TFile::OpenFlags_Read);

        if (file != TNULL)
        {
            int size = file->GetSize();
            unsigned char* buffer = (unsigned char*)tmalloc(size);
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

int Toshi::TXUIResource::GetTotalSize(unsigned char* buffer)
{
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
            m_pCust = (unsigned char*)tmalloc(m_oHeader.m_apSections[i].m_uiSize);
            Toshi::TSystem::MemCopy(m_pCust, currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            m_uiCustDataSize = m_oHeader.m_apSections[i].m_uiSize;
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA)
        {
            ProcessDATA(currentSectionBuffer);
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING)
        {
            ProcessSTRN((unsigned short*)currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            totalSize += GetStringTableSize(currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC)
        {
            totalSize += (m_oHeader.m_apSections[i].m_uiSize / sizeof(TVector3)) * 8 + 0xC;
        }
    }

    return totalSize;
}

int Toshi::TXUIResource::ProcessDATA(unsigned char* buffer)
{
    uint16_t uiType = PARSEWORD_BIG(buffer);
    buffer += 2;

    TASSERT(0 == Toshi2::TStringManager::String16Compare(GetString(uiType), _TS16(XuiCanvas)), "");

    return 0;
}

bool Toshi::TXUIResource::ProcessSTRN(unsigned short* pPtr, uint32_t size)
{
    TASSERT(TNULL == m_asStringTable, "StringTable must not be initialized");

    unsigned short* pStart = pPtr;
    unsigned short* pEnd = pPtr + (size / 2);
    m_uiStringTableCount = 1;

    while (pStart < pEnd)
    {
        unsigned short stringLength = PARSEWORD_BIG((uint8_t*)pStart);
        m_uiStringTableCount++;
        pStart += stringLength + 1;
    }

    m_asStringTable = (unsigned short**)tmalloc(m_uiStringTableCount * sizeof(unsigned short*));

    *m_asStringTable = (unsigned short*)tmalloc(2);
    **m_asStringTable = 0;

    for (size_t i = 1; i < m_uiStringTableCount; i++)
    {
        TASSERT(pPtr < pEnd, "Pointer overflow");

        unsigned short stringLength = PARSEWORD_BIG((uint8_t*)pPtr);
        unsigned short size = stringLength * sizeof(unsigned short) + sizeof(unsigned short);

        m_asStringTable[i] = (unsigned short*)tmalloc(size);

        // Blob would call FUN_006eba70 but MemCopy seems better tbh
        Toshi::TSystem::MemCopy(m_asStringTable[i], pPtr + 1, size - sizeof(unsigned short));

        pPtr += stringLength + 1;
    }

    return true;
}

int Toshi::TXUIResource::GetStringTableSize(unsigned char* pPtr, uint32_t size)
{
    TIMPLEMENT_D("TXUIResource::GetStringTableSize");
    return 0;
}
