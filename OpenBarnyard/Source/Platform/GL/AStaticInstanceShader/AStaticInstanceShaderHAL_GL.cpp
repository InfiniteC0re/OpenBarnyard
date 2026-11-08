#include "pch.h"
#include "AStaticInstanceShaderHAL_GL.h"
#include "AStaticInstanceMaterialHAL_GL.h"
#include "AStaticInstanceMeshHAL_GL.h"

#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>

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

	TRenderGLInterface* pRender        = TRenderGLInterface::Interface();
	TRenderContextGL*   pRenderContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );

	pRender->SetTextureMinFilter( 0, TRenderGLInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMagFilter( 0, TRenderGLInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMipFilter( 0, TRenderGLInterface::TEXTUREFILTER_POINT );
	pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
	pRender->SetFixedFunctionShader();

	pRender->SetTransform( TRenderGLInterface::TRANSFORM_PROJECTION, pRenderContext->GetProjectionMatrix() );
	pRender->SetTransform( TRenderGLInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaRef( 1 );
	pRender->SetAlphaFunc( GL_GREATER );
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
