#include "ToshiPCH.h"
#include "TLogFile.h"

Toshi::TLogFile::TLogFile()
{
	m_pFile = TNULL;
	m_iTotalLogCount = 0;
	m_bIsSimpleMode = false;
	m_unk = 0;
	m_LevelString = 0;
	m_typeCounts = 0;
	m_unk2 = TNULL;
	m_unk3 = 0;
}

void Toshi::TLogFile::Close()
{
	m_unk = 0;
	RecalcLevel();

	for (size_t i = 0; i < 3; i++)
	{
		Print("%s count = %d\n", cTypeStrings[i], m_typeCounts[i]);
	}

	if (m_pFile != TNULL)
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}
}

void Toshi::TLogFile::RecalcLevel()
{
}

void Toshi::TLogFile::Print(const char*, ...)
{
}

void Toshi::TLogFile::Log(TYPE type, const char* str1, const char* str2, const char* format, ...)
{
	if (m_pFile != TNULL)
	{
		if (m_bIsSimpleMode)
		{
			if (m_bAllowIndentation)
			{
				m_pFile->CPrintf("%d [%s]: %s", m_iTotalLogCount, cTypeStrings[type], m_LevelString);
			}
		}
		else
		{
			char* strTime = { 0 };
			strTime = _strtime(strTime);
			m_pFile->CPrintf("%d [%s] [%s]: %s: %s: %s", m_iTotalLogCount, cTypeStrings[type], strTime, str1, str2 != TNULL ? str2 : "", !m_bAllowIndentation ? m_LevelString : "");
		}

		va_list args;
		va_start(args, format);

		m_pFile->VCPrintf(format, args);
		m_pFile->CPrintf("\n");
		m_typeCounts[type]++;
		m_iTotalLogCount++;
	}
}
