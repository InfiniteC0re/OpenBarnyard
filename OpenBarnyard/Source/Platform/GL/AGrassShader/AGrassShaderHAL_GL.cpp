#include "pch.h"
#include "AGrassShaderHAL_GL.h"
#include "AGrassMaterialHAL_GL.h"
#include "AGrassMeshHAL_GL.h"
#include "Cameras/ACameraManager.h"
#include "Assets/AModelLoader.h"
#include "Platform/GL/AWorldShader/AWorldShader_GL.h"

#include <Toshi/TScheduler.h>
#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TVertexBlockResource_GL.h>
#include <Platform/GL/TVertexPoolResource_GL.h>
#include <Platform/GL/TIndexBlockResource_GL.h>
#include <Platform/GL/TIndexPoolResource_GL.h>
#include <Platform/GL/TTextureResourceHAL_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGrassShaderHAL );

MODAPI TTextureResourceHAL* g_pGrassShaderTerrainTexture = TNULL;
static TVector3      g_vecAnimationDataOffset;
static TVector4      g_vecAnimOffset;
static const TFLOAT  g_fAnimationSpeed = 2.25f;

// AGrassShader_D3D8_Win.vsh and the texture stages of the grass
static const TCHAR s_szGrassShader[] = R"(
layout( std140 ) uniform Constants
{
	mat4 g_mModelViewProjection;
	vec4 g_vAmbientColour;
	vec4 g_vShadowColour;
	vec4 g_vOne;
	vec4 g_vLayerOffset; // how far the layer is pushed along the normal
	vec4 g_vAnimation;   // wind (xyz), UV scale of the layer (w)
	vec4 g_vAlphaTest;
	vec4 g_vFog;         // enabled, start, end
	vec4 g_vFogColour;
	vec4 g_vStages;      // texture of stage 0
};

#ifdef VERTEX_SHADER

layout( location = 0 ) in vec3 a_vPosition;
layout( location = 1 ) in vec3 a_vNormal;
layout( location = 2 ) in vec3 a_vLighting;
layout( location = 3 ) in vec2 a_vUV;

out vec4 v_vColour;
out vec2 v_vUV;

void main()
{
	vec4 vPosition = vec4( a_vNormal, 1.0 ) * g_vLayerOffset;
	vPosition += g_vAnimation * g_vLayerOffset;
	vPosition += vec4( a_vPosition, 1.0 );

	vec4 vLighting = vec4( a_vLighting, 1.0 );
	vec4 vColour   = vLighting * g_vAmbientColour + ( g_vOne - vLighting ) * g_vShadowColour;
	vColour.w      = g_vOne.w;

	gl_Position = ToClipSpace( vPosition * g_mModelViewProjection );
	v_vColour   = saturate( vColour );
	v_vUV       = a_vUV * g_vAnimation.w;
}

#else

in vec4 v_vColour;
in vec2 v_vUV;

uniform sampler2D g_Texture0;

void main()
{
	vec4 vColour = v_vColour;

	// D3DTOP_MODULATE
	if ( g_vStages.x != 0.0 )
		vColour = saturate( texture( g_Texture0, v_vUV ) * vColour );

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

AGrassShaderHAL::AGrassShaderHAL()
    : m_uiProgram( 0 )
{
}

AGrassShaderHAL::~AGrassShaderHAL()
{
}

void AGrassShaderHAL::Flush()
{
	Validate();

	auto pRender         = TRenderGLInterface::Interface();
	auto pCurrentContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );

	pRender->SetTextureMinFilter( 0, TRenderGLInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMagFilter( 0, TRenderGLInterface::TEXTUREFILTER_LINEAR );
	pRender->SetTextureMipFilter( 0, TRenderGLInterface::TEXTUREFILTER_POINT );
	pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
	pRender->SetFixedFunctionShader();

	pRender->SetTransform( TRenderGLInterface::TRANSFORM_PROJECTION, pCurrentContext->GetProjectionMatrix() );
	pRender->SetTransform( TRenderGLInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	pRender->SetAlphaBlendEnable( TFALSE );
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetAlphaRef( 1 );
	pRender->SetAlphaFunc( GL_GREATER );

	m_oOrderTable.Render();
	pRender->SetAlphaTestEnable( TFALSE );
}

void AGrassShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		auto pRender = TRenderGLInterface::Interface();

		pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
		pRender->SetShaders( m_uiProgram, &s_oInputLayout );

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
		pRender->SetAlphaFunc( GL_GREATER );
		pRender->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );

		UpdateAnimation();
	}
}

void AGrassShaderHAL::EndFlush()
{
	auto pRender = TRenderGLInterface::Interface();

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

	m_uiProgram = TRenderGLInterface::CompileProgram( s_szGrassShader );

	if ( !m_uiProgram )
	{
		TASSERT( !"Couldn't create the shaders" );
		return TFALSE;
	}

	return AGrassShader::Validate();
}

void AGrassShaderHAL::Invalidate()
{
	if ( IsValidated() )
	{
		TRenderGLInterface::DestroyProgram( m_uiProgram );
		m_uiProgram = 0;

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
	auto pRender = TRenderGLInterface::Interface();

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
	    GL_TRIANGLE_STRIP,
	    a_pIndexPool->GetNumIndices(),
	    a_rIndexBuffer.pIndexBuffer,
	    a_rIndexBuffer.uiIndexOffset * sizeof( TIndexType ),
	    GL_UNSIGNED_SHORT,
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
		auto pRender         = TRenderGLInterface::Interface();
		auto pCurrentContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );

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
		pRender->SetTexture( 0, g_pGrassShaderTerrainTexture->GetGLTexture() );
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
				pRender->SetTexture( 0, g_aGrassLayers[ i ]->GetGLTexture() );
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
