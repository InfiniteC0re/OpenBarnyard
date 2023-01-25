#include "ToshiPCH.h"
#include "TAssetInit.h"

namespace Toshi
{
	bool TAssetInit::g_bCreateResources = true;
	bool TAssetInit::g_bAllowCrossTRBReferences;
	TTRB* TAssetInit::g_pCurrentTRB;
	TMemoryHeap* TAssetInit::g_pMemHeap = TNULL;

	void TAssetInit::InitAssets(TTRB& a_trb, bool createResources, bool allowCrossTRBReferences)
	{
		int funcCount = 0;
		const FourCCFunction* functions;

		if (!createResources)
		{
			funcCount = sizeof(g_FourCCReloadFunctions) / sizeof(*g_FourCCReloadFunctions);
			functions = g_FourCCReloadFunctions;
		}
		else
		{
			funcCount = sizeof(g_FourCCInitFunctions) / sizeof(*g_FourCCInitFunctions);
			functions = g_FourCCInitFunctions;
		}

		g_bAllowCrossTRBReferences = allowCrossTRBReferences;
		g_bCreateResources = createResources;
		g_pCurrentTRB = &a_trb;

		for (size_t i = 0; i < funcCount; i++)
		{
			Init(a_trb, functions[i].name, functions[i].func);
		}

		g_pCurrentTRB = TNULL;
	}

	void TAssetInit::DeinitAssets(TTRB& a_trb)
	{
		const FourCCFunction* functions = g_FourCCDeinitFunctions;
		for (size_t i = 0; i < 12; i++)
		{
			TAssetInit::Init(a_trb, functions[i].name, functions[i].func);
		}
	}

	void TAssetInit::Init(TTRB& a_trb, uint32_t a_unk, t_fourCCFunction a_fourCCFunc)
	{
		TTRB::SYMB* symb = a_trb.GetSymbolTable();

		for (size_t i = 0; i < symb->m_i32SymbCount; i++)
		{
			TTRB::TTRBSymbol* index = a_trb.GetSymbol(i);
			const char* symbName = a_trb.GetSymbolName(index);
			if (TMAKEFOUR(symbName) == a_unk)
			{
				a_fourCCFunc(a_trb.GetSymbolAddress(*index));
			}
		}
	}
}