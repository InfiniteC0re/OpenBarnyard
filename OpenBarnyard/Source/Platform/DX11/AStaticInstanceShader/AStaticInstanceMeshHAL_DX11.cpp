#include "pch.h"
#include "AStaticInstanceMeshHAL_DX11.h"

#include <Render/TOrderTable.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AStaticInstanceMeshHAL );

// NOTE: inlined in AStaticInstanceShaderHAL::CreateMesh and the create functions of the class
AStaticInstanceMeshHAL::AStaticInstanceMeshHAL()
    : m_pUnk24( TNULL )
{
}

AStaticInstanceMeshHAL::~AStaticInstanceMeshHAL()
{
}

// NOTE: folded with AGrassMeshHAL::Render (005f8870)
TBOOL AStaticInstanceMeshHAL::Render()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

	TRenderPacket* pRenderPacket = GetMaterial()->GetRegMaterial()->AddRenderPacket( this );
	pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );

	return TTRUE;
}

// NOTE: folded (00622df0)
void AStaticInstanceMeshHAL::SetUnknown( void* a_pUnk )
{
	m_pUnk24 = a_pUnk;
}
