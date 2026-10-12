#include "pch.h"
#include "AStaticInstanceMaterialHAL_DX8.h"

#include <Render/TOrderTable.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005f8ed0, CreateObject 005f9460, CreateObjectInPlace 005f94e0
TDEFINE_CLASS( AStaticInstanceMaterialHAL );

// NOTE: inlined in AStaticInstanceShaderHAL::CreateMaterial and the create functions of the class
AStaticInstanceMaterialHAL::AStaticInstanceMaterialHAL()
{
}

// $Barnyard: FUNCTION 005f91b0
// $Barnyard: FUNCTION 005f9770
AStaticInstanceMaterialHAL::~AStaticInstanceMaterialHAL()
{
	if ( GetRegMaterial() )
		TOrderTable::DeregisterMaterial( GetRegMaterial() );
}

// $Barnyard: FUNCTION 005f9210
void AStaticInstanceMaterialHAL::PreRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pDevice          = pRenderInterface->GetDirect3DDevice();
	TTextureResourceHAL* pTextureHAL      = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	if ( pTextureHAL )
	{
		pTextureHAL->Validate();

		if ( pTextureHAL->GetD3DTexture() )
		{
			pDevice->SetTexture( 0, pTextureHAL->GetD3DTexture() );
			pRenderInterface->SetTextureAddress( 0, pTextureHAL->GetAddressing(), TEXCOORD_UV );
		}
	}

	if ( m_Flags & FLAGS_NO_CULL )
		pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
			break;
		case BLENDMODE_MODULATE:
			pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
			break;
		case BLENDMODE_ADDITIVE:
			pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
			break;
		default:
			pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			break;
	}

	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	TRenderContextD3D* pCurrentContext = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );

	if ( pCurrentContext->IsFogEnabled() )
		pCurrentContext->EnableFogHAL();
}

// $Barnyard: FUNCTION 005f9340
void AStaticInstanceMaterialHAL::PostRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pDevice          = pRenderInterface->GetDirect3DDevice();

	pDevice->SetTexture( 0, TNULL );

	if ( m_Flags & FLAGS_NO_CULL )
		pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	pRenderInterface->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pDevice->SetTexture( 0, TNULL );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
}
