#include "pch.h"
#include "AEnhancedTexture.h"
#include "AEnhancedRenderer.h"
#include "AEnhancedGUIRenderer.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedSkinShader.h"

#include <Math/TVector2.h>
#include <Render/TCameraObject.h>
#include <Render/TViewport.h>

#include <HookHelpers.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <BYardSDK/SDKHooks.h>

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

Toshi::TCameraObject g_SunCameraObject;
const Toshi::TVector4 s_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

void AEnhancedRenderer::ScenePreRender()
{
	TRenderD3DInterface* pRender = THookedRenderD3DInterface::GetSingleton();
	TRenderContext*      pRenderContext = pRender->GetCurrentContext();

	//-----------------------------------------------------------------------------
	// Pass 1. Render scene into shadow maps
	//-----------------------------------------------------------------------------
	
	// Setup viewport size
	glViewport( 0, 0, 2048, 2048 );

	// Bind shadow map frame buffer
	enhRender::g_ShadowMap1.Bind();
	glClear( GL_DEPTH_BUFFER_BIT );

	// Make light space view matrix
	TMatrix44 oSunTransform = TMatrix44::IDENTITY;
	oSunTransform.SetTranslation( TVector3( 25.17f, -50.30f, -1.05 ) );
	
	TVector4 direction = TVector3( 19.09f, -8.36f, -35.88 ) - oSunTransform.GetTranslation3();
	direction.Normalise();
	
	oSunTransform.LookAtDirection( direction, s_vWorldUp );

	// Prepare camera
	g_SunCameraObject.SetMode( TRenderContext::CameraMode_Orthographic );
	g_SunCameraObject.SetProjectionCentreX( 0.5f );
	g_SunCameraObject.SetProjectionCentreY( 0.5f );
	g_SunCameraObject.SetFOV( TMath::DegToRad( 172.0f ) );
	g_SunCameraObject.GetTransformObject().SetMatrix( oSunTransform );
	g_SunCameraObject.SetNear( 1.0f );
	g_SunCameraObject.SetFar( 100.0f );

	// Update game's render context
	*(TCameraObject**)( TUINTPTR( pRenderContext ) + 0x3BC ) = &g_SunCameraObject;
	CALL_THIS( 0x006cd0d0, Toshi::TCameraObject*, void, &g_SunCameraObject );

	// Add the light view matrix to the top of the transform stack to render the scene from it's POV
	auto& rTransformStack = pRender->GetTransforms();
	rTransformStack.Reset();
	rTransformStack.Top().Identity();
	rTransformStack.Push( pRenderContext->GetWorldViewMatrix() );

	CALL_THIS( 0x006125d0, void*, void, *(void**)0x007b45fc, TINT, 2 ); // AModelRepos::RenderModelsOfType
	CALL_THIS( 0x006125d0, void*, void, *(void**)0x007b45fc, TINT, 1 ); // AModelRepos::RenderModelsOfType

	// Render terrain
	if ( *(TINT*)0x00796300 )
	{
		if ( *(TINT*)0x0078de44 )
		{
			CALL_THIS( 0x005dd5c0, void*, void, *(void**)0x0078de44 ); // AGateManager::Render
		}

		CALL_THIS( 0x005ea8b0, void*, void, *(void**)0x00796300 ); // ATerrain::Render
		CALL_THIS( 0x005ef3a0, void*, void, *(void**)0x00796304 ); // ATreeManager::Render
		CALL_THIS( 0x005e17a0, void*, void, *(void**)0x0078deb0 ); // AInstanceManager::Render
		CALL_THIS( 0x005e3990, void*, void, *(void**)0x007922e0 ); // ARegrowthManager::Render
	}
	
	CALL_THIS( 0x0053a320, void*, void, *(void**)0x00783c18, TBOOL, TFALSE ); // AAnimalPopulationManager::Render
	CALL( 0x006be990, void );                                                 // TRenderD3DInterface::FlushShaders

	// Unbind shadow map after scene rendering is done
	enhRender::g_ShadowMap1.Unbind();

	//-----------------------------------------------------------------------------
	// Pass 2. Render the scene into 3 GBuffer's textures
	//-----------------------------------------------------------------------------

	// Setup viewport size
	glViewport( 0, 0, m_oWindowParams.uiWidth, m_oWindowParams.uiHeight );

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
	static TPString8 s_FogColor            = TPS8D( "u_FogColor" );
	enhRender::g_ShaderLighting.SetUniform( s_DirectionalLightDir, enhRender::g_DirectionalLightDir );
	enhRender::g_ShaderLighting.SetUniform( s_FogColor, enhRender::g_FogColor );
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

	// Create buffers for shadow maps
	enhRender::g_ShadowMap1.Create();
	enhRender::g_ShadowMap1.CreateDepthTexture( 2048, 2048 );
	enhRender::g_ShadowMap1.SetDrawBuffer( GL_NONE );
	enhRender::g_ShadowMap1.Unbind();
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

struct ARenderer
{
	TCHAR PADDING[0x28];
	TCameraObject* m_pCameraObject;
};

struct ACamera
	: public Toshi::TObject
{
	Toshi::TMatrix44 m_Matrix;
	TFLOAT           m_fFOV;
	TFLOAT           m_fProjectionCentreX;
	TFLOAT           m_fProjectionCentreY;
};

MEMBER_HOOK( 0x0060b370, ARenderer, ARenderer_RenderMainScene, void, TFLOAT a_fDeltaTime )
{
	AEnhancedRenderer::GetSingleton()->ScenePreRender();

	// Render the scene
	CallOriginal( a_fDeltaTime );

	AEnhancedRenderer::GetSingleton()->ScenePostRender();
}

MEMBER_HOOK( 0x0060acc0, ARenderer, ARenderer_UpdateMainCamera, void, const TMatrix44& a_rTransformMatrix, ACamera* a_pCamera )
{
	//m_pCameraObject->SetMode( TRenderContext::CameraMode_Orthographic );
	m_pCameraObject->SetProjectionCentreX( a_pCamera->m_fProjectionCentreX );
	m_pCameraObject->SetProjectionCentreY( a_pCamera->m_fProjectionCentreY );
	m_pCameraObject->SetFOV( a_pCamera->m_fFOV );
	m_pCameraObject->GetTransformObject().SetMatrix( a_rTransformMatrix );
	m_pCameraObject->SetNear( *(TFLOAT*)( TUINT( this ) + 0x38 ) );
	m_pCameraObject->SetFar( *(TFLOAT*)( TUINT( this ) + 0x34 ) );
}

//-----------------------------------------------------------------------------
// This is absurd but Intel compiler optimized the original game's code in such
// a way that ECX register is not  updated before calling
// ComputePerspectiveFrustum so it needs to be manually set in a hooked method.
// Luckily ComputePerspectiveProjection is always called before so it can be
// used to obtain the pointer.
//-----------------------------------------------------------------------------
TRenderContextD3D* g_pRenderContext;

MEMBER_HOOK( 0x006d6c80, TRenderContextD3D, TRenderContextD3D_ComputePerspectiveProjection, void )
{
	g_pRenderContext = this;

	CallOriginal();
	enhRender::g_Projection = *(TMatrix44*)( TUINTPTR( this ) + 960 );

	// Fix depth being 0..1 (D3D) instead of -1..1 (OpenGL)
	enhRender::g_Projection.m_f33 = ( m_oProjParams.m_fFarClip + m_oProjParams.m_fNearClip ) / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip );
	enhRender::g_Projection.m_f43 = -( 2 * m_oProjParams.m_fNearClip * m_oProjParams.m_fFarClip ) / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip );
}

MEMBER_HOOK( 0x006d6d30, TRenderContextD3D, TRenderContextD3D_ComputePerspectiveFrustum, void* )
{
	return _orig_func_ptr( g_pRenderContext );
}

MEMBER_HOOK( 0x006c57e0, TRenderD3DInterface, TRenderD3DInterface_GetPixelAspectRatio, TFLOAT )
{
	return 0.8660254f;
}

void AEnhancedRenderer::InstallHooks()
{
	InstallHook<TRenderD3DInterface_BeginScene>();
	InstallHook<TRenderD3DInterface_EndScene>();

	InstallHook<ARenderer_RenderMainScene>();
	InstallHook<ARenderer_UpdateMainCamera>();

	InstallHook<TRenderContextD3D_ComputePerspectiveProjection>();
	InstallHook<TRenderContextD3D_ComputePerspectiveFrustum>();

	InstallHook<TRenderD3DInterface_GetPixelAspectRatio>();
}
