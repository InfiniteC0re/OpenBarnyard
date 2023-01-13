#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/File/TTRB.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Xui/TXUICanvas.h"

namespace Toshi
{
	class TXUIResourceTRB
	{
	public:
		void Init();
		void Deinit();

	private:
		uint8_t* m_unk;
		uint8_t* m_fileName;
		uint8_t* m_xurBuffer; // 0x8
		TXUIResource* m_resource; // 0xC
	};

	class TXUI : public TSingleton<TXUI>
	{
	public:
		TXUI();

		static TMemoryHeap* MemoryBlock() { return ms_pXUIMemoryBlock; }

		void AddResource(TXUIResourceTRB* a_resourceTrb) {};
		void RemoveResource(TXUIResourceTRB* a_resourceTrb) {};
		void SetDefaultFont(const char* a_pData);

	public:
		static TMemoryHeap* ms_pXUIMemoryBlock;
		static TMemoryHeap* ms_pXUITRBMemoryBlock;
		static TTRB::t_MemoryFuncAlloc AssetTRBAllocator;
		static TTRB::t_MemoryFuncDealloc AssetTRBDeallocator;

		// All globs offsets
		TTRB* m_trb; // 0x80
		TTRB* m_trb2; // 0x94

		TXUICanvas* m_canvas; // 0x134

		// TDList at 0x160
	};
}