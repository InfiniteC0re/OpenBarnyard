#include "ToshiPCH.h"
#include "TXUIResource.h"

bool Toshi::TXUIResource::LoadXUIB(unsigned char* buffer)
{
    m_oHeader.m_uiFileID = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)buffer);

    TASSERT(m_oHeader.m_uiFileID == IDXUR, "Not a .xur file!");

    m_oHeader.m_uiUnk = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)(buffer + 0x4));
    m_oHeader.m_uiUnk2 = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)(buffer + 0xA));
    m_oHeader.m_uiUnk3 = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)(buffer + 0xE));
    m_oHeader.m_usUnk4 = BIG_ENDIAN_TO_LITTLE(buffer + 0x8);
    m_oHeader.m_uiNumSections = BIG_ENDIAN_TO_LITTLE(buffer + 0x12);

    TASSERT(m_oHeader.m_uiNumSections > 0, "There must be one or more Sections");

    m_oHeader.m_apSections = static_cast<Section*>(tmalloc(m_oHeader.m_uiNumSections * sizeof(Section)));

    uint32_t sectionID = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)(buffer + 0x14));

    if (sectionID != IDXURSTRING && sectionID != IDXURVEC && sectionID != IDXURQUAT && sectionID != IDXURCUST)
    {
        buffer += 0x3C;
    }

    for (size_t i = 0; i < m_oHeader.m_uiNumSections; i++)
    {
        m_oHeader.m_apSections[i].m_uiSectionID = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)buffer);

        TASSERT(m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST || 
            m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA, "Invalid Section ID");

        m_oHeader.m_apSections[i].m_uiOffset = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)buffer + 4);
        m_oHeader.m_apSections[i].m_uiSize = BIG_ENDIAN_TO_LITTLE(*(uint32_t*)buffer + 8);

        buffer += sizeof(Section);
    }

    return true;
}

int Toshi::TXUIResource::ProcessSections(unsigned char* buffer)
{
    LoadXUIB(buffer);

    int smth = m_oHeader.m_uiNumSections * sizeof(Section) + 4;

    for (size_t i = 0; i < m_oHeader.m_uiNumSections; i++)
    {
        unsigned char* currentSectionBuffer = buffer + m_oHeader.m_apSections[i].m_uiOffset;

        if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT)
        {
            smth += (m_oHeader.m_apSections[i].m_uiSize >> 4) * 4 + 4;
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST)
        {
            m_pCust = (unsigned char*)tmalloc(m_oHeader.m_apSections[i].m_uiSize);
            Toshi::TSystem::MemCopy(m_pCust, currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            m_uiCustSize = m_oHeader.m_apSections[i].m_uiSize;
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA)
        {
            ProcessDATA(currentSectionBuffer);
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING)
        {
            ProcessSTRN((unsigned short*)currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
            smth += GetStringTableSize(currentSectionBuffer, m_oHeader.m_apSections[i].m_uiSize);
        }
        else if (m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC)
        {
            smth += (m_oHeader.m_apSections[i].m_uiSize / 0xC) * 8 + 0xC;
        }
    }

    return 0;
}

int Toshi::TXUIResource::ProcessDATA(unsigned char* buffer)
{
    uint16_t uiType = BIG_ENDIAN_TO_LITTLE(buffer);
    buffer += 2;

    TASSERT(0 == Toshi2::TStringManager::String16Compare(GetString(uiType), _TS16(XuiCanvas)), "");

    return 0;
}

bool Toshi::TXUIResource::ProcessSTRN(unsigned short* pPtr, uint32_t size)
{
    TASSERT(TNULL == m_asStringTable, "StringTable must not be initialized");

    unsigned short* pEnd = pPtr + size;
    m_uiStringTableCount = 1;

    while (pPtr < pEnd)
    {
        unsigned short stringLength = BIG_ENDIAN_TO_LITTLE(*pPtr);
        m_uiStringTableCount++;
        pPtr += stringLength + 1;
    }

    m_asStringTable = (unsigned short**)tmalloc(m_uiStringTableCount * sizeof(unsigned short*));

    *m_asStringTable = (unsigned short*)tmalloc(2);
    *m_asStringTable = 0;

    for (size_t i = 1; i < m_uiStringTableCount; i++)
    {
        TASSERT(pPtr < pEnd, "Pointer overflow");

        unsigned short stringLength = BIG_ENDIAN_TO_LITTLE(*pPtr);
        unsigned short size = stringLength * sizeof(unsigned short) + sizeof(unsigned short);

        m_asStringTable[i] = (unsigned short*)tmalloc(size);

        // Blob would call FUN_006eba70 but MemCopy seems better tbh
        Toshi::TSystem::MemCopy(m_asStringTable[i], pPtr + 1, size);

        pPtr += stringLength + 1;
        m_asStringTable[i][stringLength] = '\0';
    }

    return true;
}

int Toshi::TXUIResource::GetStringTableSize(unsigned char* pPtr, uint32_t size)
{
    TIMPLEMENT_D("TXUIResource::GetStringTableSize");
    return 0;
}
