#pragma once
#include "Toshi/Render/TModel.h"
#include "Toshi/Render/TTMDWin.h"
#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{
	class TModelHAL : public TModel
	{
	public:
		using t_TRBLoadCallback2 = TBOOL(*)(TModel* pModel, TTMDWin::TTRBWinHeader* pHeader);

	public:
		TBOOL Create(TTMDWin::TTRBWinHeader* pTMDHeader);

		static void SetTRBLoadCallback2(t_TRBLoadCallback2 fnCallback)
		{
			sm_pTRBLoadCallback2 = fnCallback;
		}

	private:
		inline static t_TRBLoadCallback2 sm_pTRBLoadCallback2;

	private:
		TTMDWin::TTRBWinHeader* m_pTMDHeader; // 0xBC
		char* m_pTXSModelName;                // 0xDC
	};

	//class TModelHAL
	//{
	//	TTMDWin::TTRBWinHeader* m_tmdHeader; // 0xBC
	//	char* m_txsModelName; // 0xDC

	//public:
	//	void Create(TTMDWin::TTRBWinHeader* a_tmdHeader);
	//};
}
