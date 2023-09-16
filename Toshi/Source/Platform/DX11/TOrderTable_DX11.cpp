#include "ToshiPCH.h"
#include "TRender_DX11.h"

namespace Toshi {

	TBOOL TOrderTable::Create(TShader* pShader, int priority, uint8_t index)
	{
		m_pShader = pShader;
		m_pLastRegMat = TNULL;
		SetPriority(index);
		TRenderDX11::Interface()->GetOrderTables().Insert(this);
		m_iIndex = index;

		return TTRUE;
	}

}
