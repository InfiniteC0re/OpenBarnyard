#include "ToshiPCH.h"
#include "THashTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

Toshi::THashTable::t_ItemCompareFunc Toshi::THashTable::DefaultItemCompareFunc = [](void* unk, void* unk2, TINT unk3)
{
	return Toshi::TUtil::MemCompare(unk, unk2, unk3) == 0;
};

Toshi::THashTable::t_ItemHashFunc Toshi::THashTable::DefaultItemHashFunc = [](void* unk, TINT unk2, TINT unk3)
{
	uint8_t* buffer = reinterpret_cast<uint8_t*>(unk);

	TUINT32 hash = 0;

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

	return foundBucket ? foundBucket : Append(a_pData);
}

void* Toshi::THashTable::Find(void* a_pData)
{
	TASSERT(0 != m_iItemSize);
	TINT* a_pBuckets = GetBuckets();

	if (a_pBuckets != TNULL)
	{
		TUINT32 hash = m_ItemHashFunc(a_pData, m_iItemSize, m_iBucketSize);
		TINT a_bucket = a_pBuckets[hash];

		while (a_bucket != -1)
		{
			Item& item = m_pSmth[a_bucket];
			TBOOL bRes = m_ItemCompareFunc(a_pData, (TINT*)m_pItems + m_iItemSize * (TINT)item.value, m_iItemSize);
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
			TUINT32 hash = m_ItemHashFunc(a_pData, m_iItemSize, m_iBucketSize);

			m_pSmth[m_iHashNodeCountTotal].key = m_pBuckets[hash];
			m_pSmth[m_iHashNodeCountTotal].value = (void*)m_iItemCount;

			TUtil::MemCopy((TINT*)m_pItems + m_iItemCount * m_iItemSize, a_pData, m_iItemSize);

			m_pBuckets[hash] = m_iHashNodeCountTotal;

			m_iItemCount++;
			m_iHashNodeCountTotal++;

			return &m_pSmth[m_iHashNodeCountTotal];
		}
	}
	return TNULL;
}

TBOOL Toshi::THashTable::Create(TINT a_iItemCountTotal, TINT a_iItemSize, TINT a_iBucketSize, TINT a_iHashNodeCount)
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
		m_pBuckets = new TINT[m_iBucketSize];
		m_pSmth = new Item[m_iHashNodeCount];
		m_pItems = new TINT[m_iItemSize * m_iItemCountTotal];

		SetItemHashFunction(DefaultItemHashFunc);
		SetItemCompareFunction(DefaultItemCompareFunc);

		for (TINT i = 0; i < m_iBucketSize; i++)
		{
			m_pBuckets[i] = -1;
		}

		return TTRUE;
	}
	return TFALSE;
}
