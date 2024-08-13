#include "pch.h"
#include "AWorldMaterial_DX8.h"
#include "AWorldShader_DX8.h"

#include <Toshi/TScheduler.h>
#include <Render/TOrderTable.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

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
		m_aUVOffsetsX[ i ]	 = 0.0f;
		m_aUVOffsetsY[ i ]	 = 0.0f;
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
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pD3DDevice		  = pRenderInterface->GetDirect3DDevice();

	AWorldShaderHAL::s_RenderStateFlags = 0x1B;

	// Setup textures
	for ( TUINT i = 0; i < TARRAYSIZE( m_aTextures ); i++ )
	{
		auto pTexture = TDYNAMICCAST( TTextureResourceHAL, m_aTextures[ i ] );

		if ( pTexture == TNULL )
		{
			pD3DDevice->SetTexture( i, NULL );
		}
		else
		{
			pTexture->Validate();
			auto pD3DTexture = pTexture->GetD3DTexture();

			if ( pD3DTexture )
			{
				pD3DDevice->SetTexture( i, pD3DTexture );
				pD3DDevice->SetTextureStageState( i, D3DTSS_MIPFILTER, 2 );
				pD3DDevice->SetTextureStageState( i, D3DTSS_MINFILTER, 2 );
				pD3DDevice->SetTextureStageState( i, D3DTSS_MAGFILTER, 2 );
				pRenderInterface->SetTextureAddress( i, pTexture->GetAddressUState(), 0 );
				pRenderInterface->SetTextureAddress( i, pTexture->GetAddressVState(), 1 );
			}
		}
	}

	if ( m_iNumTex > 1 )
	{
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 0xd );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, 2 );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, 1 );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 0xd );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, 2 );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, 1 );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, 1 );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, 1 );
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
	m_aUVOffsetsX[ 0 ]	 = m_fUVAnimX;
	m_aUVOffsetsY[ 0 ]	 = m_fUVAnimY;

	SetupRenderer();
}

void AWorldMaterialHAL::PostRender()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	auto pD3DDevice		  = pRenderInterface->GetDirect3DDevice();

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
	pD3DDevice->SetTexture( 0, NULL );
	pD3DDevice->SetTexture( 1, NULL );
	pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 128 );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

	TDYNAMICCAST( AWorldShaderHAL, m_pShader )->SetAlphaRef( 128 );
	pCurrentContext->DisableFogHAL();

	if ( m_iNumTex > 1 )
	{
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
	}

	AWorldShaderHAL::s_RenderStateFlags = 0x1B;
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
		case 0:
			SetOrderTable( pShader->GetOrderTable( 0 ) );
			break;
		case 1:
			SetOrderTable( pShader->GetOrderTable( 2 ) );
			break;
		case 2:
			SetOrderTable( pShader->GetOrderTable( 2 ) );
			break;
		case 3:
			SetOrderTable( pShader->GetOrderTable( 3 ) );
			break;
		case 5:
			SetOrderTable( pShader->GetOrderTable( 6 ) );
			break;
		case 6:
			SetOrderTable( pShader->GetOrderTable( 4 ) );
			break;
		case 7:
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
		m_pAlphaBlendMaterial->m_iNumTex		= m_iNumTex;
		m_pAlphaBlendMaterial->m_aTextures[ 0 ] = m_aTextures[ 0 ];
		m_pAlphaBlendMaterial->m_aTextures[ 1 ] = m_aTextures[ 1 ];
		m_pAlphaBlendMaterial->m_aTextures[ 2 ] = m_aTextures[ 2 ];
		m_pAlphaBlendMaterial->m_aTextures[ 3 ] = m_aTextures[ 3 ];

		m_pAlphaBlendMaterial->SetFlags( FLAGS_HAS_BLENDMODE, TTRUE );
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

	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	auto pD3DDevice		  = pRenderInterface->GetDirect3DDevice();
	pD3DDevice->SetVertexShaderConstant( 7, &vOffsets, 1 );

	// Setup render state for the selected blending state

	switch ( m_eBlendMode )
	{
		case 1:
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		case 3:
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 2 );
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		default:
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			break;
	}

	if ( m_Flags & FLAGS_HAS_BLENDMODE )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 1 );
		TDYNAMICCAST( AWorldShaderHAL, m_pShader )->SetAlphaRef( 1 );
	}
	else
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 128 );
		TDYNAMICCAST( AWorldShaderHAL, m_pShader )->SetAlphaRef( 128 );
	}

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, FALSE );
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	if ( pCurrentContext->IsFogEnabled() )
	{
		pCurrentContext->EnableFogHAL();
	}
}
