#include "ToshiPCH.h"
#include "Render/TOrderTable.h"
#include "Platform/GL/T2Render_GL.h"

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
	g_pRenderGL->RegisterOrderTable( this );

	return TTRUE;
}

TOSHI_NAMESPACE_END