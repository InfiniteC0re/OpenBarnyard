#include "ToshiPCH.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Render/TAssetInit.h"
#include "Toshi/Render/TRender.h"

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
		m_TRB1.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
		m_TRB2.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
		m_TRB3.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);

		auto pRender = TRender::GetSingletonWeak();
		auto pDisplayParams = pRender->GetCurrentDisplayParams();

		float fDisplayWidth = TSTATICCAST(float, pDisplayParams->Width);
		float fDisplayHeight = TSTATICCAST(float, pDisplayParams->Height);

		m_pContext = new (MemoryBlock()) T2GUIContext();
		m_pContext->GetRootElement()->SetDimensions(fDisplayWidth, fDisplayHeight);

		m_pCanvas = new (MemoryBlock()) TXUICanvas();
		m_pCanvas->SetDimensions(fDisplayWidth, fDisplayHeight);

		m_pContext->GetRootElement()->AddChildTail(m_pCanvas);

		m_pRenderer = new (MemoryBlock()) TXUIRenderer();
		m_Str[0] = '\0';

		TTODO("TXUIShapeCache, TGenericListener");
	}

	void TXUI::SetDefaultFont(const char* a_pData)
	{
		m_TRB1.Load(a_pData);
		TAssetInit::InitAssets(m_TRB1, TTRUE, TFALSE);

	}

	void TXUIResourceTRB::Init()
	{
		if (m_resource == TNULL)
		{
			m_resource = new (TXUI::MemoryBlock()) TXUIResource();
			TBOOL bRes = m_resource->Load(m_xurBuffer);
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