#include "pch.h"
#include "AWorldMaterial_DX11.h"
#include "AWorldShader_DX11.h"

#include <Toshi/TScheduler.h>
#include <Render/TOrderTable.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AWorldMaterialHAL );

AWorldMaterialHAL::AWorldMaterialHAL()
{
	m_pAlphaBlendMaterial = TNULL;
	m_pAssignedOrderTable = TNULL;

	for ( TUINT i = 0; i < MAX_TEXTURES; i++ )
	{
		m_aHasUVOffsets[ i ] = TFALSE;
		m_aUVOffsetsX[ i ]   = 0.0f;
		m_aUVOffsetsY[ i ]   = 0.0f;
	}
}

AWorldMaterialHAL::~AWorldMaterialHAL()
{
	if ( m_pRegMaterial )
	{
		TOrderTable::DeregisterMaterial( m_pRegMaterial );
	}

	if ( m_pAlphaBlendMaterial )
	{
		delete m_pAlphaBlendMaterial;
	}
}

void AWorldMaterialHAL::OnDestroy()
{
	if ( m_pAssignedOrderTable != TNULL )
	{
		TOrderTable::DeregisterMaterial( m_pRegMaterial );
		m_pAssignedOrderTable = TNULL;
	}

	BaseClass::OnDestroy();
}

void AWorldMaterialHAL::PreRender()
{
	auto pRender = TRenderD3DInterface::Interface();

	AWorldShaderHAL::s_RenderStateFlags = AWorldShaderHAL::RENDERSTATE_ALPHABLEND;

	// Setup textures
	for ( TUINT i = 0; i < TARRAYSIZE( m_aTextures ); i++ )
	{
		auto pTexture = TDYNAMICCAST( TTextureResourceHAL, m_aTextures[ i ] );

		if ( pTexture == TNULL )
		{
			pRender->SetTexture( i, TNULL );
		}
		else
		{
			pTexture->Validate();
			auto pD3DTexture = pTexture->GetD3DTexture();

			if ( pD3DTexture )
			{
				pRender->SetTexture( i, pD3DTexture );
				pRender->SetTextureMipFilter( i, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
				pRender->SetTextureMinFilter( i, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
				pRender->SetTextureMagFilter( i, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
				pRender->SetTextureAddress( i, pTexture->GetAddressUState(), TEXCOORD_U );
				pRender->SetTextureAddress( i, pTexture->GetAddressVState(), TEXCOORD_V );
			}
		}
	}

	// Animate UV
	TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
	m_fUVAnimX += fDeltaTime * m_fUVAnimSpeedX;
	m_fUVAnimY += fDeltaTime * m_fUVAnimSpeedY;

	// Make sure value of m_fUVAnimX is in [-1; 1] range so we won't overflow
	if ( m_fUVAnimX <= 1.0f )
	{
		if ( m_fUVAnimX < -1.0f && !isnan( m_fUVAnimX ) )
		{
			m_fUVAnimX += 1.0f;
		}
	}
	else
	{
		m_fUVAnimX -= 1.0f;
	}

	// Make sure value of m_fUVAnimY is in [-1; 1] range so we won't overflow
	if ( m_fUVAnimY <= 1.0f )
	{
		if ( m_fUVAnimY < -1.0f && !isnan( m_fUVAnimY ) )
		{
			m_fUVAnimY += 1.0f;
		}
	}
	else
	{
		m_fUVAnimY -= 1.0f;
	}

	// Save current UV offset
	m_aHasUVOffsets[ 0 ] = TTRUE;
	m_aUVOffsetsX[ 0 ]   = m_fUVAnimX;
	m_aUVOffsetsY[ 0 ]   = m_fUVAnimY;

	SetupRenderer();
}

void AWorldMaterialHAL::PostRender()
{
	auto pRender         = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	pRender->SetTexture( 0, TNULL );
	pRender->SetTexture( 1, TNULL );
	pRender->SetAlphaRef( 128 );
	pRender->SetZWriteEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TTRUE );
	TDYNAMICCAST( AWorldShaderHAL, m_pShader )->SetAlphaRef( 128 );
	pCurrentContext->DisableFogHAL();

	AWorldShaderHAL::s_RenderStateFlags = AWorldShaderHAL::RENDERSTATE_ALPHABLEND;
}

TBOOL AWorldMaterialHAL::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return BaseClass::Create( a_eBlendMode );
}

void AWorldMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode )
{
	auto pShader = TDYNAMICCAST( AWorldShaderHAL, m_pShader );

	switch ( a_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			SetOrderTable( pShader->GetOrderTable( 0 ) );
			break;
		case BLENDMODE_ALPHA:
			SetOrderTable( pShader->GetOrderTable( 2 ) );
			break;
		case BLENDMODE_MODULATE:
			SetOrderTable( pShader->GetOrderTable( 2 ) );
			break;
		case BLENDMODE_ADDITIVE:
			SetOrderTable( pShader->GetOrderTable( 3 ) );
			break;
		case 5:
			SetOrderTable( pShader->GetOrderTable( 6 ) );
			break;
		case 6:
			SetOrderTable( pShader->GetOrderTable( 4 ) );
			break;
		case BLENDMODE_START_RENDER:
			SetOrderTable( pShader->GetOrderTable( 7 ) );
			break;
		default:
			TASSERT( !"Invalid blend state" );
			break;
	}

	BaseClass::SetBlendMode( a_eBlendMode );
}

void AWorldMaterialHAL::CopyToAlphaBlendMaterial()
{
	if ( m_pAlphaBlendMaterial )
	{
		m_pAlphaBlendMaterial->m_iNumTex        = m_iNumTex;
		m_pAlphaBlendMaterial->m_aTextures[ 0 ] = m_aTextures[ 0 ];
		m_pAlphaBlendMaterial->m_aTextures[ 1 ] = m_aTextures[ 1 ];
		m_pAlphaBlendMaterial->m_aTextures[ 2 ] = m_aTextures[ 2 ];
		m_pAlphaBlendMaterial->m_aTextures[ 3 ] = m_aTextures[ 3 ];

		m_pAlphaBlendMaterial->SetFlags( FLAGS_BLENDING, TTRUE );
	}
}

void AWorldMaterialHAL::SetOrderTable( Toshi::TOrderTable* a_pOrderTable )
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

void AWorldMaterialHAL::SetupRenderer()
{
	// Upload UV offsets
	TVector4 vOffsets( 0, 0, 0, 0 );

	if ( m_aHasUVOffsets[ 0 ] )
	{
		vOffsets.x = m_aUVOffsetsX[ 0 ];
		vOffsets.y = m_aUVOffsetsY[ 0 ];
	}

	if ( m_aHasUVOffsets[ 1 ] )
	{
		vOffsets.z = m_aUVOffsetsX[ 1 ];
		vOffsets.w = m_aUVOffsetsY[ 1 ];
	}

	auto pRender         = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	pRender->SetVec4InVSBuffer( AWorldShaderHAL::CONSTANT_UVOFFSETS, &vOffsets );

	// Setup render state for the selected blending state
	switch ( m_eBlendMode )
	{
		case 1:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRender->SetZWriteEnable( TFALSE );
			break;
		case 3:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_ONE );
			pRender->SetZWriteEnable( TFALSE );
			break;
		default:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRender->SetZWriteEnable( TTRUE );
			break;
	}

	if ( m_Flags & FLAGS_BLENDING )
	{
		pRender->SetAlphaRef( 1 );
		TDYNAMICCAST( AWorldShaderHAL, m_pShader )->SetAlphaRef( 1 );
	}
	else
	{
		pRender->SetAlphaRef( 128 );
		TDYNAMICCAST( AWorldShaderHAL, m_pShader )->SetAlphaRef( 128 );
	}

	pRender->SetCullMode( D3D11_CULL_NONE );

	if ( pCurrentContext->IsFogEnabled() )
	{
		pCurrentContext->EnableFogHAL();
	}
}
