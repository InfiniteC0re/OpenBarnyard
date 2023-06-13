#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi2/T2GUI/T2GUIContext.h"
#include "Toshi/Xui/TXUIRenderer.h"
#include "Toshi/Xui/TXUICanvas.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/File/TTRB.h"

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
		uint8_t* m_xurBuffer;     // 0x8
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

	public:
		char m_Str[128];            // 0x0000
		TTRB m_TRB1;                // 0x0080
		TTRB m_TRB2;                // 0x0094
		TTRB m_TRB3;                // 0x00A8
		T2GUIContext* m_pContext;   // 0x015C
		T2GUIRenderer* m_pRenderer; // 0x0160
		TXUICanvas* m_pCanvas;      // 0x014C
	};
}