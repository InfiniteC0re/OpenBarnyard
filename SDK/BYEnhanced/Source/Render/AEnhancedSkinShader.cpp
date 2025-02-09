#include "pch.h"
#include "AEnhancedSkinShader.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedTexture.h"
#include "HookHelpers.h"
#include "AEnhancedRenderer.h"

#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>

#include <glm/mat4x4.hpp>

#include <immintrin.h>
#include <intrin.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TUINT g_uiRenderState = 0;

class ASkinMaterialWrapper
    : public TMaterial
{
public:
	void PreRender()
	{
		if ( m_pTexture )
		{
			auto pD3DTexture = m_pTexture->GetD3DTexture();

			if ( !m_pTextures[ 0 ] || pD3DTexture != (IDirect3DTexture8*)m_pTextures[ 1 ] )
			{
				auto pInvalidNative = THookedRenderD3DInterface::GetSingleton()->GetInvalidTexture();

				if ( m_pTexture != pInvalidNative )
				{
					m_pTextures[ 0 ] = TREINTERPRETCAST( TTexture*, AEnhancedTextureManager::GetAssociation( pD3DTexture ) );
					m_pTextures[ 1 ] = TREINTERPRETCAST( TTexture*, pD3DTexture );

					if ( m_pLighting1 )
					{
						T2GLTexture* pGLTexture = AEnhancedTextureManager::GetAssociation( m_pLighting1->GetD3DTexture() );
						T2Render::SetTexture2D( 0, pGLTexture );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

						m_pTextures[ 2 ] = TREINTERPRETCAST( TTexture*, pGLTexture );
					}

					if ( m_pLighting2 )
					{
						T2GLTexture* pGLTexture = AEnhancedTextureManager::GetAssociation( m_pLighting2->GetD3DTexture() );
						T2Render::SetTexture2D( 0, pGLTexture );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

						m_pTextures[ 3 ] = TREINTERPRETCAST( TTexture*, pGLTexture );
					}

					if ( m_pLighting3 )
					{
						T2GLTexture* pGLTexture = AEnhancedTextureManager::GetAssociation( m_pLighting3->GetD3DTexture() );
						T2Render::SetTexture2D( 0, pGLTexture );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

						m_pTextures[ 4 ] = TREINTERPRETCAST( TTexture*, pGLTexture );
					}

					if ( m_pLighting4 )
					{
						T2GLTexture* pGLTexture = AEnhancedTextureManager::GetAssociation( m_pLighting4->GetD3DTexture() );
						T2Render::SetTexture2D( 0, pGLTexture );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
						glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

						m_pTextures[ 5 ] = TREINTERPRETCAST( TTexture*, pGLTexture );
					}
				}
			}

			T2Render::SetTexture2D( 0, ( m_pTextures[ 0 ] ) ? TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 0 ] ) : T2TextureManager::GetSingleton()->GetInvalidTexture() );

			if ( m_pLighting1 && m_pTextures[ 2 ] )
				T2Render::SetTexture2D( 1, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 2 ] ) );

			if ( m_pLighting2 && m_pTextures[ 3 ] )
				T2Render::SetTexture2D( 2, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 3 ] ) );

			if ( m_pLighting3 && m_pTextures[ 4 ] )
				T2Render::SetTexture2D( 3, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 4 ] ) );

			if ( m_pLighting4 && m_pTextures[ 5 ] )
				T2Render::SetTexture2D( 4, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 5 ] ) );
		}
		else
		{
			glBindTexture( GL_TEXTURE_2D, T2TextureManager::GetSingleton()->GetInvalidTexture()->GetHandle() );
		}

		if ( ( ( m_iBlendMode == 0 ) || ( m_iBlendMode == 1 ) ) || ( m_iBlendMode != 3 ) )
		{
			if ( !( g_uiRenderState & 1 ) )
			{
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glDepthMask( GL_TRUE );
				g_uiRenderState &= ~2;
				g_uiRenderState |= 1;
			}
		}
		else if ( !( g_uiRenderState & 2 ) )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			glDepthMask( GL_FALSE );
			g_uiRenderState &= ~1;
			g_uiRenderState |= 2;
		}
	}

public:
	TTextureResourceHAL* m_pTexture;
	TTextureResourceHAL* m_pLighting1;
	TTextureResourceHAL* m_pLighting2;
	TTextureResourceHAL* m_pLighting3;
	TTextureResourceHAL* m_pLighting4;
	TINT                 m_iBlendMode;
	TINT                 UNK1;
	TINT                 UNK2;
	TINT                 UNK3;
	TBOOL                m_bIsSkin;
	TBOOL                m_bFlag2;
	TBOOL                m_bFlag3;
};

AEnhancedSkinShader::AEnhancedSkinShader()
{
	// Compile shaders
	m_hVertexShader     = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/SkinShader.vs" );
	m_hFragmentShader   = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/SkinShader.fs" );
	m_hVertexShaderHD   = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/SkinShader_HD.vs" );
	m_hFragmentShaderHD = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/SkinShader_HD.fs" );

	// Create shader programs
	m_oShaderProgram   = T2Render::CreateShaderProgram( m_hVertexShader, m_hFragmentShader );
	m_oShaderProgramHD = T2Render::CreateShaderProgram( m_hVertexShaderHD, m_hFragmentShaderHD );

	// Set default shader uniforms...

	static TPString8 s_LightColourParams = TPS8D( "u_LightColourParams" );
	static TPString8 s_UpAxis            = TPS8D( "u_UpAxis" );
	static TPString8 s_LightColour       = TPS8D( "u_LightColour" );

	// HD Shader
	T2Render::SetShaderProgram( m_oShaderProgramHD );
	m_oShaderProgramHD.SetUniform( s_LightColourParams, TVector4( 1.0f, 0.0f, 0.0f, 1.0f ) );

	// Default Shader
	T2Render::SetShaderProgram( m_oShaderProgram );
	m_oShaderProgram.SetUniform( s_UpAxis, TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
	m_oShaderProgram.SetUniform( s_LightColour, TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_oShaderProgram.SetUniform( s_LightColourParams, TVector4( 1.0f, 0.0f, 0.0f, 1.0f ) );

	InstallHooks();
}

AEnhancedSkinShader::~AEnhancedSkinShader()
{
}

void AEnhancedSkinShader::PreRender()
{
	g_uiRenderState = 0;

	glEnable( GL_BLEND );

	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_BACK );

	glEnable( GL_DEPTH_TEST );

	auto pRenderContext = TSTATICCAST(
	    Toshi::TRenderContext,
	    THookedRenderD3DInterface::GetSingleton()->GetCurrentContext()
	);

	m_WorldViewMatrix  = *(Toshi::TMatrix44*)( TUINT( pRenderContext ) + 0x8C );
	m_ViewWorldMatrix.Invert( m_WorldViewMatrix );

	static TPString8 s_Projection = TPS8D( "u_Projection" );
	static TPString8 s_ViewWorld  = TPS8D( "u_ViewWorld" );

	T2Render::SetShaderProgram( m_oShaderProgram );
	m_oShaderProgram.SetUniform( s_Projection, enhRender::g_Projection );
	m_oShaderProgram.SetUniform( s_ViewWorld, m_ViewWorldMatrix );

	T2Render::SetShaderProgram( m_oShaderProgramHD );
	m_oShaderProgramHD.SetUniform( s_Projection, enhRender::g_Projection );
	m_oShaderProgramHD.SetUniform( s_ViewWorld, m_ViewWorldMatrix );
}

void AEnhancedSkinShader::Render( TRenderPacket* a_pRenderPacket )
{
	auto pMesh       = a_pRenderPacket->GetMesh();
	auto pVertexPool = *TREINTERPRETCAST( TVertexPoolResource**, TUINT( pMesh ) + 0x18 );
	auto pMaterial   = TSTATICCAST( ASkinMaterialWrapper, pMesh->GetMaterial() );
	
	static TPString8 s_UpAxis         = TPS8D( "u_UpAxis" );
	static TPString8 s_LightingLerp1  = TPS8D( "u_LightingLerp1" );
	static TPString8 s_LightingLerp2  = TPS8D( "u_LightingLerp2" );
	static TPString8 s_Model          = TPS8D( "u_Model" );
	static TPString8 s_AmbientColor   = TPS8D( "u_AmbientColor" );
	static TPString8 s_LightDirection = TPS8D( "u_LightDirection" );
	static TPString8 s_ModelView      = TPS8D( "u_ModelView" );
	static TPString8 s_NumBones       = TPS8D( "u_NumBones" );
	static TPString8 s_BoneTransforms = TPS8D( "u_BoneTransforms" );

	T2Shader* pShaderProgram = TNULL;
	if ( pMaterial->m_bIsSkin && pMaterial->m_bFlag2 && pMaterial->m_bFlag3 )
	{
		T2Render::SetShaderProgram( m_oShaderProgramHD );
		pShaderProgram = &m_oShaderProgramHD;

		TMatrix44 inverseModelView;
		inverseModelView.InvertOrthogonal( a_pRenderPacket->GetModelViewMatrix() );

		TVector4 upVector;
		upVector.Negate3( inverseModelView.AsBasisVector4( 2 ) );

		pShaderProgram->SetUniform( s_UpAxis, upVector );

		TFLOAT fLerpFactor = a_pRenderPacket->GetShadeCoeff() * ( 1.0f / 255.0f );
		pShaderProgram->SetUniform( s_LightingLerp1, TVector4( 1.0f - fLerpFactor, 1.0f - fLerpFactor, 1.0f - fLerpFactor, 0.0f ) );
		pShaderProgram->SetUniform( s_LightingLerp2, TVector4( fLerpFactor, fLerpFactor, fLerpFactor, 0.0f ) );
	}
	else
	{
		pShaderProgram = &m_oShaderProgram;
		T2Render::SetShaderProgram( m_oShaderProgram );

		// TODO: Use simple shader instead of HD...
	}

	auto&     modelView = a_pRenderPacket->GetModelViewMatrix();
	TMatrix44 worldModelView;

	for ( TINT i = 0; i < 4; i++ )
	{
		worldModelView.AsBasisVector4( i ).x = modelView.m_f11 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f12 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f13 * m_WorldViewMatrix.AsBasisVector3( i ).z;
		worldModelView.AsBasisVector4( i ).y = modelView.m_f21 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f22 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f23 * m_WorldViewMatrix.AsBasisVector3( i ).z;
		worldModelView.AsBasisVector4( i ).z = modelView.m_f31 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f32 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f33 * m_WorldViewMatrix.AsBasisVector3( i ).z;
		worldModelView.AsBasisVector4( i ).w = modelView.m_f14 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f24 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f34 * m_WorldViewMatrix.AsBasisVector3( i ).z;
	};

	TFLOAT lightDirX = -a_pRenderPacket->GetLightDirection().x;
	TFLOAT lightDirY = -a_pRenderPacket->GetLightDirection().y;
	TFLOAT lightDirZ = -a_pRenderPacket->GetLightDirection().z;

	TVector3 lightDirWorld;
	lightDirWorld.x = worldModelView.m_f11 * lightDirX + worldModelView.m_f21 * lightDirY + worldModelView.m_f31 * lightDirZ;
	lightDirWorld.y = worldModelView.m_f12 * lightDirX + worldModelView.m_f22 * lightDirY + worldModelView.m_f32 * lightDirZ;
	lightDirWorld.z = worldModelView.m_f13 * lightDirX + worldModelView.m_f23 * lightDirY + worldModelView.m_f33 * lightDirZ;
	lightDirWorld.Normalize();

	TVector4 ambientColour = a_pRenderPacket->GetAmbientColour();
	TVector4 lightColour   = a_pRenderPacket->GetLightColour();

	TMatrix44 modelMatrix;
	modelMatrix.Multiply( m_ViewWorldMatrix, a_pRenderPacket->GetModelViewMatrix() );

	pShaderProgram->SetUniform( s_Model, modelMatrix );
	pShaderProgram->SetUniform( s_AmbientColor, ambientColour );
	pShaderProgram->SetUniform( s_LightDirection, lightDirWorld );
	pShaderProgram->SetUniform( s_ModelView, a_pRenderPacket->GetModelViewMatrix() );

	auto pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();
	auto uiNumSubMeshes    = *(TUINT16*)( TUINT( pMesh ) + 0x16 );

	for ( TUINT16 i = 0; i < uiNumSubMeshes; i++ )
	{
		auto pSubMesh        = ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x94 * i;
		auto pIndexPool      = *TREINTERPRETCAST( TIndexPoolResource**, TUINT( pSubMesh ) + 0x4 );
		auto iRenderBufferID = *TREINTERPRETCAST( TINT*, TUINT( pSubMesh ) + 0x8 );

		// Update bones
		auto iSubMeshNumBones = *TREINTERPRETCAST( TINT32*, TUINT( pSubMesh ) + 0x20 );
		auto pSubMeshBones    = TREINTERPRETCAST( TUINT32*, TUINT( pSubMesh ) + 0x24 );

		__m512 s_aBoneTransforms[ 28 ];
		for ( TINT k = 0; k < iSubMeshNumBones && k < TARRAYSIZE( s_aBoneTransforms ); k++ )
		{
			s_aBoneTransforms[ k ] = _mm512_loadu_ps( &pSkeletonInstance->GetBone( pSubMeshBones[ k ] ).m_Transform );
		}

		pShaderProgram->SetUniform( s_BoneTransforms, (TMatrix44*)s_aBoneTransforms, iSubMeshNumBones );
		pShaderProgram->SetUniform( s_NumBones, iSubMeshNumBones );

		// Draw
		ARenderBufferCollection::GetSingleton()->GetRenderBuffer( iRenderBufferID )->Bind();
		glDrawElements( GL_TRIANGLE_STRIP, pIndexPool->GetNumIndices(), GL_UNSIGNED_SHORT, TNULL );
	}
}

MEMBER_HOOK( 0x005f3ba0, ASkinMaterialWrapper, ASkinMaterial_PreRender, void )
{
	CallOriginal();
	ASkinMaterialWrapper::PreRender();
}

class ASkinShaderHAL
{};

MEMBER_HOOK( 0x005f3230, ASkinShaderHAL, ASkinShader_StartFlush, void )
{
	CallOriginal();
	AEnhancedSkinShader::GetSingleton()->PreRender();
}

MEMBER_HOOK( 0x005f4830, ASkinShaderHAL, ASkinShader_Render, void, Toshi::TRenderPacket* a_pRenderPacket )
{
	CallOriginal( a_pRenderPacket );
	AEnhancedSkinShader::GetSingleton()->Render( a_pRenderPacket );
}

void AEnhancedSkinShader::InstallHooks()
{
	InstallHook<ASkinMaterial_PreRender>();
	InstallHook<ASkinShader_StartFlush>();
	InstallHook<ASkinShader_Render>();
}
