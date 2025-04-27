#include "pch.h"
#include "HookHelpers.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedTexture.h"
#include "AEnhancedRenderer.h"
#include "ARenderBufferCollection.h"

#include <Render/TTransformObject.h>

#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AEnhancedWorldShader::AEnhancedWorldShader()
{
	// Compile shaders
	m_hVertexShader   = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/WorldShader.vs" );
	m_hFragmentShader = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/WorldShader.fs" );

	// Create shader program
	m_oShaderProgram = T2Render::CreateShaderProgram( m_hVertexShader, m_hFragmentShader );

	InstallHooks();
}

AEnhancedWorldShader::~AEnhancedWorldShader()
{
}

void AEnhancedWorldShader::PreRender()
{
	auto pShadowColor   = (Toshi::TVector4*)( *(TUINT*)( 0x0079a854 ) + 0xF0 );
	auto pAmbientColor  = (Toshi::TVector4*)( *(TUINT*)( 0x0079a854 ) + 0x100 );
	auto pRenderContext = TSTATICCAST(
	    Toshi::TRenderContext,
	    THookedRenderD3DInterface::GetSingleton()->GetCurrentContext()
	);

	T2Render::SetShaderProgram( m_oShaderProgram );

	static TPString8 s_uViewWorld    = TPS8D( "u_ViewWorld" );
	static TPString8 s_uColor        = TPS8D( "u_Color" );
	static TPString8 s_uShadowColor  = TPS8D( "u_ShadowColor" );
	static TPString8 s_uAmbientColor = TPS8D( "u_AmbientColor" );
	static TPString8 s_uProjection   = TPS8D( "u_Projection" );

	TMatrix44 matViewWorld;
	matViewWorld.Invert( pRenderContext->GetWorldViewMatrix() );

	m_oShaderProgram.SetUniform( s_uViewWorld, matViewWorld );
	m_oShaderProgram.SetUniform( s_uColor, Toshi::TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_oShaderProgram.SetUniform( s_uShadowColor, *pShadowColor );
	m_oShaderProgram.SetUniform( s_uAmbientColor, *pAmbientColor );
	m_oShaderProgram.SetUniform( s_uProjection, enhRender::g_Projection );

	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_BACK );

	T2Render::GetRenderContext().EnableDepthTest( TTRUE );
}

void AEnhancedWorldShader::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	AWorldMesh* pMesh = TSTATICCAST( AWorldMesh, a_pRenderPacket->GetMesh() );

	QueueMultiDraw(
		a_pRenderPacket,
	    ARenderBufferCollection::GetSingleton()->GetRenderBuffer( pMesh->m_pSubMeshes[ 0 ].iRenderBufferId )
	);
}

void AEnhancedWorldShader::CreateMultiDrawCommand( Toshi::TRenderPacket* a_pRenderPacket )
{
	ARenderBuffer renderBuffer = ARenderBufferCollection::GetSingleton()->GetRenderBuffer( TSTATICCAST( AWorldMesh, a_pRenderPacket->GetMesh() )->m_pSubMeshes[ 0 ].iRenderBufferId );

	AEnhancedRenderer* pRenderer = AEnhancedRenderer::GetSingleton();

	// HACK: set number of commands related to this render packet
	a_pRenderPacket->m_pUnk = (void*)1;

	pRenderer->AddMultiDrawCommand(
	    renderBuffer->pIndexBuffer->size / 2,
	    1,
	    renderBuffer->pIndexBuffer->offset / 2,
	    renderBuffer->pVertexBuffer->offset / 44,
	    pRenderer->AddMultiDrawModelViewMatrix( a_pRenderPacket->GetModelViewMatrix() )
	);
}

void AEnhancedWorldShader::QueueMultiDraw( Toshi::TRenderPacket* a_pRenderPacket, const ARenderBuffer& a_rcRenderBuffer )
{
	TINT iNumCmds = (TINT)a_pRenderPacket->m_pUnk;

	if ( iNumCmds != 0 && !m_oMultiDrawLastBuffer || m_oMultiDrawLastBuffer->pVAO != a_rcRenderBuffer->pVAO )
	{
		FlushMultiDraw();
		m_oMultiDrawLastBuffer = a_rcRenderBuffer;
	}

	m_iMultiDrawSize += iNumCmds;
}

void AEnhancedWorldShader::FlushMultiDraw()
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

class AWorldMaterial
	: public Toshi::TMaterial
{
public:
	static constexpr TUINT MAX_TEXTURES = 4;

	using BLENDMODE = TINT;

public:
	Toshi::TTexture* m_aTextures[ MAX_TEXTURES ];
	BLENDMODE        m_eBlendMode;
	TFLOAT           m_fUVAnimX;
	TFLOAT           m_fUVAnimY;
	TFLOAT           m_fUVAnimSpeedX;
	TFLOAT           m_fUVAnimSpeedY;
};

MEMBER_HOOK( 0x005f6f70, AWorldMaterial, AWorldMaterial_PreRender, void )
{
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal();

	TTextureResourceHAL* pTexture = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );
	T2GLTexture*         pGLTexture = AEnhancedTextureManager::GetAssociation( pTexture->GetD3DTexture() );
	
	T2Render::GetRenderContext().EnableBlend( m_eBlendMode != 0 );

	if ( T2Render::GetTexture2D( 0 ) != pGLTexture->GetHandle() )
	{
		AEnhancedWorldShader::GetSingleton()->FlushMultiDraw();
		T2Render::SetTexture2D( 0, *pGLTexture );
	}
}

class AWorldShaderHAL
{};

MEMBER_HOOK( 0x005f6510, AWorldShaderHAL, AWorldShaderHAL_StartFlush, void )
{
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal();

	AEnhancedWorldShader::GetSingleton()->PreRender();
}

MEMBER_HOOK( 0x005f6cb0, AWorldShaderHAL, AWorldShaderHAL_Render, void, Toshi::TRenderPacket* a_pRenderPacket )
{
	if ( *(TBOOL*)( TUINT( this ) + 0xEC ) == TFALSE )
		return;

	AEnhancedWorldShader::GetSingleton()->Render( a_pRenderPacket );
	
	if ( !enhRender::g_bIsShadowPass )
		CallOriginal( a_pRenderPacket );
}

void AEnhancedWorldShader::InstallHooks()
{
	InstallHook<AWorldMaterial_PreRender>();
	InstallHook<AWorldShaderHAL_StartFlush>();
	InstallHook<AWorldShaderHAL_Render>();
}
