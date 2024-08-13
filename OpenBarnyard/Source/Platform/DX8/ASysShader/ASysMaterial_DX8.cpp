#include "pch.h"
#include "ASysMaterial_DX8.h"

#ifdef TOSHI_SKU_WINDOWS
#	include "Platform/DX8/TRenderInterface_DX8.h"
#	include "Platform/DX8/TTextureResourceHAL_DX8.h"
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASysMaterialHAL );

ASysMaterialHAL::ASysMaterialHAL()
{
	m_pAssignedOrderTable = TNULL;
	m_pTexture			  = TNULL;
	m_Unk1				  = 0;
	m_Unk2				  = 0;
	m_Unk3				  = 0;
}

ASysMaterialHAL::~ASysMaterialHAL()
{
	auto pRegMat = GetRegMaterial();

	if ( pRegMat )
	{
		TOrderTable::DeregisterMaterial( pRegMat );
	}
}

void ASysMaterialHAL::SetOrderTable( Toshi::TOrderTable* a_pOrderTable )
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

TBOOL ASysMaterialHAL::Create( BLENDMODE a_eBlendMode )
{
	// This is a bug in the original code
	// Should probably be ASysMaterial::SetBlendMode instead of just SetBlendMode
	SetBlendMode( a_eBlendMode );
	ASysMaterialHAL::SetBlendMode( a_eBlendMode );

	return TMaterial::Create();
}

void ASysMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode )
{
	TASSERT( a_eBlendMode >= BLENDMODE_DEFAULT && a_eBlendMode < BLENDMODE_NUMOF );

	switch ( a_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 0 ) );
			break;
		case BLENDMODE_5:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 3 ) );
			break;
		case BLENDMODE_6:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 3 ) );
			break;
		case BLENDMODE_7:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 1 ) );
			break;
		case BLENDMODE_8:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 2 ) );
			break;
		case BLENDMODE_9:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 3 ) );
			break;
		default:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 1 ) );
			break;
	}

	ASysMaterial::SetBlendMode( a_eBlendMode );
}

void ASysMaterialHAL::PreRender()
{
	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

	if ( m_pTexture )
	{
		TTextureResourceHAL* pTextureHAL = TSTATICCAST( TTextureResourceHAL, m_pTexture );
		pTextureHAL->Validate();

		if ( pTextureHAL->GetD3DTexture() )
		{
			( pDevice->SetTexture )( 0, pTextureHAL->GetD3DTexture() );
			TRenderD3DInterface::Interface()->SetTextureAddress( 0, pTextureHAL->GetAddress(), -1 );
		}
	}
	else
	{
		pDevice->SetTexture( 0, TNULL );
	}

	if ( m_Flags & FLAGS_NO_CULL )
	{
		( pDevice->SetRenderState )( D3DRS_CULLMODE, D3DCULL_NONE );
	}

	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			break;
		case BLENDMODE_1:
		case BLENDMODE_8:
		case BLENDMODE_9:
		case BLENDMODE_10:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		case BLENDMODE_2:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 1 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 3 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		case BLENDMODE_3:
		case BLENDMODE_6:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 2 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		case BLENDMODE_7:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 2 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			pDevice->SetRenderState( D3DRS_ZENABLE, 0 );
			break;
		default:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			break;
	}
}

void ASysMaterialHAL::PostRender()
{
	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

	pDevice->SetTexture( 0, TNULL );

	if ( m_Flags & FLAGS_NO_CULL )
	{
		pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	}

	TRenderD3DInterface::Interface()->SetTextureAddress( 0, 0, -1 );
	pDevice->SetTexture( 0, TNULL );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
}
