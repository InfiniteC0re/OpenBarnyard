#include "pch.h"
#include "ASysShader_DX11.h"
#include "ASysMaterial_DX11.h"
#include "ASysMesh_DX11.h"

#include "Platform/DX11/TRenderInterface_DX11.h"
#include "Platform/DX11/TRenderContext_DX11.h"
#include "Platform/DX11/TVertexPoolResource_DX11.h"
#include "Platform/DX11/TIndexPoolResource_DX11.h"

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

	auto pRender        = TRenderD3DInterface::Interface();
	auto pRenderContext = TSTATICCAST( TRenderContextD3D, pRender->GetCurrentContext() );

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
}

void ASysShaderHAL::EndFlush()
{
	auto pRender = TRenderD3DInterface::Interface();

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

	auto         pRender  = TRenderD3DInterface::Interface();
	ASysMeshHAL* pMeshHAL = TSTATICCAST( ASysMeshHAL, pPacket->GetMesh() );

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_WORLD, pPacket->GetModelViewMatrix() );
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
		    bTriStrip ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		    bTriStrip ? pIndexPool->GetNumIndices() : pIndexPool->GetNumIndices() / 3 * 3,
		    indexHALBuffer.pIndexBuffer,
		    indexHALBuffer.uiIndexOffset * sizeof( TIndexType ),
		    DXGI_FORMAT_R16_UINT,
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
