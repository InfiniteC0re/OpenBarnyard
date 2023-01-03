#pragma once
#include "Toshi/Utils/TSingleton.h"

#include <cstdint>
#include <Toshi/File/TTRB.h>
#include "Toshi/Render/TRender.h"
#include "Toshi/Xui/TXUICanvas.h"

namespace Toshi
{
	class TXUI : public TSingleton<TXUI>
	{
	public:

		TXUI();

		static TMemoryHeap* MemoryBlock() { return ms_pXUIMemoryBlock; }

	public:
		static TMemoryHeap* ms_pXUIMemoryBlock;
		static TMemoryHeap* ms_pXUITRBMemoryBlock;
		static TTRB::t_MemoryFuncAlloc AssetTRBAllocator;
		static TTRB::t_MemoryFuncDealloc AssetTRBDeallocator;

		// All globs offsets
		TTRB* m_trb; // 0x80
		TTRB* m_trb2; // 0x94

		TXUICanvas* m_canvas; // 0x134
		TXUICanvas* m_canvas; // 0x134

		// TDList at 0x160
	};
}