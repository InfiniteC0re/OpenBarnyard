#pragma once
#include <File/TTRB.h>
#include <Toshi/T2SList.h>

class ATerrainSection;

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
	friend class ATerrainInterface;
	friend class ATerrainVIS;
	friend class ATerrainSection;

public:
	ATerrainLODBlock( TUINT a_uiHeapSize, const TCHAR* a_szName );
	~ATerrainLODBlock();

	void SetupTRB( Toshi::TTRB* a_pTRB, ATerrainLODBlock* a_pOther );
	void UpdateLastAccessTime();

	void Assign( ATerrainSection* a_pVISGroup, ATerrainLODType a_eLODType );

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

	AMemoryPoolAllocator&	  GetAllocator() { return m_Allocator; }
	Toshi::TMemory::MemBlock* GetMemBlock() { return m_pCreatedMemBlock; }

private:
	TUINT32*				  m_pAllocatedSize;
	TUINT					  m_Unk1;
	TUINT*					  m_pUnk2;
	ATerrainSection*		  m_pVISGroup;
	ATerrainLODType			  m_eLODType;
	TFLOAT					  m_fLastAccessTime;
	TUINT					  m_uiHeapSize;
	Toshi::TMemory::MemBlock* m_pCreatedMemBlock;
	AMemoryPoolAllocator	  m_Allocator;
	TBOOL					  m_bIsUnused;
};
