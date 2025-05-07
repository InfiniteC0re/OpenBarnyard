#include "pch.h"
#include "AGrassMaterialHAL_DX8.h"

#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

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

// $Barnyard: FUNCTION 005f80c0
// $Barnyard: FUNCTION 005f8810
AGrassMaterialHAL::~AGrassMaterialHAL()
{
}

// $Barnyard: FUNCTION 005f8120
void AGrassMaterialHAL::PreRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	IDirect3DDevice8*    pDevice          = pRenderInterface->GetDirect3DDevice();
	TTextureResourceHAL* pTextureHAL      = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	extern TTextureResourceHAL* g_pGrassShaderTerrainTexture; // AGrassShaderHAL_DX8.cpp
	g_pGrassShaderTerrainTexture = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

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
		pDevice->SetRenderState( D3DRS_CULLMODE, 1 );

	switch ( m_eBlendMode )
	{
		case 0:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			break;
		case 1:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		case 2:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 1 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 3 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		case 3:
			pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
			pDevice->SetRenderState( D3DRS_DESTBLEND, 2 );
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
	}

	pCurrentContext->EnableFogHAL();
}

// $Barnyard: FUNCTION 005f8220
void AGrassMaterialHAL::PostRender()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pDevice          = pRenderInterface->GetDirect3DDevice();

	if ( m_Flags & FLAGS_NO_CULL )
		pDevice->SetRenderState( D3DRS_CULLMODE, 2 );

	pRenderInterface->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pDevice->SetTexture( 0, TNULL );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
}
