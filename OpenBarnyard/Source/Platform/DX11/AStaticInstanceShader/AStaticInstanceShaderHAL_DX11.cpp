#include "pch.h"
#include "AStaticInstanceShaderHAL_DX11.h"
#include "AStaticInstanceMaterialHAL_DX11.h"
#include "AStaticInstanceMeshHAL_DX11.h"

#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AStaticInstanceShaderHAL );

AStaticInstanceShaderHAL::AStaticInstanceShaderHAL()
{
}

AStaticInstanceShaderHAL::~AStaticInstanceShaderHAL()
{
}

void AStaticInstanceShaderHAL::Flush()
{
	Validate();

	TRenderD3DInterface* pRender        = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pRenderContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	pRender->SetTextureMinFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMagFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMipFilter( 0, TRenderD3DInterface::TEXTUREFILTER_POINT );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
	pRender->SetFixedFunctionShader();

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_PROJECTION, pRenderContext->GetProjectionMatrix() );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaRef( 1 );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
	pRender->SetFogEnable( TFALSE );

	m_oOrderTable.Render();

	pRender->SetAlphaTestEnable( TFALSE );
	pRender->SetAlphaBlendEnable( TFALSE );
	pRender->SetFogEnable( TFALSE );
}

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

AStaticInstanceMaterial* AStaticInstanceShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	AStaticInstanceMaterialHAL* pMaterial = new AStaticInstanceMaterialHAL();
	pMaterial->SetShader( this );

	m_oOrderTable.RegisterMaterial( pMaterial );
	return pMaterial;
}

AStaticInstanceMesh* AStaticInstanceShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	AStaticInstanceMeshHAL* pMesh = new AStaticInstanceMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}
