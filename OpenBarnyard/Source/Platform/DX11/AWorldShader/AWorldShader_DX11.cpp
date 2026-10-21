#include "pch.h"
#include "AWorldShader_DX11.h"
#include "AWorldMaterial_DX11.h"
#include "AWorldMesh_DX11.h"
#include "World/AWorld.h"

#include <Render/TRenderPacket.h>

#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TVertexBlockResource_DX11.h>
#include <Platform/DX11/TVertexPoolResource_DX11.h>
#include <Platform/DX11/TIndexBlockResource_DX11.h>
#include <Platform/DX11/TIndexPoolResource_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AWorldShaderHAL );

// AWorldShader_D3D8_Win.vsh and the texture stages of the world materials
static const TCHAR s_szWorldShader[] = R"(
cbuffer Constants : register( b0 )
{
	float4x4 g_mModelViewProjection;
	float4   g_vAmbientColour;
	float4   g_vShadowColour;
	float4   g_vOne;
	float4   g_vUVOffsets;    // texture 0 (xy), texture 1 (zw)
	float4   g_vAlphaWeights; // weight of the lighting (x) and of the opaque alpha (y)
	float4   g_vAlphaTest;
	float4   g_vFog;          // enabled, start, end
	float4   g_vFogColour;
	float4   g_vStages;       // texture of stage 0, texture of stage 1
};

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vLighting : COLOR;
	float2 vUV : TEXCOORD;
};

struct PS_IN
{
	float4 vPosition : SV_POSITION;
	float4 vColour : COLOR;
	float2 vUV0 : TEXCOORD0;
	float2 vUV1 : TEXCOORD1;
};

PS_IN vs_main( VS_IN In )
{
	float4 vLighting = float4( In.vLighting, 1.0f );
	float4 vColour   = vLighting * g_vAmbientColour + ( g_vOne - vLighting ) * g_vShadowColour;
	vColour.w        = g_vOne.w * g_vAlphaWeights.y + vLighting.x * g_vAlphaWeights.x;

	PS_IN Out;
	Out.vPosition = mul( float4( In.vPosition, 1.0f ), g_mModelViewProjection );
	Out.vColour   = saturate( vColour );
	Out.vUV0      = g_vUVOffsets.xy + In.vUV;
	Out.vUV1      = g_vUVOffsets.zw + In.vUV;
	return Out;
}

Texture2D    g_Texture0 : register( t0 );
Texture2D    g_Texture1 : register( t1 );
SamplerState g_Sampler0 : register( s0 );
SamplerState g_Sampler1 : register( s1 );

float4 ps_main( PS_IN In ) : SV_TARGET
{
	float4 vColour = In.vColour;

	// D3DTOP_MODULATE
	if ( g_vStages.x != 0.0f )
		vColour = saturate( g_Texture0.Sample( g_Sampler0, In.vUV0 ) * vColour );

	// D3DTOP_BLENDTEXTUREALPHA
	if ( g_vStages.y != 0.0f )
	{
		float4 vTexture = g_Texture1.Sample( g_Sampler1, In.vUV1 );
		vColour         = saturate( lerp( vColour, vTexture, vTexture.a ) );
	}

	AlphaTest( vColour.a, g_vAlphaTest );

	// D3DFOG_LINEAR
	if ( g_vFog.x != 0.0f )
		vColour.rgb = lerp( g_vFogColour.rgb, vColour.rgb, saturate( ( g_vFog.z - In.vPosition.w ) / ( g_vFog.z - g_vFog.y ) ) );

	return vColour;
}
)";

AWorldShaderHAL::AWorldShaderHAL()
    : m_ShadowColour( 0.3f, 0.3f, 0.3f, 1.0f ), m_AmbientColour( 1.0f, 1.0f, 1.0f, 1.0f )
{
	m_pVertexShader       = TNULL;
	m_pPixelShader        = TNULL;
	m_pInputLayout        = TNULL;
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
		auto pRender = TRenderD3DInterface::Interface();

		Validate();
		pRender->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );

		static constexpr TVector4 s_vOne( 1.0f, 1.0f, 1.0f, 1.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_AMBIENT, &m_AmbientColour );
		pRender->SetVec4InVSBuffer( CONSTANT_SHADOW, &m_ShadowColour );
		pRender->SetVec4InVSBuffer( CONSTANT_ONE, &s_vOne );

		pRender->SetZWriteEnable( TTRUE );
		pRender->SetAlphaTestEnable( TTRUE );
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
		pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
		pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? D3D11_CULL_BACK : D3D11_CULL_FRONT );
		pRender->SetFogEnable( TFALSE );
	}
}

void AWorldShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		auto pRender = TRenderD3DInterface::Interface();

		pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );

		Validate();
		pRender->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );

		static constexpr TVector4 s_vOne( 1.0f, 1.0f, 1.0f, 1.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_AMBIENT, &m_AmbientColour );
		pRender->SetVec4InVSBuffer( CONSTANT_SHADOW, &m_ShadowColour );
		pRender->SetVec4InVSBuffer( CONSTANT_ONE, &s_vOne );

		pRender->SetZWriteEnable( TTRUE );
		pRender->SetAlphaTestEnable( TTRUE );
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
		pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
		pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? D3D11_CULL_BACK : D3D11_CULL_FRONT );
	}
}

void AWorldShaderHAL::EndFlush()
{
	TRenderD3DInterface::Interface()->SetFogEnable( TFALSE );
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

	ID3DBlob* pVSBlob = TRenderD3DInterface::CompileShader( s_szWorldShader, "vs_main", "vs_4_0", TNULL );
	ID3DBlob* pPSBlob = TRenderD3DInterface::CompileShader( s_szWorldShader, "ps_main", "ps_4_0", TNULL );

	if ( !pVSBlob || !pPSBlob ||
	     !TRenderD3DInterface::CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader ) ||
	     !TRenderD3DInterface::CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader ) )
	{
		TASSERT( !"Couldn't create the shaders" );
		return TFALSE;
	}

	static constexpr D3D11_INPUT_ELEMENT_DESC s_aInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof( WorldVertex, Position ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof( WorldVertex, Normal ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof( WorldVertex, Color ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof( WorldVertex, UV ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	TRenderD3DInterface::Interface()->GetDevice()->CreateInputLayout( s_aInputLayout, TARRAYSIZE( s_aInputLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );

	pVSBlob->Release();
	pPSBlob->Release();

	return AWorldShader::Validate();
}

void AWorldShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	if ( m_pVertexShader )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pPixelShader )
	{
		m_pPixelShader->Release();
		m_pPixelShader = TNULL;
	}

	if ( m_pInputLayout )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}

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
		auto pRender         = TRenderD3DInterface::Interface();
		auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

		TMatrix44 mMVP;
		mMVP.Multiply( pCurrentContext->GetProjectionMatrix(), a_pRenderPacket->GetModelViewMatrix() );

		TMatrix44 mMVPTransposed;
		mMVPTransposed.Transpose( mMVP );
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

		pRender->DrawIndexed(
		    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		    pIndexPool->GetNumIndices(),
		    indexBuffer.pIndexBuffer,
		    indexBuffer.uiIndexOffset * sizeof( TIndexType ),
		    DXGI_FORMAT_R16_UINT,
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
