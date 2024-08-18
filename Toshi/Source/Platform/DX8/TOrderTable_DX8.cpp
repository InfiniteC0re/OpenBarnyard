#include "ToshiPCH.h"
#include "TRenderInterface_DX8.h"
#include "Render/TOrderTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL TOrderTable::Create( TShader* a_pShader, TINT a_iPriority )
{
	m_pLastRegMat = TNULL;
	m_pShader     = a_pShader;

	TNode::SetPriority( a_iPriority );
	TRenderD3DInterface::Interface()->GetOrderTables().Insert( this );

	return TTRUE;
}

TOSHI_NAMESPACE_END
