#pragma once
namespace Toshi
{
	class THashTable
	{
		using t_HashCompareFunc = bool (*) (void*, void*, int);

		int m_iItemSize; // 0x4
		void* m_pBuckets; // 0x18
		t_HashCompareFunc m_HashCompareFunc; // 0x28

	public:

		void Create(int capacity, int itemSize, int unk, int unk2);
		void SetItemCompareFunction(t_HashCompareFunc a_HashCompareFunc) { m_HashCompareFunc = a_HashCompareFunc; }
	};
}


