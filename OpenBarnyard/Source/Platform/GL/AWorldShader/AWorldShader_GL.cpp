#include "pch.h"
#include "AWorldShader_GL.h"
#include "AWorldMaterial_GL.h"
#include "AWorldMesh_GL.h"
#include "World/AWorld.h"

#include <Render/TRenderPacket.h>

#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TVertexBlockResource_GL.h>
#include <Platform/GL/TVertexPoolResource_GL.h>
#include <Platform/GL/TIndexBlockResource_GL.h>
#include <Platform/GL/TIndexPoolResource_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AWorldShaderHAL );

// AWorldShader_D3D8_Win.vsh and the texture stages of the world materials
static const TCHAR s_szWorldShader[] = R"(
layout( std140 ) uniform Constants
{
	mat4 g_mModelViewProjection;
	vec4 g_vAmbientColour;
	vec4 g_vShadowColour;
	vec4 g_vOne;
	vec4 g_vUVOffsets;    // texture 0 (xy), texture 1 (zw)
	vec4 g_vAlphaWeights; // weight of the lighting (x) and of the opaque alpha (y)
	vec4 g_vAlphaTest;
	vec4 g_vFog;          // enabled, start, end
	vec4 g_vFogColour;
	vec4 g_vStages;       // texture of stage 0, texture of stage 1
};

#ifdef VERTEX_SHADER

layout( location = 0 ) in vec3 a_vPosition;
layout( location = 2 ) in vec3 a_vLighting;
layout( location = 3 ) in vec2 a_vUV;

out vec4 v_vColour;
out vec2 v_vUV0;
out vec2 v_vUV1;

void main()
{
	vec4 vLighting = vec4( a_vLighting, 1.0 );
	vec4 vColour   = vLighting * g_vAmbientColour + ( g_vOne - vLighting ) * g_vShadowColour;
	vColour.w      = g_vOne.w * g_vAlphaWeights.y + vLighting.x * g_vAlphaWeights.x;

	gl_Position = ToClipSpace( vec4( a_vPosition, 1.0 ) * g_mModelViewProjection );
	v_vColour   = saturate( vColour );
	v_vUV0      = g_vUVOffsets.xy + a_vUV;
	v_vUV1      = g_vUVOffsets.zw + a_vUV;
}

#else

in vec4 v_vColour;
in vec2 v_vUV0;
in vec2 v_vUV1;

uniform sampler2D g_Texture0;
uniform sampler2D g_Texture1;

void main()
{
	vec4 vColour = v_vColour;

	// D3DTOP_MODULATE
	if ( g_vStages.x != 0.0 )
		vColour = saturate( texture( g_Texture0, v_vUV0 ) * vColour );

	// D3DTOP_BLENDTEXTUREALPHA
	if ( g_vStages.y != 0.0 )
	{
		vec4 vTexture = texture( g_Texture1, v_vUV1 );
		vColour       = saturate( mix( vColour, vTexture, vTexture.a ) );
	}

	AlphaTest( vColour.a, g_vAlphaTest );

	// D3DFOG_LINEAR
	if ( g_vFog.x != 0.0 )
		vColour.rgb = mix( g_vFogColour.rgb, vColour.rgb, saturate( ( g_vFog.z - 1.0 / gl_FragCoord.w ) / ( g_vFog.z - g_vFog.y ) ) );

	o_vColour = vColour;
}

#endif
)";

static const TGLVertexAttribute s_aInputAttributes[] = {
	{ 0, 0, 3, GL_FALSE, GL_FLOAT, offsetof( WorldVertex, Position ) },
	{ 1, 0, 3, GL_FALSE, GL_FLOAT, offsetof( WorldVertex, Normal ) },
	{ 2, 0, 3, GL_FALSE, GL_FLOAT, offsetof( WorldVertex, Color ) },
	{ 3, 0, 2, GL_FALSE, GL_FLOAT, offsetof( WorldVertex, UV ) },
};

static const TGLInputLayout s_oInputLayout = { s_aInputAttributes, TARRAYSIZE( s_aInputAttributes ) };

AWorldShaderHAL::AWorldShaderHAL()
    : m_ShadowColour( 0.3f, 0.3f, 0.3f, 1.0f ), m_AmbientColour( 1.0f, 1.0f, 1.0f, 1.0f )
{
	m_uiProgram           = 0;
	m_iAlphaRef           = 128;
	m_bIsHighEndMode      = TTRUE;
	m_bRenderEnvMap       = TFALSE;
	m_bAlphaBlendMaterial = TFALSE;
}

AWorldShaderHAL::~AWorldShaderHAL()
{
}

void AWorldShaderHAL::Flush()
{
	if ( IsValidated() )
	{
		auto pRender = TRenderGLInterface::Interface();

		Validate();
		pRender->SetShaders( m_uiProgram, &s_oInputLayout );

		static constexpr TVector4 s_vOne( 1.0f, 1.0f, 1.0f, 1.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_AMBIENT, &m_AmbientColour );
		pRender->SetVec4InVSBuffer( CONSTANT_SHADOW, &m_ShadowColour );
		pRender->SetVec4InVSBuffer( CONSTANT_ONE, &s_vOne );

		pRender->SetZWriteEnable( TTRUE );
		pRender->SetAlphaTestEnable( TTRUE );
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetAlphaFunc( GL_GREATER );
		pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
		pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? TRenderGLInterface::CULL_BACK : TRenderGLInterface::CULL_FRONT );
		pRender->SetFogEnable( TFALSE );
	}
}

void AWorldShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		auto pRender = TRenderGLInterface::Interface();

		pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );

		Validate();
		pRender->SetShaders( m_uiProgram, &s_oInputLayout );

		static constexpr TVector4 s_vOne( 1.0f, 1.0f, 1.0f, 1.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_AMBIENT, &m_AmbientColour );
		pRender->SetVec4InVSBuffer( CONSTANT_SHADOW, &m_ShadowColour );
		pRender->SetVec4InVSBuffer( CONSTANT_ONE, &s_vOne );

		pRender->SetZWriteEnable( TTRUE );
		pRender->SetAlphaTestEnable( TTRUE );
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetAlphaFunc( GL_GREATER );
		pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
		pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? TRenderGLInterface::CULL_BACK : TRenderGLInterface::CULL_FRONT );
	}
}

void AWorldShaderHAL::EndFlush()
{
	TRenderGLInterface::Interface()->FlushStripBatch();
	TRenderGLInterface::Interface()->SetFogEnable( TFALSE );
}

TBOOL AWorldShaderHAL::Create()
{
	m_aOrderTables[ 0 ].Create( this, -3000 );
	m_aOrderTables[ 1 ].Create( this, 100 );
	m_aOrderTables[ 2 ].Create( this, 101 );
	m_aOrderTables[ 3 ].Create( this, 601 );
	m_aOrderTables[ 4 ].Create( this, -400 );
	m_aOrderTables[ 5 ].Create( this, 500 );
	m_aOrderTables[ 6 ].Create( this, -6005 );
	m_aOrderTables[ 7 ].Create( this, -7000 );

	return AWorldShader::Create();
}

TBOOL AWorldShaderHAL::Validate()
{
	if ( IsValidated() )
	{
		return TTRUE;
	}

	SetHighEndMode( TTRUE );

	m_uiProgram = TRenderGLInterface::CompileProgram( s_szWorldShader );

	if ( !m_uiProgram )
	{
		TASSERT( !"Couldn't create the shaders" );
		return TFALSE;
	}

	return AWorldShader::Validate();
}

void AWorldShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	TRenderGLInterface::DestroyProgram( m_uiProgram );
	m_uiProgram = 0;

	TShader::Invalidate();
}

TBOOL AWorldShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL AWorldShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void AWorldShaderHAL::Render( TRenderPacket* a_pRenderPacket )
{
	if ( IsHighEndMode() && a_pRenderPacket && a_pRenderPacket->GetMesh() )
	{
		auto pRender         = TRenderGLInterface::Interface();
		auto pCurrentContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );

		TMatrix44 mMVP;
		mMVP.Multiply( pCurrentContext->GetProjectionMatrix(), a_pRenderPacket->GetModelViewMatrix() );

		TMatrix44 mMVPTransposed;
		mMVPTransposed.Transpose( mMVP );

		auto pBatchMesh      = TSTATICCAST( AWorldMeshHAL, a_pRenderPacket->GetMesh() );
		auto pBatchIndexPool = TSTATICCAST( TIndexPoolResource, pBatchMesh->GetSubMesh( 0 )->pIndexPool );
		auto pBatchPool      = TSTATICCAST( TVertexPoolResource, pBatchMesh->GetVertexPool() );

		// Fading meshes change the alpha test between packets
		const TBOOL    bCanBatch = a_pRenderPacket->GetAlpha() >= 1.0f && pBatchIndexPool->GetIndices() != TNULL;
		const TBOOL    bIsWater  = pBatchMesh->IsWater();
		const TUINT16* pIndices  = bCanBatch ? pBatchIndexPool->GetIndices() : TNULL;

		TVertexBlockResource::HALBuffer oBatchVertices;
		pBatchPool->GetHALBuffer( &oBatchVertices );

		if ( pRender->IsStripBatchOpen() )
		{
			if ( bCanBatch && bIsWater == m_bBatchIsWater && TUtil::MemCompare( &mMVPTransposed, &m_oBatchMVP, sizeof( m_oBatchMVP ) ) == 0 &&
			     pRender->AddStripToBatch( pIndices, pBatchIndexPool->GetNumIndices(), oBatchVertices.apVertexBuffers[ 0 ], sizeof( WorldVertex ), oBatchVertices.uiVertexOffset * sizeof( WorldVertex ) ) )
			{
				return;
			}

			pRender->FlushStripBatch();
		}

		pRender->SetVec4InVSBuffer( CONSTANT_MVP, &mMVPTransposed, 4 );

		if ( a_pRenderPacket->GetAlpha() >= 1.0f || isnan( a_pRenderPacket->GetAlpha() ) )
		{
			if ( s_RenderStateFlags & RENDERSTATE_ALPHAREF )
			{
				pRender->SetAlphaRef( m_iAlphaRef );
				s_RenderStateFlags &= ~RENDERSTATE_ALPHAREF;
			}
		}
		else
		{
			pRender->SetAlphaRef( TUINT8( m_iAlphaRef * a_pRenderPacket->GetAlpha() ) );
			s_RenderStateFlags |= RENDERSTATE_ALPHAREF;
		}

		auto pMesh     = TSTATICCAST( AWorldMeshHAL, a_pRenderPacket->GetMesh() );
		auto pMaterial = TSTATICCAST( AWorldMaterialHAL, pMesh->GetMaterial() );

		if ( pMaterial->GetBlendMode() != AWorldMaterial::BLENDMODE_DEFAULT || a_pRenderPacket->GetAlpha() < 1.0f )
		{
			if ( ISZERO( s_RenderStateFlags & RENDERSTATE_ALPHABLEND ) )
			{
				pRender->SetAlphaBlendEnable( TTRUE );
				s_RenderStateFlags |= RENDERSTATE_ALPHABLEND;
			}
		}
		else if ( !ISZERO( s_RenderStateFlags & RENDERSTATE_ALPHABLEND ) )
		{
			pRender->SetAlphaBlendEnable( TFALSE );
			s_RenderStateFlags &= ~RENDERSTATE_ALPHABLEND;
		}

		TVector4 vAlphaWeights;
		vAlphaWeights.x = 0.0f;
		vAlphaWeights.y = 1.0f;

		if ( pMesh->IsWater() )
		{
			vAlphaWeights.x = 1.0f;
			vAlphaWeights.y = 0.0f;
		}

		vAlphaWeights.w = 1.0f;
		vAlphaWeights.z = 0.0f;
		pRender->SetVec4InVSBuffer( CONSTANT_ALPHAWEIGHTS, &vAlphaWeights );

		TVector4 vFog;
		TVector4 vFogColour;
		pRender->GetFogParams( vFog, vFogColour );

		const TVector4 vAlphaTest = pRender->GetAlphaTestParams();
		const TVector4 vStages( pRender->GetTexture( 0 ) ? 1.0f : 0.0f, pMaterial->GetTextureNum() > 1 && pRender->GetTexture( 1 ) ? 1.0f : 0.0f, 0.0f, 0.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_ALPHATEST, &vAlphaTest );
		pRender->SetVec4InVSBuffer( CONSTANT_FOG, &vFog );
		pRender->SetVec4InVSBuffer( CONSTANT_FOGCOLOUR, &vFogColour );
		pRender->SetVec4InVSBuffer( CONSTANT_STAGES, &vStages );

		auto pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );
		TVALIDPTR( pVertexPool );

		TVertexBlockResource::HALBuffer vertexBuffer;
		pVertexPool->GetHALBuffer( &vertexBuffer );

		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pMesh->GetSubMesh( 0 )->pIndexPool );
		TVALIDPTR( pIndexPool );

		TIndexBlockResource::HALBuffer indexBuffer;
		pIndexPool->GetHALBuffer( &indexBuffer );

		if ( bCanBatch && pRender->AddStripToBatch( pIndices, pIndexPool->GetNumIndices(), vertexBuffer.apVertexBuffers[ 0 ], sizeof( WorldVertex ), vertexBuffer.uiVertexOffset * sizeof( WorldVertex ) ) )
		{
			m_oBatchMVP     = mMVPTransposed;
			m_bBatchIsWater = bIsWater;
			return;
		}

		pRender->DrawIndexed(
		    GL_TRIANGLE_STRIP,
		    pIndexPool->GetNumIndices(),
		    indexBuffer.pIndexBuffer,
		    indexBuffer.uiIndexOffset * sizeof( TIndexType ),
		    GL_UNSIGNED_SHORT,
		    vertexBuffer.apVertexBuffers[ 0 ],
		    sizeof( WorldVertex ),
		    vertexBuffer.uiVertexOffset * sizeof( WorldVertex )
		);
	}
}

void AWorldShaderHAL::EnableRenderEnvMap( TBOOL a_bEnable )
{
	m_bRenderEnvMap = a_bEnable;
}

TBOOL AWorldShaderHAL::IsAlphaBlendMaterial()
{
	return m_bAlphaBlendMaterial;
}

void AWorldShaderHAL::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
	m_bAlphaBlendMaterial = a_bIsAlphaBlendMaterial;
}

AWorldMaterial* AWorldShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	auto pMaterial = new AWorldMaterialHAL();
	pMaterial->SetShader( this );

	if ( AWorldShaderHAL::IsAlphaBlendMaterial() )
	{
		auto pAlphaBlendMaterial = new AWorldMaterialHAL();
		pAlphaBlendMaterial->SetShader( this );
		pAlphaBlendMaterial->Create( 1 );

		pMaterial->SetAlphaBlendMaterial( pAlphaBlendMaterial );
	}

	return pMaterial;
}

AWorldMesh* AWorldShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	auto pMesh = new AWorldMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

TBOOL AWorldShaderHAL::IsHighEndMode()
{
	return m_bIsHighEndMode;
}

void AWorldShaderHAL::SetHighEndMode( TBOOL a_bEnable )
{
	m_bIsHighEndMode = a_bEnable && IsCapableShaders();
}

TBOOL AWorldShaderHAL::IsCapableShaders()
{
	return TTRUE;
}

TBOOL AWorldShaderHAL::IsRenderEnvMapEnabled()
{
	return m_bRenderEnvMap;
}

void* AWorldShaderHAL::CreateUnknown( void*, void*, void*, void* )
{
	TASSERT( !"What's this?" );
	return TNULL;
}

void AWorldShaderHAL::SetColours( const Toshi::TVector4& a_rShadowColour, const Toshi::TVector4 a_rAmbientColour )
{
	m_ShadowColour  = a_rShadowColour;
	m_AmbientColour = a_rAmbientColour;

	Toshi::TMath::Clip( m_ShadowColour.x, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_ShadowColour.y, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_ShadowColour.z, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_AmbientColour.x, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_AmbientColour.y, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_AmbientColour.z, 0.0f, 1.0f );
}
