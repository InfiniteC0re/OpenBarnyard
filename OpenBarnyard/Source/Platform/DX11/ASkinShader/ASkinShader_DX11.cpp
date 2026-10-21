#include "pch.h"
#include "ASkinShader_DX11.h"
#include "ASkinMesh_DX11.h"
#include "ASkinMaterial_DX11.h"

#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TTextureResourceHAL_DX11.h>
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

TDEFINE_CLASS( ASkinShaderHAL );

// ASkinShader_D3D8_Win.vsh (and its HD version) and the ps.1.4 pixel shader of the HD lighting
static const TCHAR s_szSkinShader[] = R"(
cbuffer Constants : register( b0 )
{
	float4x4 g_mModelViewProjection : packoffset( c0 );
	float4   g_vBoneIndexScale      : packoffset( c4 );
	float4   g_aBoneRows[ 84 ]      : packoffset( c5 );   // 3 rows of the transposed transform of each bone
	float4   g_vLightDirection      : packoffset( c89 );
	float4   g_vAmbientColour       : packoffset( c90 );  // alpha of the packet (w)
	float4   g_vLightColour         : packoffset( c91 );
	float4   g_vLightingRange       : packoffset( c93 );  // min (z) and max (w) of the diffuse lighting
	float4   g_vUpVector            : packoffset( c94 );
	float4   g_vAlphaTest           : packoffset( c96 );
	float4   g_vFog                 : packoffset( c97 );  // enabled, start, end
	float4   g_vFogColour           : packoffset( c98 );
	float4   g_vLightingLerp        : packoffset( c99 );
	float4   g_vStages              : packoffset( c100 ); // texture of stage 0
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
	float2 vLightingUV : TEXCOORD1;
};

void SkinVertex( VS_IN In, out float4 a_vPosition, out float3 a_vNormal )
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
	a_vPosition      = float4( dot( vRow0, vPosition ), dot( vRow1, vPosition ), dot( vRow2, vPosition ), 1.0f );
	a_vNormal        = float3( dot( vRow0.xyz, In.vNormal ), dot( vRow1.xyz, In.vNormal ), dot( vRow2.xyz, In.vNormal ) );
}

PS_IN vs_main( VS_IN In )
{
	float4 vPosition;
	float3 vNormal;
	SkinVertex( In, vPosition, vNormal );

	float fDiffuse = clamp( dot( vNormal, -g_vLightDirection.xyz ), g_vLightingRange.z, g_vLightingRange.w );

	PS_IN Out;
	Out.vPosition   = mul( vPosition, g_mModelViewProjection );
	Out.vColour.rgb = fDiffuse * g_vLightColour.rgb + ( 1.0f - fDiffuse ) * g_vAmbientColour.rgb;
	Out.vColour.a   = g_vAmbientColour.a;
	Out.vColour     = saturate( Out.vColour );
	Out.vUV         = In.vUV;
	Out.vLightingUV = 0.0f;
	return Out;
}

PS_IN vs_hd( VS_IN In )
{
	float4 vPosition;
	float3 vNormal;
	SkinVertex( In, vPosition, vNormal );

	PS_IN Out;
	Out.vPosition   = mul( vPosition, g_mModelViewProjection );
	Out.vColour     = float4( 0.0f, 0.0f, 0.0f, saturate( g_vAmbientColour.a ) );
	Out.vUV         = In.vUV;
	Out.vLightingUV = float2( dot( vNormal, -g_vLightDirection.xyz ), dot( vNormal, g_vUpVector.xyz ) );
	return Out;
}

Texture2D    g_Texture : register( t0 );
Texture2D    g_aLightingTextures[ 4 ] : register( t1 );
SamplerState g_Sampler : register( s0 );
SamplerState g_aLightingSamplers[ 4 ] : register( s1 );

float4 ApplyFog( float4 a_vColour, float a_fDepth )
{
	if ( g_vFog.x != 0.0f )
		a_vColour.rgb = lerp( g_vFogColour.rgb, a_vColour.rgb, saturate( ( g_vFog.z - a_fDepth ) / ( g_vFog.z - g_vFog.y ) ) );

	return a_vColour;
}

// The texture stages: texture * diffuse
float4 ps_main( PS_IN In ) : SV_TARGET
{
	float4 vColour = In.vColour;

	if ( g_vStages.x != 0.0f )
		vColour = saturate( g_Texture.Sample( g_Sampler, In.vUV ) * vColour );

	AlphaTest( vColour.a, g_vAlphaTest );
	return ApplyFog( vColour, In.vPosition.w );
}

float4 ps_hd( PS_IN In ) : SV_TARGET
{
	float4 vTexture   = g_Texture.Sample( g_Sampler, In.vUV );
	float4 vLighting0 = g_aLightingTextures[ 0 ].Sample( g_aLightingSamplers[ 0 ], In.vLightingUV );
	float4 vLighting1 = g_aLightingTextures[ 1 ].Sample( g_aLightingSamplers[ 1 ], In.vLightingUV );
	float4 vLighting2 = g_aLightingTextures[ 2 ].Sample( g_aLightingSamplers[ 2 ], In.vLightingUV );
	float4 vLighting3 = g_aLightingTextures[ 3 ].Sample( g_aLightingSamplers[ 3 ], In.vLightingUV );

	float4 vShadow    = lerp( vLighting2, vLighting0, g_vLightingLerp );
	float4 vHighlight = lerp( vLighting3, vLighting1, g_vLightingLerp );

	float4 vColour;
	vColour.rgb = saturate( saturate( vTexture.rgb - vShadow.rgb ) + vHighlight.rgb );
	vColour.a   = vTexture.a * In.vColour.a;

	AlphaTest( vColour.a, g_vAlphaTest );
	return ApplyFog( vColour, In.vPosition.w );
}
)";

ASkinShaderHAL::ASkinShaderHAL()
    : m_pVertexShader( TNULL )
    , m_pVertexShaderHD( TNULL )
    , m_pPixelShader( TNULL )
    , m_pPixelShaderHD( TNULL )
    , m_pInputLayout( TNULL )
    , m_iAlphaRef( 128 )
    , m_bRenderEnvMap( TFALSE )
    , m_bHighEndSkinning( TTRUE )
    , m_bLightScattering( TTRUE )
    , m_bIsAlphaBlendMaterial( TFALSE )
{
}

ASkinShaderHAL::~ASkinShaderHAL()
{
}

void ASkinShaderHAL::SetStates()
{
	auto pRender = TRenderD3DInterface::Interface();

	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
	pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? D3D11_CULL_BACK : D3D11_CULL_FRONT );
}

void ASkinShaderHAL::Flush()
{
	if ( IsValidated() )
	{
		SetStates();
		m_aOrderTables[ 0 ].Render();
		TRenderD3DInterface::Interface()->SetFogEnable( TFALSE );
	}

	BaseClass::Flush();
}

void ASkinShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		TRenderD3DInterface::Interface()->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
		SetStates();
	}
}

void ASkinShaderHAL::EndFlush()
{
	TRenderD3DInterface::Interface()->SetFogEnable( TFALSE );
}

TBOOL ASkinShaderHAL::Create()
{
	m_aOrderTables[ 0 ].Create( this, -390 );
	m_aOrderTables[ 1 ].Create( this, 1000 );
	m_aOrderTables[ 2 ].Create( this, -400 );

	return BaseClass::Create();
}

TBOOL ASkinShaderHAL::Validate()
{
	if ( IsValidated() )
		return TTRUE;

	EnableHighEndSkinning( TTRUE );

	ID3DBlob* pVSBlob   = TRenderD3DInterface::CompileShader( s_szSkinShader, "vs_main", "vs_4_0", TNULL );
	ID3DBlob* pVSHDBlob = TRenderD3DInterface::CompileShader( s_szSkinShader, "vs_hd", "vs_4_0", TNULL );
	ID3DBlob* pPSBlob   = TRenderD3DInterface::CompileShader( s_szSkinShader, "ps_main", "ps_4_0", TNULL );
	ID3DBlob* pPSHDBlob = TRenderD3DInterface::CompileShader( s_szSkinShader, "ps_hd", "ps_4_0", TNULL );

	if ( !pVSBlob || !pVSHDBlob || !pPSBlob || !pPSHDBlob ||
	     !TRenderD3DInterface::CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader ) ||
	     !TRenderD3DInterface::CreateVertexShader( pVSHDBlob->GetBufferPointer(), pVSHDBlob->GetBufferSize(), &m_pVertexShaderHD ) ||
	     !TRenderD3DInterface::CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader ) ||
	     !TRenderD3DInterface::CreatePixelShader( pPSHDBlob->GetBufferPointer(), pPSHDBlob->GetBufferSize(), &m_pPixelShaderHD ) )
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
	pVSHDBlob->Release();
	pPSBlob->Release();
	pPSHDBlob->Release();

	return BaseClass::Validate();
}

void ASkinShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	ID3D11DeviceChild* apObjects[] = { m_pVertexShader, m_pVertexShaderHD, m_pPixelShader, m_pPixelShaderHD, m_pInputLayout };

	for ( auto pObject : apObjects )
	{
		if ( pObject )
			pObject->Release();
	}

	m_pVertexShader   = TNULL;
	m_pVertexShaderHD = TNULL;
	m_pPixelShader    = TNULL;
	m_pPixelShaderHD  = TNULL;
	m_pInputLayout    = TNULL;

	BaseClass::Invalidate();
}

TBOOL ASkinShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL ASkinShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void ASkinShaderHAL::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	if ( !a_pRenderPacket || !a_pRenderPacket->GetMesh() )
		return;

	auto pMesh     = TSTATICCAST( ASkinMeshHAL, a_pRenderPacket->GetMesh() );
	auto pMaterial = TSTATICCAST( ASkinMaterialHAL, pMesh->GetMaterial() );

	if ( pMesh->GetNumSubMeshes() == 0 ) return;

	auto pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();
	TVALIDPTR( pSkeletonInstance );

	TVector4 vLightingLerp = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );

	auto pRender         = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );

	Validate();

	if ( pMaterial->IsHDLighting() && pMaterial->HasLighting1Tex() && pMaterial->HasLighting2Tex() )
	{
		// The HD lighting uses 4 shading textures
		pRender->SetShaders( m_pVertexShaderHD, m_pPixelShaderHD, m_pInputLayout );

		TMatrix44 oViewModel;
		oViewModel.Invert( a_pRenderPacket->GetModelViewMatrix() );

		TVector4 vUpVector;
		vUpVector.Negate3( oViewModel.AsBasisVector4( 2 ) );
		vUpVector.w = 1.0f;
		pRender->SetVec4InVSBuffer( CONSTANT_UPVECTOR, &vUpVector );

		TFLOAT fShadeCoeff = a_pRenderPacket->GetShadeCoeff() * ( 1.0f / 256.0f );

		if ( pMaterial->GetSomeTexture() )
			fShadeCoeff = fShadeCoeff - 0.3f;

		vLightingLerp.x = 1.0f - fShadeCoeff;
		vLightingLerp.y = vLightingLerp.x;
		vLightingLerp.z = vLightingLerp.x;
		vLightingLerp.w = 0.0f;

		for ( TUINT i = 0; i < ASkinMaterial::LT_NUMOF; i++ )
		{
			pRender->SetTexture( 1 + i, pMaterial->GetLightingTexture( ASkinMaterial::ELightingTexture( i ) )->GetD3DTexture() );
			pRender->SetTextureMipFilter( 1 + i, TRenderD3DInterface::TEXTUREFILTER_NONE );
		}
	}
	else
	{
		static constexpr TVector4 s_vUpVector = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_UPVECTOR, &s_vUpVector );

		pRender->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );
	}

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

	TVector4 vLightColour   = a_pRenderPacket->GetLightColour();
	TVector4 vAmbientColour = a_pRenderPacket->GetAmbientColour();
	vAmbientColour.w        = a_pRenderPacket->GetAlpha();

	TMatrix44 oModelView = a_pRenderPacket->GetModelViewMatrix();
	TMatrix44 oWorldModelView;

	for ( TINT i = 0; i < 4; i++ )
	{
		oWorldModelView.AsBasisVector4( i ).x = oModelView.m_f11 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f12 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f13 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).y = oModelView.m_f21 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f22 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f23 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).z = oModelView.m_f31 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f32 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f33 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).w = oModelView.m_f14 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f24 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f34 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
	}

	TVector3 vLightDir;
	vLightDir.x = oWorldModelView.m_f11 * fLightDirX + oWorldModelView.m_f21 * fLightDirY + oWorldModelView.m_f31 * fLightDirZ;
	vLightDir.y = oWorldModelView.m_f12 * fLightDirX + oWorldModelView.m_f22 * fLightDirY + oWorldModelView.m_f32 * fLightDirZ;
	vLightDir.z = oWorldModelView.m_f13 * fLightDirX + oWorldModelView.m_f23 * fLightDirY + oWorldModelView.m_f33 * fLightDirZ;
	vLightDir.Normalize();

	const TVector4 vLightDirWorld( vLightDir.x, vLightDir.y, vLightDir.z, 0.0f );

	static constexpr TVector4 s_vLightingRange = TVector4( 1.0f, 0.0f, 0.0f, 1.0f );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTDIRECTION, &vLightDirWorld );
	pRender->SetVec4InVSBuffer( CONSTANT_AMBIENTCOLOUR, &vAmbientColour );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTCOLOUR, &vLightColour );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTINGRANGE, &s_vLightingRange );

	TINT iAlpha = TINT( a_pRenderPacket->GetAlpha() * 128.0f );
	if ( m_iAlphaRef < iAlpha ) iAlpha = m_iAlphaRef;
	pRender->SetAlphaRef( iAlpha );

	if ( pMaterial->GetBlendMode() != ASkinMaterial::BLENDMODE_DEFAULT || a_pRenderPacket->GetAlpha() < 1.0f )
	{
		if ( ISZERO( sm_eRenderStateFlags & RENDERSTATE_ALPHABLEND ) )
		{
			pRender->SetAlphaBlendEnable( TTRUE );
			sm_eRenderStateFlags |= RENDERSTATE_ALPHABLEND;
		}
	}
	else if ( !ISZERO( sm_eRenderStateFlags & RENDERSTATE_ALPHABLEND ) )
	{
		pRender->SetAlphaBlendEnable( TFALSE );
		sm_eRenderStateFlags &= ~RENDERSTATE_ALPHABLEND;
	}

	TVector4 vFog;
	TVector4 vFogColour;
	pRender->GetFogParams( vFog, vFogColour );

	const TVector4 vAlphaTest = pRender->GetAlphaTestParams();
	const TVector4 vStages( pRender->GetTexture( 0 ) ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f );
	pRender->SetVec4InVSBuffer( CONSTANT_ALPHATEST, &vAlphaTest );
	pRender->SetVec4InVSBuffer( CONSTANT_FOG, &vFog );
	pRender->SetVec4InVSBuffer( CONSTANT_FOGCOLOUR, &vFogColour );
	pRender->SetVec4InVSBuffer( CONSTANT_LIGHTINGLERP, &vLightingLerp );
	pRender->SetVec4InVSBuffer( CONSTANT_STAGES, &vStages );

	auto pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );

	TVertexBlockResource::HALBuffer vertexHALBuffer;
	pVertexPool->GetHALBuffer( &vertexHALBuffer );

	for ( TINT i = 0; i < pMesh->GetNumSubMeshes(); i++ )
	{
		auto pSubMesh = pMesh->GetSubMesh( i );
		TASSERT( pSubMesh->uiNumBones <= MAX_BONES );

		TINT iCurrentRegister = CONSTANT_BONES;

		for ( TUINT k = 0; k < pSubMesh->uiNumBones; k++ )
		{
			auto pBone = &pSkeletonInstance->GetBone( pSubMesh->aBones[ k ] );

			TMatrix44 oBoneTransform;
			oBoneTransform.Transpose( pBone->m_Transform );
			pRender->SetVec4InVSBuffer( iCurrentRegister, &oBoneTransform, 3 );

			iCurrentRegister += 3;
		}

		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pSubMesh->pIndexPool );
		TVALIDPTR( pIndexPool );

		TIndexBlockResource::HALBuffer indexHALBuffer;
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

void ASkinShaderHAL::EnableRenderEnvMap( TBOOL a_bEnable )
{
	m_bRenderEnvMap = a_bEnable;
}

TBOOL ASkinShaderHAL::IsHighEndSkinning()
{
	return m_bHighEndSkinning;
}

void ASkinShaderHAL::EnableHighEndSkinning( TBOOL a_bEnable )
{
	m_bHighEndSkinning = a_bEnable && IsCapableHighEndSkinning();
}

TBOOL ASkinShaderHAL::IsCapableHighEndSkinning()
{
	return TTRUE;
}

TBOOL ASkinShaderHAL::IsLightScattering()
{
	return m_bLightScattering;
}

void ASkinShaderHAL::SetLightScattering( TBOOL a_bEnable )
{
}

TBOOL ASkinShaderHAL::IsAlphaBlendMaterial()
{
	return m_bIsAlphaBlendMaterial;
}

void ASkinShaderHAL::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
	m_bIsAlphaBlendMaterial = a_bIsAlphaBlendMaterial;
}

ASkinMaterial* ASkinShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	auto pMaterial = new ASkinMaterialHAL();
	pMaterial->SetShader( this );

	if ( TNULL != a_szName )
		pMaterial->SetName( a_szName );

	if ( ASkinShaderHAL::IsAlphaBlendMaterial() )
	{
		auto pAlphaBlendMaterial = new ASkinMaterialHAL();
		pAlphaBlendMaterial->SetShader( this );
		pAlphaBlendMaterial->Create( 1 );

		pMaterial->SetAlphaBlendMaterial( pAlphaBlendMaterial );
	}

	return pMaterial;
}

ASkinMesh* ASkinShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	auto pMesh = new ASkinMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

TINT ASkinShaderHAL::AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )
{
	TASSERT( !"Not implemeted in the original game" );
	return 0;
}

void ASkinShaderHAL::SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )
{
	TASSERT( !"Not implemeted in the original game" );
}

void ASkinShaderHAL::RemoveLight( TINT a_iIndex )
{
	TASSERT( !"Not implemeted in the original game" );
}

TBOOL ASkinShaderHAL::IsEnableRenderEnvMap()
{
	return m_bRenderEnvMap;
}

void ASkinShaderHAL::SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA )
{
	m_SomeColour = ( ( a_uiA << 8 | a_uiB ) << 8 | a_uiG ) << 8 | a_uiR;
}

TINT ASkinShaderHAL::SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha )
{
	if ( m_SomeColourComponents[ 1 ] < 2 )
		m_Unk3 = a_fAlpha * ( 1.0f / 128.0f );
	else if ( m_SomeColourComponents[ 0 ] == 0 )
		m_Unk3 = 1.0f;
	else
		m_Unk3 = 0.0f;

	return m_Unk2;
}

void ASkinShaderHAL::SetUnknown2( TINT a_Unknown )
{
	m_Unk2 = a_Unknown;
}

void ASkinShaderHAL::SetAlphaRef( TINT a_iAlphaRef )
{
	TRenderD3DInterface::Interface()->SetAlphaRef( 1 );
	m_iAlphaRef = a_iAlphaRef;
}
