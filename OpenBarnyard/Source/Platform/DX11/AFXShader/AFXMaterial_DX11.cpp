#include "pch.h"
#include "AFXMaterial_DX11.h"
#include "AFXShader_DX11.h"

#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>
#include <Render/TOrderTable.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AFXMaterialHAL );

// NOTE: Inlined in CreateObject (005fba70), CreateObjectInPlace (005fbae0) and AFXShaderHAL::CreateMaterial
AFXMaterialHAL::AFXMaterialHAL()
    : m_pAlphaBlendMaterial( TNULL )
{
}

AFXMaterialHAL::~AFXMaterialHAL()
{
	if ( m_pRegMaterial )
		TOrderTable::DeregisterMaterial( m_pRegMaterial );

	if ( m_pAlphaBlendMaterial )
		delete m_pAlphaBlendMaterial;
}

void AFXMaterialHAL::PreRender()
{
	auto pRender  = TRenderD3DInterface::Interface();
	auto pTexture = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	if ( pTexture )
	{
		pTexture->Validate();

		if ( pTexture->GetD3DTexture() )
		{
			pRender->SetTexture( 0, pTexture->GetD3DTexture() );
			pRender->SetTextureMipFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		}
	}
	else
	{
		pRender->SetTexture( 0, TNULL );
	}

	if ( m_eBlendMode == BLENDMODE_ADDITIVE )
	{
		pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		pRender->SetDestBlend( D3D11_BLEND_ONE );
		pRender->SetZWriteEnable( TFALSE );
	}
	else
	{
		pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
	}

	auto pShader = TSTATICCAST( AFXShaderHAL, m_pShader );

	if ( m_Flags & FLAGS_BLENDING )
	{
		pRender->SetAlphaRef( 1 );
		pShader->m_iAlphaRef = 1;
	}
	else
	{
		pRender->SetAlphaRef( 128 );
		pShader->m_iAlphaRef = 128;
	}

	pRender->SetCullMode( D3D11_CULL_NONE );
}

void AFXMaterialHAL::PostRender()
{
	auto pRender = TRenderD3DInterface::Interface();
	auto pShader = TSTATICCAST( AFXShaderHAL, m_pShader );

	pRender->SetTexture( 0, TNULL );
	pRender->SetAlphaRef( 128 );
	pRender->SetZWriteEnable( TTRUE );
	pShader->m_iAlphaRef = 128;

	if ( m_Flags & FLAGS_NO_CULL )
		pRender->SetCullMode( pShader->IsEnableRenderEnvMap() == TTRUE ? D3D11_CULL_BACK : D3D11_CULL_FRONT );
}

void AFXMaterialHAL::CopyToAlphaBlendMaterial()
{
	if ( m_pAlphaBlendMaterial )
	{
		m_pAlphaBlendMaterial->m_iNumTex        = m_iNumTex;
		m_pAlphaBlendMaterial->m_aTextures[ 0 ] = m_aTextures[ 0 ];
		m_pAlphaBlendMaterial->m_aTextures[ 1 ] = m_aTextures[ 1 ];
		m_pAlphaBlendMaterial->m_aTextures[ 2 ] = m_aTextures[ 2 ];
		m_pAlphaBlendMaterial->m_aTextures[ 3 ] = m_aTextures[ 3 ];

		m_pAlphaBlendMaterial->SetFlags( TMaterial::FLAGS_BLENDING, TTRUE );
	}
}
