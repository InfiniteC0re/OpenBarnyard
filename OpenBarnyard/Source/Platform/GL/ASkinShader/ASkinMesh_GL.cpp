#include "pch.h"
#include "ASkinMesh_GL.h"
#include "ASkinShader_GL.h"
#include "ASkinMaterial_GL.h"

#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TTextureResourceHAL_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASkinMeshHAL );

ASkinMeshHAL::~ASkinMeshHAL()
{
}

TBOOL ASkinMeshHAL::Render()
{
	auto pRenderInterface = TRenderGLInterface::Interface();
	auto pCurrentContext  = TRenderContextGL::Upcast( pRenderInterface->GetCurrentContext() );

	if ( TSkeletonInstance* pSkeletonInstance = pCurrentContext->GetSkeletonInstance() )
	{
		TMaterial* pMaterial;

		if ( !TDYNAMICCAST( ASkinShaderHAL, m_pOwnerShader )->IsAlphaBlendMaterial() ||
		     pCurrentContext->GetAlphaBlend() >= 1.0f )
		{
			pMaterial = TDYNAMICCAST( ASkinMaterialHAL, m_pMaterial );
		}
		else
		{
			pMaterial = TDYNAMICCAST( ASkinMaterialHAL, m_pMaterial )->GetAlphaBlendMaterial();
		}

		auto pRenderPacket = pMaterial->AddRenderPacket( this );
		pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );
		pRenderPacket->SetSkeletonInstance( pSkeletonInstance );
		pRenderPacket->SetAmbientColour( pCurrentContext->GetAmbientColour().AsVector3() );
		pRenderPacket->SetLightColour( pRenderInterface->GetLightColour().AsBasisVector3( 0 ) );
		pRenderPacket->SetLightDirection( pRenderInterface->GetLightDirection().AsBasisVector3( 0 ) );
		pRenderPacket->SetAlpha( pCurrentContext->GetAlphaBlend() );
		pRenderPacket->SetShadeCoeff( TUINT( pCurrentContext->GetShadeCoeff() * 255.0f ) );

		ASkinShaderHAL::sm_oWorldViewMatrix = pCurrentContext->GetWorldViewMatrix();
	}

	return TTRUE;
}
