#include "ToshiPCH.h"
#include "TRenderContext.h"

namespace Toshi
{
	void TRenderContext::SetModelViewMatrix(const TMatrix44& a_rMatrix)
	{
		m_eFlags |= 0x300;
		m_mModelViewMatrix = TMatrix44(a_rMatrix);
		m_eFlags &= ~0x300;
	}

	void TRenderContext::SetWorldViewMatrix(const TMatrix44& a_rMatrix)
	{
		m_eFlags |= 0x100;
		m_mModelViewMatrix = TMatrix44(a_rMatrix);
		m_eFlags &= ~0x100;
	}
}