#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi2/T2SList.h>

class ATerrainVISGroup;

using ATerrainLODType = TINT8;
enum ATerrainLODType_ : ATerrainLODType
{
	ATerrainLODType_None = -1,
	ATerrainLODType_High,
	ATerrainLODType_Low,
	ATerrainLODType_NUMOF,
};

class ATerrainLODBlock :
	public Toshi::T2SList<ATerrainLODBlock>::Node
{
public:
	friend class ATerrainVIS;

public:
	ATerrainLODBlock(TUINT a_uiHeapSize, const char* a_szName);
	~ATerrainLODBlock();

	void SetupTRB(Toshi::TTRB* a_pTRB, ATerrainLODBlock* a_pOther);

	AMemoryPoolAllocator& GetAllocator() { return m_Allocator; }
	Toshi::TMemoryHeap* GetHeap() { return m_pCreatedHeap; }

private:
	TUINT32* m_pAllocatedSize;
	TUINT m_Unk1;
	TUINT* m_pUnk2;
	ATerrainVISGroup* m_pGroup;
	ATerrainLODType m_eLODType;
	TFLOAT m_fLastUpdateTime;
	TUINT m_uiHeapSize;
	Toshi::TMemoryHeap* m_pCreatedHeap;
	AMemoryPoolAllocator m_Allocator;
	TBOOL m_bLoaded;
};
