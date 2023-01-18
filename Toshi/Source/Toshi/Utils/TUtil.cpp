#include "ToshiPCH.h"
#include "TUtil.h"
#include "TRegion.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Core/TError.h"

namespace Toshi
{
	void TUtil::LogInitialise()
	{
		TLogFile* logfile = new TLogFile();
		m_pLogFile1 = logfile;
		m_pLogFile2 = logfile;
		TIMPLEMENT("FUN_0065db40(\" * .log\",9);");
		time_t* seconds;
		GetUnixSeconds((uint64_t*)seconds);
		tm* time = gmtime(seconds);
	}

	bool TUtil::ToshiCreate(int argc, char** argv, TMemory& memorySettings)
    {
        TRegion::LockRegion();
		
		// Initialize TMemory and dlmalloc
        memorySettings.Init();

		// Initialize other systems
		TLog::Create();
        TSystemManager::Create();
        TFileManager::Create();

		Create();

		new TError(0x1800, 0x100);

        return true;
    }

    void TUtil::MemSet(void* ptr, size_t value, size_t size)
    {
		if (size >= sizeof(void*))
		{
			size_t* pos = static_cast<size_t*>(ptr);
			size_t  stepCount = size / sizeof(void*);
			size -= stepCount * sizeof(void*);

			for (; stepCount != 0; stepCount--) *(pos++) = value;

			ptr = pos;
		}

		while (size > 0)
		{
			uint8_t stepSize = size & 0b11;
			if (stepSize == 0) stepSize = 4;

			if (size == 1)
			{
				*(uint8_t*)ptr = (uint8_t)value;
			}
			else
			{
				if (size == 2)
				{
					*(uint16_t*)ptr = (uint16_t)value;
				}
				else if (size == 3)
				{
					*(uint16_t*)ptr = (uint16_t)value;
					*(uint8_t*)((uintptr_t)ptr + sizeof(uint16_t)) = (uint8_t)value;
				}
				else if (size == 4)
				{
					*(uint32_t*)ptr = (uint32_t)value;
				}
			}

			ptr = reinterpret_cast<void*>((uintptr_t)ptr + stepSize);
			size -= stepSize;
		}
    }

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