#include "pch.h"
#include "ATerrainLODBlock.h"
#include "ATerrainSection.h"

#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ATerrainLODBlock::ATerrainLODBlock(TUINT a_uiHeapSize, const TCHAR* a_szName)
{
	m_pAllocatedSize = TNULL;
	m_pUnk2 = 0;
	m_pVISGroup = TNULL;
	m_eLODType = ATerrainLODType_None;
	m_fLastAccessTime = 0.0f;
	m_uiHeapSize = a_uiHeapSize;
	m_bIsUnused = TTRUE;

	m_pCreatedMemBlock = g_pMemory->CreateMemBlock(m_uiHeapSize, a_szName, g_pMemory->GetGlobalBlock(), 0);
	m_Allocator.SetMemBlock(m_pCreatedMemBlock);
}

ATerrainLODBlock::~ATerrainLODBlock()
{
	g_pMemory->DestroyMemBlock(m_pCreatedMemBlock);
	m_pCreatedMemBlock = (TMemory::MemBlock*)1;
	m_Allocator.SetMemBlock(TNULL);

	static TUINT32 s_AllocatedSizeAfterDestroy = 0;
	static TUINT32 s_Unk2AfterDestroy = 0;

	m_pAllocatedSize = &s_AllocatedSizeAfterDestroy;
	m_pUnk2 = &s_Unk2AfterDestroy;
}

void ATerrainLODBlock::SetupTRB(TTRB* a_pTRB, ATerrainLODBlock* a_pOther)
{
	Node::InsertAfter(a_pOther);
	a_pTRB->SetMemoryFunctions(
		[](TTRB::AllocType alloctype, TUINT32 size, TINT16 unk1, TUINT32 unk2, void* userData) -> void*
		{
			auto pBlock = TSTATICCAST(ATerrainLODBlock, userData);

			if (pBlock->m_pAllocatedSize)
			{
				*pBlock->m_pAllocatedSize += size;
			}

			return TMemalign(128, size, pBlock->Next()->m_pCreatedMemBlock);
		},
		[](TTRB::AllocType alloctype, void* ptr, TINT16 unk1, TUINT32 unk2, void* userData)
		{
			TFree(ptr);
		},
		a_pOther
	);
}

void ATerrainLODBlock::UpdateLastAccessTime()
{
	m_fLastAccessTime = g_oSystemManager.GetScheduler()->GetTotalTime();
}

void ATerrainLODBlock::Assign(ATerrainSection* a_pVISGroup, ATerrainLODType a_eLODType)
{
	if (m_pVISGroup)
	{
		m_pVISGroup->DestroyLOD(m_eLODType);
		m_pVISGroup = TNULL;
		m_eLODType = ATerrainLODType_None;
	}

	m_fLastAccessTime = 0.0f;
	m_pVISGroup = a_pVISGroup;
	m_eLODType = a_eLODType;

	if (a_pVISGroup)
	{
		UpdateLastAccessTime();
	}
	else
	{
		m_bIsUnused = TTRUE;
	}
}
