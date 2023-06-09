#pragma once
#include "Toshi/Render/TModel.h"
#include "Toshi/Render/TTMDBase.h"
#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{
	namespace TTMDWin
	{
		struct TTRBWinHeader
		{
			char* m_pTXSModelName;
			int m_iLODCount;
			uint32_t m_Unknown;
			TTMDBase::SkeletonHeader* m_pSkeletonHeader;
			TSkeleton* m_pSkeleton;
			TModelCollision* m_pModelCollision;
		};
	};

	class TModelHAL : public TModel
	{
	public:
		TBOOL Create(TTMDWin::TTRBWinHeader* pTMDHeader);

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
