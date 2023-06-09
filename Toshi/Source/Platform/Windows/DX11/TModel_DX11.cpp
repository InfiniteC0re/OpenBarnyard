#pragma once
#include "ToshiPCH.h"
#include "TModel_DX11.h"

namespace Toshi {
	
	TBOOL TModelHAL::Create(TTMDWin::TTRBWinHeader* pTMDHeader)
	{
		TASSERT(TFALSE == IsCreated());

		m_pTMDHeader = pTMDHeader;
		m_pResourceName = pTMDHeader->m_pTXSModelName;
		CreateSkeleton(pTMDHeader->m_pSkeletonHeader, pTMDHeader->m_pSkeleton, TTRUE);
		CreateCollision(pTMDHeader->m_pModelCollision);

		m_Flags.Set(Flags::Created | Flags::TrbLoaded);
		sm_pTRBLoadCallback2(this, pTMDHeader);

		return TTRUE;
	}

}