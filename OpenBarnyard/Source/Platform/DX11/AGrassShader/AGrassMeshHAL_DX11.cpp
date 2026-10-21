#include "pch.h"
#include "AGrassMeshHAL_DX11.h"

#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>

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

TBOOL AGrassMeshHAL::Render()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

	TRenderPacket* pRenderPacket = GetMaterial()->GetRegMaterial()->AddRenderPacket( this );
	pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );

	return TTRUE;
}

void AGrassMeshHAL::SetUnknown( void* a_pUnk )
{
	m_pUnk1 = a_pUnk;
}

void AGrassMeshHAL::CreateHAL( TUINT32 a_eFlags, TUINT16 a_uiMaxVertices, CellMeshSphere* a_pCellMeshSphere )
{
	m_pCellMeshSphere = a_pCellMeshSphere;
	AGrassMesh::Create( a_eFlags, a_uiMaxVertices );
}
