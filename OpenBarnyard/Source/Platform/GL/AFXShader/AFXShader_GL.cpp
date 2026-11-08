#include "pch.h"
#include "AFXShader_GL.h"
#include "AFXMaterial_GL.h"
#include "AFXMesh_GL.h"

#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TRenderInterface_GL.h>
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

TDEFINE_CLASS( AFXShaderHAL );

// NOTE: Retail never creates a vertex shader for AFXShaderHAL and draws through the low end FVF,
// ignoring the constants it uploads. They match the ASkinShader vertex shader, so it's used here.
static const TCHAR s_szFXShader[] = R"(
layout( std140 ) uniform Constants
{
	mat4 g_mModelViewProjection;
	vec4 g_vBoneIndexScale;
	vec4 g_aBoneRows[ 84 ]; // 3 rows of the transposed transform of each bone
	vec4 g_vLightDirection;
	vec4 g_vAmbientColour;  // alpha of the packet (w)
	vec4 g_vLightColour;
	vec4 g_vUnused92;
	vec4 g_vLightingRange;  // min (z) and max (w) of the diffuse lighting
	vec4 g_vUnused94;
	vec4 g_vUnused95;
	vec4 g_vAlphaTest;
	vec4 g_vStages;         // texture of stage 0
};

#ifdef VERTEX_SHADER

layout( location = 0 ) in vec3 a_vPosition;
layout( location = 1 ) in vec3 a_vNormal;
layout( location = 2 ) in vec4 a_vWeights;
layout( location = 3 ) in vec4 a_vBones;
layout( location = 4 ) in vec2 a_vUV;

void SkinVertex( out vec4 a_vOutPosition, out vec3 a_vOutNormal )
{
	vec4 vBones   = g_vBoneIndexScale * a_vBones.bgra;
	vec4 vWeights = a_vWeights.bgra;
	vec4 vRow0    = vec4( 0.0 );
	vec4 vRow1    = vec4( 0.0 );
	vec4 vRow2    = vec4( 0.0 );

	for ( int i = 0; i < 4; i++ )
	{
		int iBone = int( vBones[ i ] + 0.5 );
		vRow0 += g_aBoneRows[ iBone + 0 ] * vWeights[ i ];
		vRow1 += g_aBoneRows[ iBone + 1 ] * vWeights[ i ];
		vRow2 += g_aBoneRows[ iBone + 2 ] * vWeights[ i ];
	}

	vec4 vPosition = vec4( a_vPosition, 1.0 );
	a_vOutPosition = vec4( dot( vRow0, vPosition ), dot( vRow1, vPosition ), dot( vRow2, vPosition ), 1.0 );
	a_vOutNormal   = vec3( dot( vRow0.xyz, a_vNormal ), dot( vRow1.xyz, a_vNormal ), dot( vRow2.xyz, a_vNormal ) );
}

out vec4 v_vColour;
out vec2 v_vUV;

void main()
{
	vec4 vPosition;
	vec3 vNormal;
	SkinVertex( vPosition, vNormal );

	float fDiffuse = clamp( dot( vNormal, -g_vLightDirection.xyz ), g_vLightingRange.z, g_vLightingRange.w );

	gl_Position = ToClipSpace( vPosition * g_mModelViewProjection );
	v_vColour   = saturate( vec4( fDiffuse * g_vLightColour.rgb + ( 1.0 - fDiffuse ) * g_vAmbientColour.rgb, g_vAmbientColour.a ) );
	v_vUV       = a_vUV;
}

#else

in vec4 v_vColour;
in vec2 v_vUV;

uniform sampler2D g_Texture0;

// The texture stages: texture * diffuse
void main()
{
	vec4 vColour = v_vColour;

	if ( g_vStages.x != 0.0 )
		vColour = saturate( texture( g_Texture0, v_vUV ) * vColour );

	AlphaTest( vColour.a, g_vAlphaTest );
	o_vColour = vColour;
}

#endif
)";

static const TGLVertexAttribute s_aInputAttributes[] = {
	{ 0, 0, 3, GL_FALSE, GL_FLOAT, 0 },
	{ 1, 0, 3, GL_FALSE, GL_FLOAT, 12 },
	{ 2, 0, 4, GL_TRUE, GL_UNSIGNED_BYTE, 24 },
	{ 3, 0, 4, GL_TRUE, GL_UNSIGNED_BYTE, 28 },
	{ 4, 0, 2, GL_FALSE, GL_FLOAT, 32 },
};

static const TGLInputLayout s_oInputLayout = { s_aInputAttributes, TARRAYSIZE( s_aInputAttributes ) };

AFXShaderHAL::AFXShaderHAL()
    : m_uiProgram( 0 )
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

	auto pRender = TRenderGLInterface::Interface();

	Validate();
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaFunc( GL_GREATER );
	pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
	pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? TRenderGLInterface::CULL_BACK : TRenderGLInterface::CULL_FRONT );

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

	m_uiProgram = TRenderGLInterface::CompileProgram( s_szFXShader );

	if ( !m_uiProgram )
	{
		TASSERT( !"Couldn't create the shaders" );
		return TFALSE;
	}

	return TShader::Validate();
}

void AFXShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	TRenderGLInterface::DestroyProgram( m_uiProgram );
	m_uiProgram = 0;

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

	auto pRender         = TRenderGLInterface::Interface();
	auto pCurrentContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );
	auto pBones          = a_pRenderPacket->GetSkeletonInstance()->GetBones();

	Validate();
	pRender->SetShaders( m_uiProgram, &s_oInputLayout );

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
		    GL_TRIANGLE_STRIP,
		    pIndexPool->GetNumIndices(),
		    indexHALBuffer.pIndexBuffer,
		    indexHALBuffer.uiIndexOffset * sizeof( TIndexType ),
		    GL_UNSIGNED_SHORT,
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
