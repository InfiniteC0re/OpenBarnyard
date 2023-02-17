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
		Toshi::TUtil::GetSingletonWeak()->m_pLogFile1 = logfile;
		Toshi::TUtil::GetSingletonWeak()->m_pLogFile2 = logfile;
		
		TrimLog("*.log", 9);
		time_t seconds;
		time(&seconds);

		tm* time = gmtime(&seconds);

		char filename[256];
		T2String8::Format(filename, "deblob_%d%02d%02d_%02d_%02d_%02d.log", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
		GetLog()->Create(filename, "Toshi 2.0", false);
		GetLog()->AllowIndentation(true);
		GetLog()->SetSimpleMode(true);
	}

	void TUtil::Log(const char* format, ...)
	{
		if (GetLog() != TNULL)
		{
			va_list args;
			va_start(args, format);

			char str[0x800];

			T2String8::FormatV(str, sizeof(str), format, &args);

			va_end(args);

			GetLog()->Log(TLogFile::Type_Info, "Toshi", "Kernel", str);
			GetSingletonWeak()->m_Emitter.Throw(GetLog());
		}
	}

	void TUtil::Log(TLogFile::Type logtype, const char* format, ...)
	{
		if (GetLog() != TNULL)
		{
			va_list args;
			va_start(args, format);

			char str[2048];
			T2String8::FormatV(str, sizeof(str), format, &args);
			va_end(args);

			GetLog()->Log(logtype, "Toshi", "Kernel", format, str);
			GetSingletonWeak()->m_Emitter.Throw(GetLog());
		}
	}

	void TUtil::LogConsole(const char* format, ...)
	{
		if (GetLog() != TNULL)
		{
			va_list vargs;
			va_start(vargs, format);

			char str[1024];
			int size = T2String8::FormatV(str, sizeof(str), format, &vargs);
			str[size] = '\n';
			str[size + 1] = '\0';
			OutputDebugStringA(str);
			str[size] = '\0';
			TOSHI_CORE_TRACE(str);
		}
	}

	void TUtil::LogSet(TLogFile* a_logFile)
	{
		Log("Changing log file.");
		TUtil* util = Toshi::TUtil::GetSingleton();
		GetLog()->Close();
		util->m_pLogFile2 = a_logFile == TNULL ? util->m_pLogFile1 : a_logFile;
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
		TError::CreateSingleton(0x1800, 0x100);
        
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