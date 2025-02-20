#include "pch.h"
#include "AppBoot.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AModelLoader.h"
#include "Animation/AModelRepos.h"
#include "Terrain/ATerrainInterface.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "Render/ARenderer.h"
#include "Render/AGlowViewport.h"
#include "Cameras/ACameraManager.h"
#include "Input/AInputHandler.h"
#include "Movie/AMoviePlayer.h"

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
#endif // TOSHI_SKU_WINDOWS

#include <resource.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ARenderer );

ARenderer::ARenderer()
    : m_RenderGUIEmitter( this ), m_AnimationUpdateStartEmitter( this ), m_AnimationUpdateEndEmitter( this ), m_pViewport( TNULL ), m_pCameraObject( TNULL ), m_pHALViewport1( TNULL ), m_pHALViewport2( TNULL ), m_bRenderGUI( TTRUE ), m_fNear( 1.0f ), m_fFar( 280.0f )
{
}

TBOOL ARenderer::CreateTRender()
{
#if defined( TOSHI_SKU_WINDOWS ) && defined( TRENDERINTERFACE_DX8 )
	TString8 strRenderInterfaceName = "TRenderD3DInterface";
#endif

	TKernelInterface::GetSingleton()->LoadInterface( strRenderInterfaceName );

	TClass*              pRendererClass = TClass::Find( strRenderInterfaceName );
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

	if ( !m_DisplayModes.IsModeExists( iWidth, iHeight, iColourDepth ) )
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
	displayParams.eDepthStencilFormat = 0;

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

	if ( pRenderer->CreateDisplay( displayParams ) )
	{
		TTODO( "Log info about choosen device" );
		return TTRUE;
	}

	return TFALSE;
}

TBOOL ARenderer::CreateTRenderResources()
{
	TIMPLEMENT();

	auto pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );

	{
		auto pResource = TSTATICCAST(
		    TVertexFactoryResource,
		    pRenderer->CreateResource(
		        TClass::Find( "TVertexFactoryResource" ),
		        "VFSYSVNDUV1",
		        TNULL
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
		        TNULL
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
		        TNULL
		    )
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_VFWORLD );

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams                       = 1;
		vertexFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = 44;
		vertexFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;
		pResource->Create( &vertexFormat, 54050, 0 );
	}

	{
		auto pResource = TSTATICCAST(
		    TIndexFactoryResource,
		    pRenderer->CreateResource(
		        TClass::Find( "TIndexFactoryResource" ),
		        "IFSYS",
		        TNULL
		    )
		);

		pRenderer->SetResourceExplicit( pResource, SYSRESOURCE_IFSYS );

		TIndexFactoryFormat indexFormat;
		indexFormat.m_uiUnk = 2;
		pResource->Create( &indexFormat, 64700, 0 );
	}

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

TRenderAdapter::Mode::Device* ARenderer::FindSuitableDevice( TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder )
{
	auto                          pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	TRenderAdapter::Mode::Device* pDevice   = TNULL;

	if ( a_bReverseOrder )
	{
		a_rDisplayParams.eDepthStencilFormat = 4;
		pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

		if ( !pDevice )
		{
			a_rDisplayParams.eDepthStencilFormat = 2;
			pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

			if ( !pDevice )
			{
				a_rDisplayParams.eDepthStencilFormat = 3;
				pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

				if ( !pDevice )
				{
					a_rDisplayParams.eDepthStencilFormat = 0;
					pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

					if ( !pDevice )
					{
						a_rDisplayParams.eDepthStencilFormat = 1;
						pDevice                              = pRenderer->FindDevice( a_rDisplayParams );
					}
				}
			}
		}
	}
	else
	{
		pDevice                              = pRenderer->FindDevice( a_rDisplayParams );
		a_rDisplayParams.eDepthStencilFormat = 0;

		if ( !pDevice )
		{
			a_rDisplayParams.eDepthStencilFormat = 1;
			pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

			if ( !pRenderer->FindDevice( a_rDisplayParams ) )
			{
				a_rDisplayParams.eDepthStencilFormat = 2;
				pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

				if ( !pRenderer->FindDevice( a_rDisplayParams ) )
				{
					a_rDisplayParams.eDepthStencilFormat = 4;
					pDevice                              = pRenderer->FindDevice( a_rDisplayParams );

					if ( !pRenderer->FindDevice( a_rDisplayParams ) )
					{
						a_rDisplayParams.eDepthStencilFormat = 3;
						pDevice                              = pRenderer->FindDevice( a_rDisplayParams );
					}
				}
			}
		}
	}

	return pDevice;
}

void ARenderer::RenderGUI()
{
	auto pRender   = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	auto pViewport = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport();

	auto pOldContext = pRender->SetCurrentRenderContext( pViewport->GetRenderContext() );
	;

	TTODO( "Save and restore value of 'm_AmbientColor?' when it's figured out" );

	pViewport->Begin();

	// Render UI that is not attached to context
	AGUI2::GetContext()->GetRenderer()->BeginScene();
	m_RenderGUIEmitter.Throw( 0 );
	AGUI2::GetContext()->GetRenderer()->EndScene();

	// Render UI that is attached to the context
	AGUI2::GetContext()->Render();

	pViewport->End();

	pRender->FlushShaders();
	pRender->SetCurrentRenderContext( pOldContext );
}

TBOOL ARenderer::OnCreate()
{
	TIMPLEMENT();

	TOrderTable::CreateStaticData( 2000, 4000 );
	TBOOL bCreatedTRender = CreateTRender();

	TASSERT( bCreatedTRender );

	if ( bCreatedTRender )
	{
		( new AWorldShaderHAL() )->Create();
		( new ASkinShaderHAL() )->Create();
		( new AGrassShaderHAL() )->Create();
		( new ASysShaderHAL() )->Create();
		CreateMainViewport();

		AModelLoader::CreateSingleton();
		AMaterialLibraryManager::CreateSingleton();
		AKeyFrameLibraryManager::CreateSingleton();
		AGlowViewport::CreateSingleton( TLightIDList::MAX_NUM_LIGHTS * SPLITSCREEN_MAX_NUM_PLAYERS );
	}

	return bCreatedTRender;
}

void ARenderer::UpdateMainCamera( const Toshi::TMatrix44& a_rTransformMatrix, const ACamera* a_pCamera )
{
	m_pCameraObject->SetProjectionCentreX( a_pCamera->m_fProjectionCentreX );
	m_pCameraObject->SetProjectionCentreY( a_pCamera->m_fProjectionCentreY );
	m_pCameraObject->SetFOV( a_pCamera->m_fFOV );
	m_pCameraObject->GetTransformObject().SetMatrix( a_rTransformMatrix );
	m_pCameraObject->SetFar( m_fFar );
	m_pCameraObject->SetNear( m_fNear );
}

void ARenderer::RenderMainScene( TFLOAT a_fDeltaTime )
{
	if ( ARootTask::GetSingleton()->ShouldRenderMainScene() )
	{
		auto pViewport        = m_pViewport;
		auto pViewportContext = pViewport->GetRenderContext();
		auto pCameraObject    = m_pCameraObject;

		pViewport->AllowDepthClear( TTRUE );

		auto pRender     = TRenderD3DInterface::Interface();
		auto pOldContext = pRender->SetCurrentRenderContext( pViewportContext );

		auto& rTransformStack = pRender->GetTransforms();
		rTransformStack.Reset();
		rTransformStack.Top().Identity();

		pViewport->Begin();
		pCameraObject->Render();

		if ( !ARootTask::GetSingleton()->IsPaused() )
		{
			TBOOL bFlag1;
			m_AnimationUpdateStartEmitter.Throw( &bFlag1 );

			AModelRepos::GetSingleton()->Update( a_fDeltaTime );
			TTODO( "AAnimatableObjectManager::UpdateAttachedObjects" );

			TBOOL bFlag2;
			m_AnimationUpdateEndEmitter.Throw( &bFlag2 );
		}

		rTransformStack.Push( pViewportContext->GetWorldViewMatrix() );
		pViewportContext->SetCameraObject( pCameraObject );

		pRender->FlushShaders();
		pViewportContext->EnableFog( TTRUE );

		if ( ATerrainInterface::GetSingleton() )
		{
			ATerrainInterface::GetSingleton()->Render();
		}

		pRender->FlushShaders();
		rTransformStack.Pop();
		pViewport->End();

		//m_pHALViewport2->Begin();
		//m_pHALViewport2->End();

		pViewport->AllowBackgroundClear( TTRUE );
		pViewport->AllowDepthClear( TTRUE );

		pRender->SetCurrentRenderContext( pOldContext );
	}
}

TBOOL ARenderer::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();

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

		pRender->EndScene();
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

void ARenderer::ForceUpdate30FPS()
{
	OnUpdate( 1.0f / 30.0f );
}

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
