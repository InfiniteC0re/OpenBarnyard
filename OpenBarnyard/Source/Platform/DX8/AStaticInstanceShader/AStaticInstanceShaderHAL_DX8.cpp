#include "pch.h"
#include "AStaticInstanceShaderHAL_DX8.h"
#include "AStaticInstanceMaterialHAL_DX8.h"
#include "AStaticInstanceMeshHAL_DX8.h"

#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005f8ec0, CreateObject 005f93c0, CreateObjectInPlace 005f9420
TDEFINE_CLASS( AStaticInstanceShaderHAL );

// $Barnyard: FUNCTION 005f8ef0
AStaticInstanceShaderHAL::AStaticInstanceShaderHAL()
{
}

// $Barnyard: FUNCTION 005f9630
// $Barnyard: FUNCTION 005f8f40
AStaticInstanceShaderHAL::~AStaticInstanceShaderHAL()
{
}

// $Barnyard: FUNCTION 005f8fd0
void AStaticInstanceShaderHAL::Flush()
{
	Validate();

	IDirect3DDevice8*  pDevice        = TRenderD3DInterface::Interface()->GetDirect3DDevice();
	TRenderContextD3D* pRenderContext = TRenderContextD3D::Upcast( TRenderD3DInterface::Interface()->GetCurrentContext() );

	pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, 1 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 1 );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
	pDevice->SetVertexShader( D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ );

	static TMatrix44 s_Identity = TMatrix44::IDENTITY;

	pDevice->SetTransform( D3DTS_PROJECTION, pRenderContext->GetProjectionMatrix() );
	pDevice->SetTransform( D3DTS_VIEW, s_Identity );
	pDevice->SetPixelShader( 0 );

	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHAREF, 1 );
	pDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );
	pDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_LIGHTING, 0 );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

	m_oOrderTable.Render();

	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
}

// $Barnyard: FUNCTION 005f8fa0
TBOOL AStaticInstanceShaderHAL::Create()
{
	m_oOrderTable.Create( this, -380 );
	TBOOL bCreated = AStaticInstanceShader::Create();
	Validate();

	return bCreated;
}

// NOTE: folded with ASysShaderHAL::Validate (005f0640)
TBOOL AStaticInstanceShaderHAL::Validate()
{
	if ( !IsValidated() )
		return AStaticInstanceShader::Validate();

	return TTRUE;
}

// NOTE: folded with ASysShaderHAL::Invalidate (005f0660)
void AStaticInstanceShaderHAL::Invalidate()
{
	if ( IsValidated() )
		AStaticInstanceShader::Invalidate();
}

// NOTE: folded with ASysShaderHAL::TryInvalidate (005f3690)
TBOOL AStaticInstanceShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

// NOTE: folded with ASysShaderHAL::TryValidate (005f36a0)
TBOOL AStaticInstanceShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 005f96e0
AStaticInstanceMaterial* AStaticInstanceShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	AStaticInstanceMaterialHAL* pMaterial = new AStaticInstanceMaterialHAL();
	pMaterial->SetShader( this );

	m_oOrderTable.RegisterMaterial( pMaterial );
	return pMaterial;
}

// $Barnyard: FUNCTION 005f9650
AStaticInstanceMesh* AStaticInstanceShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	AStaticInstanceMeshHAL* pMesh = new AStaticInstanceMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}
