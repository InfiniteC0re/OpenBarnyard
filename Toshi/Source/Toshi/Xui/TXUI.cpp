#include "ToshiPCH.h"
#include "Toshi/Xui/TXUI.h"

namespace Toshi
{
	TMemoryHeap* TXUI::ms_pXUIMemoryBlock = TNULL;
	TMemoryHeap* TXUI::ms_pXUITRBMemoryBlock = TNULL;

	TTRB::t_MemoryFuncAlloc TXUI::AssetTRBAllocator = [](TTRB::AllocType alloctype, size_t size, short unk1, size_t unk2, void* userData)
	{
		return ms_pXUITRBMemoryBlock->Memalign(unk1, size);
	};

	TTRB::t_MemoryFuncDealloc TXUI::AssetTRBDeallocator = [](TTRB::AllocType alloctype, void* ptr, short unk1, size_t unk2, void* userData)
	{
		return ms_pXUITRBMemoryBlock->Free(ptr);
	};

	TXUI::TXUI()
	{
		m_trb->SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
		m_trb2->SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
		m_canvas = new (MemoryBlock()) TXUICanvas();
		m_canvas->SetDimensions(176.0f, 176.0f);
		//TRender::GetSingleton()->
	}

	void TXUIResourceTRB::Init()
	{
		if (m_resource == TNULL)
		{
			m_resource = new (TXUI::MemoryBlock()) TXUIResource();
			m_resource->Load(m_trbBuffer);
			TXUI::GetSingleton()->AddResource(this);
		}
	}
}