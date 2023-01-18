#include "ToshiPCH.h"
#include "TLogFile.h"

Toshi::TLogFile::TLogFile()
{
	m_pFile = TNULL;
	m_iTotalLogCount = 0;
	m_bIsSimpleMode = false;
	m_curLevel = 0;
	*m_LevelString = 0;
	*m_typeCounts = 0;
	m_unk2 = TNULL;
	m_unk3 = 0;
}

int Toshi::TLogFile::Create(const char* fileName, const char* str2, bool createNew)
{
	TASSERT(TNULL == m_pFile);
	if (m_pFile != TNULL) return 2;
	
	int flags = TFile::FileMode_CreateNew | TFile::FileMode_NoBuffer | TFile::FileMode_Write;

	if (createNew)
	{
		flags = TFile::FileMode_NoBuffer | TFile::FileMode_Write;
	}

	m_pFile = TFile::Create(fileName, flags);

	if (m_pFile == TNULL) return 1;
	if (createNew) m_pFile->Seek(0, TFile::TSEEK_END);

	m_iTotalLogCount = 0;
	*m_LevelString = 0;
	m_curLevel = 0;

	time_t t;
	time(&t);

	m_pFile->CPrintf("Log created [%s]:[%s]: %s\n", fileName, str2, ctime(&t));
	m_pFile->CPrintf("Compilation: %s", __TIMESTAMP__);

	return 0;
}

void Toshi::TLogFile::Close()
{
	m_curLevel = 0;
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
	m_curLevel = TMath::Min(m_curLevel, cLevelMax);
	m_curLevel = TMath::Max(m_curLevel, 0);

	for (size_t i = 0; i < m_curLevel; i++)
	{
		m_LevelString[i] = 9;
	}
	m_LevelString[m_curLevel] = 0;
}

void Toshi::TLogFile::Print(const char* format, ...)
{
	if (m_pFile != TNULL)
	{
		va_list args;
		va_start(args, format);

		char str[0x400];

		T2String8::FormatV(str, sizeof(str), format, args);

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
