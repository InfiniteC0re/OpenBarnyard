#include "pch.h"
#include "AFXMesh_DX11.h"
#include "AFXShader_DX11.h"
#include "AFXMaterial_DX11.h"

#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AFXMeshHAL );

// BYTE_007789ec, never changed
static TBOOL s_bRenderEnabled = TTRUE;

// BYTE_0079af18, never set
static TBOOL s_bUseDefaultLighting = TFALSE;

// NOTE: Inlined in CreateObject (005fbb50), CreateObjectInPlace (005fbbc0) and AFXShaderHAL::CreateMesh
AFXMeshHAL::AFXMeshHAL()
{
}

AFXMeshHAL::~AFXMeshHAL()
{
}

TBOOL AFXMeshHAL::Render()
{
	if ( s_bRenderEnabled )
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

		if ( TSkeletonInstance* pSkeletonInstance = pCurrentContext->GetSkeletonInstance() )
		{
			TFLOAT     fAlpha = pCurrentContext->GetAlphaBlend();
			TMaterial* pMaterial;

			if ( TSTATICCAST( AFXShaderHAL, m_pOwnerShader )->IsAlphaBlendMaterial() && fAlpha < 1.0f )
				pMaterial = TSTATICCAST( AFXMaterialHAL, m_pMaterial )->GetAlphaBlendMaterial();
			else
				pMaterial = m_pMaterial;

			auto pRenderPacket = pMaterial->GetRegMaterial()->AddRenderPacket( this );
			pRenderPacket->SetModelViewMatrix( pCurrentContext->GetModelViewMatrix() );
			pRenderPacket->SetSkeletonInstance( pSkeletonInstance );
			pRenderPacket->SetAmbientColour( pCurrentContext->GetAmbientColour().AsVector3() );
			pRenderPacket->SetLightColour( pRenderInterface->GetLightColour().AsBasisVector3( 0 ) );
			pRenderPacket->SetLightDirection( pRenderInterface->GetLightDirection().AsBasisVector3( 0 ) );
			pRenderPacket->SetAlpha( fAlpha );

			if ( s_bUseDefaultLighting )
			{
				static TVector3 s_vAmbientColour = TVector3::VEC_ZERO;
				static TVector3 s_vLightColour   = TVector3( 1.0f, 1.0f, 1.0f );

				pRenderPacket->SetAmbientColour( s_vAmbientColour );
				pRenderPacket->SetLightColour( s_vLightColour );
			}

			AFXShaderHAL::sm_oWorldViewMatrix = pCurrentContext->GetWorldViewMatrix();
		}
	}

	return TTRUE;
}
