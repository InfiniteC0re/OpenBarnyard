#include "pch.h"
#include "ASysShader_GL.h"
#include "ASysMaterial_GL.h"
#include "ASysMesh_GL.h"

#include "Platform/GL/TRenderInterface_GL.h"
#include "Platform/GL/TRenderContext_GL.h"
#include "Platform/GL/TVertexPoolResource_GL.h"
#include "Platform/GL/TIndexPoolResource_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASysShaderHAL );

ASysShaderHAL::ASysShaderHAL()
{
}

ASysShaderHAL::~ASysShaderHAL()
{
}

void ASysShaderHAL::Flush()
{
	StartFlush();
	m_aOrderTables[ 0 ].Render();
	EndFlush();
}

void ASysShaderHAL::StartFlush()
{
	Validate();

	auto pRender        = TRenderGLInterface::Interface();
	auto pRenderContext = TSTATICCAST( TRenderContextGL, pRender->GetCurrentContext() );

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
}

void ASysShaderHAL::EndFlush()
{
	auto pRender = TRenderGLInterface::Interface();

	pRender->SetAlphaTestEnable( TFALSE );
	pRender->SetAlphaBlendEnable( TFALSE );
}

TBOOL ASysShaderHAL::Create()
{
	TASSERT( TFALSE == IsCreated() );

	m_aOrderTables[ 0 ].Create( this, -1000 );
	m_aOrderTables[ 1 ].Create( this, 2000 );
	m_aOrderTables[ 2 ].Create( this, -6000 );
	m_aOrderTables[ 3 ].Create( this, -5990 );

	ASysShader::Create();
	return Validate();
}

TBOOL ASysShaderHAL::Validate()
{
	if ( !IsValidated() )
	{
		return ASysShader::Validate();
	}

	return TTRUE;
}

void ASysShaderHAL::Invalidate()
{
	if ( IsValidated() )
	{
		return ASysShader::Invalidate();
	}
}

TBOOL ASysShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL ASysShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void ASysShaderHAL::Render( TRenderPacket* pPacket )
{
	TASSERT( pPacket->GetMesh()->IsExactly( &TGetClass( ASysMeshHAL ) ) );

	auto         pRender  = TRenderGLInterface::Interface();
	ASysMeshHAL* pMeshHAL = TSTATICCAST( ASysMeshHAL, pPacket->GetMesh() );

	pRender->SetTransform( TRenderGLInterface::TRANSFORM_WORLD, pPacket->GetModelViewMatrix() );
	pRender->SetDepthBias( -pMeshHAL->GetZBias(), 0.0f );

	auto pVertexPool = TDYNAMICCAST( TVertexPoolResource, pMeshHAL->GetVertexPool() );
	auto pIndexPool  = TDYNAMICCAST( TIndexPoolResource, pMeshHAL->GetIndexPool() );

	TVertexBlockResource::HALBuffer vertexHALBuffer;
	pVertexPool->GetHALBuffer( &vertexHALBuffer );

	TIndexBlockResource::HALBuffer indexHALBuffer;
	pIndexPool->GetHALBuffer( &indexHALBuffer );

	if ( ms_bRenderEnabled )
	{
		const TBOOL bTriStrip = pIndexPool->GetFlags() & TIndexPoolResourceInterface::FLAGS_TRISTRIP;

		pRender->DrawIndexed(
		    bTriStrip ? GL_TRIANGLE_STRIP : GL_TRIANGLES,
		    bTriStrip ? pIndexPool->GetNumIndices() : pIndexPool->GetNumIndices() / 3 * 3,
		    indexHALBuffer.pIndexBuffer,
		    indexHALBuffer.uiIndexOffset * sizeof( TIndexType ),
		    GL_UNSIGNED_SHORT,
		    vertexHALBuffer.apVertexBuffers[ 0 ],
		    24,
		    vertexHALBuffer.uiVertexOffset * 24
		);
	}

	pRender->SetDepthBias( 0, 0.0f );
}

ASysMaterial* ASysShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	auto pMaterialHAL = new ASysMaterialHAL();
	pMaterialHAL->SetShader( this );
	pMaterialHAL->SetOrderTable( &m_aOrderTables[ 0 ] );

	return pMaterialHAL;
}

ASysMesh* ASysShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	auto pMeshHAL = new ASysMeshHAL();
	pMeshHAL->SetOwnerShader( this );

	return pMeshHAL;
}
