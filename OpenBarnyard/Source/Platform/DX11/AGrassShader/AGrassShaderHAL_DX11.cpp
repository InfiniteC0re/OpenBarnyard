#include "pch.h"
#include "AGrassShaderHAL_DX11.h"
#include "AGrassMaterialHAL_DX11.h"
#include "AGrassMeshHAL_DX11.h"
#include "Cameras/ACameraManager.h"
#include "Assets/AModelLoader.h"
#include "Platform/DX11/AWorldShader/AWorldShader_DX11.h"

#include <Toshi/TScheduler.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TVertexBlockResource_DX11.h>
#include <Platform/DX11/TVertexPoolResource_DX11.h>
#include <Platform/DX11/TIndexBlockResource_DX11.h>
#include <Platform/DX11/TIndexPoolResource_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGrassShaderHAL );

TTextureResourceHAL* g_pGrassShaderTerrainTexture = TNULL;
static TVector3      g_vecAnimationDataOffset;
static TVector4      g_vecAnimOffset;
static const TFLOAT  g_fAnimationSpeed = 2.25f;

// AGrassShader_D3D8_Win.vsh and the texture stages of the grass
static const TCHAR s_szGrassShader[] = R"(
cbuffer Constants : register( b0 )
{
	float4x4 g_mModelViewProjection : packoffset( c0 );
	float4   g_vAmbientColour       : packoffset( c4 );
	float4   g_vShadowColour        : packoffset( c5 );
	float4   g_vOne                 : packoffset( c6 );
	float4   g_vLayerOffset         : packoffset( c7 );  // how far the layer is pushed along the normal
	float4   g_vAnimation           : packoffset( c8 );  // wind (xyz), UV scale of the layer (w)
	float4   g_vAlphaTest           : packoffset( c9 );
	float4   g_vFog                 : packoffset( c10 ); // enabled, start, end
	float4   g_vFogColour           : packoffset( c11 );
	float4   g_vStages              : packoffset( c12 ); // texture of stage 0
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
	float2 vUV : TEXCOORD0;
};

PS_IN vs_main( VS_IN In )
{
	float4 vPosition = float4( In.vNormal, 1.0f ) * g_vLayerOffset;
	vPosition += g_vAnimation * g_vLayerOffset;
	vPosition += float4( In.vPosition, 1.0f );

	float4 vLighting = float4( In.vLighting, 1.0f );
	float4 vColour   = vLighting * g_vAmbientColour + ( g_vOne - vLighting ) * g_vShadowColour;
	vColour.w        = g_vOne.w;

	PS_IN Out;
	Out.vPosition = mul( vPosition, g_mModelViewProjection );
	Out.vColour   = saturate( vColour );
	Out.vUV       = In.vUV * g_vAnimation.w;
	return Out;
}

Texture2D    g_Texture : register( t0 );
SamplerState g_Sampler : register( s0 );

float4 ps_main( PS_IN In ) : SV_TARGET
{
	float4 vColour = In.vColour;

	// D3DTOP_MODULATE
	if ( g_vStages.x != 0.0f )
		vColour = saturate( g_Texture.Sample( g_Sampler, In.vUV ) * vColour );

	AlphaTest( vColour.a, g_vAlphaTest );

	// D3DFOG_LINEAR
	if ( g_vFog.x != 0.0f )
		vColour.rgb = lerp( g_vFogColour.rgb, vColour.rgb, saturate( ( g_vFog.z - In.vPosition.w ) / ( g_vFog.z - g_vFog.y ) ) );

	return vColour;
}
)";

AGrassShaderHAL::AGrassShaderHAL()
    : m_pVertexShader( TNULL )
    , m_pPixelShader( TNULL )
    , m_pInputLayout( TNULL )
{
}

AGrassShaderHAL::~AGrassShaderHAL()
{
}

void AGrassShaderHAL::Flush()
{
	Validate();

	auto pRender         = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	pRender->SetTextureMinFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMagFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMipFilter( 0, TRenderD3DInterface::TEXTUREFILTER_POINT );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
	pRender->SetFixedFunctionShader();

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_PROJECTION, pCurrentContext->GetProjectionMatrix() );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	pRender->SetAlphaBlendEnable( TFALSE );
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaRef( 1 );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );

	m_oOrderTable.Render();
	pRender->SetAlphaTestEnable( TFALSE );
}

void AGrassShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		auto pRender = TRenderD3DInterface::Interface();

		pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
		pRender->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );

		AWorldShaderHAL*          pWorldShader    = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );
		TVector4                  vecShadowColor  = pWorldShader->GetShadowColour();
		TVector4                  vecAmbientColor = pWorldShader->GetAmbientColour();
		static constexpr TVector4 vecColor        = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );

		pRender->SetVec4InVSBuffer( CONSTANT_AMBIENT, &vecAmbientColor );
		pRender->SetVec4InVSBuffer( CONSTANT_SHADOW, &vecShadowColor );
		pRender->SetVec4InVSBuffer( CONSTANT_ONE, &vecColor );

		pRender->SetZWriteEnable( TTRUE );
		pRender->SetAlphaTestEnable( TTRUE );
		pRender->SetAlphaBlendEnable( TFALSE );
		pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
		pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );

		UpdateAnimation();
	}
}

void AGrassShaderHAL::EndFlush()
{
	auto pRender = TRenderD3DInterface::Interface();

	pRender->SetFogEnable( TFALSE );
	pRender->SetAlphaBlendEnable( TTRUE );
}

TBOOL AGrassShaderHAL::Create()
{
	m_oOrderTable.Create( this, -2000 );
	AGrassShader::Create();

	return Validate();
}

TBOOL AGrassShaderHAL::Validate()
{
	if ( IsValidated() )
		return TTRUE;

	ID3DBlob* pVSBlob = TRenderD3DInterface::CompileShader( s_szGrassShader, "vs_main", "vs_4_0", TNULL );
	ID3DBlob* pPSBlob = TRenderD3DInterface::CompileShader( s_szGrassShader, "ps_main", "ps_4_0", TNULL );

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

	return AGrassShader::Validate();
}

void AGrassShaderHAL::Invalidate()
{
	if ( IsValidated() )
	{
		ID3D11DeviceChild* apObjects[] = { m_pVertexShader, m_pPixelShader, m_pInputLayout };

		for ( auto pObject : apObjects )
		{
			if ( pObject )
				pObject->Release();
		}

		m_pVertexShader = TNULL;
		m_pPixelShader  = TNULL;
		m_pInputLayout  = TNULL;

		AGrassShader::Invalidate();
	}
}

TBOOL AGrassShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL AGrassShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

static void DrawGrassLayer( TIndexPoolResource* a_pIndexPool, const TIndexBlockResource::HALBuffer& a_rIndexBuffer, const TVertexBlockResource::HALBuffer& a_rVertexBuffer )
{
	auto pRender = TRenderD3DInterface::Interface();

	TVector4 vFog;
	TVector4 vFogColour;
	pRender->GetFogParams( vFog, vFogColour );

	const TVector4 vAlphaTest = pRender->GetAlphaTestParams();
	const TVector4 vStages( pRender->GetTexture( 0 ) ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f );
	pRender->SetVec4InVSBuffer( AGrassShaderHAL::CONSTANT_ALPHATEST, &vAlphaTest );
	pRender->SetVec4InVSBuffer( AGrassShaderHAL::CONSTANT_FOG, &vFog );
	pRender->SetVec4InVSBuffer( AGrassShaderHAL::CONSTANT_FOGCOLOUR, &vFogColour );
	pRender->SetVec4InVSBuffer( AGrassShaderHAL::CONSTANT_STAGES, &vStages );

	pRender->DrawIndexed(
	    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	    a_pIndexPool->GetNumIndices(),
	    a_rIndexBuffer.pIndexBuffer,
	    a_rIndexBuffer.uiIndexOffset * sizeof( TIndexType ),
	    DXGI_FORMAT_R16_UINT,
	    a_rVertexBuffer.apVertexBuffers[ 0 ],
	    sizeof( WorldVertex ),
	    a_rVertexBuffer.uiVertexOffset * sizeof( WorldVertex )
	);
}

void AGrassShaderHAL::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	AGrassMeshHAL* pMesh;

	if ( a_pRenderPacket && ( pMesh = TSTATICCAST( AGrassMeshHAL, a_pRenderPacket->GetMesh() ), pMesh != TNULL ) )
	{
		auto pRender         = TRenderD3DInterface::Interface();
		auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

		TMatrix44 matMVP;
		matMVP.Multiply( pCurrentContext->GetProjectionMatrix(), a_pRenderPacket->GetModelViewMatrix() );

		TMatrix44 matMVPTransposed;
		matMVPTransposed.Transpose( matMVP );
		pRender->SetVec4InVSBuffer( CONSTANT_MVP, &matMVPTransposed, 4 );

		TVertexBlockResource::HALBuffer oVertexBuffer;
		TVertexPoolResource*            pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );
		pVertexPool->GetHALBuffer( &oVertexBuffer );

		TIndexBlockResource::HALBuffer oIndexBuffer;
		TIndexPoolResource*            pIndexPool = TSTATICCAST( TIndexPoolResource, pMesh->GetSubMesh( 0 )->pIndexPool );
		pIndexPool->GetHALBuffer( &oIndexBuffer );

		TVector4 vecOffset = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_vecAnimOffset.w  = 1.0f;

		pRender->SetVec4InVSBuffer( CONSTANT_ANIMATION, &g_vecAnimOffset );
		pRender->SetVec4InVSBuffer( CONSTANT_LAYER, &vecOffset );
		pRender->SetTexture( 0, g_pGrassShaderTerrainTexture->GetD3DTexture() );
		DrawGrassLayer( pIndexPool, oIndexBuffer, oVertexBuffer );

		ACamera* pCamera   = ACameraManager::GetSingleton()->GetCurrentCamera();
		TVector4 vecCamPos = pCamera->GetMatrix().GetTranslation();

		// Transform coordinates to the actual world position, since they are exported rotated
		TVector4 vecMeshBounding;
		vecMeshBounding.x = pMesh->GetCellMeshSphere()->m_BoundingSphere.AsVector4().x;
		vecMeshBounding.y = -pMesh->GetCellMeshSphere()->m_BoundingSphere.AsVector4().z;
		vecMeshBounding.z = pMesh->GetCellMeshSphere()->m_BoundingSphere.AsVector4().y;
		vecMeshBounding.w = pMesh->GetCellMeshSphere()->m_BoundingSphere.GetRadius();

		TFLOAT fDistanceToCamera = TVector4::DistanceXZ( vecMeshBounding, vecCamPos ) - vecMeshBounding.w;

		if ( fDistanceToCamera <= 100.0f )
		{
			constexpr TINT MAX_LAYERS = 5;
			TINT           iNumLayers = MAX_LAYERS;

			if ( fDistanceToCamera > 20.0f )
			{
				iNumLayers = ( fDistanceToCamera >= 50.0f ) ?
				    1 :
				    MAX_LAYERS - 1 - TMath::Min( TINT( ( fDistanceToCamera - 20.0f ) / 10.0f ), MAX_LAYERS - 2 );
			}

			g_vecAnimOffset.w = 2.0f;
			TFLOAT fStepSize  = 0.25f / iNumLayers;

			for ( TINT i = 1; i <= iNumLayers; i++ )
			{
				pRender->SetTexture( 0, g_aGrassLayers[ i ]->GetD3DTexture() );
				pRender->SetVec4InVSBuffer( CONSTANT_ANIMATION, &g_vecAnimOffset );

				vecOffset.x += fStepSize;
				vecOffset.y += fStepSize;
				vecOffset.z += fStepSize;
				pRender->SetVec4InVSBuffer( CONSTANT_LAYER, &vecOffset );
				DrawGrassLayer( pIndexPool, oIndexBuffer, oVertexBuffer );
			}
		}
	}
}

AGrassMaterial* AGrassShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	AGrassMaterialHAL* pMaterial = new AGrassMaterialHAL();
	pMaterial->SetShader( this );

	m_oOrderTable.RegisterMaterial( pMaterial );
	return pMaterial;
}

AGrassMesh* AGrassShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	AGrassMeshHAL* pMesh = new AGrassMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

void AGrassShaderHAL::UpdateAnimation()
{
	TFLOAT fCurrentTime = g_oSystemManager.GetScheduler()->GetTotalTime();

	TVector3 vecAnim = TVector3(
	    fCurrentTime * 0.70f * g_fAnimationSpeed,
	    fCurrentTime * 0.92f * g_fAnimationSpeed,
	    fCurrentTime * 0.20f * g_fAnimationSpeed
	);

	TMath::NormaliseAngle( vecAnim.x );
	TMath::NormaliseAngle( vecAnim.y );
	TMath::NormaliseAngle( vecAnim.z );

	g_vecAnimationDataOffset.x = TMath::Sin( vecAnim.x );
	g_vecAnimationDataOffset.y = TMath::Sin( vecAnim.y );
	g_vecAnimationDataOffset.z = TMath::Sin( vecAnim.z );
	g_vecAnimationDataOffset.Multiply( 0.075f );

	g_vecAnimOffset.x = g_vecAnimationDataOffset.x;
	g_vecAnimOffset.y = g_vecAnimationDataOffset.y;
	g_vecAnimOffset.z = g_vecAnimationDataOffset.z;
}
