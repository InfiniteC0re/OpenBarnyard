#pragma once

TOSHI_NAMESPACE_START

class THashTable
{
public:
	struct Bucket
	{
		TUINT iNextBucketId;
		TINT  iItemIndex;
	};

	inline static constexpr TINT INVALID_BUCKET_ID = -1;

	using t_ItemCompareFunc = TBOOL ( * )( const void* a_pMem1, const void* a_pMem2, TSIZE a_iSize );
	using t_ItemHashFunc    = TUINT32 ( * )( const void* a_pMem, TSIZE a_iSize, TUINT32 a_uiMaxValue );

public:
	THashTable();
	~THashTable();

	TINT* GetHashToBucketIds() const { return m_pHashToBucketId; }
	TINT  GetItemCount() const { return m_iItemCount; }
	void* GetItemArray() const { return m_pItems; }

	void DeleteBucketMemory();
	void Destroy();

	Bucket* Find( void* a_pData );
	Bucket* Insert( void* a_pData );
	Bucket* Append( void* a_pData );

	TBOOL Create( TINT a_iItemCountTotal, TINT a_iItemSize, TINT a_iBucketSize, TINT a_iHashNodeCount );
	void  SetItemCompareFunction( t_ItemCompareFunc a_HashCompareFunc ) { m_ItemCompareFunc = a_HashCompareFunc; }
	void  SetItemHashFunction( t_ItemHashFunc a_HashCompareFunc ) { m_ItemHashFunc = a_HashCompareFunc; }

private:
	static t_ItemCompareFunc DefaultItemCompareFunc;
	static t_ItemHashFunc    DefaultItemHashFunc;

private:
	TINT              m_iBucketSize;         // 0x0
	TINT              m_iItemSize;           // 0x4
	TINT              m_iItemCount;          // 0x8
	TINT              m_iItemCountTotal;     // 0xC
	TINT              m_iHashNodeCount;      // 0x10
	TINT              m_iHashNodeCountTotal; // 0x14
	TINT*             m_pHashToBucketId;     // 0x18
	Bucket*           m_pBuckets;            // 0x1C
	TBYTE*            m_pItems;              // 0x20
	t_ItemHashFunc    m_ItemHashFunc;        // 0x24
	t_ItemCompareFunc m_ItemCompareFunc;     // 0x28
};

TOSHI_NAMESPACE_END
