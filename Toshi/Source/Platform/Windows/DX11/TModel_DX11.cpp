#pragma once
#include "ToshiPCH.h"
#include "TModel_DX11.h"

namespace Toshi {
	
	TBOOL TModelHAL::Create(TTMDWin::TTRBWinHeader* pTMDHeader)
	{
        CreateResource(pTMDHeader->m_pTXSModelName);

		m_pTMDHeader = pTMDHeader;
		m_pName = pTMDHeader->m_pTXSModelName;
		CreateSkeleton(pTMDHeader->m_pSkeletonHeader, pTMDHeader->m_pSkeleton, TTRUE);
		CreateCollision(pTMDHeader->m_pModelCollision);

		m_Flags.Set(Flags::Created | Flags::TrbLoaded);

		if (sm_pTRBLoadCallback2)
		{
			sm_pTRBLoadCallback2(this, pTMDHeader);
		}

		return TTRUE;
	}

}