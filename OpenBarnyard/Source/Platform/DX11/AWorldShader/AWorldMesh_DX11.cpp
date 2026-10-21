#include "pch.h"
#include "AWorldMesh_DX11.h"
#include "AWorldShader_DX11.h"
#include "AWorldMaterial_DX11.h"

#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AWorldMeshHAL );

TBOOL AWorldMeshHAL::Render()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

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
