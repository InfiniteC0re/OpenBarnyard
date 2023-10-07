#pragma once
#include "Toshi/File/TTRB.h"

namespace Toshi
{
	class TAssetInit
	{
	public:
		TAssetInit() = delete;

		static TBOOL g_bCreateResources;
		static TBOOL g_bAllowCrossTRBReferences;
		static TTRB* g_pCurrentTRB;
		static TMemoryHeap* g_pMemHeap;

	private:
		using t_fourCCFunction = void (*)(void* a_pData);

		struct FourCCFunction
		{
			uint32_t name;
			t_fourCCFunction func;
		};

	public:
		static FourCCFunction constexpr g_FourCCReloadFunctions[5] = {
			
		};

		static FourCCFunction constexpr g_FourCCInitFunctions[12] = {
			
		};

		static FourCCFunction constexpr g_FourCCDeinitFunctions[12] = {
			
		};

	public:
		static void InitAssets(TTRB& a_trb, TBOOL createResources, TBOOL allowCrossTRBReferences);
		static void DeinitAssets(TTRB& a_trb);
		static void Init(TTRB& a_trb, uint32_t a_unk, t_fourCCFunction a_fourCCFunc);
		static TTRB* GetInitialisingTRB() { return g_pCurrentTRB; }
	};
}