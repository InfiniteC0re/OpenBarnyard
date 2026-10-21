#include "pch.h"
#include "ASysMaterial_DX11.h"

#include "Platform/DX11/TRenderInterface_DX11.h"
#include "Platform/DX11/TTextureResourceHAL_DX11.h"

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
	m_pTexture            = TNULL;
	m_Unk1                = 0;
	m_Unk2                = 0;
	m_Unk3                = 0;
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
		case BLENDMODE_UNTEXTURED_SKY:
			SetOrderTable( GetSysShaderHAL()->GetOrderTable( 2 ) );
			break;
		case BLENDMODE_TEXTURED_SKY:
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
	auto pRender = TRenderD3DInterface::Interface();

	if ( m_pTexture )
	{
		TTextureResourceHAL* pTextureHAL = TSTATICCAST( TTextureResourceHAL, m_pTexture );
		pTextureHAL->Validate();

		if ( pTextureHAL->GetD3DTexture() )
		{
			pRender->SetTexture( 0, pTextureHAL->GetD3DTexture() );
			pRender->SetTextureAddress( 0, pTextureHAL->GetAddressing(), TEXCOORD_UV );
		}
	}
	else
	{
		pRender->SetTexture( 0, TNULL );
	}

	if ( m_Flags & FLAGS_NO_CULL )
	{
		pRender->SetCullMode( D3D11_CULL_NONE );
	}

	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRender->SetZWriteEnable( TTRUE );
			break;
		case BLENDMODE_1:
		case BLENDMODE_UNTEXTURED_SKY:
		case BLENDMODE_TEXTURED_SKY:
		case BLENDMODE_10:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRender->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_2:
			pRender->SetSrcBlend( D3D11_BLEND_ZERO );
			pRender->SetDestBlend( D3D11_BLEND_SRC_COLOR );
			pRender->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_3:
		case BLENDMODE_6:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_ONE );
			pRender->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_7:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_ONE );
			pRender->SetZWriteEnable( TFALSE );
			pRender->SetZEnable( TFALSE );
			break;
		default:
			pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
			pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
			pRender->SetZWriteEnable( TTRUE );
			break;
	}
}

void ASysMaterialHAL::PostRender()
{
	auto pRender = TRenderD3DInterface::Interface();
	pRender->SetTexture( 0, TNULL );

	if ( m_Flags & FLAGS_NO_CULL )
	{
		pRender->SetCullMode( D3D11_CULL_FRONT );
	}

	pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pRender->SetTexture( 0, TNULL );
	pRender->SetZWriteEnable( TTRUE );
	pRender->SetZEnable( TTRUE );
}
