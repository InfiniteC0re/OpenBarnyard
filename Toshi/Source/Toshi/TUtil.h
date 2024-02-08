#pragma once
#include "File/TLogFile.h"
#include "Toshi/TSingleton.h"
#include "Toshi/TEvent.h"

#include <Windows.h>

namespace Toshi {

	class TPString8Pool;

	class TUtil : public TSingleton<TUtil>
	{
	public:
		//-----------------------------------------------------------------------------
		// Class members
		//-----------------------------------------------------------------------------

		struct LogEvent
		{
			constexpr LogEvent(TLogFile* a_pFile, TLogFile::Type a_eType, const char* a_szString) :
				m_pFile(a_pFile),
				m_eType(a_eType),
				m_szString(a_szString)
			{ }

			TLogFile* m_pFile;
			TLogFile::Type m_eType;
			const char* m_szString;
		};

		struct TOSHIParams
		{
			const char* szCommandLine = "";
			const char* szLogFileName = "toshi";
			const char* szLogAppName = "Toshi";
			const char* szLogAppDirName = "Kernel";
			TINT iUnused1 = 0;
			TINT iUnused2 = 0;
		};

	public:
		TUtil();

	private:
		TLogFile* m_pDefaultLogFile;
		TLogFile* m_pCurrentLogFile;
		TEmitter<TUtil, LogEvent> m_LogEmitter;

	public:
		//-----------------------------------------------------------------------------
		// Static members
		//-----------------------------------------------------------------------------

		static TBOOL ToshiCreate(const TOSHIParams& a_rToshiParams);

		static void ToshiDestroy();
		
		static const char* GetTime();

		static void  MemSet(void* ptr, size_t value, size_t size) { std::memset(ptr, value, size); }
		static void* MemCopy(void* dst, const void* src, size_t size) { return std::memcpy(dst, src, size); }
		static void  MemClear(void* ptr, size_t size) { std::memset(ptr, 0, size); }
		static int   MemCompare(void* ptr1, void* ptr2, int size) { return std::memcmp(ptr1, ptr2, size); }

		static void Log(const char* a_szFormat, ...);
		static void Log(TLogFile::Type a_eLogType, const char* a_szFormat, ...);
		static void TrimLog(const char* fileExtension, size_t trimTo);

		static void LogDown() { TUtil::GetSingletonSafe()->m_pCurrentLogFile->Down(); }
		static void LogUp() { TUtil::GetSingletonSafe()->m_pCurrentLogFile->Up(); }

		static void LogConsole(const char* a_szFormat, ...);
		static void LogSet(TLogFile* a_logFile);

		static TLogFile* GetCurrentLogFile() { return TUtil::GetSingleton()->m_pCurrentLogFile; }
		static TEmitter<TUtil, LogEvent>& GetLogEmitter() { return TUtil::GetSingleton()->m_LogEmitter; }

		static void SetGlobalMutex(HANDLE a_hGlobalMutex) { ms_hGlobalMutex = a_hGlobalMutex; }
		
		static TPString8Pool* SetTPStringPool(TPString8Pool* a_pStringPool) { return std::exchange(*ms_poStringPool, a_pStringPool); }
		
		static TPString8Pool* GetTPStringPool()
		{
			TASSERT(ms_poStringPool != TNULL);
			return *ms_poStringPool;
		}

	private:
		static void Create();

		static TBOOL CreateKernelInterface();

		static void LogInitialise();
		static void CreateTPStringPool();
		static void DestroyTPStringPool();

	private:
		inline static TPString8Pool** ms_poStringPool;
		inline static HANDLE ms_hGlobalMutex;
		inline static TOSHIParams ms_oToshiParams;
		
	public:
#pragma region CRC

		// Source: https://lentz.com.au/blog/tag/crc-table-generator

		/*
			LICENCE

			This package may be freely distributed provided the files remain together,
			in their original unmodified form.
			All files, executables and sourcecode remain the copyrighted property of
			Arjen G. Lentz and LENTZ SOFTWARE-DEVELOPMENT.
			Licence for any use granted, provided this notice & CRC.DOC are included.
			For executable applications, credit should be given in the appropriate
			places in the program and documentation.
			These notices must be retained in any copies of any part of this
			documentation and/or software.

			Any use of, or operation on (including copying/distributing) any of
			the above mentioned files implies full and unconditional acceptance of
			this licence and disclaimer.
		*/

		/* ------------------------------------------------------------------------- */
		/* CRC-32 CCITT                                                              */
		/* ------------------------------------------------------------------------- */
		#define CRC32POLY      (0xEDB88320L)    /* Generator polynomial number       */
		#define CRC32POST(crc) (~(crc))         /* CRC Postconditioning before xmit  */

		#define crc32upd(crctab,crc,c) \
					  ((crctab)[((int) (crc) ^ (c)) & 0xff] ^ ((crc) >> 8))

		#define CRC_TABSIZE (256)               /* Normal 256-entry table            */

		//=============================================================================

		inline static uint32_t s_aiCRC32LUT[CRC_TABSIZE] = {};

		static void CRCInitialise();
		static uint32_t CRC32(unsigned char* buffer, uint32_t len);

#pragma endregion

	};
}
