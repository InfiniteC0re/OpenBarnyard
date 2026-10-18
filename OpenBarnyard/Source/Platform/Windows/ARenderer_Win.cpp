#include "pch.h"
#include "AppBoot.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AModelLoader.h"
#include "Animation/AModelRepos.h"
#include "Terrain/ATerrainInterface.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "Render/ARenderer.h"
#include "Render/AGlowViewport.h"
#include "Cameras/ACameraManager.h"
#include "Input/AInputHandler.h"
#include "Movie/AMoviePlayer.h"
#include "Helpers/AHintManager.h"
#include "Helpers/ATreeManager.h"
#include "Helpers/AInstanceManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "GameInterface/AGameStateController.h"
#include "Render/ARainRenderer.h"
#include "Units/AUnitManager.h"
#include "Units/AAnimalPopulationManager.h"
#include "Render/ASunFlareEffect.h"
#include "Render/ADebugOverlay.h"
#include "Render/AWorldDebugDraw.h"
#include "FX/AParticleEffectManager.h"

#include <Render/TNullResource.h>

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/TVertexFactoryResource_DX8.h"
#  include "Platform/DX8/TIndexFactoryResource_DX8.h"
#  include "Platform/DX8/TTextureFactoryHAL_DX8.h"
#  include "Platform/DX8/TRenderInterface_DX8.h"
#  include "Platform/DX8/TViewport_DX8.h"

#  include "Platform/DX8/ASysShader/ASysShader_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinShader_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassShaderHAL_DX8.h"
#  include "Platform/DX8/AStaticInstanceShader/AStaticInstanceShaderHAL_DX8.h"
#  include "Platform/DX8/ALineShader/ALineShaderHAL_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include <resource.h>

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Helpers/AGateManager.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/AReplayManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0060aca0, CreateObject 0060c520, CreateObjectInPlace 0060c580
TDEFINE_CLASS( ARenderer );

// $Barnyard: FUNCTION 0060b030
ARenderer::ARenderer()
    : m_RenderGUIEmitter( this ), m_AnimationUpdateStartEmitter( this ), m_AnimationUpdateEndEmitter( this ), m_pViewport( TNULL ), m_pCameraObject( TNULL ), m_pHALViewport1( TNULL ), m_pHALViewport2( TNULL ), m_bRenderGUI( TTRUE ), m_fNear( 1.0f ), m_fFar( 280.0f ), m_eScreenCaptureStatus( SCREENCAPTURE_NONE ), m_pScreenCapture( TNULL ), m_pScreenCaptureBuffer( TNULL ), m_szScreenCaptureName( TNULL ), m_uiScreenCaptureWidth( 0 ), m_uiScreenCaptureHeight( 0 ), m_pSkyDome( TNULL ), m_iScreenshotIndex( 0 )
{
}

// $Barnyard: FUNCTION 006154c0
TBOOL ARenderer::CreateTRender()
{
#if defined( TOSHI_SKU_WINDOWS ) && defined( TRENDERINTERFACE_DX8 )
	TString8 strRenderInterfaceName = "TRenderD3DInterface";
#endif

	TKernelInterface::GetSingleton()->LoadInterface( strRenderInterfaceName );

	const TClass*        pRendererClass = TClass::Find( strRenderInterfaceName );
	TRenderD3DInterface* pRenderer      = TSTATICCAST( TRenderD3DInterface, pRendererClass->CreateObject() );

	pRenderer->SetAutoCreateSystemResources( TFALSE );
	pRenderer->Create( "Barnyard" );

	HMODULE hModuleHandle = GetModuleHandleA( TNULL );
	HICON   hIcon         = LoadIconA( hModuleHandle, MAKEINTRESOURCE( IDI_ICON1 ) );
	SendMessageA( pRenderer->GetMSWindow()->GetHWND(), 0x80, TRUE, (LPARAM)hIcon );

	m_DisplayModes.Initialise();

	CreateTRenderResources();

	TINT  iWidth       = 800;
	TINT  iHeight      = 600;
	TINT  iColourDepth = 32;
	TBOOL bWindowed    = TTRUE;

	if ( AOptions::GetSingleton() )
	{
		iWidth    = AOptions::GetSingleton()->GetWidth();
		iHeight   = AOptions::GetSingleton()->GetHeight();
		bWindowed = AOptions::GetSingleton()->IsWindowed();
		pRenderer->SetGamma( AOptions::GetSingleton()->GetGamma() );
	}

	if ( !m_DisplayModes.DoesModeExist( iWidth, iHeight, iColourDepth ) )
	{
		// Incompatible display mode
		iWidth       = 800;
		iHeight      = 600;
		iColourDepth = 32;
	}

	TRenderInterface::DISPLAYPARAMS displayParams;
	displayParams.uiWidth             = iWidth;
	displayParams.uiHeight            = iHeight;
	displayParams.bWindowed           = bWindowed;
	displayParams.uiColourDepth       = 32;
	displayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D16;

	auto pDevice = FindSuitableDevice( displayParams, TTRUE );

	if ( !pDevice )
	{
		if ( displayParams.uiColourDepth == 32 )
		{
			displayParams.uiColourDepth = 16;
			pDevice                     = FindSuitableDevice( displayParams, TTRUE );

			if ( pDevice == TNULL )
			{
				pRenderer->OnInitializationFailureDisplay();
				return TFALSE;
			}
		}
		else
		{
			pRenderer->OnInitializationFailureDisplay();
			return TFALSE;
		}
	}

	if ( !pRenderer->CreateDisplay( displayParams ) )
		return TFALSE;

	// Log info about the chosen device
	TINFO( "--------------\n" );
	TINFO( "Found Device [%s]:\n", pDevice->GetTypeString() );
	TINFO( "  Width=%d\n", pDevice->GetMode()->GetWidth() );
	TINFO( "  Height=%d\n", pDevice->GetMode()->GetHeight() );
	TINFO( "  ColourDepth=Supports[%s], Using [%d]\n", pDevice->GetMode()->Is32Bit() ? "32" : "16", displayParams.uiColourDepth );
	TINFO( "  SupportsHardwareTnL=%s\n", pDevice->SupportsHardwareTransfomations() ? "Yes" : "No" );
	TINFO( "  SupportsWindowed=%s, Using %s\n", pDevice->CanRenderWindowed() ? "Yes" : "No", displayParams.bWindowed == TTRUE ? "Windowed" : "Fullscreen" );
	TINFO( "--------------\n" );

	return TTRUE;
}

// $Barnyard: FUNCTION 006150e0
TBOOL ARenderer::CreateTRenderResources()
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );

	// Vertex factories
	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "VFactories", TNULL ), SYSRESOURCE_VFactories );

	{
		auto pResource = TSTATICCAST(
		    TVertexFactoryResource,
		    pRenderer->CreateResource(
		        TClass::Find( "TVertexFactoryResource" ),
		        "VFSYSVNDUV1",
		        pRenderer->GetSystemResource<TResource>( SYSRESOURCE_VFactories )
		    )
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_VFSYSVNDUV1 );

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams                       = 1;
		vertexFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = 24;
		vertexFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;
		pResource->Create( &vertexFormat, 54050, 0 );
	}

	{
		auto pResource = TSTATICCAST(
		    TVertexFactoryResource,
		    pRenderer->CreateResource(
		        TClass::Find( "TVertexFactoryResource" ),
		        "VFSKIN",
		        pRenderer->GetSystemResource<TResource>( SYSRESOURCE_VFactories )
		    )
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_VFSKIN );

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams                       = 1;
		vertexFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = 40;
		vertexFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;
		pResource->Create( &vertexFormat, 54050, 0 );
	}

	{
		auto pResource = TSTATICCAST(
		    TVertexFactoryResource,
		    pRenderer->CreateResource(
		        TClass::Find( "TVertexFactoryResource" ),
		        "VFWORLD",
		        pRenderer->GetSystemResource<TResource>( SYSRESOURCE_VFactories )
		    )
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_VFWORLD );

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams                       = 1;
		vertexFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = sizeof( WorldVertex );
		vertexFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;
		pResource->Create( &vertexFormat, 54050, 0 );
	}

	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "Shaders", TNULL ), SYSRESOURCE_Shaders );

	// Index factories
	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "IFactories", TNULL ), SYSRESOURCE_IFactories );

	{
		auto pResource = TSTATICCAST(
		    TIndexFactoryResource,
		    pRenderer->CreateResource(
		        TClass::Find( "TIndexFactoryResource" ),
		        "IFSYS",
		        pRenderer->GetSystemResource<TResource>( SYSRESOURCE_IFactories )
		    )
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_IFSYS );

		TIndexFactoryFormat indexFormat;
		indexFormat.uiIndexSize = 2;
		pResource->Create( &indexFormat, 64700, 0 );
	}

	// Storage
	TResource* pStorage = pRenderer->CreateResource( &TGetClass( TNullResource ), "Storage", TNULL );
	pRenderer->SetResourceExplicit( pStorage, SYSRESOURCE_Storage );
	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "Meshes", pStorage ), SYSRESOURCE_Meshes );
	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "Textures", pStorage ), SYSRESOURCE_Textures );
	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "Sounds", pStorage ), SYSRESOURCE_Sounds );
	pRenderer->SetResourceExplicit( pRenderer->CreateResource( &TGetClass( TNullResource ), "GFXFiles", pStorage ), SYSRESOURCE_GFXFiles );

	{
		auto pResource = pRenderer->CreateResource(
		    TClass::Find( "TTextureFactoryHAL" ),
		    "TextureFactory",
		    TNULL
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_TEXTUREFACTORY );
		pResource->Create();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0060aee0
TRenderAdapter::Mode::Device* ARenderer::FindSuitableDevice( TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder )
{
	auto                          pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	TRenderAdapter::Mode::Device* pDevice   = TNULL;

	if ( a_bReverseOrder )
	{
		a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D32;
		pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

		if ( !pDevice )
		{
			a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D24_X8;
			pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

			if ( !pDevice )
			{
				a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D24_S8;
				pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

				if ( !pDevice )
				{
					a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D16;
					pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

					if ( !pDevice )
					{
						a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D15_S1;
						pDevice                              = pRenderer->FindDevice( a_rDisplayParams );
					}
				}
			}
		}
	}
	else
	{
		pDevice                              = pRenderer->FindDevice( a_rDisplayParams );
		a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D16;

		if ( !pDevice )
		{
			a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D15_S1;
			pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

			if ( !pRenderer->FindDevice( a_rDisplayParams ) )
			{
				a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D24_X8;
				pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

				if ( !pRenderer->FindDevice( a_rDisplayParams ) )
				{
					a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D32;
					pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

					if ( !pRenderer->FindDevice( a_rDisplayParams ) )
					{
						a_rDisplayParams.eDepthStencilFormat = TRenderInterface::DEPTHSTENCILFORMAT_D24_S8;
						pDevice                              = pRenderer->FindDevice( a_rDisplayParams );
					}
				}
			}
		}
	}

	return pDevice;
}

// $Barnyard: FUNCTION 0060b6f0
void ARenderer::RenderGUI()
{
	auto pRender   = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	auto pViewport = g_pGUISystem->GetRenderObject( 0 )->GetViewport();

	auto pOldContext = pRender->SetCurrentRenderContext( pViewport->GetRenderContext() );

	// The GUI is rendered with a white ambient colour
	TVector4 vOldAmbientColour = pViewport->GetRenderContext()->GetAmbientColour();
	pViewport->GetRenderContext()->SetAmbientColour( TVector4( 1.0f, 1.0f, 1.0f, 0.0f ) );

	pViewport->Begin();

	// Render UI that is not attached to context
	AGUI2::GetContext()->GetRenderer()->BeginScene();
	m_RenderGUIEmitter.Throw( 0 );
	AGUI2::GetContext()->GetRenderer()->EndScene();

	// Render UI that is attached to the context
	AGUI2::GetContext()->Render();

	pViewport->End();

	pRender->FlushShaders();
	pViewport->GetRenderContext()->SetAmbientColour( vOldAmbientColour );
	pRender->SetCurrentRenderContext( pOldContext );
}

// $Barnyard: FUNCTION 0060aff0
static TBOOL OnRenderDeviceLost()
{
	if ( ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome() )
	{
		return TTRUE;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	return TTRUE;
#endif
}

// $Barnyard: FUNCTION 0060b100
// $Barnyard: FUNCTION 0060c5c0
ARenderer::~ARenderer()
{
	TRenderInterface* pRender = TRenderInterface::GetSingleton();
	pRender->FlushDyingResources();
	pRender->FlushDyingResources();
	pRender->FlushDyingResources();
	pRender->Destroy();

	if ( TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton() )
		delete pRenderInterface;
}

// $Barnyard: FUNCTION 0060b010
static TBOOL OnRenderDeviceFound()
{
	if ( ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome() )
	{
		pSkyDome->SetDirty();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0060c7c0
TBOOL ARenderer::OnCreate()
{
	TOrderTable::CreateStaticData( 2000, 4000 );
	const TBOOL bCreatedTRender = CreateTRender();

	TASSERT( bCreatedTRender );

	if ( bCreatedTRender )
	{
		( new ALineShaderHAL() )->Create();
		( new AWorldShaderHAL() )->Create();
		( new ASkinShaderHAL() )->Create();
		( new AGrassShaderHAL() )->Create();
		( new AStaticInstanceShaderHAL() )->Create();
		( new ASysShaderHAL() )->Create();
		CreateMainViewport();

		AModelLoader::CreateSingleton();
		AMaterialLibraryManager::CreateSingleton();
		AKeyFrameLibraryManager::CreateSingleton();
		AModelLoader::GetSingleton()->InitialiseStatic();
		AGlowViewport::CreateSingleton( TLightIDList::MAX_NUM_LIGHTS * SPLITSCREEN_MAX_NUM_PLAYERS );
		// NOTE: the original creates the rain renderer here and keeps it at +0x80,
		// ARainRenderer is a singleton created by AGameSystemManager::OnCreate instead

		m_pSkyDome = new ASkyDome();
		m_pSkyDome->Create( 180.0f, 75.0, 14, 6 );

		new ASunFlareEffect();

		// Set default fog settings
		TRenderContext* pRenderContext = m_pViewport->GetRenderContext();
		pRenderContext->SetFogColor(
		    TVector4(
		        0.5568628f,
		        0.64705884f,
		        0.7529412f,
		        0.0f
		    )
		);
		pRenderContext->SetFogDistance( 60.0f, 160.0f );
		pRenderContext->EnableFog( TTRUE );

		g_pRenderD3D->SetDeviceLostCallback( OnRenderDeviceLost );
		g_pRenderD3D->SetDeviceFoundCallback( OnRenderDeviceFound );

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
		ADebugOverlay::CreateSingleton();
		AWorldDebugDraw::CreateSingleton();
#endif
	}

	return bCreatedTRender;
}

// Invalidates and destroys the shader if it's created
template <class T>
static void DestroyShader()
{
	if ( T::IsSingletonCreated() )
	{
		T* pShader = T::GetSingleton();
		pShader->Invalidate();
		pShader->OnDestroy();
		delete pShader;
	}
}

// $Barnyard: FUNCTION 0060b1d0
void ARenderer::OnDestroy()
{
#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
	if ( ADebugOverlay::IsSingletonCreated() )
		ADebugOverlay::DestroySingleton();

	if ( AWorldDebugDraw::IsSingletonCreated() )
		AWorldDebugDraw::DestroySingleton();
#endif

	if ( m_pSkyDome )
		delete m_pSkyDome;

	// NOTE: the original destroys the weather manager (+0x80) here,
	// ARainRenderer is a singleton destroyed by AGameSystemManager instead

	if ( ASunFlareEffect::IsSingletonCreated() )
		delete ASunFlareEffect::GetSingleton();

	AModelLoader::DestroyStatic();

	if ( AModelLoader::IsSingletonCreated() )
		delete AModelLoader::GetSingleton();

	if ( AMaterialLibraryManager::IsSingletonCreated() )
		delete AMaterialLibraryManager::GetSingleton();

	if ( AKeyFrameLibraryManager::IsSingletonCreated() )
		delete AKeyFrameLibraryManager::GetSingleton();

	if ( AGlowViewport::IsSingletonCreated() )
		delete AGlowViewport::GetSingleton();

	if ( m_pViewport )
		delete m_pViewport;

	if ( m_pCameraObject )
		delete m_pCameraObject;

	if ( m_pHALViewport1 )
		delete m_pHALViewport1;

	if ( m_pHALViewport2 )
		delete m_pHALViewport2;

	DestroyShader<AGrassShader>();
	DestroyShader<AStaticInstanceShader>();
	DestroyShader<ASysShader>();
	DestroyShader<ALineShader>();
	DestroyShader<AWorldShader>();
	DestroyShader<ASkinShader>();

	TTask::OnDestroy();
}

// $Barnyard: FUNCTION 0060acc0
void ARenderer::UpdateMainCamera( const Toshi::TMatrix44& a_rTransformMatrix, const ACamera* a_pCamera )
{
	m_pCameraObject->SetProjectionCentreX( a_pCamera->m_fProjectionCentreX );
	m_pCameraObject->SetProjectionCentreY( a_pCamera->m_fProjectionCentreY );
	m_pCameraObject->SetFOV( a_pCamera->m_fFOV );
	m_pCameraObject->GetTransformObject().SetMatrix( a_rTransformMatrix );
	m_pCameraObject->SetFar( m_fFar );
	m_pCameraObject->SetNear( m_fNear );
}

// $Barnyard: FUNCTION 0060b370
void ARenderer::RenderMainScene( TFLOAT a_fDeltaTime )
{
	if ( !ARootTask::GetSingleton()->IsTransitioning() )
	{
		m_pViewport->AllowDepthClear( TTRUE );

		auto  pRender         = TRenderD3DInterface::Interface();
		auto  pCameraObject   = m_pCameraObject;
		auto  pViewport       = m_pViewport;
		auto  pOldContext     = pRender->SetCurrentRenderContext( pViewport->GetRenderContext() );
		auto& rTransformStack = pRender->GetTransforms();

		rTransformStack.Reset();
		rTransformStack.PushNull().Identity();

		pViewport->Begin();
		pCameraObject->Render();

		if ( !ARootTask::GetSingleton()->IsPaused() )
		{
			TBOOL bStartFlag;
			m_AnimationUpdateStartEmitter.Throw( &bStartFlag );

			g_pModelRepos->Update( a_fDeltaTime );
			AAnimatableObjectManager::GetSingleton()->UpdateAttachedObjects( a_fDeltaTime );

			TBOOL bEndFlag;
			m_AnimationUpdateEndEmitter.Throw( &bEndFlag );
		}

		//-----------------------------------------------------------------------------
		// Main pass
		//-----------------------------------------------------------------------------
		rTransformStack.Push( pViewport->GetRenderContext()->GetWorldViewMatrix() );

		TCameraObject* pOldCameraObject = pViewport->GetRenderContext()->GetCameraObject();
		pViewport->GetRenderContext()->SetCameraObject( pCameraObject );

		// Render the sky dome without fog
		TRenderContext* pContext    = pRender->GetCurrentContext();
		TBOOL           bFogEnabled = pContext->IsFogEnabled();

		pRender->FlushShaders();
		pContext->EnableFog( TFALSE );
		m_pSkyDome->Render1();
		m_pSkyDome->Render2();
		pRender->FlushShaders();
		pContext->EnableFog( bFogEnabled );

		// Model instances without the glow flag
		g_pModelRepos->RenderModels( AModelRepos::RENDERMASK_NORMAL );

		// Terrain, gates, trees, instances and regrowth
		if ( ATerrainInterface::GetSingleton() )
		{
			ATerrainInterface::GetSingleton()->Render();

			if ( AGateManager::IsSingletonCreated() )
				AGateManager::GetSingleton()->Render();

			ATreeManager::GetSingleton()->Render();
			AInstanceManager::GetSingleton()->Render();

			ARegrowthManager::GetSingleton()->Render();
		}

		// NOTE: retail keeps the weather manager at ARenderer+0x80, here it's a singleton created by AGameSystemManager
		if ( ARainRenderer::IsSingletonCreated() )
			ARainRenderer::GetSingleton()->Render();

		ASunFlareEffect::GetSingleton()->Render();

		pRender->GetCurrentContext()->SetModelViewMatrix( pRender->GetCurrentContext()->GetWorldViewMatrix() );

		AParticleEffectManager::GetSingleton()->GetParticleSystem()->Render();

		AGameStateController::GetSingleton()->RenderStates( a_fDeltaTime );

		if ( AHintManager::IsSingletonCreated() )
			AHintManager::GetSingleton()->Render( a_fDeltaTime );

		// Glowing model instances
		pRender->FlushShaders();
		g_pModelRepos->RenderModels( AModelRepos::RENDERMASK_GLOW );

		AGameStateController::GetSingleton()->PostRenderStates( a_fDeltaTime );

		if ( AHintManager::IsSingletonCreated() )
			AHintManager::GetSingleton()->PostRender( a_fDeltaTime );

		if ( ATerrainInterface::GetSingleton() )
			ATerrainInterface::GetSingleton()->RenderGlowModels();

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
		// Development: the world debug drawing, with the depth of the scene
		if ( AWorldDebugDraw::IsSingletonCreated() )
		{
			pRender->FlushShaders();
			AWorldDebugDraw::GetSingleton()->Render();
		}
#endif

		pViewport->GetRenderContext()->SetCameraObject( pOldCameraObject );
		rTransformStack.Pop();

		pRender->FlushShaders();
		pViewport->End();

		m_pHALViewport2->Begin();
		m_pHALViewport2->End();

		//-----------------------------------------------------------------------------
		// Overlay pass (keeps the colour and the depth of the main pass)
		//-----------------------------------------------------------------------------
		pViewport->AllowBackgroundClear( TFALSE );
		pViewport->AllowDepthClear( TFALSE );
		pViewport->Begin();
		pCameraObject->Render();

		rTransformStack.Push( pViewport->GetRenderContext()->GetWorldViewMatrix() );

		pOldCameraObject = pViewport->GetRenderContext()->GetCameraObject();
		pViewport->GetRenderContext()->SetCameraObject( pCameraObject );

		AGameStateController::GetSingleton()->RenderStatesOverlay( a_fDeltaTime );

		if ( AHintManager::IsSingletonCreated() )
			AHintManager::GetSingleton()->RenderOverlay( a_fDeltaTime );

		if ( AAnimalPopulationManager::IsSingletonCreated() )
		{
			AAnimalPopulationManager::GetSingleton()->RenderMarkers( TFALSE );
			AAnimalPopulationManager::GetSingleton()->RemoveTemporaryMarkers();
		}

		if ( AReplayManager::IsSingletonCreated() )
			AReplayManager::GetSingleton()->Render( TFALSE );

		pViewport->GetRenderContext()->SetCameraObject( pOldCameraObject );
		rTransformStack.Pop();

		pViewport->End();
		pViewport->AllowBackgroundClear( TTRUE );
		pViewport->AllowDepthClear( TTRUE );

		pRender->SetCurrentRenderContext( pOldContext );
	}
}

// $Barnyard: FUNCTION 0060c5e0
TBOOL ARenderer::OnUpdate( TFLOAT a_fDeltaTime )
{
	TRenderD3DInterface* pRender = TRenderD3DInterface::Interface();

	if ( g_oTheApp.IsDestroyed() )
		return TTRUE;

	TBOOL bMoviePlaying = AMoviePlayer::IsSingletonCreated() && AMoviePlayer::GetSingleton()->IsMoviePlaying();

	pRender->Update( a_fDeltaTime );

	if ( g_oSystemManager.IsPaused() )
		return TFALSE;

	// Update transforms of glow objects
	AGlowViewport::GetSingleton()->Update();

	ARootTask* pRootTask = ARootTask::GetSingleton();
	if ( pRootTask->IsGameSystemCreated() )
	{
		// Updates the units that are updated externally (AAnimatedUnit, AStaticUnit)
		if ( AUnitManager::IsSingletonCreated() )
			AUnitManager::GetSingleton()->Update( a_fDeltaTime );

		if ( pRootTask->IsRenderWorld() )
		{
			m_pSkyDome->Update( a_fDeltaTime );

			// NOTE: the original keeps the weather manager at ARenderer+0x80, here it's a singleton
			if ( ARainRenderer::IsSingletonCreated() )
				ARainRenderer::GetSingleton()->Update( a_fDeltaTime );
		}

		auto pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

		pRender->BeginScene();

		if ( bMoviePlaying )
		{
			AMoviePlayer::GetSingleton()->Render( a_fDeltaTime );
		}
		else if ( pRootTask->IsRenderWorld() )
		{
			const ACamera* pCamera    = ACameraManager::GetSingleton()->GetCurrentCamera();
			TMatrix44      oCamMatrix = pCamera->GetMatrix();
			oCamMatrix.SetTranslation( pCamera->GetMatrix().GetTranslation() );

			UpdateMainCamera( oCamMatrix, pCamera );
			m_pViewport->AllowDepthClear( TFALSE );

			pRenderContext->EnableFog( TTRUE );
			RenderMainScene( a_fDeltaTime );
		}

		if ( m_bRenderGUI )
			RenderGUI();

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
		if ( ADebugOverlay::IsSingletonCreated() )
			ADebugOverlay::GetSingleton()->Render( a_fDeltaTime );
#endif

		pRender->EndScene();

		UpdateScreenCapture( pRender );
		return TTRUE;
	}
	else
	{
		pRender->BeginScene();

		if ( bMoviePlaying )
			AMoviePlayer::GetSingleton()->Render( a_fDeltaTime );

		if ( m_bRenderGUI )
			RenderGUI();

		pRender->EndScene();
		return TTRUE;
	}
}

// $Barnyard: FUNCTION 0060c120
void ARenderer::UpdateScreenCapture( TRenderInterface* a_pRender )
{
	if ( m_eScreenCaptureStatus == SCREENCAPTURE_REQUESTED )
	{
		m_pScreenCapture = a_pRender->CreateCapture();
		m_pScreenCapture->Create( TRenderCapture::FORMAT_RGBA32, m_uiScreenCaptureWidth, m_uiScreenCaptureHeight );
		m_pScreenCapture->Request();
		m_eScreenCaptureStatus = SCREENCAPTURE_CAPTURING;
	}
	else if ( m_eScreenCaptureStatus == SCREENCAPTURE_CAPTURING )
	{
		if ( m_pScreenCapture->Poll() )
		{
			m_eScreenCaptureStatus = SCREENCAPTURE_READY;

			if ( m_pScreenCaptureBuffer )
				TFree( m_pScreenCaptureBuffer );

			if ( m_szScreenCaptureName )
			{
				m_pScreenCaptureBuffer = TSTATICCAST( TUINT8, m_pScreenCapture->ObtainBuffer() );
				SaveScreenCapture( m_szScreenCaptureName );

				m_pScreenCaptureBuffer = TNULL;
				m_szScreenCaptureName  = TNULL;
				m_eScreenCaptureStatus = SCREENCAPTURE_NONE;
			}
			else
			{
				TUINT uiBufferSize     = 4 * m_uiScreenCaptureWidth * m_uiScreenCaptureHeight;
				m_pScreenCaptureBuffer = TSTATICCAST( TUINT8, TMalloc( uiBufferSize ) );

				if ( m_pScreenCaptureBuffer )
				{
					// NOTE: the original prints this when the allocation succeeds
					TWARN( "Could not take screen capture because we're out of memory.\n" );
					TUtil::MemCopy( m_pScreenCaptureBuffer, m_pScreenCapture->ObtainBuffer(), uiBufferSize );
				}
			}

			m_pScreenCapture->Destroy();
			a_pRender->DestroyCapture( m_pScreenCapture );
			m_pScreenCapture = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 00403e30
// Writes 32-bit RGBA pixels to an uncompressed TGA file (bottom-up, BGRA)
static void WriteTGA( const TCHAR* a_szFileName, TUINT16 a_uiWidth, TUINT16 a_uiHeight, const TUINT8* a_pPixels )
{
	TUINT8 aHeader[ 18 ];
	TUtil::MemClear( aHeader, sizeof( aHeader ) );
	aHeader[ 2 ]  = 2;  // Uncompressed true-colour image
	aHeader[ 16 ] = 32; // Bits per pixel

	*TREINTERPRETCAST( TUINT16*, &aHeader[ 12 ] ) = a_uiWidth;
	*TREINTERPRETCAST( TUINT16*, &aHeader[ 14 ] ) = a_uiHeight;

	TFile* pFile = TFile::Create( a_szFileName, TFILEMODE_WRITE | TFILEMODE_CREATENEW );
	pFile->Write( aHeader, sizeof( aHeader ) );

	TUINT8*       pRow     = TSTATICCAST( TUINT8, TMalloc( 4 * a_uiWidth ) );
	const TUINT8* pInPixel = a_pPixels + 4 * a_uiWidth * a_uiHeight - 1;

	// The rows are written from the last one
	for ( TUINT i = a_uiHeight; i != 0; i-- )
	{
		TUINT8* pOutPixel = pRow + 4 * a_uiWidth - 4;

		for ( TUINT j = a_uiWidth; j != 0; j-- )
		{
			pOutPixel[ 3 ] = 0xFF;
			pOutPixel[ 0 ] = pInPixel[ -1 ];
			pOutPixel[ 1 ] = pInPixel[ -2 ];
			pOutPixel[ 2 ] = pInPixel[ -3 ];

			pInPixel -= 4;
			pOutPixel -= 4;
		}

		pFile->Write( pRow, 4 * a_uiWidth );
	}

	TFree( pRow );
	pFile->Destroy();
}

// $Barnyard: FUNCTION 0060bfb0
TBOOL ARenderer::SaveScreenCapture( const TCHAR* a_szName )
{
	if ( m_eScreenCaptureStatus != SCREENCAPTURE_READY )
		return TFALSE;

	TString8 strName( a_szName );
	TINT     iExtension = strName.Find( "." );

	if ( iExtension != -1 )
		strName = strName.Mid( 0, iExtension );

	// Find the first free index
	TString8 strFilePath;
	strFilePath.Format( "Data/Screenshots/%s%d.tga", strName.GetString(), m_iScreenshotIndex );

	for ( TFile* pFile = TFileManager::GetSingleton()->CreateFile( strFilePath, TFILEMODE_READ ); pFile != TNULL; pFile = TFileManager::GetSingleton()->CreateFile( strFilePath, TFILEMODE_READ ) )
	{
		pFile->Destroy();
		strFilePath.Format( "Data/Screenshots/%s%d.tga", strName.GetString(), ++m_iScreenshotIndex );
	}

	WriteTGA( strFilePath, TUINT16( m_uiScreenCaptureWidth ), TUINT16( m_uiScreenCaptureHeight ), m_pScreenCaptureBuffer );
	return TTRUE;
}

// $Barnyard: FUNCTION 0060acb0
void ARenderer::ForceUpdate30FPS()
{
	OnUpdate( 1.0f / 30.0f );
}

// $Barnyard: FUNCTION 0060c210
void ARenderer::CreateMainViewport()
{
	auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	m_pViewport = new TViewport( TTRUE );
	m_pViewport->SetX( 0 );
	m_pViewport->SetY( 0 );
	m_pViewport->SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
	m_pViewport->SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
	m_pViewport->SetMinZ( 0.2f );
	m_pViewport->SetMaxZ( 1.0f );
	m_pViewport->AllowBackgroundClear( TTRUE );
	m_pViewport->AllowDepthClear( TTRUE );
	m_pViewport->SetBackgroundColor( 128, 128, 128, 0 );
	m_pViewport->Enable( TTRUE );
	m_pViewport->EnableDefaultBeginRender( TTRUE );
	m_pViewport->SetMemoryAllocatorBlock( AMemory::GetMemBlock( AMemory::POOL_Viewport ) );

	m_pHALViewport1 = new TViewportHAL( TTRUE );
	m_pHALViewport1->SetX( 0 );
	m_pHALViewport1->SetY( 0 );
	m_pHALViewport1->SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
	m_pHALViewport1->SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
	m_pHALViewport1->SetMinZ( 0.2f );
	m_pHALViewport1->SetMaxZ( 1.0f );
	m_pHALViewport1->AllowDepthClear( TFALSE );
	m_pHALViewport1->AllowBackgroundClear( TTRUE );
	m_pHALViewport1->SetBackgroundColor( 0, 0, 0, 0 );
	m_pHALViewport1->Enable( TTRUE );
	m_pHALViewport1->EnableDefaultBeginRender( TTRUE );
	m_pHALViewport1->SetMemoryAllocatorBlock( AMemory::GetMemBlock( AMemory::POOL_Viewport ) );

	m_pHALViewport2 = new TViewportHAL( TTRUE );
	m_pHALViewport2->SetX( 0 );
	m_pHALViewport2->SetY( 0 );
	m_pHALViewport2->SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
	m_pHALViewport2->SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
	m_pHALViewport2->SetMinZ( 0.2f );
	m_pHALViewport2->SetMaxZ( 1.0f );
	m_pHALViewport2->AllowBackgroundClear( TFALSE );
	m_pHALViewport2->AllowDepthClear( TFALSE );
	m_pHALViewport2->Enable( TTRUE );
	m_pHALViewport2->EnableDefaultBeginRender( TTRUE );
	m_pHALViewport2->SetMemoryAllocatorBlock( AMemory::GetMemBlock( AMemory::POOL_Viewport ) );

	m_pCameraObject = new TCameraObject();
}

// $Barnyard: FUNCTION 0060bf70
void ARenderer::GetScreenRay( const ACamera* a_pCamera, const TVector2& a_rcScreenPos, TVector4& a_rOutPosition, TVector4& a_rOutDirection )
{
	TQuaternion qRotation;
	a_pCamera->GetRotation( qRotation );

	GetScreenRay( a_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), qRotation, a_rcScreenPos, a_rOutPosition, a_rOutDirection );
}

// $Barnyard: FUNCTION 0060adf0
void ARenderer::GetScreenRay( const TVector4& a_rcCameraPos, const TQuaternion& a_rcCameraRot, const TVector2& a_rcScreenPos, TVector4& a_rOutPosition, TVector4& a_rOutDirection )
{
	const TRenderContext::PROJECTIONPARAMS& rcProjParams = m_pViewport->GetRenderContext()->GetProjectionParams();

	// A point on the ray in the camera space
	TVector4 vPoint;
	vPoint.x = ( a_rcScreenPos.x - rcProjParams.m_Centre.x ) / rcProjParams.m_Proj.x * 200.0f;
	vPoint.y = ( a_rcScreenPos.y - rcProjParams.m_Centre.y ) / rcProjParams.m_Proj.y * 200.0f;
	vPoint.z = 200.0f;
	vPoint.w = 1.0f;

	TMatrix44 mCamera;
	mCamera.SetFromQuaternion( a_rcCameraRot );
	mCamera.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcCameraPos;

	TMatrix44::TransformVector( a_rOutDirection, mCamera, vPoint );

	a_rOutPosition = TVector4::VEC_ZERO;
	TMatrix44::TransformVector( a_rOutPosition, mCamera, a_rOutPosition );

	a_rOutDirection.x -= a_rOutPosition.x;
	a_rOutDirection.y -= a_rOutPosition.y;
	a_rOutDirection.z -= a_rOutPosition.z;
	a_rOutDirection.Normalise();
}

// $Barnyard: FUNCTION 0060b960
void ARenderer::WorldToScreen( TVector4& a_rPoint )
{
	TRenderInterface*                       pRender        = TRenderInterface::GetSingleton();
	TRenderContext*                         pRenderContext = m_pViewport->GetRenderContext();
	const TRenderContext::PROJECTIONPARAMS& rcProjParams   = pRenderContext->GetProjectionParams();

	TVector4 vView;
	TMatrix44::TransformVector( vView, pRenderContext->GetWorldViewMatrix(), a_rPoint );

	if ( vView.z >= rcProjParams.m_fNearClip )
	{
		TFLOAT fInvZ = 1.0f / vView.z;
		TFLOAT fX    = vView.x * fInvZ * rcProjParams.m_Proj.x + rcProjParams.m_Centre.x;
		TFLOAT fY    = vView.y * fInvZ * rcProjParams.m_Proj.y + rcProjParams.m_Centre.y;

		TFLOAT fScale = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport()->GetWidth() / TFLOAT( pRender->GetCurrentDisplayParams()->uiWidth );
		a_rPoint.x    = fX * fScale;
		a_rPoint.y    = fScale * fY;
	}
	else
	{
		a_rPoint.x = -100.0f;
		a_rPoint.y = -100.0f;
	}
}

// $Barnyard: FUNCTION 0060ba20
void ARenderer::WorldToGUI( TVector4& a_rPoint )
{
	WorldToScreen( a_rPoint );

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fWidth, fHeight;
	AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );

	a_rPoint.x = a_rPoint.x / ( TFLOAT( pDisplayParams->uiWidth ) / fWidth );
	a_rPoint.y = a_rPoint.y / ( TFLOAT( pDisplayParams->uiHeight ) / fHeight );
}

// $Barnyard: FUNCTION 0060cb30
TBOOL ARenderer::GetScreenPosition( const TVector4& a_rcPosition, TVector2& a_rOutScreenPos )
{
	return GetScreenPosition( a_rcPosition, a_rOutScreenPos, 0.0f, 0.0f, 0.0f, 0.0f );
}

// $Barnyard: FUNCTION 0060ad50
TBOOL ARenderer::ProjectToScreen( ACamera* a_pCamera, const TVector4& a_rcPosition, TVector2& a_rOutScreenPos )
{
	const TRenderContext::PROJECTIONPARAMS& rcParams = m_pViewport->GetRenderContext()->GetProjectionParams();

	TMatrix44 mView = a_pCamera->GetMatrix();
	mView.InvertOrthogonal();

	TVector4 vView;
	TMatrix44::TransformVector( vView, mView, a_rcPosition );

	TFLOAT fInvZ      = 1.0f / vView.z;
	a_rOutScreenPos.x = vView.x * fInvZ * rcParams.m_Proj.x + rcParams.m_Centre.x;
	a_rOutScreenPos.y = vView.y * fInvZ * rcParams.m_Proj.y + rcParams.m_Centre.y;

	return vView.z >= rcParams.m_fNearClip && vView.z <= rcParams.m_fFarClip;
}

// $Barnyard: FUNCTION 0060ba80
TBOOL ARenderer::ProjectToGUI( ACamera* a_pCamera, const TVector4& a_rcPosition, TVector2& a_rOutGUIPos )
{
	TBOOL bInView = ProjectToScreen( a_pCamera, a_rcPosition, a_rOutGUIPos );

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );

	TFLOAT fScaleY = TFLOAT( pDisplayParams->uiHeight ) / fHeight;
	a_rOutGUIPos.x = a_rOutGUIPos.x / ( TFLOAT( pDisplayParams->uiWidth ) / fWidth );
	a_rOutGUIPos.y = a_rOutGUIPos.y / fScaleY;

	return bInView;
}

// $Barnyard: FUNCTION 0060b8d0
void ARenderer::ScreenToGUIViewport( const TVector2& a_rcScreenPos, TVector2& a_rOutPos )
{
	TViewport* pGUIViewport = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport();
	TFLOAT     fGUIWidth    = pGUIViewport->GetWidth();
	TFLOAT     fGUIHeight   = pGUIViewport->GetHeight();

	TRenderContext* pRenderContext = m_pViewport->GetRenderContext();
	TFLOAT          fWidth         = pRenderContext->GetWidth();
	TFLOAT          fHeight        = pRenderContext->GetHeight();

	a_rOutPos.x = a_rcScreenPos.x;
	a_rOutPos.y = a_rcScreenPos.y;
	a_rOutPos.x = a_rOutPos.x / fWidth * fGUIWidth;
	a_rOutPos.y = a_rOutPos.y / fHeight * fGUIHeight;

	TMath::Clip( a_rOutPos.x, 0.0f, fGUIWidth );
	TMath::Clip( a_rOutPos.y, 0.0f, fGUIHeight );
}

// $Barnyard: FUNCTION 0060b830
void ARenderer::GUIViewportToScreen( const TVector2& a_rcGUIPos, TVector2& a_rOutPos )
{
	TViewport* pGUIViewport = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport();
	TFLOAT     fGUIWidth    = pGUIViewport->GetWidth();
	TFLOAT     fGUIHeight   = pGUIViewport->GetHeight();

	TRenderContext* pRenderContext = m_pViewport->GetRenderContext();
	TFLOAT          fWidth         = pRenderContext->GetWidth();
	TFLOAT          fHeight        = pRenderContext->GetHeight();

	a_rOutPos.x = a_rcGUIPos.x;
	a_rOutPos.y = a_rcGUIPos.y;
	a_rOutPos.x = a_rOutPos.x / fGUIWidth * fWidth;
	a_rOutPos.y = a_rOutPos.y / fGUIHeight * fHeight;

	TMath::Clip( a_rOutPos.x, 0.0f, fWidth );
	TMath::Clip( a_rOutPos.y, 0.0f, fHeight );
}

// $Barnyard: FUNCTION 0060b800
void ARenderer::GetGUIViewportSize( TVector2& a_rOutSize )
{
	TViewport* pGUIViewport = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport();
	a_rOutSize.x            = pGUIViewport->GetWidth();
	a_rOutSize.y            = pGUIViewport->GetHeight();
}

// $Barnyard: FUNCTION 0060ad30
void ARenderer::GetMainViewportSize( TVector2& a_rOutSize )
{
	TRenderContext* pRenderContext = m_pViewport->GetRenderContext();
	a_rOutSize.x                   = pRenderContext->GetWidth();
	a_rOutSize.y                   = pRenderContext->GetHeight();
}

// $Barnyard: FUNCTION 0060baf0
TBOOL ARenderer::GetScreenPosition( const TVector4& a_rcPosition, TVector2& a_rOutScreenPos, TFLOAT a_fMarginLeft, TFLOAT a_fMarginRight, TFLOAT a_fMarginTop, TFLOAT a_fMarginBottom )
{
	TRenderContext* pRenderContext = m_pViewport->GetRenderContext();
	TFLOAT          fWidth         = pRenderContext->GetWidth();
	TFLOAT          fHeight        = pRenderContext->GetHeight();

	// NOTE: retail gets the current camera for every vector
	ACamera*         pCamera  = ACameraManager::GetSingleton()->GetCurrentCamera();
	const TMatrix44& rcMatrix = pCamera->GetMatrix();

	TVector4 vecCameraPos = rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TVector4 vecForward( rcMatrix.m_f31, rcMatrix.m_f32, rcMatrix.m_f33, 1.0f );
	TVector4 vecUp( -rcMatrix.m_f21, -rcMatrix.m_f22, -rcMatrix.m_f23, 1.0f );

	TVector4 vecDirection( a_rcPosition.x - vecCameraPos.x, a_rcPosition.y - vecCameraPos.y, a_rcPosition.z - vecCameraPos.z, a_rcPosition.w );
	vecDirection.Normalise();

	if ( vecDirection.MagnitudeSq() < 0.010000001f )
	{
		a_rOutScreenPos.x = -1.0f;
		a_rOutScreenPos.y = -1.0f;
		return TFALSE;
	}

	// The angles of the edges of the screen (moved in by the margins)
	TFLOAT fHalfFOVX   = pCamera->GetFOV() * 0.5f;
	TFLOAT fHalfFOVY   = fHalfFOVX / fWidth * fHeight;
	TFLOAT fHalfWidth  = fWidth * 0.5f;
	TFLOAT fLeft       = ( fHalfWidth - a_fMarginLeft ) * fHalfFOVX / fHalfWidth;
	TFLOAT fRight      = ( fHalfWidth - a_fMarginRight ) * fHalfFOVX / fHalfWidth;
	TFLOAT fHalfHeight = fHeight * 0.5f;
	TFLOAT fTop        = ( fHalfHeight - a_fMarginTop ) * fHalfFOVY / fHalfHeight;
	TFLOAT fBottom     = ( fHalfHeight - a_fMarginBottom ) * fHalfFOVY / fHalfHeight;

	// Split the direction into the vertical part and the horizontal one
	TFLOAT fUpDot = TVector4::DotProduct3( vecUp, vecDirection );

	TVector4 vecVertical;
	vecVertical.Multiply( vecUp, fUpDot );

	TVector4 vecHorizontal( vecDirection.x - vecVertical.x, vecDirection.y - vecVertical.y, vecDirection.z - vecVertical.z, vecDirection.w );

	if ( vecHorizontal.MagnitudeSq() < 0.0001f )
	{
		// Straight above or below the camera
		a_rOutScreenPos.x = fHalfWidth;
		a_rOutScreenPos.y = ( fUpDot > 0.0f ) ? a_fMarginTop : fHeight - a_fMarginBottom;
		return TFALSE;
	}

	vecHorizontal.Normalise();

	// The horizontal angle
	TFLOAT fCos = TVector4::DotProduct3( vecHorizontal, vecForward );
	TMath::Clip( fCos, -1.0f, 1.0f );

	TVector4 vecCross;
	vecCross.CrossProduct( vecHorizontal, vecForward );

	TFLOAT fAngleX = TMath::ACos( fCos ) * TMath::Sign( TVector4::DotProduct3( vecCross, vecUp ) );
	TBOOL  bOnScreenX;

	if ( fAngleX < -fLeft || fAngleX > fRight )
	{
		bOnScreenX        = TFALSE;
		a_rOutScreenPos.x = ( fAngleX >= 0.0f ) ? fWidth - a_fMarginRight : a_fMarginLeft;
	}
	else
	{
		bOnScreenX        = TTRUE;
		a_rOutScreenPos.x = ( fAngleX / fHalfFOVX + 1.0f ) * fWidth * 0.5f;
	}

	// The vertical angle
	TFLOAT fCosY = TVector4::DotProduct3( vecHorizontal, vecDirection );
	TMath::Clip( fCosY, -1.0f, 1.0f );

	TFLOAT fAngleY = TMath::ACos( fCosY ) * TMath::Sign( -fUpDot );
	TBOOL  bOnScreenY;

	if ( fAngleY < -fTop || fAngleY > fBottom )
	{
		bOnScreenY        = TFALSE;
		a_rOutScreenPos.y = ( fAngleY < 0.0f ) ? a_fMarginTop : fHeight - a_fMarginBottom;
	}
	else
	{
		bOnScreenY        = TTRUE;
		a_rOutScreenPos.y = ( fAngleY / fHalfFOVY + 1.0f ) * fHeight * 0.5f;
	}

	return bOnScreenX && bOnScreenY;
}
