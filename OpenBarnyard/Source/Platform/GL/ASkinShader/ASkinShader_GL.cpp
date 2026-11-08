#include "pch.h"
#include "ASkinShader_GL.h"
#include "ASkinMesh_GL.h"
#include "ASkinMaterial_GL.h"

#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TTextureResourceHAL_GL.h>
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

TDEFINE_CLASS( ASkinShaderHAL );

// ASkinShader_D3D8_Win.vsh (and its HD version) and the ps.1.4 pixel shader of the HD lighting
static const TCHAR s_szSkinShader[] = R"(
layout( std140 ) uniform Constants
{
	mat4 g_mModelViewProjection;
	vec4 g_vBoneIndexScale;
	vec4 g_vLightDirection;
	vec4 g_vAmbientColour;  // alpha of the packet (w)
	vec4 g_vLightColour;
	vec4 g_vLightingRange;  // min (z) and max (w) of the diffuse lighting
	vec4 g_vUpVector;
	vec4 g_vAlphaTest;
	vec4 g_vFog;            // enabled, start, end
	vec4 g_vFogColour;
	vec4 g_vLightingLerp;
	vec4 g_vStages;         // texture of stage 0
	vec4 g_aBoneRows[ 84 ]; // 3 rows of the transposed transform of each bone
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
out vec2 v_vLightingUV;

void main()
{
	vec4 vPosition;
	vec3 vNormal;
	SkinVertex( vPosition, vNormal );

	gl_Position = ToClipSpace( vPosition * g_mModelViewProjection );
	v_vUV       = a_vUV;

#  ifdef HD_LIGHTING
	v_vColour     = vec4( 0.0, 0.0, 0.0, saturate( g_vAmbientColour.a ) );
	v_vLightingUV = vec2( dot( vNormal, -g_vLightDirection.xyz ), dot( vNormal, g_vUpVector.xyz ) );
#  else
	float fDiffuse = clamp( dot( vNormal, -g_vLightDirection.xyz ), g_vLightingRange.z, g_vLightingRange.w );

	v_vColour     = saturate( vec4( fDiffuse * g_vLightColour.rgb + ( 1.0 - fDiffuse ) * g_vAmbientColour.rgb, g_vAmbientColour.a ) );
	v_vLightingUV = vec2( 0.0 );
#  endif
}

#else

in vec4 v_vColour;
in vec2 v_vUV;
in vec2 v_vLightingUV;

uniform sampler2D g_Texture0;

vec4 ApplyFog( vec4 a_vColour )
{
	if ( g_vFog.x != 0.0 )
		a_vColour.rgb = mix( g_vFogColour.rgb, a_vColour.rgb, saturate( ( g_vFog.z - 1.0 / gl_FragCoord.w ) / ( g_vFog.z - g_vFog.y ) ) );

	return a_vColour;
}

#  ifdef HD_LIGHTING

uniform sampler2D g_Texture1;
uniform sampler2D g_Texture2;
uniform sampler2D g_Texture3;
uniform sampler2D g_Texture4;

void main()
{
	vec4 vTexture   = texture( g_Texture0, v_vUV );
	vec4 vLighting0 = texture( g_Texture1, v_vLightingUV );
	vec4 vLighting1 = texture( g_Texture2, v_vLightingUV );
	vec4 vLighting2 = texture( g_Texture3, v_vLightingUV );
	vec4 vLighting3 = texture( g_Texture4, v_vLightingUV );

	vec4 vShadow    = mix( vLighting2, vLighting0, g_vLightingLerp );
	vec4 vHighlight = mix( vLighting3, vLighting1, g_vLightingLerp );

	vec4 vColour;
	vColour.rgb = saturate( saturate( vTexture.rgb - vShadow.rgb ) + vHighlight.rgb );
	vColour.a   = vTexture.a * v_vColour.a;

	AlphaTest( vColour.a, g_vAlphaTest );
	o_vColour = ApplyFog( vColour );
}

#  else

// The texture stages: texture * diffuse
void main()
{
	vec4 vColour = v_vColour;

	if ( g_vStages.x != 0.0 )
		vColour = saturate( texture( g_Texture0, v_vUV ) * vColour );

	AlphaTest( vColour.a, g_vAlphaTest );
	o_vColour = ApplyFog( vColour );
}

#  endif

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

ASkinShaderHAL::ASkinShaderHAL()
    : m_uiProgram( 0 )
    , m_uiProgramHD( 0 )
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
	auto pRender = TRenderGLInterface::Interface();

	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaFunc( GL_GREATER );
	pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
	pRender->SetCullMode( m_bRenderEnvMap == TTRUE ? TRenderGLInterface::CULL_BACK : TRenderGLInterface::CULL_FRONT );
}

void ASkinShaderHAL::Flush()
{
	if ( IsValidated() )
	{
		SetStates();
		m_aOrderTables[ 0 ].Render();
		TRenderGLInterface::Interface()->SetFogEnable( TFALSE );
	}

	BaseClass::Flush();
}

void ASkinShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		TRenderGLInterface::Interface()->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
		SetStates();
	}
}

void ASkinShaderHAL::EndFlush()
{
	TRenderGLInterface::Interface()->SetFogEnable( TFALSE );
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

	m_uiProgram   = TRenderGLInterface::CompileProgram( s_szSkinShader );
	m_uiProgramHD = TRenderGLInterface::CompileProgram( s_szSkinShader, "#define HD_LIGHTING\n" );

	if ( !m_uiProgram || !m_uiProgramHD )
	{
		TASSERT( !"Couldn't create the shaders" );
		return TFALSE;
	}

	return BaseClass::Validate();
}

void ASkinShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	TRenderGLInterface::DestroyProgram( m_uiProgram );
	TRenderGLInterface::DestroyProgram( m_uiProgramHD );

	m_uiProgram   = 0;
	m_uiProgramHD = 0;

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

	auto pRender         = TRenderGLInterface::Interface();
	auto pCurrentContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );

	Validate();

	if ( pMaterial->IsHDLighting() && pMaterial->HasLighting1Tex() && pMaterial->HasLighting2Tex() )
	{
		// The HD lighting uses 4 shading textures
		pRender->SetShaders( m_uiProgramHD, &s_oInputLayout );

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
			pRender->SetTexture( 1 + i, pMaterial->GetLightingTexture( ASkinMaterial::ELightingTexture( i ) )->GetGLTexture() );
			pRender->SetTextureMipFilter( 1 + i, TRenderGLInterface::TEXTUREFILTER_NONE );
		}
	}
	else
	{
		static constexpr TVector4 s_vUpVector = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		pRender->SetVec4InVSBuffer( CONSTANT_UPVECTOR, &s_vUpVector );

		pRender->SetShaders( m_uiProgram, &s_oInputLayout );
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

		pRender->SetConstantUploadLimit( iCurrentRegister );

		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pSubMesh->pIndexPool );
		TVALIDPTR( pIndexPool );

		TIndexBlockResource::HALBuffer indexHALBuffer;
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
	TRenderGLInterface::Interface()->SetAlphaRef( 1 );
	m_iAlphaRef = a_iAlphaRef;
}
