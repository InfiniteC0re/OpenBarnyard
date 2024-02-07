#include "pch.h"
#include "AGUI2.h"
#include "AGUI2TextureSectionManager.h"
#include "SDKHooks.h"

#include <Toshi/TSystem.h>

TOSHI_NAMESPACE_USING

void AGUI2::GetDimensions(TFLOAT& a_rWidth, TFLOAT& a_rHeight)
{
	m_pRootElement->GetDimensions(a_rWidth, a_rHeight);
}

AGUI2Renderer* AGUI2::GetRenderer()
{
	TASSERT(ms_ppCurrentContext != TNULL);
	return (*ms_ppCurrentContext)->GetRenderer();
}
