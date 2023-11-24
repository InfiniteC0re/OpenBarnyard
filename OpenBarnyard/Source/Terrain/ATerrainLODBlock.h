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
	friend class ATerrain;
	friend class ATerrainVIS;

public:
	ATerrainLODBlock(TUINT a_uiHeapSize, const char* a_szName);
	~ATerrainLODBlock();

	void SetupTRB(Toshi::TTRB* a_pTRB, ATerrainLODBlock* a_pOther);
	void UpdateLastAccessTime();

	void Assign(ATerrainVISGroup* a_pVISGroup, ATerrainLODType a_eLODType);

	void SetUsed()
	{
		UpdateLastAccessTime();
		m_bIsUnused = TFALSE;
	}

	void SetUnused()
	{
		m_bIsUnused = TTRUE;
	}

	TBOOL IsUsed() const { return !m_bIsUnused; }
	TBOOL IsUnused() const { return m_bIsUnused; }

	AMemoryPoolAllocator& GetAllocator() { return m_Allocator; }
	Toshi::TMemoryHeap* GetHeap() { return m_pCreatedHeap; }

private:
	TUINT32* m_pAllocatedSize;
	TUINT m_Unk1;
	TUINT* m_pUnk2;
	ATerrainVISGroup* m_pVISGroup;
	ATerrainLODType m_eLODType;
	TFLOAT m_fLastAccessTime;
	TUINT m_uiHeapSize;
	Toshi::TMemoryHeap* m_pCreatedHeap;
	AMemoryPoolAllocator m_Allocator;
	TBOOL m_bIsUnused;
};
