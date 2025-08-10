#include "pch.h"
#include "AEnhancedSkinShader.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedTexture.h"
#include "HookHelpers.h"
#include "AEnhancedRenderer.h"
#include "AEnhancedWorldShader.h"

#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

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

					/*if ( m_pLighting1 )
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
					}*/
				}
			}

			T2GLTexture* pTex = ( m_pTextures[ 0 ] ) ? TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 0 ] ) : T2TextureManager::GetSingleton()->GetInvalidTexture();

			if ( T2Render::GetTexture2D( 0 ) != pTex->GetHandle() )
			{
				AEnhancedSkinShader::GetSingleton()->FlushMultiDraw();
				T2Render::SetTexture2D( 0, pTex );
			}

			/*if ( m_pLighting1 && m_pTextures[ 2 ] )
				T2Render::SetTexture2D( 1, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 2 ] ) );

			if ( m_pLighting2 && m_pTextures[ 3 ] )
				T2Render::SetTexture2D( 2, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 3 ] ) );

			if ( m_pLighting3 && m_pTextures[ 4 ] )
				T2Render::SetTexture2D( 3, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 4 ] ) );

			if ( m_pLighting4 && m_pTextures[ 5 ] )
				T2Render::SetTexture2D( 4, TREINTERPRETCAST( T2GLTexture*, m_pTextures[ 5 ] ) );*/
		}
		else
		{
			glBindTexture( GL_TEXTURE_2D, T2TextureManager::GetSingleton()->GetInvalidTexture()->GetHandle() );
		}

		T2Render::GetRenderContext().EnableBlend( m_iBlendMode != 0 );

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

	// Create shader programs
	m_oShaderProgram   = T2Render::CreateShaderProgram( m_hVertexShader, m_hFragmentShader );

	InstallHooks();
}

AEnhancedSkinShader::~AEnhancedSkinShader()
{
}

void AEnhancedSkinShader::PreRender()
{
	g_uiRenderState = 0;

	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_BACK );

	T2Render::GetRenderContext().EnableBlend( TTRUE );
	T2Render::GetRenderContext().EnableDepthTest( TTRUE );

	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

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

	m_oMultiDrawLastBuffer.Clear();
	m_iMultiDrawSize = 0;
}

void AEnhancedSkinShader::Render( TRenderPacket* a_pRenderPacket )
{
	TINT iNumCmds = ( TINT ) a_pRenderPacket->m_pUnk;

	// Skip render packets that don't have any commands related to it
	if ( iNumCmds == 0 )
		return;

	TMesh* pMesh = a_pRenderPacket->GetMesh();
	T2Render::SetShaderProgram( m_oShaderProgram );

	ASkinMaterialWrapper* pMaterial = TSTATICCAST( ASkinMaterialWrapper, pMesh->GetMaterial() );

	TSkeletonInstance* pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();
	TUINT16            uiNumSubMeshes    = *(TUINT16*)( TUINT( pMesh ) + 0x16 );

	static TPString8 s_Shininess = TPS8D( "u_Shininess" );
	m_oShaderProgram.SetUniform( s_Shininess, ( pMaterial->m_bIsSkin && pMaterial->m_bFlag2 && pMaterial->m_bFlag3 ) ? 0.12f : 0.0f );

	for ( TUINT16 i = 0; i < iNumCmds; i++ )
	{
		auto pSubMesh        = ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x94 * i;
		auto iRenderBufferID = *TREINTERPRETCAST( TINT*, TUINT( pSubMesh ) + 0x8 );

		// Update bones
		auto iSubMeshNumBones = *TREINTERPRETCAST( TINT32*, TUINT( pSubMesh ) + 0x20 );
		auto pSubMeshBones    = TREINTERPRETCAST( TUINT32*, TUINT( pSubMesh ) + 0x24 );

		// Make sure the shader is flushed
		if ( i == 0 && iSubMeshNumBones > 1 )
			FlushMultiDraw();

		TMatrix44 s_aBoneTransforms[ 28 ];
		for ( TINT k = 0; k < iSubMeshNumBones && k < TARRAYSIZE( s_aBoneTransforms ); k++ )
		{
			//s_aBoneTransforms[ k ] = _mm512_loadu_ps( &pSkeletonInstance->GetBone( pSubMeshBones[ k ] ).m_Transform );
			s_aBoneTransforms[ k ] = pSkeletonInstance->GetBone( pSubMeshBones[ k ] ).m_Transform;
		}

		static TPString8 s_NumBones       = TPS8D( "u_NumBones" );
		static TPString8 s_BoneTransforms = TPS8D( "u_BoneTransforms" );
		
		TBOOL bSkeletonChanged = TFALSE;
		bSkeletonChanged |= m_oShaderProgram.SetUniform( s_BoneTransforms, (TMatrix44*)s_aBoneTransforms, iSubMeshNumBones );
		bSkeletonChanged |= m_oShaderProgram.SetUniform( s_NumBones, iSubMeshNumBones );

		QueueMultiDraw( 
			ARenderBufferCollection::GetSingleton()->GetRenderBuffer( iRenderBufferID )
		);

		if ( bSkeletonChanged )
			FlushMultiDraw();
	}
}

void AEnhancedSkinShader::CreateMultiDrawCommand( Toshi::TRenderPacket* a_pRenderPacket )
{
	AEnhancedRenderer* pRenderer = AEnhancedRenderer::GetSingleton();
	TMesh*             pMesh     = a_pRenderPacket->GetMesh();

	TSkeletonInstance* pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();
	TUINT16            uiNumSubMeshes    = *(TUINT16*)( TUINT( pMesh ) + 0x16 );

	// HACK: set number of commands related to this render packet.
	a_pRenderPacket->m_pUnk = (void*)0;

	for ( TUINT16 i = 0; i < uiNumSubMeshes; i++ )
	{
		auto pSubMesh        = ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x94 * i;
		auto iRenderBufferID = *TREINTERPRETCAST( TINT*, TUINT( pSubMesh ) + 0x8 );

		ARenderBuffer renderBuffer = ARenderBufferCollection::GetSingleton()->GetRenderBuffer( iRenderBufferID );

		TUINT uiNumTriangles  = renderBuffer->pIndexBuffer->size / 2;
		TUINT uiFirstIndex    = renderBuffer->pIndexBuffer->offset / 2;
		TINT  iBaseVertex     = renderBuffer->pVertexBuffer->offset / 40;
		TINT  iMatrixIndex    = pRenderer->AddMultiDrawModelViewMatrix( a_pRenderPacket->GetModelViewMatrix() );
		TBOOL bCommandCreated = TFALSE;
		
		if ( i == 0 && uiNumSubMeshes == 1 && pRenderer->GetNumMultiDrawCommands() > 0 )
		{
			DrawElementsIndirectCommand* pCmd = pRenderer->GetPrevMultiDrawCommand();

			if ( pCmd->count == uiNumTriangles && pCmd->firstIndex == uiFirstIndex && pCmd->baseVertex == iBaseVertex )
			{
				pCmd->instanceCount += 1;
				bCommandCreated = TTRUE;

				// HACK: set number of commands related to this render packet.
				// Setting to 0 since this render packet is a part of the previous one now.
				a_pRenderPacket->m_pUnk = (void*)0;
			}
		}

		if ( !bCommandCreated )
		{
			pRenderer->AddMultiDrawCommand(
			    uiNumTriangles,
			    1,
			    uiFirstIndex,
			    iBaseVertex,
			    iMatrixIndex
			);

			// HACK: set number of commands related to this render packet.
			a_pRenderPacket->m_pUnk = (void*)( (TINT)a_pRenderPacket->m_pUnk + 1 );
		}
	}	
}

void AEnhancedSkinShader::QueueMultiDraw( const ARenderBuffer& a_rcRenderBuffer )
{
	if ( !m_oMultiDrawLastBuffer || m_oMultiDrawLastBuffer->pVAO != a_rcRenderBuffer->pVAO )
	{
		FlushMultiDraw();
		m_oMultiDrawLastBuffer = a_rcRenderBuffer;
	}

	m_iMultiDrawSize += 1;
}

void AEnhancedSkinShader::FlushMultiDraw()
{
	if ( m_oMultiDrawLastBuffer && m_iMultiDrawSize != 0 )
	{
		m_oMultiDrawLastBuffer->pVAO->Bind();

		glMultiDrawElementsIndirect(
		    GL_TRIANGLE_STRIP,
		    GL_UNSIGNED_SHORT,
		    AEnhancedRenderer::GetSingleton()->GetMultiDrawCommandOffset(),
		    m_iMultiDrawSize,
		    0
		);

		AEnhancedRenderer::GetSingleton()->SeekMultiDrawCursor( m_iMultiDrawSize );

		m_iMultiDrawSize = 0;
		m_oMultiDrawLastBuffer.Clear();
	}
}

MEMBER_HOOK( 0x005f3ba0, ASkinMaterialWrapper, ASkinMaterial_PreRender, void )
{
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal();

	ASkinMaterialWrapper::PreRender();
}

class ASkinShaderHAL
{};

TBOOL g_bIsFlushingSkinShader = TFALSE;

MEMBER_HOOK( 0x005f3230, ASkinShaderHAL, ASkinShader_StartFlush, void )
{
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal();

	AEnhancedSkinShader::GetSingleton()->PreRender();
	g_bIsFlushingSkinShader = TTRUE;
}

MEMBER_HOOK( 0x005f3370, ASkinShaderHAL, ASkinShader_EndFlush, void )
{
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal();

	// NOTE: This method is shared across skin and world shaders in the original game so this hack is required
	if ( g_bIsFlushingSkinShader )
		AEnhancedSkinShader::GetSingleton()->FlushMultiDraw();
	else
		AEnhancedWorldShader::GetSingleton()->FlushMultiDraw();

	g_bIsFlushingSkinShader = TFALSE;
}

MEMBER_HOOK( 0x005f4830, ASkinShaderHAL, ASkinShader_Render, void, Toshi::TRenderPacket* a_pRenderPacket )
{
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal( a_pRenderPacket );

	AEnhancedSkinShader::GetSingleton()->Render( a_pRenderPacket );
}

void AEnhancedSkinShader::InstallHooks()
{
	InstallHook<ASkinMaterial_PreRender>();
	InstallHook<ASkinShader_StartFlush>();
	InstallHook<ASkinShader_EndFlush>();
	InstallHook<ASkinShader_Render>();
}
