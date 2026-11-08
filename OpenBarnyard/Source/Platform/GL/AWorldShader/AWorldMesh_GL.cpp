#include "pch.h"
#include "AWorldMesh_GL.h"
#include "AWorldShader_GL.h"
#include "AWorldMaterial_GL.h"

#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AWorldMeshHAL );

TBOOL AWorldMeshHAL::Render()
{
	auto pRenderInterface = TRenderGLInterface::Interface();
	auto pCurrentContext  = TRenderContextGL::Upcast( pRenderInterface->GetCurrentContext() );

	TMaterial* pMaterial;

	if ( !TSTATICCAST( AWorldShaderHAL, m_pOwnerShader )->IsAlphaBlendMaterial() ||
	     pCurrentContext->GetAlphaBlend() >= 1.0f )
	{
		pMaterial = TSTATICCAST( AWorldMaterialHAL, m_pMaterial );
	}
	else
	{
		pMaterial = TSTATICCAST( AWorldMaterialHAL, m_pMaterial )->GetAlphaBlendMaterial();
	}

	auto pRenderPacket = pMaterial->AddRenderPacket( this );
	pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );
	pRenderPacket->SetAlpha( 1.0f );

	return TTRUE;
}
