#include "ToshiPCH.h"
#include "TCStringPool.h"

Toshi::TCStringPool::TCStringPool(int unk, int count)
{
	m_unk = 0;
	m_size = unk;
	m_iCount = count;

	if (m_iCount < 1)
	{
		m_oStringPool = TNULL;
	}
	else
	{
		m_oStringPool = new StringPool();
		m_oStringPool->m_szStrings = new TString8[count];
		m_oStringPool->count = m_iCount;
	}
}
