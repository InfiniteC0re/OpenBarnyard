#include "pch.h"
#include "AGrassMaterialHAL_DX11.h"

#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGrassMaterialHAL );

AGrassMaterialHAL::AGrassMaterialHAL()
{
}

AGrassMaterialHAL::~AGrassMaterialHAL()
{
}

void AGrassMaterialHAL::PreRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	TTextureResourceHAL* pTextureHAL      = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	extern TTextureResourceHAL* g_pGrassShaderTerrainTexture; // AGrassShaderHAL_DX11.cpp
	g_pGrassShaderTerrainTexture = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

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
			pRenderInterface->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRenderInterface->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRenderInterface->SetZWriteEnable( TTRUE );
			break;
		case BLENDMODE_ALPHA:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRenderInterface->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRenderInterface->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_MODULATE:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_ZERO );
			pRenderInterface->SetDestBlend( D3D11_BLEND_SRC_COLOR );
			pRenderInterface->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_ADDITIVE:
			pRenderInterface->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRenderInterface->SetDestBlend( D3D11_BLEND_ONE );
			pRenderInterface->SetZWriteEnable( TFALSE );
			break;
	}

	pCurrentContext->EnableFogHAL();
}

void AGrassMaterialHAL::PostRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();

	if ( m_Flags & FLAGS_NO_CULL )
		pRenderInterface->SetCullMode( D3D11_CULL_FRONT );

	pRenderInterface->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pRenderInterface->SetTexture( 0, TNULL );
	pRenderInterface->SetZWriteEnable( TTRUE );
}
