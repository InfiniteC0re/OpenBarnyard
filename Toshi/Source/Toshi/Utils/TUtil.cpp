#include "ToshiPCH.h"
#include "TUtil.h"

namespace Toshi
{
    // Source: https://lentz.com.au/blog/tag/crc-table-generator
	void TUtil::CRCInitialise()
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
            s_aiCRC32LUT[i] = crc;
        }
	}

    // Source: https://lentz.com.au/blog/tag/crc-table-generator
	uint32_t TUtil::CRC32(unsigned char* buffer, uint32_t len)
	{
        uint32_t crc = 0;

        while (len--)
            crc = crc32upd(s_aiCRC32LUT, crc, *buffer++);

        return CRC32POST(crc);
	}
}