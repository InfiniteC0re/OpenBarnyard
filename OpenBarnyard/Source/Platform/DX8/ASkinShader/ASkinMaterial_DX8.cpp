#include "pch.h"
#include "ASkinMaterial_DX8.h"
#include "ASkinShader_DX8.h"

#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

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
	auto pRender    = TRenderD3DInterface::Interface();
	auto pD3DDevice = pRender->GetDirect3DDevice();

	TTextureResourceHAL* pTexture    = TDYNAMICCAST( TTextureResourceHAL, m_pTexture );
	TTextureResourceHAL* pLT0Texture = TNULL;
	TTextureResourceHAL* pLT1Texture = TNULL;

	ASkinShaderHAL::sm_eRenderStateFlags = 0x1b;

	if ( m_bIsHDLighting )
	{
		pLT0Texture = TDYNAMICCAST( TTextureResourceHAL, m_apLightingTextures[ LT_0 ] );
		pLT1Texture = TDYNAMICCAST( TTextureResourceHAL, m_apLightingTextures[ LT_1 ] );

		pRender->SetTextureAddress( 1, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );
		pRender->SetTextureAddress( 2, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );
		pRender->SetTextureAddress( 3, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );
		pRender->SetTextureAddress( 4, ADDRESSINGMODE_CLAMP, TEXCOORD_UV );

		pD3DDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, 2 );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_MAGFILTER, 2 );
		pD3DDevice->SetTextureStageState( 3, D3DTSS_MAGFILTER, 2 );
		pD3DDevice->SetTextureStageState( 4, D3DTSS_MAGFILTER, 2 );

		m_bHasLighting1Tex = pLT0Texture != TNULL;
		m_bHasLighting2Tex = pLT1Texture != TNULL;
	}

	if ( pTexture != TNULL )
	{
		pTexture->Validate();

		if ( pTexture->GetD3DTexture() != TNULL )
		{
			pD3DDevice->SetTexture( 0, pTexture->GetD3DTexture() );

			if ( pLT1Texture == TNULL )
			{
				pD3DDevice->SetTexture( 1, TNULL );
				pD3DDevice->SetTexture( 2, TNULL );
			}
			else
			{
				pD3DDevice->SetTexture( 1, pLT1Texture->GetD3DTexture() );
				pD3DDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, 0 );

				if ( pLT0Texture == TNULL )
				{
					pD3DDevice->SetTexture( 2, TNULL );
				}
				else
				{
					pD3DDevice->SetTexture( 2, pLT0Texture->GetD3DTexture() );
					pD3DDevice->SetTextureStageState( 2, D3DTSS_MIPFILTER, 0 );
				}
			}

			pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, 2 );
			pRender->SetTextureAddress( 0, pTexture->GetAddressUState(), TEXCOORD_U );
			pRender->SetTextureAddress( 0, pTexture->GetAddressVState(), TEXCOORD_V );
		}
	}
	else
	{
		pD3DDevice->SetTexture( 0, TNULL );
	}

	if ( m_eBlendMode == 0 || m_eBlendMode == 1 || m_eBlendMode != 3 )
	{
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	}
	else
	{
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 2 );
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	}

	if ( m_Flags & FLAGS_NO_CULL )
		pD3DDevice->SetRenderState( D3DRS_CULLMODE, 1 );

	auto pShader = TDYNAMICCAST( ASkinShaderHAL, GetShader() );
	pShader->SetAlphaRef( ( m_Flags & FLAGS_BLENDING ) ? 1 : 128 );

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, 0 );
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, 1 );

	auto pRenderContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	if ( pRenderContext->IsFogEnabled() )
		pRenderContext->EnableFogHAL();
}

void ASkinMaterialHAL::PostRender()
{
	auto pRender    = TRenderD3DInterface::Interface();
	auto pD3DDevice = pRender->GetDirect3DDevice();
	auto pShader    = TDYNAMICCAST( ASkinShaderHAL, GetShader() );

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
	pD3DDevice->SetTexture( 0, TNULL );
	pD3DDevice->SetTexture( 1, TNULL );
	pD3DDevice->SetTexture( 2, TNULL );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

	pShader->SetAlphaRef( 128 );

	if ( m_Flags & FLAGS_NO_CULL )
	{
		pD3DDevice->SetRenderState( D3DRS_CULLMODE, pShader->IsEnableRenderEnvMap() ? D3DCULL_CCW : D3DCULL_CW );
	}

	pD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	ASkinShaderHAL::sm_eRenderStateFlags = 0x1b;
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
		case 0:
			SetOrderTable( pShader->GetOrderTable( 0 ) );
			break;
		case 1:
		case 5:
			SetOrderTable( pShader->GetOrderTable( 1 ) );
			break;
		case 2:
		case 6:
			SetOrderTable( pShader->GetOrderTable( 1 ) );
			break;
		case 3:
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
