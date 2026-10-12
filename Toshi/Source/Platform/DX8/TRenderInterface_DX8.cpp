// DirectX 8 Render Interface Implementation
// This file contains the implementation of the DirectX 8 render interface for the Toshi engine

#include "ToshiPCH.h"
#include "TModel_DX8.h"
#include "TRenderCapture_DX8.h"
#include "TRenderContext_DX8.h"
#include "TRenderAdapter_DX8.h"
#include "TRenderInterface_DX8.h"
#include "TTextureFactoryHAL_DX8.h"
#include "TTextureResourceHAL_DX8.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "TDevScreenshot_DX8.h"
#endif

#include "Render/TShader.h"
#include "Input/TInputInterface.h"

#include <dxerr8.h>
#pragma comment( lib, "DxErr8.lib" )

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006c57d0, CreateObject 006c6d60, CreateObjectInPlace 006c6d80
TDEFINE_CLASS( TRenderD3DInterface );

// $Barnyard: FUNCTION 006c58e0
// Tests if the device can begin and end a scene successfully
void TRenderInterface::BeginEndSceneHAL()
{
	auto pHAL = TSTATICCAST( TRenderD3DInterface, this );

	if ( S_OK == pHAL->GetDirect3DDevice()->BeginScene() )
	{
		pHAL->GetDirect3DDevice()->EndScene();
	}
}

// $Barnyard: FUNCTION 006c6880
// Constructor - Initializes DirectX 8 render interface with default values
TRenderD3DInterface::TRenderD3DInterface()
{
	m_pDirect3D                          = TNULL;
	m_pDirectDevice                      = TNULL;
	m_fPixelAspectRatio                  = 1.0f;
	m_AcceleratorTable                   = NULL;
	m_pDevice                            = TNULL;
	m_oDisplayParams.uiWidth             = 640; // Default width
	m_oDisplayParams.uiHeight            = 480; // Default height
	m_oDisplayParams.uiColourDepth       = 32;  // Default color depth
	m_oDisplayParams.eDepthStencilFormat = DEPTHSTENCILFORMAT_D16;
	m_oDisplayParams.bWindowed           = TTRUE;
	m_fBrightness                        = 0.5f; // Default brightness
	m_fSaturate                          = 0.5f; // Default saturation
	m_bExited                            = TFALSE;
	m_bCheckedCapableColourCorrection    = TFALSE;
	m_bCapableColourCorrection           = TFALSE;
	m_bFailed                            = TFALSE;
	m_fnOnDeviceLostCallback             = TNULL;
	m_fnOnDeviceFoundCallback            = TNULL;
	m_fContrast                          = 0.583012f; // Default contrast
	m_fGamma                             = 0.420849f; // Default gamma
	m_bChangedColourSettings             = TTRUE;
	m_bEnableColourCorrection            = TTRUE;
}

// $Barnyard: FUNCTION 006c6da0
// Destructor - Cleans up DirectX resources
TRenderD3DInterface::~TRenderD3DInterface()
{
	Destroy();
}

// $Barnyard: FUNCTION 006c6990
// Creates the display with specified parameters
// Returns true if successful, false otherwise
TBOOL TRenderD3DInterface::CreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	if ( !TRenderInterface::CreateDisplay() )
	{
		OnInitializationFailureDisplay();
		return TFALSE;
	}

	// Find appropriate device for the display parameters
	m_pDevice        = TSTATICCAST( TD3DAdapter::Mode::Device, FindDevice( a_rParams ) );
	m_oDisplayParams = a_rParams;

	if ( m_pDevice )
	{
		auto pDisplayParams = GetCurrentDisplayParams();

		// Get desktop window dimensions
		RECT clientRect;
		GetClientRect( GetDesktopWindow(), &clientRect );

		// Handle large displays
		if ( 2000 < clientRect.right )
		{
			clientRect.right /= 2;
		}

		TUINT32 uiWindowPosX = 0;
		TUINT32 uiWindowPosY = 0;

		// Calculate window position for windowed mode
		if ( pDisplayParams->bWindowed )
		{
			auto pMode   = GetCurrentDevice()->GetMode();
			uiWindowPosX = ( clientRect.right - pMode->GetWidth() ) / 2;
			uiWindowPosY = ( clientRect.bottom - pMode->GetHeight() ) / 2;
		}

		// Initialize presentation parameters
		TUtil::MemClear( &m_PresentParams, sizeof( m_PresentParams ) );
		m_PresentParams.Windowed               = pDisplayParams->bWindowed;
		m_PresentParams.BackBufferCount        = 1;
		m_PresentParams.MultiSampleType        = D3DMULTISAMPLE_NONE;
		m_PresentParams.SwapEffect             = D3DSWAPEFFECT_DISCARD;
		m_PresentParams.EnableAutoDepthStencil = TRUE;
		m_PresentParams.hDeviceWindow          = m_Window.GetHWND();
		m_PresentParams.AutoDepthStencilFormat = TD3DAdapter::Mode::Device::DEPTHSTENCILFORMATS[ pDisplayParams->eDepthStencilFormat ];
		m_PresentParams.BackBufferWidth        = pDisplayParams->uiWidth;
		m_PresentParams.BackBufferHeight       = pDisplayParams->uiHeight;

		// Get device information
		auto pDevice        = TSTATICCAST( TD3DAdapter::Mode::Device, GetCurrentDevice() );
		auto pMode          = TSTATICCAST( TD3DAdapter::Mode, pDevice->GetMode() );
		auto pAdapter       = TSTATICCAST( TD3DAdapter, pMode->GetAdapter() );
		auto uiAdapterIndex = pAdapter->GetAdapterIndex();

		// Set back buffer format based on windowed/fullscreen mode
		if ( pDisplayParams->bWindowed )
		{
			m_PresentParams.BackBufferFormat = pMode->GetD3DDisplayMode().Format;
		}
		else
		{
			m_PresentParams.BackBufferFormat = pMode->GetBackBufferFormat( pDisplayParams->uiColourDepth );
		}

		// Create Direct3D device
		HRESULT hRes = m_pDirect3D->CreateDevice(
		    uiAdapterIndex,
		    TD3DAdapter::Mode::Device::DEVICETYPES[ pDevice->GetDeviceIndex() ],
		    m_Window.GetHWND(),
		    pDevice->GetD3DDeviceFlags(),
		    &m_PresentParams,
		    &m_pDirectDevice
		);

		if ( FAILED( hRes ) )
		{
			OnInitializationFailureDevice();
			PrintError( hRes, "Failed to create D3D Device!" );
			return TFALSE;
		}

		// Initialize device states
		SetDeviceDefaultStates();

		// Set window mode
		if ( pDisplayParams->bWindowed )
		{
			m_Window.SetWindowed();
		}
		else
		{
			m_Window.SetFullscreen();
		}

		// Handle multi-monitor setup
		if ( uiAdapterIndex != 0 )
		{
			HMONITOR hMonitor = m_pDirect3D->GetAdapterMonitor( uiAdapterIndex );

			MONITORINFO monitorInfo = { .cbSize = sizeof( monitorInfo ) };
			GetMonitorInfoA( hMonitor, &monitorInfo );

			uiWindowPosX += monitorInfo.rcMonitor.left;
			uiWindowPosY += monitorInfo.rcMonitor.right;
		}

		// Set window position and size
		m_Window.SetPosition( uiWindowPosX, uiWindowPosY, pDisplayParams->uiWidth, pDisplayParams->uiHeight );

		// Get back buffer surface description
		IDirect3DSurface8* pSurface;
		m_pDirectDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurface );
		pSurface->GetDesc( &m_SurfaceDesk );
		pSurface->Release();

		// Set cursor position to center of window
#ifdef BARNYARD_COMMUNITY_PATCH
		// Background test runs leave the user's mouse cursor alone
		if ( !TMSWindow::IsBackgroundMode() )
#endif
			SetCursorPos(
			    uiWindowPosX + pDisplayParams->uiWidth / 2,
			    uiWindowPosY + pDisplayParams->uiHeight / 2
			);

		m_pDirectDevice->ShowCursor( TRUE );

		// Create invalid texture pattern
		TUINT invalidTextureData[ 32 ];
		for ( TINT i = 0; i < 32; i++ )
		{
			invalidTextureData[ i ] = 0xff0fff0f;
		}

		auto pTextureFactory = GetSystemResource<TTextureFactoryHAL>( SYSRESOURCE_TEXTUREFACTORY );
		m_pInvalidTexture    = pTextureFactory->CreateTextureFromMemory( invalidTextureData, sizeof( invalidTextureData ), TTexture::TEXTUREFLAGS_UNK0 | TTexture::TEXTUREFLAGS_LAYOUT_4444, 8, 8 );

		// Enable color correction and mark display as created
		EnableColourCorrection( TTRUE );
		m_bDisplayCreated = TTRUE;

		return TTRUE;
	}

	OnInitializationFailureDisplay();
	return TFALSE;
}

// $Barnyard: FUNCTION 006c6520
TBOOL TRenderD3DInterface::DestroyDisplay()
{
	TASSERT( IsDisplayCreated() );
	FlushDyingResources();

	if ( IsDisplayCreated() )
	{
		if ( m_pDirectDevice )
		{
			m_pDirectDevice->Release();
			m_pDirectDevice = TNULL;
		}

		return TRenderInterface::DestroyDisplay();
	}
	else
	{
		return TTRUE;
	}
}

// $Barnyard: FUNCTION 006c6560
TBOOL TRenderD3DInterface::Update( TFLOAT a_fDeltaTime )
{
	TRenderInterface::Update( a_fDeltaTime );
	m_Window.Update( a_fDeltaTime );

	return !m_bExited;
}

// $Barnyard: FUNCTION 006c6590
TBOOL TRenderD3DInterface::BeginScene()
{
	TASSERT( !IsInScene() );

	if ( TRenderInterface::BeginScene() )
	{
		HRESULT hRes = m_pDirectDevice->BeginScene();

		if ( SUCCEEDED( hRes ) )
		{
			RECT windowRect;
			GetWindowRect( m_Window.GetHWND(), &windowRect );

			D3DVIEWPORT8 viewport = {
				.X      = 0,
				.Y      = 0,
				.Width  = DWORD( windowRect.right - windowRect.left ),
				.Height = DWORD( windowRect.bottom - windowRect.top ),
				.MinZ   = 0.0f,
				.MaxZ   = 1.0f,
			};

			m_pDirectDevice->SetViewport( &viewport );
			UpdateColourSettings();

			m_bInScene = TTRUE;
			m_BeginSceneEmitter.Throw( 0 );
		}
		else
		{
			return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c6fd0
TBOOL TRenderD3DInterface::EndScene()
{
	TASSERT( IsInScene() );

	if ( IsInScene() )
	{
		m_EndSceneEmitter.Throw( TNULL );
		HRESULT hRes = m_pDirectDevice->EndScene();

		if ( SUCCEEDED( hRes ) )
		{
#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
			// Development: "-devshot" saves the finished frame (GUI included) before it's presented
			TDevScreenshot::Update( m_pDirectDevice );
#endif

			HRESULT hPresentRes = m_pDirectDevice->Present( NULL, NULL, NULL, NULL );

			if ( hPresentRes == D3DERR_DEVICELOST )
			{
				HRESULT hCooperativeLevel = m_pDirectDevice->TestCooperativeLevel();

				if ( hCooperativeLevel == D3DERR_DEVICELOST )
				{
					OnD3DDeviceLost();
				}
				else if ( hCooperativeLevel == D3DERR_DEVICENOTRESET )
				{
					OnD3DDeviceFound();
				}
			}

			m_bInScene = TFALSE;
		}
		else
		{
			return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c5950
TRenderAdapter::Mode::Device* TRenderD3DInterface::GetCurrentDevice()
{
	return m_pDevice;
}

// $Barnyard: FUNCTION 006c5960
TRenderInterface::DISPLAYPARAMS* TRenderD3DInterface::GetCurrentDisplayParams()
{
	return &m_oDisplayParams;
}

// $Barnyard: FUNCTION 006c7310
TBOOL TRenderD3DInterface::Create()
{
	return Create( TClassObjectName.GetName() );
}

// $Barnyard: FUNCTION 006be990
void TRenderD3DInterface::FlushShaders()
{
	Interface()->FlushOrderTables();

	for ( auto it = TShader::sm_oShaderList.GetRootShader(); it != TNULL; it = it->GetNextShader() )
	{
		it->Flush();
	}
}

TBOOL TRenderD3DInterface::CreateVertexShader( const DWORD* a_ShaderDeclaration, const DWORD* a_pFunction, DWORD* a_pOutVertexShader )
{
	auto pD3DDevice                       = Interface()->GetDirect3DDevice();
	auto pCurrentDevice                   = Interface()->GetCurrentDevice();
	auto bSupportsHardwareTransformations = pCurrentDevice->SupportsHardwareTransfomations();

	HRESULT hRes = pD3DDevice->CreateVertexShader(
	    a_ShaderDeclaration,
	    a_pFunction,
	    a_pOutVertexShader,
	    !bSupportsHardwareTransformations ? D3DCREATE_PUREDEVICE : 0
	);

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failure to create the vertex shader!" );
		return TFALSE;
	}

	return TTRUE;
}

void TRenderD3DInterface::DestroyVertexShader( DWORD a_hVertexShader )
{
	auto    pD3DDevice = Interface()->GetDirect3DDevice();
	HRESULT hRes       = pD3DDevice->DeleteVertexShader( a_hVertexShader );
	TASSERT( SUCCEEDED( hRes ) );
}

TBOOL TRenderD3DInterface::CreatePixelShader( const DWORD* a_pFunction, DWORD* a_pOutPixelShader )
{
	auto pD3DDevice                       = Interface()->GetDirect3DDevice();
	auto pCurrentDevice                   = Interface()->GetCurrentDevice();
	auto bSupportsHardwareTransformations = pCurrentDevice->SupportsHardwareTransfomations();

	HRESULT hRes = pD3DDevice->CreatePixelShader( a_pFunction, a_pOutPixelShader );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failure to create the pixel shader!" );
		return TFALSE;
	}

	return TTRUE;
}

void TRenderD3DInterface::DestroyPixelShader( DWORD a_hPixelShader )
{
	auto    pD3DDevice = Interface()->GetDirect3DDevice();
	HRESULT hRes       = pD3DDevice->DeletePixelShader( a_hPixelShader );
	TASSERT( SUCCEEDED( hRes ) );
}

// $Barnyard: FUNCTION 006c5970
void TRenderD3DInterface::PrintError( TINT32 a_eError, const TCHAR* a_szInfo )
{
	if ( !a_szInfo )
	{
		a_szInfo = "D3D Error";
	}

	const TCHAR* errString      = GetErrorString( a_eError );
	const TCHAR* errDescription = GetErrorDescription( a_eError );

	TString8 string = TString8::VarArgs( "> %s: D3D Error [%s] : Description [%s] !\n", a_szInfo, errString, errDescription );
	OutputDebugStringA( string );

	TASSERT( TFALSE, string.GetString() );
}

// $Barnyard: FUNCTION 006c64a0
TBOOL TRenderD3DInterface::Destroy()
{
	// NOTE: no IsCreated() assert, the destructor calls this again after ARenderer destroyed the renderer
	if ( IsCreated() )
	{
		FlushDyingResources();
		DestroyAllShaderResources();
		FlushDyingResources();

		DestroySystemResources();
		DestroyDisplay();

		m_Window.UnregisterWindowClass();
		DestroyAccelTable();

		if ( m_pDirect3D )
		{
			m_pDirect3D->Release();
			m_pDirect3D = TNULL;
		}

		return TRenderInterface::Destroy();
	}
	else
	{
		return TTRUE;
	}
}

// $Barnyard: FUNCTION 006c5f10
void TRenderD3DInterface::RenderIndexPrimitive( TINT a_ePrimitiveType, TINT a_iNumVertices, TINT a_iNumPrimitives, TINT a_pIndices, TINT a_pVertices, TINT a_iVertexStride )
{
	// Position and colour
	static const DWORD s_aDeclaration[] = {
		D3DVSD_STREAM( 0 ),
		D3DVSD_REG( D3DVSDE_POSITION, D3DVSDT_FLOAT3 ),
		D3DVSD_REG( D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR ),
		D3DVSD_END()
	};

	static const D3DPRIMITIVETYPE s_aPrimitiveTypes[] = {
		D3DPT_POINTLIST,
		D3DPT_LINELIST,
		D3DPT_LINESTRIP,
		D3DPT_TRIANGLELIST,
		D3DPT_TRIANGLESTRIP,
		D3DPT_TRIANGLEFAN
	};

	static DWORD s_hVertexShader = DWORD( -1 ); // 0x0077cfd0

	if ( s_hVertexShader == DWORD( -1 ) )
	{
		TBOOL bHardwareTransformations = GetCurrentDevice()->SupportsHardwareTransfomations();
		m_pDirectDevice->CreateVertexShader( s_aDeclaration, TNULL, &s_hVertexShader, bHardwareTransformations ? 0 : D3DUSAGE_SOFTWAREPROCESSING );

		if ( s_hVertexShader == DWORD( -1 ) )
			return;
	}

	TRenderContextD3D* pContext = TRenderContextD3D::Upcast( GetCurrentContext() );

	static TMatrix44 s_oIdentity = TMatrix44::IDENTITY;

	m_pDirectDevice->SetTransform( D3DTS_VIEW, TREINTERPRETCAST( const D3DMATRIX*, &s_oIdentity ) );
	m_pDirectDevice->SetTransform( D3DTS_PROJECTION, TREINTERPRETCAST( const D3DMATRIX*, &pContext->GetProjectionMatrix() ) );
	m_pDirectDevice->SetTransform( D3DTS_WORLD, TREINTERPRETCAST( const D3DMATRIX*, &pContext->GetModelViewMatrix() ) );
	m_pDirectDevice->SetVertexShader( s_hVertexShader );
	m_pDirectDevice->DrawIndexedPrimitiveUP(
	    s_aPrimitiveTypes[ a_ePrimitiveType ],
	    0,
	    a_iNumVertices,
	    a_iNumPrimitives,
	    TREINTERPRETCAST( const void*, a_pIndices ),
	    D3DFMT_INDEX16,
	    TREINTERPRETCAST( const void*, a_pVertices ),
	    TUINT16( a_iVertexStride )
	);
}

// $Barnyard: FUNCTION 006c57e0
TFLOAT TRenderD3DInterface::GetPixelAspectRatio()
{
	return m_fPixelAspectRatio;
}

// $Barnyard: FUNCTION 006c57f0
TBOOL TRenderD3DInterface::SetPixelAspectRatio( TFLOAT a_fPixelAspectRatio )
{
	m_fPixelAspectRatio = a_fPixelAspectRatio;
	return TTRUE;
}

// $Barnyard: FUNCTION 006c67c0
void TRenderD3DInterface::FlushOrderTables()
{
	TASSERT( TTRUE == IsInScene() );

	for ( auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++ )
	{
		it->Flush();
	}
}

// $Barnyard: FUNCTION 006c6030
TRenderContext* TRenderD3DInterface::CreateRenderContext()
{
	return new TRenderContextD3D( this );
}

// $Barnyard: FUNCTION 006c6240
TRenderCapture* TRenderD3DInterface::CreateCapture()
{
	return new TRenderCaptureD3D();
}

// $Barnyard: FUNCTION 006c6260
void TRenderD3DInterface::DestroyCapture( TRenderCapture* a_pRenderCapture )
{
	a_pRenderCapture->Destroy();
	delete a_pRenderCapture;
}

// $Barnyard: FUNCTION 006c6230
void* TRenderD3DInterface::CreateUnknown( const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	return TNULL;
}

// $Barnyard: FUNCTION 006c6280
TModel* TRenderD3DInterface::CreateModelTMD( TTMD* a_pTMD, TBOOL a_bLoad )
{
	auto pModel = new TModelHAL();

	if ( pModel )
	{
		if ( !pModel->Create( a_pTMD, a_bLoad ) )
		{
			pModel->Delete();
			return TNULL;
		}
	}

	return pModel;
}

// $Barnyard: FUNCTION 006c62d0
TModel* TRenderD3DInterface::CreateModelTMDFile( const TCHAR* a_szFilePath, TBOOL a_bLoad )
{
	auto pModel = new TModelHAL();

	if ( pModel )
	{
		if ( !pModel->Create( a_szFilePath, a_bLoad ) )
		{
			pModel->Delete();
			return TNULL;
		}
	}

	return pModel;
}

// $Barnyard: FUNCTION 006c6320
TModel* TRenderD3DInterface::CreateModelTRB( const TCHAR* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	auto pModel = new TModelHAL();

	if ( pModel )
	{
		if ( !pModel->Create( a_szFilePath, a_bLoad, a_pAssetTRB, a_ui8FileNameLen ) )
		{
			pModel->Delete();
			return TNULL;
		}
	}

	return pModel;
}

// $Barnyard: FUNCTION 006c6420
TDebugText* TRenderD3DInterface::CreateDebugText()
{
	m_pDebugText = InitDebugText( this, 0 );
	return m_pDebugText;
}

// $Barnyard: FUNCTION 006c6440
void TRenderD3DInterface::DestroyDebugText()
{
	if ( m_pDebugText )
	{
		m_pDebugText->Destroy();

#ifdef BARNYARD_COMMUNITY_PATCH
		// Retail doesn't clear the pointer
		m_pDebugText = TNULL;
#endif
	}
}

// $Barnyard: FUNCTION 006c6f80
TBOOL TRenderD3DInterface::RecreateDisplay( const DISPLAYPARAMS& a_rDisplayParams )
{
	if ( IsCreated() )
	{
		if ( IsDisplayCreated() )
		{
			OnD3DDeviceLost();
			DestroyDisplay();

			if ( CreateDisplay( a_rDisplayParams ) )
			{
				OnD3DDeviceFound();
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006c5a00
void TRenderD3DInterface::SetContrast( TFLOAT a_fConstrast )
{
	TMath::Clip( a_fConstrast, 0.0f, 1.0f );
	m_fContrast              = a_fConstrast;
	m_bChangedColourSettings = TTRUE;
}

// $Barnyard: FUNCTION 006c5a30
void TRenderD3DInterface::SetBrightness( TFLOAT a_fBrightness )
{
	TMath::Clip( a_fBrightness, 0.0f, 1.0f );
	m_fBrightness            = a_fBrightness;
	m_bChangedColourSettings = TTRUE;
}

// $Barnyard: FUNCTION 006c5a60
void TRenderD3DInterface::SetGamma( TFLOAT a_fGamma )
{
	TMath::Clip( a_fGamma, 0.0f, 1.0f );
	m_fGamma                 = a_fGamma;
	m_bChangedColourSettings = TTRUE;
}

// $Barnyard: FUNCTION 006c5a90
void TRenderD3DInterface::SetSaturate( TFLOAT a_fSaturate )
{
	TMath::Clip( a_fSaturate, 0.0f, 1.0f );
	m_fSaturate              = a_fSaturate;
	m_bChangedColourSettings = TTRUE;
}

// $Barnyard: FUNCTION 006c6950
TFLOAT TRenderD3DInterface::GetContrast() const
{
	return m_fContrast;
}

// $Barnyard: FUNCTION 006c6960
TFLOAT TRenderD3DInterface::GetBrightness() const
{
	return m_fBrightness;
}

// $Barnyard: FUNCTION 006c6970
TFLOAT TRenderD3DInterface::GetGamma() const
{
	return m_fGamma;
}

// $Barnyard: FUNCTION 006c6980
TFLOAT TRenderD3DInterface::GetSaturate() const
{
	return m_fSaturate;
}

// $Barnyard: FUNCTION 006c66b0
void TRenderD3DInterface::UpdateColourSettings()
{
	if ( IsColourCorrection() )
	{
		GetCurrentColourRamp();
		m_pDirectDevice->SetGammaRamp( 0, &m_GammaRamp );
	}
}

// $Barnyard: FUNCTION 006c5e70
TBOOL TRenderD3DInterface::IsCapableColourCorrection()
{
	if ( !m_bCheckedCapableColourCorrection )
	{
		D3DCAPS8 caps;
		HRESULT  hRes              = m_pDirectDevice->GetDeviceCaps( &caps );
		m_bCapableColourCorrection = SUCCEEDED( hRes ) && HASANYFLAG( caps.Caps2, D3DCAPS2_FULLSCREENGAMMA );
	}

	return m_bCapableColourCorrection;
}

// $Barnyard: FUNCTION 006c5ec0
void TRenderD3DInterface::EnableColourCorrection( TBOOL a_bEnable )
{
	m_bEnableColourCorrection = a_bEnable && IsCapableColourCorrection();
}

// $Barnyard: FUNCTION 006c5ef0
void TRenderD3DInterface::ForceEnableColourCorrection( TBOOL a_bEnable )
{
	m_bEnableColourCorrection = a_bEnable;
}

// $Barnyard: FUNCTION 006c5f00
TBOOL TRenderD3DInterface::IsColourCorrection()
{
	return m_bEnableColourCorrection;
}

// $Barnyard: FUNCTION 006c6de0
void TRenderD3DInterface::OnD3DDeviceLost()
{
	g_oSystemManager.Pause( TTRUE );

	if ( TInputInterface* pInputInterface = TInputInterface::GetSingleton() )
	{
		pInputInterface->UnacquireAll();
		pInputInterface->FlushAll();
		pInputInterface->StopAllRepeats();
	}

	if ( m_bFailed )
		return;

	m_bFailed = TTRUE;

	// Release everything that lives in the default pool
	// $Barnyard: FUNCTION 006c5910
	TResource::Recurse( []( TResource* a_pResource, void* a_pUserData ) {
		a_pResource->TryInvalidate();
		return TTRUE;
	},
	                    m_Resources.GetRoot()->Child(), TTRUE, TNULL );

	if ( m_pDebugText )
		TSTATICCAST( TDebugD3DText, m_pDebugText )->OnDeviceLost();

	// $Barnyard: FUNCTION 006c5920
	for ( TShader* pShader = TShader::sm_oShaderList.GetRootShader(); pShader != TNULL; pShader = pShader->GetNextShader() )
		pShader->TryInvalidate();

	if ( m_fnOnDeviceLostCallback )
		m_fnOnDeviceLostCallback();

	FlushDyingResources();
	g_oSystemManager.Pause( TTRUE );
}

// $Barnyard: FUNCTION 006c6e80
void TRenderD3DInterface::OnD3DDeviceFound()
{
	if ( m_bFailed == TTRUE )
	{
		HRESULT hRes = m_pDirectDevice->Reset( &m_PresentParams );

		if ( FAILED( hRes ) )
			PrintError( hRes, "D3D Reset FAILED!!!" );

		if ( hRes == D3D_OK )
		{
			// $Barnyard: FUNCTION 006c5930
			TResource::Recurse( []( TResource* a_pResource, void* a_pUserData ) {
				a_pResource->TryValidate();
				return TTRUE;
			},
			                    m_Resources.GetRoot()->Child(), TTRUE, TNULL );

			if ( m_pDebugText )
				TSTATICCAST( TDebugD3DText, m_pDebugText )->OnDeviceFound();

			// $Barnyard: FUNCTION 006c5940
			for ( TShader* pShader = TShader::sm_oShaderList.GetRootShader(); pShader != TNULL; pShader = pShader->GetNextShader() )
				pShader->TryValidate();

			if ( m_fnOnDeviceFoundCallback )
				m_fnOnDeviceFoundCallback();

			SetDeviceDefaultStates();
			g_oSystemManager.Pause( TFALSE );
			m_bFailed = TFALSE;
		}
	}

	if ( TInputInterface* pInputInterface = TInputInterface::GetSingleton() )
	{
		pInputInterface->UnacquireAll();
		pInputInterface->FlushAll();
		pInputInterface->StopAllRepeats();
	}

	g_oSystemManager.Pause( TFALSE );
}

// Applies the gamma setting (0.5 is neutral) to a_fValue
// $Barnyard: FUNCTION 006c5ac0
static TFLOAT ApplyGamma( TFLOAT a_fGamma, TFLOAT a_fValue )
{
	if ( a_fGamma > 0.5f )
	{
		TFLOAT fAmount = ( a_fGamma - 0.5f ) + ( a_fGamma - 0.5f );
		return TMath::Pow( a_fValue, 1.0f / ( 1.0f - fAmount + fAmount * 6.0f ) );
	}

	if ( a_fGamma < 0.5f )
	{
		TFLOAT fAmount = ( a_fGamma - 0.5f ) * -2.0f;
		return TMath::Pow( a_fValue, 1.0f / ( 1.0f - fAmount + fAmount * 0.5f ) );
	}

	return a_fValue;
}

// Converts a float to an integer rounding towards zero
// $Barnyard: FUNCTION 006b6970
static TINT FloatToIntTruncate( TFLOAT a_fValue )
{
	return TINT( a_fValue );
}

// $Barnyard: FUNCTION 006c5b60
void TRenderD3DInterface::GetCurrentColourRamp()
{
	if ( m_bChangedColourSettings != TTRUE )
		return;

	m_bChangedColourSettings = TFALSE;

	TFLOAT aRamps[ 3 ][ 256 ];

	for ( TUINT i = 0; i < 256; i++ )
	{
		TFLOAT fValue    = i * ( 1.0f / 255.0f );
		aRamps[ 0 ][ i ] = fValue;
		aRamps[ 1 ][ i ] = fValue;
		aRamps[ 2 ][ i ] = fValue;
	}

	// Gamma
	for ( TUINT i = 0; i < 256; i++ )
	{
		for ( TINT k = 0; k < 3; k++ )
			aRamps[ k ][ i ] = ApplyGamma( m_fGamma, aRamps[ k ][ i ] );
	}

	// Contrast (0.5 is neutral)
	for ( TUINT i = 0; i < 256; i++ )
	{
		for ( TINT k = 0; k < 3; k++ )
		{
			TFLOAT fValue = aRamps[ k ][ i ];

			if ( m_fContrast < 0.5f )
				aRamps[ k ][ i ] = m_fContrast * ( fValue - 0.5f ) + m_fContrast * ( fValue - 0.5f ) + 0.5f;
			else if ( m_fContrast > 0.5f )
				aRamps[ k ][ i ] = TMath::Gain( m_fContrast, fValue );
		}
	}

	// Brightness (0.5 is neutral)
	for ( TUINT i = 0; i < 256; i++ )
	{
		for ( TINT k = 0; k < 3; k++ )
		{
			TFLOAT fValue = aRamps[ k ][ i ] + m_fBrightness - 0.5f;
			TMath::Clip( fValue, 0.0f, 1.0f );
			aRamps[ k ][ i ] = fValue;
		}
	}

	for ( TUINT i = 0; i < 256; i++ )
	{
		m_GammaRamp.red[ i ]   = WORD( FloatToIntTruncate( aRamps[ 0 ][ i ] * 65535.0f ) );
		m_GammaRamp.green[ i ] = WORD( FloatToIntTruncate( aRamps[ 1 ][ i ] * 65535.0f ) );
		m_GammaRamp.blue[ i ]  = WORD( FloatToIntTruncate( aRamps[ 2 ][ i ] * 65535.0f ) );
	}
}

// $Barnyard: FUNCTION 006d9fd0
TDebugD3DText* TRenderD3DInterface::InitDebugText( TRenderD3DInterface* a_pRenderer, TINT a_iBufferSize )
{
	if ( a_iBufferSize == 0 )
		a_iBufferSize = 5120;

	// The text buffer follows the object
	void* pMem = TMalloc( a_iBufferSize + 28 );

	if ( !pMem )
		return TNULL;

	TDebugD3DText* pDebugText = new ( pMem ) TDebugD3DText();
	pDebugText->m_pRenderer   = a_pRenderer;
	pDebugText->m_iBufferSize = a_iBufferSize;
	pDebugText->Reset();

	pDebugText->m_pFont = new CD3DFont( "Arial", 10, CD3DFont::D3DFONT_BOLD );
	pDebugText->m_pFont->InitDeviceObjects( a_pRenderer->m_pDirectDevice );
	pDebugText->m_pFont->RestoreDeviceObjects();

	return pDebugText;
}

// $Barnyard: FUNCTION 006c66e0
// Sets default Direct3D device states
void TRenderD3DInterface::SetDeviceDefaultStates()
{
	// Enable Z-buffer
	m_pDirectDevice->SetRenderState( D3DRS_ZENABLE, 1 );
	// Set culling mode to counter-clockwise
	m_pDirectDevice->SetRenderState( D3DRS_CULLMODE, 2 );
	// Disable lighting
	m_pDirectDevice->SetRenderState( D3DRS_LIGHTING, 0 );
	// Set texture filtering to linear
	m_pDirectDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, 2 );
	m_pDirectDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, 2 );
	m_pDirectDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, 2 );
}

// $Barnyard: FUNCTION 006c6110
// Sets texture addressing mode for a specific texture stage and coordinate
void TRenderD3DInterface::SetTextureAddress( TINT a_iStage, ADDRESSINGMODE a_eAddressing, TEXCOORD a_eTextureCoordinate )
{
	// Map texture addressing type to D3D address mode
	DWORD addressMode;
	switch ( a_eAddressing )
	{
		case ADDRESSINGMODE_WRAP:
			addressMode = D3DTADDRESS_WRAP;
			break;
		case ADDRESSINGMODE_MIRROR:
			addressMode = D3DTADDRESS_MIRROR;
			break;
		case ADDRESSINGMODE_CLAMP:
			addressMode = D3DTADDRESS_CLAMP;
			break;
		case ADDRESSINGMODE_BORDER:
			addressMode = D3DTADDRESS_BORDER;
			break;
		default:
			TERROR( "TRenderD3DInterface::SetTextureAddress: Invalid addressing mode specified!\n" );
			return;
	}

	// Set either U or V coordinate addressing
	if ( a_eTextureCoordinate == TEXCOORD_U )
	{
		m_pDirectDevice->SetTextureStageState( a_iStage, D3DTSS_ADDRESSU, addressMode );
	}
	else if ( a_eTextureCoordinate == TEXCOORD_V )
	{
		m_pDirectDevice->SetTextureStageState( a_iStage, D3DTSS_ADDRESSV, addressMode );
	}
	else
	{
		TASSERT( a_eTextureCoordinate == TEXCOORD_UV );
		m_pDirectDevice->SetTextureStageState( a_iStage, D3DTSS_ADDRESSU, addressMode );
		m_pDirectDevice->SetTextureStageState( a_iStage, D3DTSS_ADDRESSV, addressMode );
	}
}

// $Barnyard: FUNCTION 006c6070
// Clears a region of the screen with specified color, depth, and stencil values
void TRenderD3DInterface::ClearRegion( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, CLEARFLAGS a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil )
{
	// Build clear flags based on parameters
	DWORD eFlags = ( a_eClearFlags & CLEARFLAGS_TARGET ) ? D3DCLEAR_TARGET : 0;

	if ( a_eClearFlags & CLEARFLAGS_DEPTH )
	{
		eFlags = eFlags | D3DCLEAR_ZBUFFER;
		TMath::Clip( a_fZ, 0.0f, 1.0f );
	}

	if ( a_eClearFlags & CLEARFLAGS_STENCIL )
	{
		eFlags = eFlags | D3DCLEAR_STENCIL;
	}

	// Clear the specified region
	m_pDirectDevice->Clear(
	    0,
	    NULL,
	    eFlags,
	    ( ( a_uiColorR | 0xffffff00 ) << 8 | (TUINT)a_uiColorG ) << 8 | (TUINT)a_uiColorB,
	    a_fZ,
	    a_uiStencil
	);
}

// $Barnyard: FUNCTION 006c6760
TBOOL TRenderD3DInterface::IsTextureFormatSupported( TTEXTURERESOURCEFORMAT a_eTextureFormat )
{
	switch ( a_eTextureFormat )
	{
		case TTEXTURERESOURCEFORMAT::R8G8B8A8:
			return IsTextureFormatSupportedImpl( D3DFMT_A8R8G8B8 );
		case TTEXTURERESOURCEFORMAT::R8G8B8:
			return IsTextureFormatSupportedImpl( D3DFMT_X8R8G8B8 );
		case TTEXTURERESOURCEFORMAT::R5G5B5A1:
			return IsTextureFormatSupportedImpl( D3DFMT_A1R5G5B5 );
		case TTEXTURERESOURCEFORMAT::DDS:
			return TTRUE;
		default:
			return TFALSE;
	}
}

// $Barnyard: FUNCTION 006c6380
TBOOL TRenderD3DInterface::IsTextureFormatSupportedImpl( D3DFORMAT a_eFormat )
{
	auto pDevice = GetCurrentDevice();

	return SUCCEEDED(
	    m_pDirect3D->CheckDeviceFormat(
	        pDevice->GetMode()->GetAdapter()->GetAdapterIndex(),
	        TD3DAdapter::Mode::Device::GetD3DDeviceType( pDevice->GetDeviceIndex() ),
	        m_PresentParams.BackBufferFormat,
	        0,
	        D3DRTYPE_TEXTURE,
	        a_eFormat
	    )
	);
}

// $Barnyard: FUNCTION 006c63f0
TBOOL TRenderD3DInterface::Supports32BitTextures()
{
	return IsTextureFormatSupported( TTEXTURERESOURCEFORMAT::R8G8B8A8 ) && IsTextureFormatSupported( TTEXTURERESOURCEFORMAT::R8G8B8 );
}

// $Barnyard: FUNCTION 006c5800
void TRenderD3DInterface::OnInitializationFailureDevice()
{
	CHAR caption[ 1024 ];
	CHAR text[ 1024 ];

	GetPrivateProfileStringA( "Setup", "IDS_D3DDEVICEERRORTITLE", "Initialization failure", caption, 0x400, ".\\Setup.ini" );
	GetPrivateProfileStringA( "Setup", "IDS_D3DDEVICEERROR", "Failed to initialize Direct3D. Please ensure DirectX8.1b is installed AND DirectX8.1b drivers for your video card", text, 0x400, ".\\Setup.ini" );
	MessageBoxA( NULL, text, caption, 0 );
}

// $Barnyard: FUNCTION 006c5870
void TRenderD3DInterface::OnInitializationFailureDisplay()
{
	CHAR caption[ 1024 ];
	CHAR text[ 1024 ];

	GetPrivateProfileStringA( "Setup", "IDS_D3DDISPLAYERRORTITLE", "Initialization failure", caption, 0x400, ".\\Setup.ini" );
	GetPrivateProfileStringA( "Setup", "IDS_D3DDISPLAYERROR", "Failed to create the display. Please run the Barnyard setup program and reconfigure", text, 0x400, ".\\Setup.ini" );
	MessageBoxA( NULL, text, caption, 0 );
}

// $Barnyard: FUNCTION 006c72a0
// Creates the DirectX 8 render interface
TBOOL TRenderD3DInterface::Create( const TCHAR* a_szWindowName )
{
	TASSERT( TFALSE == IsCreated() );

	if ( TRenderInterface::Create() )
	{
		// Create Direct3D 8 interface
		m_pDirect3D = Direct3DCreate8( D3D_SDK_VERSION );

		if ( m_pDirect3D )
		{
			// Initialize adapter database and create accelerator table
			BuildAdapterDatabase();
			CreateAccelTable();

			// Create window
			if ( m_Window.Create( this, a_szWindowName ) )
			{
				// Create system resources if needed
				if ( m_bCreateSystemResources )
				{
					CreateSystemResources();
				}

				return TTRUE;
			}
		}
		else
		{
			OnInitializationFailureDevice();
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006c7090
void TRenderD3DInterface::BuildAdapterDatabase()
{
	UINT uiAdapterCount = m_pDirect3D->GetAdapterCount();

	for ( UINT i = 0; i < uiAdapterCount; i++ )
	{
		auto pAdapter = new TD3DAdapter();
		pAdapter->SetAdapterIndex( i );

		D3DDISPLAYMODE displayMode;
		auto           pIdentifier = pAdapter->GetD3DIdentifier8();
		m_pDirect3D->GetAdapterIdentifier( i, D3DENUM_NO_WHQL_LEVEL, pIdentifier );
		m_pDirect3D->GetAdapterDisplayMode( i, &displayMode );

		pAdapter->SetDriver( pIdentifier->Driver );
		pAdapter->SetDescription( pIdentifier->Description );
		pAdapter->SetDriverVersionLowPart( pIdentifier->DriverVersion.LowPart );
		pAdapter->SetDriverVersionHighPart( pIdentifier->DriverVersion.HighPart );
		pAdapter->SetDeviceId( pIdentifier->DeviceId );
		pAdapter->SetVendorId( pIdentifier->VendorId );
		pAdapter->SetSubSysId( pIdentifier->SubSysId );
		pAdapter->SetRevision( pIdentifier->Revision );
		pAdapter->SetDeviceIdentifier( pIdentifier->DeviceIdentifier );

		pAdapter->GetMode().SetD3DDisplayMode( displayMode );
		pAdapter->EnumerateOutputs( this );

		GetAdapterList()->InsertTail( pAdapter );
	}
}

void TRenderD3DInterface::DestroyAccelTable()
{
	if ( m_AcceleratorTable )
	{
		DestroyAcceleratorTable( m_AcceleratorTable );
		m_AcceleratorTable = NULL;
	}
}

// $Barnyard: FUNCTION 006c6640
void TRenderD3DInterface::CreateAccelTable()
{
	DestroyAccelTable();

	ACCEL accel[ 2 ];
	accel[ 0 ].fVirt   = 1;
	accel[ 0 ].key     = 27;
	accel[ 0 ].cmd     = 0;
	accel[ 1 ].fVirt   = 16;
	accel[ 1 ].key     = 13;
	accel[ 1 ].cmd     = 0;
	m_AcceleratorTable = CreateAcceleratorTableA( accel, 2 );
}

// $Barnyard: FUNCTION 006ded40
const TCHAR* TRenderD3DInterface::GetErrorString( TINT32 a_eError )
{
	return DXGetErrorString8A( a_eError );
}

// $Barnyard: FUNCTION 006e16f0
const TCHAR* TRenderD3DInterface::GetErrorDescription( TINT32 a_eError )
{
	return DXGetErrorDescription8A( a_eError );
}

void TRenderD3DInterface::RegisterOrderTable( TOrderTable* a_pOrderTable )
{
	m_OrderTables.Insert( a_pOrderTable );
}

TOSHI_NAMESPACE_END
