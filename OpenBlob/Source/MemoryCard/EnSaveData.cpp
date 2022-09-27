#include "ToshiPCH.h"
#include "EnSaveData.h"

uint32_t EnSaveData::CRCTable[0x100] = {};

bool EnSaveData::Validate()
{
    sizeof(SaveData);
    return false;
}

void EnSaveData::GenerateCRC()
{
    uint32_t crc;

    for (int i = 0; i <= 255; i++) {
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

    uint32_t crc = 0;

    while (len--)
        crc = crc32upd(CRCTable, crc, *buffer++);

    return CRC32POST(crc);
}
