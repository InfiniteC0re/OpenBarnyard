#include "pch.h"
#include "EnSaveData.h"

uint32_t EnSaveData::CRCTable[0x100] = {};

bool EnSaveData::Validate()
{
    int result = Toshi2::T2String8::CompareStrings(m_savedata->header.m_id, "BB", 2);

    if (result == 0 && m_savedata->header.m_size == 3208)
    {
        uint32_t prevCRC = m_savedata->header.m_crc;
        m_savedata->header.m_crc = 0;
        m_savedata->header.m_crc = CalculateCRC(m_buffer, m_size);

        if (m_savedata->header.m_crc != prevCRC) return false;

        float fVal1 = 0.5f;
        float fVal2 = 1.0f;

    }

    sizeof(SaveData);
    return false;
}

// Source: https://lentz.com.au/blog/tag/crc-table-generator
void EnSaveData::GenerateCRC()
{
    uint32_t crc;

    for (int i = 0; i < CRC_TABSIZE; i++) {
        crc = i;
        for (int j = 8; j > 0; j--) {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC32POLY;
            else
                crc >>= 1;
        }
        CRCTable[i] = crc;
    }
}


uint32_t EnSaveData::CalculateCRC(unsigned char* buffer, int len)
{
    // FUN_0065d810
    // Source: https://lentz.com.au/blog/tag/crc-table-generator

    uint32_t crc = 0;
    
    while (len--)
        crc = crc32upd(CRCTable, crc, *buffer++);

    return CRC32POST(crc);
}
