#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "TModelCollision.h"

namespace Toshi {

	namespace TTMDWin
	{
		struct TTRBWinHeader
		{
			char* m_pTXSModelName;
			int m_iLODCount;
			float m_fUnknown;
			TTMDBase::SkeletonHeader* m_pSkeletonHeader;
			TSkeleton* m_pSkeleton;
			TModelCollision* m_pModelCollision;
		};
	};

}