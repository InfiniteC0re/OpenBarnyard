#include "ToshiPCH.h"
#include "TLogFile.h"
#include "File/TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

static constexpr const TCHAR* kTypeStrings[]
{
	"Info",
	"Warning",
	"Error"
};

TSTATICASSERT(Toshi::TLogFile::Type_NUMOF == TARRAYSIZE(kTypeStrings));

#if SUPPORT_COLOURED_LOGS

static constexpr const WORD kTypeColours[]
{
	FOREGROUND_GREEN,
	FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
	FOREGROUND_INTENSITY | FOREGROUND_RED
};

TSTATICASSERT(Toshi::TLogFile::Type_NUMOF == TARRAYSIZE(kTypeColours));

#endif // SUPPORT_COLOURED_LOGS

namespace Toshi {

	TLogFile::TLogFile()
	{
		m_pFile = TNULL;
		m_iTotalLogCount = 0;
		m_bIsSimpleMode = TFALSE;
		m_curLevel = 0;
		*m_LevelString = 0;
		*m_typeCounts = 0;
		m_unk2 = TNULL;
		m_unk3 = 0;
	}

	TLogFile::~TLogFile()
	{
		TASSERT(TNULL == m_pFile, "TFile must be NULL");
		Close();
	}

	TLogFile::Error TLogFile::Create(const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting)
	{
		TASSERT(TNULL == m_pFile);
		
		if (m_pFile != TNULL)
		{
			return Error::AlreadyCreated;
		}

		TIMPLEMENT_D("uVar1 = FUN_008289A0(fileName);");
		TIMPLEMENT_D("FUN_00828A40(uVar1);");

		TFILEMODE fileMode = writeExisting ?
			TFILEMODE_NOBUFFER | TFILEMODE_WRITE :
			TFILEMODE_NOBUFFER | TFILEMODE_WRITE | TFILEMODE_CREATENEW;

		m_pFile = TFile::Create(fileName, fileMode);

		if (m_pFile == TNULL)
		{
			return Error::OpenFile;
		}

		if (writeExisting)
		{
			m_pFile->Seek(0, TSEEK_END);
		}

		m_iTotalLogCount = 0;
		*m_LevelString = 0;
		m_curLevel = 0;

		m_pFile->CPrintf("Log created [%s]:[%s]: %s\n", fileName, str2, TUtil::GetTime());
		m_pFile->CPrintf("Compilation: %s\n", __TIMESTAMP__);

		return Error::OK;
	}

	void TLogFile::Close()
	{
		m_curLevel = 0;
		RecalcLevel();

		for (size_t i = 0; i < 3; i++)
		{
			Print("%s count = %d\n", kTypeStrings[i], m_typeCounts[i]);
		}

		if (m_pFile != TNULL)
		{
			m_pFile->Destroy();
			m_pFile = TNULL;
		}
	}

	void TLogFile::RecalcLevel()
	{
		m_curLevel = TMath::Min<TUINT32>(m_curLevel, cLevelMax);
		m_curLevel = TMath::Max<TUINT32>(m_curLevel, 0);

		for (size_t i = 0; i < m_curLevel; i++)
		{
			m_LevelString[i] = 9;
		}

		m_LevelString[m_curLevel] = 0;
	}

	void TLogFile::Print(const TCHAR* format, ...)
	{
		if (m_pFile != TNULL)
		{
			va_list args;
			va_start(args, format);

			TCHAR str[1024];
			T2String8::FormatV(str, sizeof(str), format, args);

			va_end(args);

			if (m_bAllowIndentation)
			{
				m_pFile->CPrintf("%s%s", m_LevelString, str);
			}
			else
			{
				m_pFile->CPrintf(str);
			}
		}
	}

	void TLogFile::Log(Type type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ...)
	{
		if (m_pFile != TNULL)
		{
#if SUPPORT_COLOURED_LOGS && defined(TOSHI_SKU_WINDOWS)
			HANDLE hStd;
			WORD wOldColorAttrs;
			
			if (m_bColouring)
			{
				hStd = GetStdHandle(STD_OUTPUT_HANDLE);
			
				CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
				GetConsoleScreenBufferInfo(hStd, &csbiInfo);
				wOldColorAttrs = csbiInfo.wAttributes;

				SetConsoleTextAttribute(hStd, kTypeColours[type]);
			}

			if (m_bIsSimpleMode)
			{
				if (m_bAllowIndentation)
				{
					m_pFile->CPrintf("[%s]: %s", kTypeStrings[type], m_LevelString);
				}
			}
			else
			{
				TCHAR strTime[9];
				_strtime(strTime);

				m_pFile->CPrintf("[%s] [%s/%s] [%s]: %s", strTime, str1, str2 != TNULL ? str2 : "", kTypeStrings[type], !m_bAllowIndentation ? m_LevelString : "");
			}
#else // SUPPORT_COLOURED_LOGS && defined(TOSHI_SKU_WINDOWS)
			if (m_bIsSimpleMode)
			{
				if (m_bAllowIndentation)
				{
					m_pFile->CPrintf("%d [%s]: %s", m_iTotalLogCount, kTypeStrings[type], m_LevelString);
				}
			}
			else
			{
				TCHAR strTime[128];
				_strtime(strTime);

				m_pFile->CPrintf("%d [%s] [%s]: %s: %s: %s", m_iTotalLogCount, kTypeStrings[type], strTime, str1, str2 != TNULL ? str2 : "", !m_bAllowIndentation ? m_LevelString : "");
			}
#endif // SUPPORT_COLOURED_LOGS && defined(TOSHI_SKU_WINDOWS)

			va_list args;
			va_start(args, format);
			m_pFile->VCPrintf(format, args);
			va_end(args);

#if SUPPORT_COLOURED_LOGS && defined(TOSHI_SKU_WINDOWS)
			if (m_bColouring)
			{
				SetConsoleTextAttribute(hStd, wOldColorAttrs);
			}
#endif // SUPPORT_COLOURED_LOGS && defined(TOSHI_SKU_WINDOWS)

			m_typeCounts[type]++;
			m_iTotalLogCount++;
		}
	}
}