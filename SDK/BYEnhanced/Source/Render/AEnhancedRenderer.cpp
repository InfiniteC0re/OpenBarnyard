#include "pch.h"
#include "AEnhancedTexture.h"
#include "AEnhancedRenderer.h"
#include "AEnhancedGUIRenderer.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedSkinShader.h"

#include <AHooks.h>

#include <Math/TVector2.h>
#include <Render/TCameraObject.h>
#include <Render/TViewport.h>
#include <Render/TShader.h>

#include <HookHelpers.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/GL/T2SharedRenderBuffer_GL.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <BYardSDK/SDKHooks.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AEnhancedRenderer::AEnhancedRenderer()
    : m_vecMultiDrawCommands( GetGlobalAllocator(), 1024, 1024 )
{
}

TBOOL AEnhancedRenderer::Create()
{
	// Setup window parameters
	m_oWindowParams.pchTitle    = "Barnyard Enhanced";
	m_oWindowParams.bIsWindowed = TTRUE;
	m_oWindowParams.uiWidth     = 1280;
	m_oWindowParams.uiHeight    = 720;

	// Create display
	TBOOL bDisplayCreated = T2Render::CreateSingleton()->Create( m_oWindowParams );
	TASSERT( TTRUE == bDisplayCreated );

	// Create buffers
	m_oMultiDrawShaderStorage = T2Render::CreateShaderStorageBuffer( TNULL, 8 * 1024 * 1024, GL_DYNAMIC_DRAW );
	m_oMultiDrawShaderStorage.Unbind();
	m_oIndirectBuffer = T2Render::CreateIndirectBuffer( TNULL, 1024 * 1024, GL_DYNAMIC_DRAW );
	m_oIndirectBuffer.Unbind();

	InstallHooks();

	// Create shaders and initialise other modules here
	AEnhancedTextureManager::CreateSingleton();
	AEnhancedGUIRenderer::CreateSingleton();
	ARenderBufferCollection::CreateSingleton()->Create( 10000 );
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

void AEnhancedRenderer::ResetMultiDraw()
{
	m_vecMultiDrawCommands.Clear();
	m_iMultiDrawNumMatrices = 0;
	m_bMultiDrawDirty       = TFALSE;
}

void AEnhancedRenderer::AddMultiDrawCommand( TUINT a_uiCount, TUINT a_uiInstanceCount, TUINT a_uiFirstIndex, TINT a_iBaseVertex, TUINT a_uiBaseInstance )
{
	m_vecMultiDrawCommands.EmplaceBack( a_uiCount, a_uiInstanceCount, a_uiFirstIndex, a_iBaseVertex, a_uiBaseInstance );
	m_bMultiDrawDirty = TTRUE;
}

TINT AEnhancedRenderer::AddMultiDrawModelViewMatrix( const Toshi::TMatrix44& a_rcModelView )
{
	m_oMultiDrawData.matrices[ m_iMultiDrawNumMatrices++ ] = a_rcModelView;
	m_bMultiDrawDirty                                      = TTRUE;

	return m_iMultiDrawNumMatrices - 1;
}

void AEnhancedRenderer::FlushMultiDrawCommands()
{
	m_iMultiDrawCursor = 0;

	if ( m_bMultiDrawDirty && m_vecMultiDrawCommands.Size() > 0 )
	{
		m_oIndirectBuffer.UpdateData( &m_vecMultiDrawCommands[ 0 ], 0, m_vecMultiDrawCommands.Size() * sizeof( DrawElementsIndirectCommand ) );
		m_oMultiDrawShaderStorage.UpdateData( &m_oMultiDrawData, 0, sizeof( TMatrix44 ) * m_iMultiDrawNumMatrices );
		m_bMultiDrawDirty = TFALSE;
	}
}

DrawElementsIndirectCommand* AEnhancedRenderer::GetPrevMultiDrawCommand()
{
	TASSERT( m_vecMultiDrawCommands.Size() > 0 );
	return &m_vecMultiDrawCommands[ m_vecMultiDrawCommands.Size() - 1 ];
}

Toshi::TCameraObject g_SunCameraObject;
const Toshi::TVector4 s_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

struct ACamera
    : public Toshi::TObject
{
	Toshi::TMatrix44 m_Matrix;
	TFLOAT           m_fFOV;
	TFLOAT           m_fProjectionCentreX;
	TFLOAT           m_fProjectionCentreY;
};

#define SHADOW_MAP_SIZE 4096

void AEnhancedRenderer::ScenePreRender()
{
	TRenderD3DInterface* pRender = THookedRenderD3DInterface::GetSingleton();
	TRenderContext*      pRenderContext = pRender->GetCurrentContext();

	//-----------------------------------------------------------------------------
	// Pass 1. Render scene into shadow maps
	//-----------------------------------------------------------------------------
	
	ACamera*  pCamera       = CALL_THIS( 0x0045b870, void*, ACamera*, *(void**)0x007822e0 );
	TMatrix44 oCamTransform = pCamera->m_Matrix;

	enhRender::g_bIsShadowPass = TTRUE;
	
	// Setup viewport size
	glViewport( 0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE );

	// Bind shadow map frame buffer
	enhRender::g_ShadowMap1.Bind();
	glClear( GL_DEPTH_BUFFER_BIT );

	// Make light space view matrix
	TMatrix44 oSunTransform = TMatrix44::IDENTITY;
	enhRender::g_DirectionalLightDir.Normalize();
	oSunTransform.SetTranslation( TVector3(
		oCamTransform.m_f41 - enhRender::g_DirectionalLightDir.x * 40.0f,
		oCamTransform.m_f42 - enhRender::g_DirectionalLightDir.y * 40.0f,
	    oCamTransform.m_f43 - enhRender::g_DirectionalLightDir.z * 40.0f
	) );

	oSunTransform.LookAtDirection( enhRender::g_DirectionalLightDir, s_vWorldUp );

	g_SunCameraObject.SetMode( TRenderContext::CameraMode_Orthographic );
	g_SunCameraObject.SetProjectionCentreX( 0.5f );
	g_SunCameraObject.SetProjectionCentreY( 0.5f );
	g_SunCameraObject.SetFOV( TMath::DegToRad( 50.9f ) );
	g_SunCameraObject.GetTransformObject().SetMatrix( oSunTransform );
	g_SunCameraObject.SetNear( 1.0f );
	g_SunCameraObject.SetFar( 250.0f );

	// Update game's render context
	*(TCameraObject**)( TUINTPTR( pRenderContext ) + 0x3BC ) = &g_SunCameraObject;
	CALL_THIS( 0x006cd0d0, Toshi::TCameraObject*, void, &g_SunCameraObject );

	// Add the light view matrix to the top of the transform stack to render the scene from it's POV
	auto& rTransformStack = pRender->GetTransforms();
	rTransformStack.Reset();
	rTransformStack.PushNull().Identity();
	rTransformStack.Push( pRenderContext->GetWorldViewMatrix() );

	// Update light view matrix
	enhRender::g_LightViewMatrix.Set( enhRender::g_Projection );
	enhRender::g_LightViewMatrix.Multiply( pRenderContext->GetWorldViewMatrix() );

	CALL_THIS( 0x006125d0, void*, void, *(void**)0x007b45fc, TINT, 2 ); // AModelRepos::RenderModelsOfType
	CALL_THIS( 0x006125d0, void*, void, *(void**)0x007b45fc, TINT, 1 ); // AModelRepos::RenderModelsOfType

	// Render terrain
	if ( *(TINT*)0x00796300 )
	{
		if ( *(TINT*)0x0078de44 )
			CALL_THIS( 0x005dd5c0, void*, void, *(void**)0x0078de44 ); // AGateManager::Render

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
	GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers( TARRAYSIZE( attachments ), attachments );

	enhRender::g_bIsShadowPass = TFALSE;
}

void AEnhancedRenderer::ScenePostRender()
{
	TRenderD3DInterface* pRender        = THookedRenderD3DInterface::GetSingleton();
	TRenderContext*      pRenderContext = pRender->GetCurrentContext();

	enhRender::g_FrameBufferDeferred.Unbind();

	// Get camera transform
	ACamera*  pCamera       = CALL_THIS( 0x0045b870, void*, ACamera*, *(void**)0x007822e0 );
	TMatrix44 oCamTransform = pCamera->m_Matrix;
	
	// Reset draw buffers attachments
	GLuint attachment = GL_COLOR_ATTACHMENT0;
	glDrawBuffers( 1, &attachment );

	// Deferred rendering pass
	T2Render::SetShaderProgram( enhRender::g_ShaderLighting );
	T2Render::SetTexture2D( 0, enhRender::g_FrameBufferDeferred.GetAttachment( 0 ) ); // world positions
	T2Render::SetTexture2D( 1, enhRender::g_FrameBufferDeferred.GetAttachment( 1 ) ); // normals
	T2Render::SetTexture2D( 2, enhRender::g_FrameBufferDeferred.GetAttachment( 2 ) ); // colors
	T2Render::SetTexture2D( 3, enhRender::g_FrameBufferDeferred.GetAttachment( 3 ) ); // info
	T2Render::SetTexture2D( 4, enhRender::g_FrameBufferDeferred.GetDepthTexture() );  // depth
	T2Render::SetTexture2D( 5, enhRender::g_ShadowMap1.GetDepthTexture() );           // shadow map

	static TPString8 s_DirectionalLightDir   = TPS8D( "u_DirectionalLightDir" );
	static TPString8 s_AmbientColor          = TPS8D( "u_AmbientColor" );
	static TPString8 s_DiffuseColor          = TPS8D( "u_DiffuseColor" );
	static TPString8 s_FogColor              = TPS8D( "u_FogColor" );
	static TPString8 s_LightViewMatrix       = TPS8D( "u_LightViewMatrix" );
	static TPString8 s_LightProjectionMatrix = TPS8D( "u_LightProjectionMatrix" );
	static TPString8 s_ShadowBiasMin         = TPS8D( "u_ShadowBiasMin" );
	static TPString8 s_ShadowBiasMax         = TPS8D( "u_ShadowBiasMax" );
	static TPString8 s_ShadowStrength        = TPS8D( "u_ShadowStrength" );
	static TPString8 s_Gamma                 = TPS8D( "u_Gamma" );
	static TPString8 s_Exposure              = TPS8D( "u_Exposure" );
	static TPString8 s_CamPos                = TPS8D( "u_CamPos" );
	static TPString8 s_SpecularColor         = TPS8D( "u_SpecularColor" );
	static TPString8 s_View                  = TPS8D( "u_View" );
	enhRender::g_ShaderLighting.SetUniform( s_DirectionalLightDir, enhRender::g_DirectionalLightDir );
	enhRender::g_ShaderLighting.SetUniform( s_FogColor, enhRender::g_FogColor );
	enhRender::g_ShaderLighting.SetUniform( s_SpecularColor, enhRender::g_SpecularColor );
	enhRender::g_ShaderLighting.SetUniform( s_LightViewMatrix, enhRender::g_LightViewMatrix );
	enhRender::g_ShaderLighting.SetUniform( s_ShadowBiasMin, enhRender::g_ShadowBiasMin );
	enhRender::g_ShaderLighting.SetUniform( s_ShadowBiasMax, enhRender::g_ShadowBiasMax );
	enhRender::g_ShaderLighting.SetUniform( s_ShadowStrength, enhRender::g_ShadowStrength );
	enhRender::g_ShaderLighting.SetUniform( s_Gamma, enhRender::g_Gamma );
	enhRender::g_ShaderLighting.SetUniform( s_Exposure, enhRender::g_Exposure );
	enhRender::g_ShaderLighting.SetUniform( s_AmbientColor, *(TVector4*)( ( *(TUINT*)0x0079a854 ) + 0x100 ) );
	enhRender::g_ShaderLighting.SetUniform( s_DiffuseColor, *(TVector4*)( ( *(TUINT*)0x0079a854 ) + 0xF0 ) );
	enhRender::g_ShaderLighting.SetUniform( s_CamPos, oCamTransform.GetTranslation3() );
	enhRender::g_ShaderLighting.SetUniform( s_View, oCamTransform );

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
	    GL_RGB32F,
	    GL_RGB,
	    GL_FLOAT
	);

	// Create attachment to store normals
	enhRender::g_FrameBufferDeferred.CreateAttachment(
	    1,
	    m_oWindowParams.uiWidth,
	    m_oWindowParams.uiHeight,
	    GL_RGB16F,
	    GL_RGB,
	    GL_HALF_FLOAT
	);

	// Create attachment to store albedo
	enhRender::g_FrameBufferDeferred.CreateAttachment(
	    2,
	    m_oWindowParams.uiWidth,
	    m_oWindowParams.uiHeight,
	    GL_RGB,
	    GL_RGB,
	    GL_UNSIGNED_BYTE
	);

	// Create attachment to store other information
	enhRender::g_FrameBufferDeferred.CreateAttachment(
	    3,
	    m_oWindowParams.uiWidth,
	    m_oWindowParams.uiHeight,
	    GL_RG16F,
	    GL_RG,
	    GL_HALF_FLOAT
	);

	enhRender::g_FrameBufferDeferred.Unbind();

	// Create buffers for shadow maps
	enhRender::g_ShadowMap1.Create();
	enhRender::g_ShadowMap1.CreateDepthTexture( SHADOW_MAP_SIZE, SHADOW_MAP_SIZE );
	enhRender::g_ShadowMap1.SetDrawBuffer( GL_NONE );
	enhRender::g_ShadowMap1.Unbind();
}

void AEnhancedRenderer::CreateShaders()
{
	T2CompiledShader hScreenQuad = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/ScreenQuad.vs" );

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
		T2Render::GetSingleton()->Update( 0.0f );
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

	//enhRender::g_Projection.m_f11 *= -1;

	static TFLOAT s_fTime = 0.0f;

	TMatrix44* pMatrix = (TMatrix44*)( TUINTPTR( this ) + 960 );
	//pMatrix->m_f11 *= TMath::Sin( s_fTime );
	//pMatrix->m_f22 *= TMath::Cos( s_fTime );

	s_fTime += 0.001f;

	// Fix depth being 0..1 (D3D) instead of -1..1 (OpenGL)
	enhRender::g_Projection.m_f33 = ( m_oProjParams.m_fFarClip + m_oProjParams.m_fNearClip ) / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip );
	enhRender::g_Projection.m_f43 = -( 2 * m_oProjParams.m_fNearClip * m_oProjParams.m_fFarClip ) / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip );
}

MEMBER_HOOK( 0x006d6f80, TRenderContextD3D, TRenderContextD3D_ComputeOrthographicProjection, void )
{
	CallOriginal();
	*(glm::mat4*)&enhRender::g_Projection = glm::orthoNO( -25.0f, 25.0f, -25.0f, 25.0f, 1.0f, 280.0f );
}

MEMBER_HOOK( 0x006d7030, TRenderContextD3D, TRenderContextD3D_ComputeOrthographicFrustum, void )
{
	TFrustum& frustums = *(TFrustum*)( TUINT( this ) + 0x1A8 );
	TFrustum& frustums2 = *(TFrustum*)( TUINT( this ) + 0x268 );

	// Setup planes
	frustums[ WORLDPLANE_LEFT ].Set( -1.0f, 0.0f, 0.0f, 40.0f );
	frustums[ WORLDPLANE_RIGHT ].Set( 1.0f, 0.0f, 0.0f, 40.0f );
	frustums[ WORLDPLANE_BOTTOM ].Set( 0.0f, -1.0f, 0.0f, 40.0f );
	frustums[ WORLDPLANE_TOP ].Set( 0.0f, 1.0f, 0.0f, 40.0f );
	frustums[ WORLDPLANE_NEAR ].Set( 0.0f, 0.0f, -1.0f, 1.0f );
	frustums[ WORLDPLANE_FAR ].Set( 0.0f, 0.0f, 1.0f, 280.0f );

	frustums2[ WORLDPLANE_LEFT ]   = frustums[ WORLDPLANE_LEFT ];
	frustums2[ WORLDPLANE_RIGHT ]  = frustums[ WORLDPLANE_RIGHT ];
	frustums2[ WORLDPLANE_BOTTOM ] = frustums[ WORLDPLANE_BOTTOM ];
	frustums2[ WORLDPLANE_TOP ]    = frustums[ WORLDPLANE_TOP ];
	frustums2[ WORLDPLANE_NEAR ]   = frustums[ WORLDPLANE_NEAR ];
	frustums2[ WORLDPLANE_FAR ]    = frustums[ WORLDPLANE_FAR ];
}

MEMBER_HOOK( 0x006d6d30, TRenderContextD3D, TRenderContextD3D_ComputePerspectiveFrustum, void* )
{
	return _orig_func_ptr( g_pRenderContext );
}

MEMBER_HOOK( 0x006c57e0, TRenderD3DInterface, TRenderD3DInterface_GetPixelAspectRatio, TFLOAT )
{
	//return 0.8660254f;
	return 1.0f;
}

MEMBER_HOOK( 0x006c67c0, TRenderD3DInterface, TRenderD3DInterface_FlushOrderTables, void )
{
	const TShader*         pWorldShader = *(TShader**)0x0079a854;
	const TShader*         pSkinShader  = *(TShader**)0x0079a4f8;
	TPriList<TOrderTable>& orderTables  = *(TPriList<TOrderTable>*)( TUINTPTR( this ) + 0xD28 );

	for ( auto pOrderTable = orderTables.Begin(); pOrderTable != orderTables.End(); pOrderTable++ )
	{
		TShader* pShader      = pOrderTable->GetShader();

		// TODO: Support skin shader
		if ( ( pShader == pWorldShader || pShader == pSkinShader ) && pOrderTable->m_pLastRegMat )
		{
			for ( TRegMaterial* it = pOrderTable->m_pLastRegMat; it != TNULL; it = it->GetNextRegMat() )
			{
				for ( TRenderPacket* pPacket = it->m_pLastRenderPacket; pPacket != TNULL; pPacket = pPacket->GetNextPacket() )
				{
					if ( pShader == pWorldShader )
						AEnhancedWorldShader::GetSingleton()->CreateMultiDrawCommand( pPacket );
					else if (pShader == pSkinShader)
						AEnhancedSkinShader::GetSingleton()->CreateMultiDrawCommand( pPacket );
				}
			}
		}
	}

	AEnhancedRenderer::GetSingleton()->FlushMultiDrawCommands();
	AEnhancedRenderer::GetSingleton()->ResetMultiDraw();

	CallOriginal();
}

void AEnhancedRenderer::InstallHooks()
{
	InstallHook<TRenderD3DInterface_BeginScene>();
	InstallHook<TRenderD3DInterface_EndScene>();

	InstallHook<ARenderer_RenderMainScene>();
	InstallHook<ARenderer_UpdateMainCamera>();

	InstallHook<TRenderContextD3D_ComputePerspectiveProjection>();
	InstallHook<TRenderContextD3D_ComputePerspectiveFrustum>();
	InstallHook<TRenderContextD3D_ComputeOrthographicProjection>();
	InstallHook<TRenderContextD3D_ComputeOrthographicFrustum>();

	InstallHook<TRenderD3DInterface_GetPixelAspectRatio>();
	InstallHook<TRenderD3DInterface_FlushOrderTables>();

	//AHooks::AddHook( Hook_TOrderTable_Flush, HookType_Before, TOrderTable_BeforeFlush );
}
