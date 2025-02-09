#include "pch.h"
#include "AEnhancedTexture.h"
#include "AEnhancedRenderer.h"
#include "AEnhancedGUIRenderer.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedSkinShader.h"

#include <Math/TVector2.h>

#include <HookHelpers.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TRenderInterface_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL AEnhancedRenderer::Create()
{
	// Setup window parameters
	m_oWindowParams.pchTitle    = "Barnyard Enhanced";
	m_oWindowParams.bIsWindowed = TTRUE;
	m_oWindowParams.uiWidth     = 800;
	m_oWindowParams.uiHeight    = 600;

	// Create display
	TBOOL bDisplayCreated = T2Render::CreateSingleton()->Create( m_oWindowParams );
	TASSERT( TTRUE == bDisplayCreated );

	InstallHooks();

	// Create shaders and initialise other modules here
	AEnhancedTextureManager::CreateSingleton();
	AEnhancedGUIRenderer::CreateSingleton();
	ARenderBufferCollection::CreateSingleton()->Create( 5000 );
	AEnhancedWorldShader::CreateSingleton();
	AEnhancedSkinShader::CreateSingleton();

	// Create framebuffers and shaders
	CreateFrameBuffers();
	CreateShaders();

	return bDisplayCreated;
}

// NOTE: Currently unused
TBOOL AEnhancedRenderer::Update( TFLOAT a_fDeltaTime )
{
	T2Render* pRender = T2Render::GetSingleton();

	// Update renderer and begin drawing scene
	pRender->Update( a_fDeltaTime );
	pRender->BeginScene();

	// ...

	// Rendering is done, so end scene
	pRender->EndScene();

	return TTRUE;
}

void AEnhancedRenderer::ScenePreRender()
{
	// Prepare HDR framebuffer before rendering the scene
	enhRender::g_FrameBufferDeferred.Bind();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Set attachments to draw to (deferred rendering)
	GLuint attachments[ 3 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers( TARRAYSIZE( attachments ), attachments );
}

void AEnhancedRenderer::ScenePostRender()
{
	enhRender::g_FrameBufferDeferred.Unbind();
	
	// Reset draw buffers attachments
	GLuint attachment = GL_COLOR_ATTACHMENT0;
	glDrawBuffers( 1, &attachment );

	// Deferred rendering pass
	T2Render::SetShaderProgram( enhRender::g_ShaderLighting );
	T2Render::SetTexture2D( 0, enhRender::g_FrameBufferDeferred.GetAttachment( 0 ) ); // world positions
	T2Render::SetTexture2D( 1, enhRender::g_FrameBufferDeferred.GetAttachment( 1 ) ); // normals
	T2Render::SetTexture2D( 2, enhRender::g_FrameBufferDeferred.GetAttachment( 2 ) ); // colors
	T2Render::SetTexture2D( 3, enhRender::g_FrameBufferDeferred.GetDepthTexture() );  // depth

	static TPString8 s_DirectionalLightDir = TPS8D( "u_DirectionalLightDir" );
	static TPString8 s_AmbientColor        = TPS8D( "u_AmbientColor" );
	static TPString8 s_DiffuseColor        = TPS8D( "u_DiffuseColor" );
	enhRender::g_ShaderLighting.SetUniform( s_DirectionalLightDir, enhRender::g_DirectionalLightDir );
	enhRender::g_ShaderLighting.SetUniform( s_AmbientColor, *(TVector4*)( ( *(TUINT*)0x0079a854 ) + 0x100 ) );
	enhRender::g_ShaderLighting.SetUniform( s_DiffuseColor, *(TVector4*)( ( *(TUINT*)0x0079a854 ) + 0xF0 ) );
	RenderScreenQuad();

	//// Apply HDR
	//T2Render::SetShaderProgram( enhRender::g_ShaderHDR );
	//T2Render::SetTexture2D( 0, enhRender::g_FrameBufferDeferred.GetAttachment( 0 ) );
	//RenderScreenQuad();
}

void AEnhancedRenderer::RenderScreenQuad()
{
	T2VertexArray::Unbind();

	static struct QuadVAO
	{

		QuadVAO()
		{
			static constexpr struct
			{
				TVector2 position;
				TVector2 uv;
			} s_aQuadVertices[] = {
				{ .position = TVector2( 1.0f, 1.0f ), .uv = TVector2( 1.0f, 1.0f ) },
				{ .position = TVector2( -1.0f, 1.0f ), .uv = TVector2( 0.0f, 1.0f ) },
				{ .position = TVector2( 1.0f, -1.0f ), .uv = TVector2( 1.0f, 0.0f ) },
				{ .position = TVector2( -1.0f, -1.0f ), .uv = TVector2( 0.0f, 0.0f ) },
			};

			TUINT16 aIndices[] = { 0, 1, 2, 3 };

			handle = T2Render::CreateVertexArray(
			    T2Render::CreateVertexBuffer( s_aQuadVertices, sizeof( s_aQuadVertices ), GL_STATIC_DRAW ),
			    T2Render::CreateIndexBuffer( aIndices, TARRAYSIZE( aIndices ), GL_STATIC_DRAW )
			);

			handle.GetVertexBuffer().SetAttribPointer( 0, 2, GL_FLOAT, sizeof( TVector2 ) * 2, 0 );
			handle.GetVertexBuffer().SetAttribPointer( 1, 2, GL_FLOAT, sizeof( TVector2 ) * 2, sizeof( TVector2 ), TFALSE );
		}

		T2VertexArray handle;

	} s_oVAO;

	s_oVAO.handle.Bind();
	glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, TNULL );
}

void AEnhancedRenderer::CreateFrameBuffers()
{
	enhRender::g_FrameBufferDeferred.Create();

	// Create depth texture
	enhRender::g_FrameBufferDeferred.CreateDepthTexture( m_oWindowParams.uiWidth, m_oWindowParams.uiHeight );

	// Create attachment to store positions
	enhRender::g_FrameBufferDeferred.CreateAttachment(
	    0,
	    m_oWindowParams.uiWidth,
	    m_oWindowParams.uiHeight,
	    GL_RGB16F,
	    GL_RGBA,
	    GL_FLOAT
	);

	// Create attachment to store normals
	enhRender::g_FrameBufferDeferred.CreateAttachment(
	    1,
	    m_oWindowParams.uiWidth,
	    m_oWindowParams.uiHeight,
	    GL_RGB16F,
	    GL_RGBA,
	    GL_FLOAT
	);

	// Create attachment to store albedo
	enhRender::g_FrameBufferDeferred.CreateAttachment(
	    2,
	    m_oWindowParams.uiWidth,
	    m_oWindowParams.uiHeight,
	    GL_RGB16F,
	    GL_RGBA,
	    GL_FLOAT
	);

	enhRender::g_FrameBufferDeferred.Unbind();
}

void AEnhancedRenderer::CreateShaders()
{
	T2CompiledShader hScreenQuad = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/ScreenQuad.vs" );
	
	{
		// HDR Shader
		T2CompiledShader hFragment = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/HDR.fs" );
		enhRender::g_ShaderHDR     = T2Render::CreateShaderProgram( hScreenQuad, hFragment );

		glObjectLabel( GL_PROGRAM, enhRender::g_ShaderHDR.GetProgram(), 3, "HDR" );
	}

	{
		// Lighting Shader (Deferred Rendering)
		T2CompiledShader hFragment  = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/Lighting.fs" );
		enhRender::g_ShaderLighting = T2Render::CreateShaderProgram( hScreenQuad, hFragment );

		glObjectLabel( GL_PROGRAM, enhRender::g_ShaderLighting.GetProgram(), 3, "Deferred" );
	}
}

MEMBER_HOOK( 0x006c6590, TRenderD3DInterface, TRenderD3DInterface_BeginScene, TBOOL )
{
	if ( CallOriginal() )
	{
		T2Render::GetSingleton()->BeginScene();
		return TTRUE;
	}

	return TFALSE;
}

MEMBER_HOOK( 0x006c6fd0, TRenderD3DInterface, TRenderD3DInterface_EndScene, TBOOL )
{
	if ( CallOriginal() )
	{
		T2Render::GetSingleton()->EndScene();
		return TTRUE;
	}

	return TFALSE;
}

class ARenderer
{};

MEMBER_HOOK( 0x0060b370, ARenderer, ARenderer_RenderMainScene, void, TFLOAT a_fDeltaTime )
{
	AEnhancedRenderer::GetSingleton()->ScenePreRender();

	// Render the scene
	CallOriginal( a_fDeltaTime );

	AEnhancedRenderer::GetSingleton()->ScenePostRender();
}

void AEnhancedRenderer::InstallHooks()
{
	InstallHook<TRenderD3DInterface_BeginScene>();
	InstallHook<TRenderD3DInterface_EndScene>();

	InstallHook<ARenderer_RenderMainScene>();
}
