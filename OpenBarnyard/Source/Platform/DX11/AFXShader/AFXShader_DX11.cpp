#include "pch.h"
#include "AFXShader_DX11.h"
#include "AFXMaterial_DX11.h"
#include "AFXMesh_DX11.h"

#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
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

TDEFINE_CLASS( AFXShaderHAL );

// NOTE: Retail never creates a vertex shader for AFXShaderHAL and draws through the low end FVF,
// ignoring the constants it uploads. They match the ASkinShader vertex shader, so it's used here.
static const TCHAR s_szFXShader[] = R"(
cbuffer Constants : register( b0 )
{
	float4x4 g_mModelViewProjection : packoffset( c0 );
	float4   g_vBoneIndexScale      : packoffset( c4 );
	float4   g_aBoneRows[ 84 ]      : packoffset( c5 );   // 3 rows of the transposed transform of each bone
	float4   g_vLightDirection      : packoffset( c89 );
	float4   g_vAmbientColour       : packoffset( c90 );  // alpha of the packet (w)
	float4   g_vLightColour         : packoffset( c91 );
	float4   g_vLightingRange       : packoffset( c93 );  // min (z) and max (w) of the diffuse lighting
	float4   g_vAlphaTest           : packoffset( c96 );
	float4   g_vStages              : packoffset( c97 );  // texture of stage 0
};

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float4 vWeights : BLENDWEIGHT;
	float4 vBones : BLENDINDICES;
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
	float4 vBones = g_vBoneIndexScale * In.vBones;
	float4 vRow0  = 0.0f;
	float4 vRow1  = 0.0f;
	float4 vRow2  = 0.0f;

	[unroll] for ( int i = 0; i < 4; i++ )
	{
		int iBone = int( vBones[ i ] );
		vRow0 += g_aBoneRows[ iBone + 0 ] * In.vWeights[ i ];
		vRow1 += g_aBoneRows[ iBone + 1 ] * In.vWeights[ i ];
		vRow2 += g_aBoneRows[ iBone + 2 ] * In.vWeights[ i ];
	}

	float4 vPosition = float4( In.vPosition, 1.0f );
	vPosition        = float4( dot( vRow0, vPosition ), dot( vRow1, vPosition ), dot( vRow2, vPosition ), 1.0f );
	float3 vNormal   = float3( dot( vRow0.xyz, In.vNormal ), dot( vRow1.xyz, In.vNormal ), dot( vRow2.xyz, In.vNormal ) );

	float fDiffuse = clamp( dot( vNormal, -g_vLightDirection.xyz ), g_vLightingRange.z, g_vLightingRange.w );

	PS_IN Out;
	Out.vPosition   = mul( vPosition, g_mModelViewProjection );
	Out.vColour.rgb = fDiffuse * g_vLightColour.rgb + ( 1.0f - fDiffuse ) * g_vAmbientColour.rgb;
	Out.vColour.a   = g_vAmbientColour.a;
	Out.vColour     = saturate( Out.vColour );
	Out.vUV         = In.vUV;
	return Out;
}

Texture2D    g_Texture : register( t0 );
SamplerState g_Sampler : register( s0 );

// The texture stages: texture * diffuse
float4 ps_main( PS_IN In ) : SV_TARGET
{
	float4 vColour = In.vColour;

	if ( g_vStages.x != 0.0f )
		vColour = saturate( g_Texture.Sample( g_Sampler, In.vUV ) * vColour );

	AlphaTest( vColour.a, g_vAlphaTest );
	return vColour;
}
)";

AFXShaderHAL::AFXShaderHAL()
    : m_pVertexShader( TNULL )
    , m_pPixelShader( TNULL )
    , m_pInputLayout( TNULL )
    , m_iAlphaRef( 128 )
    , m_bRenderEnvMap( TFALSE )
    , m_bHighEndSkinning( TTRUE )
    , m_bLightScattering( TTRUE )
    , m_bIsAlphaBlendMaterial( TFALSE )
    , m_bPointLights( TFALSE )
    , m_iNumLights( 0 )
    , m_pLights( TNULL )
    , m_fUnknown( 0.0f )
{
}

AFXShaderHAL::~AFXShaderHAL()
{
}

void AFXShaderHAL::OnDestroy()
{
	m_FreeLights.Clear();
	m_UsedLights.Clear();

	delete[] m_pLights;

	TShader::OnDestroy();
}

void AFXShaderHAL::Flush()
{
	if ( !IsValidated() ) return;

	auto pRender = TRenderD3DInterface::Interface();

	Validate();
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
	pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? D3D11_CULL_BACK : D3D11_CULL_FRONT );

	m_oOrderTable.Render();

	pRender->SetFogEnable( TFALSE );
}

// DOES_NOTHING
void AFXShaderHAL::StartFlush()
{
}

// DOES_NOTHING
void AFXShaderHAL::EndFlush()
{
}

TBOOL AFXShaderHAL::Create()
{
	m_oOrderTable.Create( this, -370 );

	m_pLights = new Light[ MAX_LIGHTS ];

	for ( TINT i = 0; i < MAX_LIGHTS; i++ )
	{
		m_FreeLights.PushFront( &m_pLights[ i ] );
		m_pLights[ i ].iIndex = i;
	}

	return TShader::Create();
}

TBOOL AFXShaderHAL::Validate()
{
	if ( IsValidated() )
		return TTRUE;

	EnableHighEndSkinning( TTRUE );

	ID3DBlob* pVSBlob = TRenderD3DInterface::CompileShader( s_szFXShader, "vs_main", "vs_4_0", TNULL );
	ID3DBlob* pPSBlob = TRenderD3DInterface::CompileShader( s_szFXShader, "ps_main", "ps_4_0", TNULL );

	if ( !pVSBlob || !pPSBlob ||
	     !TRenderD3DInterface::CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader ) ||
	     !TRenderD3DInterface::CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader ) )
	{
		TASSERT( !"Couldn't create the shaders" );
		return TFALSE;
	}

	static constexpr D3D11_INPUT_ELEMENT_DESC s_aInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	TRenderD3DInterface::Interface()->GetDevice()->CreateInputLayout( s_aInputLayout, TARRAYSIZE( s_aInputLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );

	pVSBlob->Release();
	pPSBlob->Release();

	return TShader::Validate();
}

void AFXShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	ID3D11DeviceChild* apObjects[] = { m_pVertexShader, m_pPixelShader, m_pInputLayout };

	for ( auto pObject : apObjects )
	{
		if ( pObject )
			pObject->Release();
	}

	m_pVertexShader = TNULL;
	m_pPixelShader  = TNULL;
	m_pInputLayout  = TNULL;

	TShader::Invalidate();
}

TBOOL AFXShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL AFXShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void AFXShaderHAL::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	if ( !a_pRenderPacket ) return;

	auto pMesh = TSTATICCAST( AFXMeshHAL, a_pRenderPacket->GetMesh() );
	if ( !pMesh || pMesh->GetNumSubMeshes() == 0 ) return;

	auto pRender         = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );
	auto pBones          = a_pRenderPacket->GetSkeletonInstance()->GetBones();

	Validate();
	pRender->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );

	TMatrix44 oModelViewProjection;
	oModelViewProjection.Multiply( pCurrentContext->GetProjectionMatrix(), a_pRenderPacket->GetModelViewMatrix() );

	TMatrix44 oModelViewProjectionTransposed;
	oModelViewProjectionTransposed.Transpose( oModelViewProjection );

	static constexpr TVector4 s_vBoneIndexScale = TVector4( 256.0f, 256.0f, 256.0f, 256.0f );
	pRender->SetVec4InVSBuffer( CONSTANT_MVP, &oModelViewProjectionTransposed, 4 );
	pRender->SetVec4InVSBuffer( CONSTANT_BONEINDEXSCALE, &s_vBoneIndexScale );

	TFLOAT fLightDirX = -a_pRenderPacket->GetLightDirection().x;
	TFLOAT fLightDirY = -a_pRenderPacket->GetLightDirection().y;
	TFLOAT fLightDirZ = -a_pRenderPacket->GetLightDirection().z;

	TVector4 vAmbientColour = TVector4( a_pRenderPacket->GetAmbientColour().x, a_pRenderPacket->GetAmbientColour().y, a_pRenderPacket->GetAmbientColour().z, 1.0f );
	TVector4 vLightColour   = TVector4( a_pRenderPacket->GetLightColour().x, a_pRenderPacket->GetLightColour().y, a_pRenderPacket->GetLightColour().z, 1.0f );

	TMatrix44 oModelView = a_pRenderPacket->GetModelViewMatrix();
	TMatrix44 oWorldModelView;

	for ( TINT i = 0; i < 4; i++ )
	{
		oWorldModelView.AsBasisVector4( i ).x = oModelView.m_f11 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f12 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f13 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).y = oModelView.m_f21 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f22 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f23 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).z = oModelView.m_f31 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f32 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f33 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).w = oModelView.m_f14 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f24 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f34 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
	}

	TVector4 vLightDirWorld;
	vLightDirWorld.x = oWorldModelView.m_f11 * fLightDirX + oWorldModelView.m_f21 * fLightDirY + oWorldModelView.m_f31 * fLightDirZ;
	vLightDirWorld.y = oWorldModelView.m_f12 * fLightDirX + oWorldModelView.m_f22 * fLightDirY + oWorldModelView.m_f32 * fLightDirZ;
	vLightDirWorld.z = oWorldModelView.m_f13 * fLightDirX + oWorldModelView.m_f23 * fLightDirY + oWorldModelView.m_f33 * fLightDirZ;
	vLightDirWorld.w = 0.0f;
	vLightDirWorld.Normalise();

	vAmbientColour.w = a_pRenderPacket->GetAlpha();

	static constexpr TVector4 s_vLightingRange = TVector4( 0.6f, 0.4f, 0.0f, 1.0f );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTDIRECTION, &vLightDirWorld );
	pRender->SetVec4InVSBuffer( CONSTANT_AMBIENTCOLOUR, &vAmbientColour );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTCOLOUR, &vLightColour );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTINGRANGE, &s_vLightingRange );

	TINT iAlpha = TINT( a_pRenderPacket->GetAlpha() * 128.0f );
	if ( iAlpha > m_iAlphaRef ) iAlpha = m_iAlphaRef;
	pRender->SetAlphaRef( iAlpha );

	const TVector4 vAlphaTest = pRender->GetAlphaTestParams();
	const TVector4 vStages( pRender->GetTexture( 0 ) ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f );
	pRender->SetVec4InVSBuffer( CONSTANT_ALPHATEST, &vAlphaTest );
	pRender->SetVec4InVSBuffer( CONSTANT_STAGES, &vStages );

	auto pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );

	TVertexBlockResource::HALBuffer vertexHALBuffer;
	pVertexPool->GetHALBuffer( &vertexHALBuffer );

	TIndexBlockResource::HALBuffer indexHALBuffer;

	for ( TUINT i = 0; i < pMesh->GetNumSubMeshes(); i++ )
	{
		auto pSubMesh = pMesh->GetSubMesh( i );
		TASSERT( pSubMesh->uiNumBones <= MAX_BONES );

		TINT iCurrentRegister = CONSTANT_BONES;

		for ( TUINT k = 0; k < pSubMesh->uiNumBones; k++ )
		{
			TMatrix44 oBoneTransform;
			oBoneTransform.Transpose( pBones[ pSubMesh->aBones[ k ] ].m_Transform );
			pRender->SetVec4InVSBuffer( iCurrentRegister, &oBoneTransform, 3 );

			iCurrentRegister += 3;
		}

		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pSubMesh->pIndexPool );
		pIndexPool->GetHALBuffer( &indexHALBuffer );

		pRender->DrawIndexed(
		    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		    pIndexPool->GetNumIndices(),
		    indexHALBuffer.pIndexBuffer,
		    indexHALBuffer.uiIndexOffset * sizeof( TIndexType ),
		    DXGI_FORMAT_R16_UINT,
		    vertexHALBuffer.apVertexBuffers[ 0 ],
		    40,
		    vertexHALBuffer.uiVertexOffset * 40
		);
	}
}

void AFXShaderHAL::EnableRenderEnvMap( TBOOL a_bEnable )
{
	m_bRenderEnvMap = a_bEnable;
}

TBOOL AFXShaderHAL::IsHighEndSkinning()
{
	return m_bHighEndSkinning;
}

void AFXShaderHAL::EnableHighEndSkinning( TBOOL a_bEnable )
{
	m_bHighEndSkinning = IsCapableHighEndSkinning() ? a_bEnable : TFALSE;
	m_bLightScattering = a_bEnable && IsCapableLightScattering();
}

TBOOL AFXShaderHAL::IsCapableHighEndSkinning()
{
	return TTRUE;
}

TBOOL AFXShaderHAL::IsLightScattering()
{
	return m_bLightScattering;
}

void AFXShaderHAL::SetLightScattering( TBOOL a_bEnable )
{
	m_bLightScattering = a_bEnable && IsCapableLightScattering();
}

TBOOL AFXShaderHAL::IsAlphaBlendMaterial()
{
	return m_bIsAlphaBlendMaterial;
}

void AFXShaderHAL::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
	m_bIsAlphaBlendMaterial = a_bIsAlphaBlendMaterial;
}

AFXMaterial* AFXShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	auto pMaterial       = new AFXMaterialHAL();
	pMaterial->m_pShader = this;
	m_oOrderTable.RegisterMaterial( pMaterial );

	auto pAlphaBlendMaterial         = new AFXMaterialHAL();
	pMaterial->m_pAlphaBlendMaterial = pAlphaBlendMaterial;
	pAlphaBlendMaterial->m_pShader   = this;
	m_oOrderTable.RegisterMaterial( pMaterial->m_pAlphaBlendMaterial );
	pMaterial->m_pAlphaBlendMaterial->Create( 1 );

	return pMaterial;
}

AFXMesh* AFXShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	auto pMesh = new AFXMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

TINT AFXShaderHAL::AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )
{
	if ( m_FreeLights.IsEmpty() )
		return -1;

	Light* pLight      = m_FreeLights.PopFront();
	pLight->vPosition  = a_rPosition;
	pLight->fIntensity = a_fIntensity;
	m_UsedLights.PushFront( pLight );
	m_iNumLights++;

	return pLight->iIndex;
}

void AFXShaderHAL::SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )
{
	if ( a_iIndex != -1 )
	{
		m_pLights[ a_iIndex ].vPosition  = a_rPosition;
		m_pLights[ a_iIndex ].fIntensity = a_fIntensity;
	}
}

void AFXShaderHAL::RemoveLight( TINT a_iIndex )
{
	if ( a_iIndex != -1 )
	{
		m_UsedLights.Erase( &m_pLights[ a_iIndex ] );
		m_FreeLights.PushFront( &m_pLights[ a_iIndex ] );
		m_iNumLights--;
	}
}

TBOOL AFXShaderHAL::IsCapablePointLights()
{
	return TTRUE;
}

void AFXShaderHAL::SetPointLights( TBOOL a_bEnabled )
{
	m_bPointLights = a_bEnabled && IsCapablePointLights();
}

TBOOL AFXShaderHAL::IsPointLightsEnabled()
{
	return m_bPointLights;
}

TBOOL AFXShaderHAL::IsCapableLightScattering()
{
	return IsCapableHighEndSkinning();
}

void AFXShaderHAL::SetUnknownFloat( TFLOAT a_fValue )
{
	m_fUnknown = a_fValue;
}

TFLOAT AFXShaderHAL::GetUnknownFloat()
{
	return m_fUnknown;
}

TBOOL AFXShaderHAL::IsEnableRenderEnvMap()
{
	return m_bRenderEnvMap;
}

void AFXShaderHAL::SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA )
{
	m_SomeColour = ( ( a_uiA << 8 | a_uiB ) << 8 | a_uiG ) << 8 | a_uiR;
}

TINT AFXShaderHAL::SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha )
{
	if ( m_SomeColourComponents[ 1 ] < 2 )
		m_Unk3 = a_fAlpha * ( 1.0f / 128.0f );
	else if ( m_SomeColourComponents[ 0 ] == 0 )
		m_Unk3 = 1.0f;
	else
		m_Unk3 = 0.0f;

	return m_Unk2;
}

void AFXShaderHAL::SetUnknown2( TINT a_Unknown )
{
	m_Unk2 = a_Unknown;
}
