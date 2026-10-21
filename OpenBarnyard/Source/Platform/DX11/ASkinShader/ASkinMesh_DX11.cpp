#include "pch.h"
#include "ASkinMesh_DX11.h"
#include "ASkinShader_DX11.h"
#include "ASkinMaterial_DX11.h"

#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>

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
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

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
