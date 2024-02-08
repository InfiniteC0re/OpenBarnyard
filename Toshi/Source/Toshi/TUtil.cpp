#include "ToshiPCH.h"
#include "TUtil.h"
#include "Toshi/TSystem.h"
#include "Toshi/TError.h"
#include "Toshi/T2FixedString.h"
#include "Render/TModelRegistry.h"

#ifdef TOSHI_DEBUG
#include <Platform/Windows/TConsoleFile_Win.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	void TUtil::LogInitialise()
	{
#ifndef TOSHI_NO_LOGS
		TLogFile* logfile = new TLogFile;
		TUtil::GetSingleton()->m_pDefaultLogFile = logfile;
		TUtil::GetSingleton()->m_pCurrentLogFile = logfile;
		
		TrimLog("*.log", 9);
		
		time_t seconds;
		time(&seconds);
		tm* time = gmtime(&seconds);

		char filename[256];

#ifdef TOSHI_DEBUG

		const char* szFileSystemName = "console";

		// Create console file system that will be able to open console for text output
		new TConsoleFileSystem("console");

#else  // TOSHI_DEBUG

		const char* szFileSystemName = "local";

#endif // TOSHI_DEBUG

		T2String8::Format(
			filename,
			"%s:barnyard_%d%02d%02d_%02d_%02d_%02d.log",
			szFileSystemName,
			time->tm_year + 1900,
			time->tm_mon + 1,
			time->tm_mday,
			time->tm_hour,
			time->tm_min,
			time->tm_sec
		);
		
		GetCurrentLogFile()->Create(filename, "Toshi 2.0", TFALSE);
		GetCurrentLogFile()->AllowIndentation(TTRUE);
		GetCurrentLogFile()->SetSimpleMode(TFALSE);
#else  // TOSHI_NO_LOGS
		TUtil::GetSingleton()->m_pDefaultLogFile = TNULL;
		TUtil::GetSingleton()->m_pCurrentLogFile = TNULL;
#endif // TOSHI_NO_LOGS
	}

	void TUtil::Log(const char* a_szFormat, ...)
	{
		if (IsSingletonCreated() && GetCurrentLogFile())
		{
			auto pLogFile = GetCurrentLogFile();

			T2FixedString2048 formatString;

			va_list args;
			va_start(args, a_szFormat);
			formatString.FormatV(a_szFormat, args);
			va_end(args);

			pLogFile->Log(TLogFile::Type_Info, "Toshi", "Kernel", formatString.Get());
			GetLogEmitter().Throw(LogEvent(pLogFile, TLogFile::Type_Info, formatString.Get()));
		}
	}

	void TUtil::Log(TLogFile::Type a_eLogType, const char* a_szFormat, ...)
	{
		auto pLogFile = GetCurrentLogFile();

		if (pLogFile)
		{
			T2FixedString2048 formatString;

			va_list args;
			va_start(args, a_szFormat);
			formatString.FormatV(a_szFormat, args);
			va_end(args);

			pLogFile->Log(a_eLogType, "Toshi", "Kernel", formatString.Get());
			GetLogEmitter().Throw(LogEvent(pLogFile, a_eLogType, formatString.Get()));
		}
	}

	void TUtil::LogConsole(const char* a_szFormat, ...)
	{
		auto pLogFile = GetCurrentLogFile();

		if (pLogFile)
		{
			T2FixedString2048 formatString;

			va_list args;
			va_start(args, a_szFormat);
			formatString.FormatV(a_szFormat, args);
			va_end(args);

			OutputDebugStringA(formatString.Get());
			printf(formatString.Get());
		}
	}

	void TUtil::LogSet(TLogFile* a_pLogFile)
	{
		Log("Changing log file.");
		TUtil* pUtil = TUtil::GetSingleton();

		if (pUtil->m_pCurrentLogFile)
		{
			pUtil->m_pCurrentLogFile->Close();
		}

		pUtil->m_pCurrentLogFile = (!a_pLogFile) ? pUtil->m_pDefaultLogFile : a_pLogFile;
	}

	TBOOL TUtil::ToshiCreate(char* a_szCommandLine, TINT a_iArg2, TINT a_iArg3)
	{
		CreateKernelInterface();
		// TODO: FUN_006c7fe0
		TFileManager::Create();
		CreateTPStringPool();
		Create();
		
		return TTRUE;
	}

	void TUtil::ToshiDestroy()
	{
		TIMPLEMENT();
#ifdef TOSHI_SKU_WINDOWS
		ReleaseMutex(ms_hGlobalMutex);
#endif

		TModelRegistry::Uninitialise();
	}

	TBOOL TUtil::CreateKernelInterface()
	{
		return TSystemManager::Create();
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