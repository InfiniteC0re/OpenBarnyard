#pragma once
namespace Toshi
{
	class THashTable
	{
	public:

		struct Item 
		{
			TUINT32 key;
			void* value;
		};

		using t_ItemCompareFunc = TBOOL (*) (void* unk, void* unk2, TINT unk3);
		using t_ItemHashFunc = TUINT32 (*) (void* unk, TINT unk2, TINT unk3);

		TINT m_iBucketSize;	                 // 0x0
		TINT m_iItemSize;                     // 0x4
		TINT m_iItemCount;                    // 0x8
		TINT m_iItemCountTotal;               // 0xC
		TINT m_iHashNodeCount;                // 0x10
		TINT m_iHashNodeCountTotal;           // 0x14
		TINT* m_pBuckets;                     // 0x18
		Item* m_pSmth;                       // 0x1C
		void* m_pItems;                      // 0x20
		t_ItemHashFunc m_ItemHashFunc;       // 0x24
		t_ItemCompareFunc m_ItemCompareFunc; // 0x28

	public:

		THashTable()
		{
			m_iBucketSize = 0;
			m_iItemSize = 0;
			m_iItemCount = 0;
			m_iItemCountTotal = 0;
			m_iHashNodeCount = 0;
			m_iHashNodeCountTotal = 0;
			m_pBuckets = TNULL;
			m_pSmth = TNULL;
			m_pItems = TNULL;
			m_ItemHashFunc = TNULL;
			m_ItemCompareFunc = TNULL;
		}

		static t_ItemCompareFunc DefaultItemCompareFunc;
		static t_ItemHashFunc DefaultItemHashFunc;

		TINT* GetBuckets() const { return m_pBuckets; }
		TINT GetItemCount() const { return m_iItemCount; }
		void* GetItemArray() const { return m_pItems; }

		void DeleteBucketMemory()
		{
			delete[] m_pBuckets;
			m_pBuckets = TNULL;
		}

		void Destroy()
		{
			delete[] m_pBuckets;
			m_pBuckets = TNULL;
			delete[] m_pSmth;
			m_pSmth = TNULL;
			delete[] m_pItems;
			m_pItems = TNULL;
		}


		void* Insert(void* a_pData);
		void* Find(void* a_pData);
		void* Append(void* a_pData);

		TBOOL Create(TINT a_iItemCountTotal, TINT a_iItemSize, TINT unk, TINT a_iHashNodeCount);
		void SetItemCompareFunction(t_ItemCompareFunc a_HashCompareFunc) { m_ItemCompareFunc = a_HashCompareFunc; }
		void SetItemHashFunction(t_ItemHashFunc a_HashCompareFunc) { m_ItemHashFunc = a_HashCompareFunc; }
	};
}


