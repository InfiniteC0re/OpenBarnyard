#include "pch.h"
#include "AGrassMeshHAL_DX8.h"

#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGrassMeshHAL );

AGrassMeshHAL::AGrassMeshHAL()
    : m_pUnk1( TNULL )
{
	
}

AGrassMeshHAL::~AGrassMeshHAL()
{
}

// $Barnyard: FUNCTION 005f8870
TBOOL AGrassMeshHAL::Render()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

	TRenderPacket* pRenderPacket = GetMaterial()->GetRegMaterial()->AddRenderPacket( this );
	pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );

	return TTRUE;
}

// $Barnyard: FUNCTION 005f8830
void AGrassMeshHAL::SetUnknown( void* a_pUnk )
{
	m_pUnk1 = a_pUnk;
}

void AGrassMeshHAL::CreateHAL( TUINT32 a_eFlags, TUINT16 a_uiMaxVertices, CellMeshSphere* a_pCellMeshSphere )
{
	m_pCellMeshSphere = a_pCellMeshSphere;
	AGrassMesh::Create( a_eFlags, a_uiMaxVertices );
}
