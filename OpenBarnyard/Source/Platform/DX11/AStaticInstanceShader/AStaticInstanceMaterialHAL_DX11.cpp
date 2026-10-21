#include "pch.h"
#include "AStaticInstanceMaterialHAL_DX11.h"

#include <Render/TOrderTable.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AStaticInstanceMaterialHAL );

// NOTE: inlined in AStaticInstanceShaderHAL::CreateMaterial and the create functions of the class
AStaticInstanceMaterialHAL::AStaticInstanceMaterialHAL()
{
}

AStaticInstanceMaterialHAL::~AStaticInstanceMaterialHAL()
{
	if ( GetRegMaterial() )
		TOrderTable::DeregisterMaterial( GetRegMaterial() );
}

void AStaticInstanceMaterialHAL::PreRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TTextureResourceHAL* pTextureHAL      = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	if ( pTextureHAL )
	{
		pTextureHAL->Validate();

		if ( pTextureHAL->GetD3DTexture() )
		{
			pRenderInterface->SetTexture( 0, pTextureHAL->GetD3DTexture() );
			pRenderInterface->SetTextureAddress( 0, pTextureHAL->GetAddressing(), TEXCOORD_UV );
		}
	}

	if ( m_Flags & FLAGS_NO_CULL )
		pRenderInterface->SetCullMode( D3D11_CULL_NONE );

	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_ONE );
			pRenderInterface->SetDestBlend( D3D11_BLEND_ZERO );
			break;
		case BLENDMODE_MODULATE:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_ZERO );
			pRenderInterface->SetDestBlend( D3D11_BLEND_SRC_COLOR );
			break;
		case BLENDMODE_ADDITIVE:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRenderInterface->SetDestBlend( D3D11_BLEND_ONE );
			break;
		default:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRenderInterface->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			break;
	}

	pRenderInterface->SetZWriteEnable( TTRUE );

	TRenderContextD3D* pCurrentContext = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

	if ( pCurrentContext->IsFogEnabled() )
		pCurrentContext->EnableFogHAL();
}

void AStaticInstanceMaterialHAL::PostRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();

	pRenderInterface->SetTexture( 0, TNULL );

	if ( m_Flags & FLAGS_NO_CULL )
		pRenderInterface->SetCullMode( D3D11_CULL_FRONT );

	pRenderInterface->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pRenderInterface->SetTexture( 0, TNULL );
	pRenderInterface->SetZWriteEnable( TTRUE );
	pRenderInterface->SetFogEnable( TFALSE );
}
