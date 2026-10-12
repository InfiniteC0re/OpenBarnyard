#include "pch.h"
#include "AStaticInstanceMeshHAL_DX8.h"

#include <Render/TOrderTable.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005f8ee0, CreateObject 005f9550, CreateObjectInPlace 005f95c0
TDEFINE_CLASS( AStaticInstanceMeshHAL );

// NOTE: inlined in AStaticInstanceShaderHAL::CreateMesh and the create functions of the class
AStaticInstanceMeshHAL::AStaticInstanceMeshHAL()
    : m_pUnk24( TNULL )
{
}

// $Barnyard: FUNCTION 005f93b0
// $Barnyard: FUNCTION 005f9790
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
