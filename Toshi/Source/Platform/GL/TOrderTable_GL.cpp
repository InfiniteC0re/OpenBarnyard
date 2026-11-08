#include "ToshiPCH.h"
#include "TRenderInterface_GL.h"
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
	TRenderGLInterface::Interface()->RegisterOrderTable( this );

	return TTRUE;
}

TOSHI_NAMESPACE_END
