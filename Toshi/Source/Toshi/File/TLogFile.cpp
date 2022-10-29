#include "ToshiPCH.h"
#include "TLogFile.h"

void Toshi::TLogFile::Close()
{
	m_unk = 0;
	RecalcLevel();

	for (size_t i = 0; i < 3; i++)
	{
		Print("%s count = %d\n", cTypeStrings[i], m_unkCounts[i]);
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
