#include "ToshiPCH.h"
#include "TCStringPool.h"

Toshi::TCStringPool::TCStringPool(int a_iMaxSize, int a_iCapacity)
{
	m_iStringCount = 0;
	m_iMaxSize = a_iMaxSize;
	m_iCapacity = a_iCapacity;

	if (m_iCapacity < 1)
	{
		m_oStringPool.m_pStringPool = TNULL;
	}
	else
	{
		m_oStringPool.m_pStringPool = new StringPool();
		m_oStringPool.m_pStringPool->m_szStrings = new TString8[a_iCapacity];
		m_oStringPool.m_iCountOfPools = m_iCapacity;
	}
}

Toshi::TCStringPool::TCStringPool(const char* a_szFileName, int unk, int unk2) : TCStringPool(unk, unk2)
{
	ReadFile(a_szFileName);
}

bool Toshi::TCStringPool::ReadFile(const char* a_szFileName)
{
	TFileManager* fileManager = TFileManager::GetSingletonWeak();
	TFile* file = fileManager->CreateFile(a_szFileName, 1);

	if (file == TNULL)
	{
		return false;
	}

	int stringCount = 0;
	file->Read(&stringCount, 4);

	int stringsSize = 0;
	file->Read(&stringsSize, 4);

	if (stringCount != 0 && stringsSize != 0)
	{
		void* strings = TMalloc(stringsSize);
		int read = file->Read(strings, stringsSize);
		fileManager->DestroyFile(file);

		if (read != stringsSize)
		{
			TFree(strings);
			return false;
		}

		if (m_iCapacity < stringCount)
		{
			InitStringPool(stringCount);
		}
		m_iStringCount = stringCount;

		StringPool* pool = new StringPool();
		pool->count = 0;

	}

	return false;
}

void Toshi::TCStringPool::InitStringPool(int a_iStringCount)
{
	// 0x10021540 JPOG

	if (a_iStringCount == 0)
	{
		if (m_oStringPool.m_pStringPool != TNULL)
		{
			delete[] m_oStringPool.m_pStringPool;
		}
		m_oStringPool.m_pStringPool = TNULL;
		m_iCapacity = 0;
		m_iStringCount = 0;
	}
	else
	{
		m_oStringPool.m_pStringPool = new StringPool[a_iStringCount];
		m_oStringPool.m_iCountOfPools++;

		int stringCount = a_iStringCount < m_iStringCount ? a_iStringCount : m_iStringCount;

		for (int i = 0; i < stringCount; i++)
		{
			TIMPLEMENT();
			//m_oStringPool.m_pStringPool[i] = new StringPool();
		}

	}

}
