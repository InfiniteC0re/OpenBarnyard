#include "pch.h"
#include "AFXMaterial_DX8.h"
#include "AFXShader_DX8.h"

#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <Render/TOrderTable.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005f9aa0
// $Barnyard: FUNCTION 005fba70
// $Barnyard: FUNCTION 005fbae0
TDEFINE_CLASS( AFXMaterialHAL );

// NOTE: Inlined in CreateObject (005fba70), CreateObjectInPlace (005fbae0) and AFXShaderHAL::CreateMaterial
AFXMaterialHAL::AFXMaterialHAL()
    : m_pAlphaBlendMaterial( TNULL )
{
}

// $Barnyard: FUNCTION 005fb680
// $Barnyard: FUNCTION 005fbe60
AFXMaterialHAL::~AFXMaterialHAL()
{
	if ( m_pRegMaterial )
		TOrderTable::DeregisterMaterial( m_pRegMaterial );

	if ( m_pAlphaBlendMaterial )
		delete m_pAlphaBlendMaterial;
}

// $Barnyard: FUNCTION 005fb730
void AFXMaterialHAL::PreRender()
{
	auto pD3DDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();
	auto pTexture   = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	if ( pTexture )
	{
		pTexture->Validate();

		if ( pTexture->GetD3DTexture() )
		{
			pD3DDevice->SetTexture( 0, pTexture->GetD3DTexture() );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, 2 );
		}
	}
	else
	{
		pD3DDevice->SetTexture( 0, TNULL );
	}

	if ( m_eBlendMode == BLENDMODE_ADDITIVE )
	{
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 2 );
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	}
	else
	{
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
	}

	if ( m_Flags & FLAGS_NO_CULL )
		pD3DDevice->SetRenderState( D3DRS_CULLMODE, 1 );

	auto pShader = TSTATICCAST( AFXShaderHAL, m_pShader );

	if ( m_Flags & FLAGS_BLENDING )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 1 );
		pShader->m_iAlphaRef = 1;
	}
	else
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 128 );
		pShader->m_iAlphaRef = 128;
	}

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, FALSE );
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, 1 );
}

// $Barnyard: FUNCTION 005fb830
void AFXMaterialHAL::PostRender()
{
	auto pD3DDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();
	auto pShader    = TSTATICCAST( AFXShaderHAL, m_pShader );

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
	pD3DDevice->SetTexture( 0, TNULL );
	pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 128 );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pShader->m_iAlphaRef = 128;

	if ( m_Flags & FLAGS_NO_CULL )
	{
		if ( pShader->IsEnableRenderEnvMap() == TTRUE )
			pD3DDevice->SetRenderState( D3DRS_CULLMODE, 3 );
		else
			pD3DDevice->SetRenderState( D3DRS_CULLMODE, 2 );
	}
}

// $Barnyard: FUNCTION 005fb6f0
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
