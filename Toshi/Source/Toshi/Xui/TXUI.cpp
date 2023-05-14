#include "ToshiPCH.h"
#include "Toshi/Xui/TXUI.h"
#include <Toshi/Render/TAssetInit.h>

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
		m_trb.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
		m_trb2.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
		m_canvas = new (MemoryBlock()) TXUICanvas();
		m_canvas->SetDimensions(176.0f, 176.0f);
		//TRender::GetSingleton()->
	}

	void TXUI::SetDefaultFont(const char* a_pData)
	{
		m_trb.Load(a_pData);
		TAssetInit::InitAssets(m_trb, true, false);

	}

	void TXUIResourceTRB::Init()
	{
		return;
		if (m_resource == TNULL)
		{
			m_resource = new (TXUI::MemoryBlock()) TXUIResource();
			bool bRes = m_resource->Load(m_xurBuffer);
			TASSERT(TTRUE == bRes, "Could not load XUR");
			TXUI::GetSingleton()->AddResource(this);
		}
	}

	void TXUIResourceTRB::Deinit()
	{
		m_resource->~TXUIResource();
		TXUI::GetSingleton()->RemoveResource(this);
	}
}