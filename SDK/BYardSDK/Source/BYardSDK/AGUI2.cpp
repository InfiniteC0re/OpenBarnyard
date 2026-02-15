#include "pch.h"
#include "AGUI2.h"
#include "SDK_T2GUITextureSectionManager.h"
#include "SDKHooks.h"

#include <Toshi/TSystem.h>

TOSHI_NAMESPACE_USING

void AGUI2::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	m_pRootElement->GetDimensions( a_rWidth, a_rHeight );
}

SDK_T2GUIRenderer* AGUI2::GetRenderer()
{
	TASSERT( ms_ppCurrentContext != TNULL );
	return ( *ms_ppCurrentContext )->GetRenderer();
}
