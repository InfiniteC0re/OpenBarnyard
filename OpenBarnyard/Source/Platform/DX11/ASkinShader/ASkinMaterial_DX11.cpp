#include "pch.h"
#include "ASkinMaterial_DX11.h"
#include "ASkinShader_DX11.h"

#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASkinMaterialHAL );

ASkinMaterialHAL::ASkinMaterialHAL()
    : m_pAlphaBlendMaterial( TNULL ), m_pAssignedOrderTable( TNULL ), m_pSomeTexture( TNULL ), m_bIsHDLighting( TTRUE )
{
}

ASkinMaterialHAL::~ASkinMaterialHAL()
{
	if ( TNULL != m_pRegMaterial )
	{
		TOrderTable::DeregisterMaterial( m_pRegMaterial );
	}

	if ( TNULL != m_pAlphaBlendMaterial )
	{
		delete m_pAlphaBlendMaterial;
	}
}

void ASkinMaterialHAL::OnDestroy()
{
	if ( m_pAssignedOrderTable )
	{
		TOrderTable::DeregisterMaterial( m_pRegMaterial );
		m_pAssignedOrderTable = TNULL;
	}

	BaseClass::OnDestroy();
}

void ASkinMaterialHAL::PreRender()
{
	auto pRender = TRenderD3DInterface::Interface();

	TTextureResourceHAL* pTexture    = TDYNAMICCAST( TTextureResourceHAL, m_pTexture );
	TTextureResourceHAL* pLT0Texture = TNULL;
	TTextureResourceHAL* pLT1Texture = TNULL;

	ASkinShaderHAL::sm_eRenderStateFlags = ASkinShaderHAL::RENDERSTATE_ALPHABLEND;

	if ( m_bIsHDLighting )
	{
		pLT0Texture = TDYNAMICCAST( TTextureResourceHAL, m_apLightingTextures[ LT_0 ] );
		pLT1Texture = TDYNAMICCAST( TTextureResourceHAL, m_apLightingTextures[ LT_1 ] );

		pRender->SetTextureAddress( 1, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );
		pRender->SetTextureAddress( 2, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );
		pRender->SetTextureAddress( 3, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );
		pRender->SetTextureAddress( 4, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );

		pRender->SetTextureMagFilter( 1, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		pRender->SetTextureMagFilter( 2, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		pRender->SetTextureMagFilter( 3, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		pRender->SetTextureMagFilter( 4, TRenderD3DInterface::TEXTUREFILTER_LINEAR );

		m_bHasLighting1Tex = pLT0Texture != TNULL;
		m_bHasLighting2Tex = pLT1Texture != TNULL;
	}

	if ( pTexture != TNULL )
	{
		pTexture->Validate();

		if ( pTexture->GetD3DTexture() != TNULL )
		{
			pRender->SetTexture( 0, pTexture->GetD3DTexture() );

			if ( pLT1Texture == TNULL )
			{
				pRender->SetTexture( 1, TNULL );
				pRender->SetTexture( 2, TNULL );
			}
			else
			{
				pRender->SetTexture( 1, pLT1Texture->GetD3DTexture() );
				pRender->SetTextureMipFilter( 1, TRenderD3DInterface::TEXTUREFILTER_NONE );

				if ( pLT0Texture == TNULL )
				{
					pRender->SetTexture( 2, TNULL );
				}
				else
				{
					pRender->SetTexture( 2, pLT0Texture->GetD3DTexture() );
					pRender->SetTextureMipFilter( 2, TRenderD3DInterface::TEXTUREFILTER_NONE );
				}
			}

			pRender->SetTextureMipFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
			pRender->SetTextureAddress( 0, pTexture->GetAddressUState(), TEXCOORD_U );
			pRender->SetTextureAddress( 0, pTexture->GetAddressVState(), TEXCOORD_V );
		}
	}
	else
	{
		pRender->SetTexture( 0, TNULL );
	}

	if ( m_eBlendMode == BLENDMODE_DEFAULT || m_eBlendMode == BLENDMODE_ALPHA || m_eBlendMode != BLENDMODE_ADDITIVE )
	{
		pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
		pRender->SetZWriteEnable( TTRUE );
	}
	else
	{
		pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		pRender->SetDestBlend( D3D11_BLEND_ONE );
		pRender->SetZWriteEnable( TFALSE );
	}

	auto pShader = TDYNAMICCAST( ASkinShaderHAL, GetShader() );
	pShader->SetAlphaRef( ( m_Flags & FLAGS_BLENDING ) ? 1 : 128 );

	pRender->SetCullMode( D3D11_CULL_NONE );

	auto pRenderContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	if ( pRenderContext->IsFogEnabled() )
		pRenderContext->EnableFogHAL();
}

void ASkinMaterialHAL::PostRender()
{
	auto pRender = TRenderD3DInterface::Interface();
	auto pShader = TDYNAMICCAST( ASkinShaderHAL, GetShader() );

	pRender->SetTexture( 0, TNULL );
	pRender->SetTexture( 1, TNULL );
	pRender->SetTexture( 2, TNULL );
	pRender->SetZWriteEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TTRUE );
	pShader->SetAlphaRef( 128 );

	if ( m_Flags & FLAGS_NO_CULL )
	{
		pRender->SetCullMode( pShader->IsEnableRenderEnvMap() == TTRUE ? D3D11_CULL_BACK : D3D11_CULL_FRONT );
	}

	pRender->SetFogEnable( TFALSE );
	ASkinShaderHAL::sm_eRenderStateFlags = ASkinShaderHAL::RENDERSTATE_ALPHABLEND;
}

TBOOL ASkinMaterialHAL::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return BaseClass::Create( a_eBlendMode );
}

void ASkinMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode )
{
	auto pShader = TDYNAMICCAST( ASkinShaderHAL, GetShader() );

	switch ( a_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			SetOrderTable( pShader->GetOrderTable( 0 ) );
			break;
		case BLENDMODE_ALPHA:
		case 5:
			SetOrderTable( pShader->GetOrderTable( 1 ) );
			break;
		case BLENDMODE_MODULATE:
		case 6:
			SetOrderTable( pShader->GetOrderTable( 1 ) );
			break;
		case BLENDMODE_ADDITIVE:
		case 4:
			SetOrderTable( pShader->GetOrderTable( 1 ) );
			break;
		case 7:
			SetOrderTable( pShader->GetOrderTable( 2 ) );
			break;
	}

	BaseClass::SetBlendMode( a_eBlendMode );
}

void ASkinMaterialHAL::CopyToAlphaBlendMaterial()
{
	if ( TNULL != m_pAlphaBlendMaterial )
	{
		m_pAlphaBlendMaterial->m_iNumTex                    = m_iNumTex;
		m_pAlphaBlendMaterial->m_pTexture                   = m_pTexture;
		m_pAlphaBlendMaterial->m_apLightingTextures[ LT_0 ] = m_apLightingTextures[ LT_0 ];
		m_pAlphaBlendMaterial->m_apLightingTextures[ LT_1 ] = m_apLightingTextures[ LT_1 ];
		m_pAlphaBlendMaterial->m_apLightingTextures[ LT_2 ] = m_apLightingTextures[ LT_2 ];
		m_pAlphaBlendMaterial->m_apLightingTextures[ LT_3 ] = m_apLightingTextures[ LT_3 ];

		m_pAlphaBlendMaterial->SetFlags( TMaterial::FLAGS_BLENDING, TTRUE );
	}
}

void ASkinMaterialHAL::SetOrderTable( Toshi::TOrderTable* a_pOrderTable )
{
	if ( a_pOrderTable != m_pAssignedOrderTable )
	{
		if ( m_pAssignedOrderTable )
		{
			TOrderTable::DeregisterMaterial( GetRegMaterial() );
		}

		if ( a_pOrderTable )
		{
			a_pOrderTable->RegisterMaterial( this );
		}

		m_pAssignedOrderTable = a_pOrderTable;
	}
}

Toshi::TTextureResourceHAL* ASkinMaterialHAL::GetLightingTexture( ELightingTexture a_eTexture ) const
{
	TASSERT( a_eTexture < LT_NUMOF );
	return TDYNAMICCAST( TTextureResourceHAL, m_apLightingTextures[ a_eTexture ] );
}

Toshi::TTextureResourceHAL* ASkinMaterialHAL::GetSomeTexture() const
{
	return TDYNAMICCAST( TTextureResourceHAL, m_pSomeTexture );
}
