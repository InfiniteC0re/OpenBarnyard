#include "ToshiPCH.h"
#include "THashTable.h"

Toshi::THashTable::t_ItemCompareFunc Toshi::THashTable::DefaultItemCompareFunc = [](void* unk, void* unk2, int unk3)
{
	return Toshi::TUtil::MemCompare(unk, unk2, unk3) == 0;
};

Toshi::THashTable::t_ItemHashFunc Toshi::THashTable::DefaultItemHashFunc = [](void* unk, int unk2, int unk3)
{
	uint8_t* buffer = reinterpret_cast<uint8_t*>(unk);

	uint32_t hash = 0;

	while (unk2 != 0)
	{
		hash = (hash * 0x10) + *buffer++;
		if ((hash & 0xF0000000) != 0)
		{
			hash = hash ^ (hash & 0xF0000000) >> 24 ^ (hash & 0xF0000000);
		}
		unk2--;
	}

	return hash - (hash / unk3) * unk3;
};

void* Toshi::THashTable::Insert(void* a_pData)
{
	TASSERT(0 != m_iItemSize);

	void* a_pBuckets = GetBuckets();
	if (a_pBuckets == TNULL) return TNULL;

	void* foundBucket = Find(a_pData);
	if (foundBucket == TNULL)
	{
		return Append(a_pData);
	}

	return TNULL;
}

void* Toshi::THashTable::Find(void* a_pData)
{
	TASSERT(0 != m_iItemSize);
	int* a_pBuckets = GetBuckets();

	if (a_pBuckets != TNULL)
	{
		uint32_t hash = m_ItemHashFunc(a_pData, m_iItemSize, m_iBucketSize);
		int a_bucket = a_pBuckets[hash];

		while (a_bucket != -1)
		{
			Item& item = m_pSmth[a_bucket];
			bool bRes = m_ItemCompareFunc(a_pData, (int*)m_pItems + m_iItemSize * (int)item.value, m_iItemSize);
			if (bRes) return &item;
			a_bucket = item.key;
		}
	}

	return TNULL;
}

void* Toshi::THashTable::Append(void* a_pData)
{
	TASSERT(0 != m_iItemSize);

	if (m_pBuckets != TNULL)
	{
		TASSERT(!((m_iHashNodeCount == m_iHashNodeCountTotal) || (m_iItemCount == m_iItemCountTotal)));
		if (!((m_iHashNodeCount == m_iHashNodeCountTotal) || (m_iItemCount == m_iItemCountTotal)))
		{
			uint32_t hash = m_ItemHashFunc(a_pData, m_iItemSize, m_iBucketSize);

			m_pSmth[m_iHashNodeCountTotal].key = m_pBuckets[hash];
			m_pSmth[m_iHashNodeCountTotal].value = (void*)m_iItemCount;

			TUtil::MemCopy((int*)m_pItems + m_iItemCount * m_iItemSize, a_pData, m_iItemSize);

			m_pBuckets[hash] = m_iHashNodeCountTotal;

			m_iItemCount++;
			m_iHashNodeCountTotal++;

			return &m_pSmth[m_iHashNodeCountTotal];
		}
	}
	return TNULL;
}

bool Toshi::THashTable::Create(int a_iItemCountTotal, int a_iItemSize, int a_iBucketSize, int a_iHashNodeCount)
{
	if (m_pBuckets == TNULL)
	{
		if (a_iHashNodeCount < a_iItemCountTotal)
		{
			a_iHashNodeCount = a_iItemCountTotal;
		}
		m_iBucketSize = a_iBucketSize;
		m_iItemSize = a_iItemSize;
		m_iItemCount = 0;
		m_iItemCountTotal = a_iItemCountTotal;
		m_iHashNodeCount = a_iHashNodeCount;
		m_pBuckets = new int[m_iBucketSize];
		m_pSmth = new Item[m_iHashNodeCount];
		m_pItems = new int[m_iItemSize * m_iItemCountTotal];

		SetItemHashFunction(DefaultItemHashFunc);
		SetItemCompareFunction(DefaultItemCompareFunc);

		for (int i = 0; i < m_iBucketSize; i++)
		{
			m_pBuckets[i] = -1;
		}

		return true;
	}
	return false;
}
