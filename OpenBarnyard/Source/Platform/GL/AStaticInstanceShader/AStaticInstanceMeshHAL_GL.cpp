#include "pch.h"
#include "AStaticInstanceMeshHAL_GL.h"

#include <Render/TOrderTable.h>
#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>

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
	TRenderGLInterface* pRenderInterface = TRenderGLInterface::Interface();
	TRenderContextGL*   pCurrentContext  = TRenderContextGL::Upcast( pRenderInterface->GetCurrentContext() );

	TRenderPacket* pRenderPacket = GetMaterial()->GetRegMaterial()->AddRenderPacket( this );
	pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );

	return TTRUE;
}

// NOTE: folded (00622df0)
void AStaticInstanceMeshHAL::SetUnknown( void* a_pUnk )
{
	m_pUnk24 = a_pUnk;
}
