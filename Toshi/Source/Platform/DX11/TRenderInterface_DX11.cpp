#include "ToshiPCH.h"
#include "TModel_DX11.h"
#include "TRenderCapture_DX11.h"
#include "TRenderContext_DX11.h"
#include "TRenderAdapter_DX11.h"
#include "TRenderInterface_DX11.h"
#include "TDebugText_DX11.h"
#include "TConstantBuffer_DX11.h"
#include "TTextureFactoryHAL_DX11.h"
#include "TTextureResourceHAL_DX11.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "TDevScreenshot_DX11.h"
#endif

#include "Render/TShader.h"
#include "Input/TInputInterface.h"

#include <d3dcompiler.h>
#include <DirectXTex.h>
#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TRenderD3DInterface );

// Prepended to every shader
static const TCHAR s_szShaderCommon[] = R"(
// Discards the pixel when a_fAlpha fails the alpha test of TRenderD3DInterface::GetAlphaTestParams
void AlphaTest( float a_fAlpha, float4 a_vParams )
{
	if ( a_vParams.x == 0.0f ) return;

	float fRef  = a_vParams.y;
	int   iFunc = int( a_vParams.z );

	bool bPass = ( iFunc == 8 ) || ( iFunc == 2 && a_fAlpha < fRef ) || ( iFunc == 3 && a_fAlpha == fRef ) ||
	    ( iFunc == 4 && a_fAlpha <= fRef ) || ( iFunc == 5 && a_fAlpha > fRef ) || ( iFunc == 6 && a_fAlpha != fRef ) ||
	    ( iFunc == 7 && a_fAlpha >= fRef );

	if ( !bPass ) discard;
}
)";

static const TCHAR s_szFixedFunctionShader[] = R"(
cbuffer Constants : register( b0 )
{
	row_major float4x4 g_mWorldViewProjection;
	float4             g_vParams; // alpha test (xyz), modulate with the texture (w)
};

struct VS_IN
{
	float3 vPosition : POSITION;
	float4 vColour : COLOR;
	float2 vUV : TEXCOORD;
};

struct PS_IN
{
	float4 vPosition : SV_POSITION;
	float4 vColour : COLOR;
	float2 vUV : TEXCOORD;
};

PS_IN vs_main( VS_IN In )
{
	PS_IN Out;
	Out.vPosition = mul( float4( In.vPosition, 1.0f ), g_mWorldViewProjection );
	Out.vColour   = In.vColour;
	Out.vUV       = In.vUV;
	return Out;
}

Texture2D    g_Texture : register( t0 );
SamplerState g_Sampler : register( s0 );

float4 ps_main( PS_IN In ) : SV_TARGET
{
	float4 vColour = In.vColour;

	if ( g_vParams.w != 0.0f )
		vColour *= g_Texture.Sample( g_Sampler, In.vUV );

	AlphaTest( vColour.a, g_vParams );
	return vColour;
}
)";

static const D3D_FEATURE_LEVEL s_aFeatureLevels[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
};

void TRenderInterface::BeginEndSceneHAL()
{
}

TRenderD3DInterface::TRenderD3DInterface()
{
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

	m_pDevice11          = TNULL;
	m_pDeviceContext     = TNULL;
	m_pDeviceContext1    = TNULL;
	m_pSwapChain         = TNULL;
	m_pBackBuffer        = TNULL;
	m_pPendingCapture    = TNULL;

	TUtil::MemClear( m_apVSConstantBuffers, sizeof( m_apVSConstantBuffers ) );
	TUtil::MemClear( m_apPSConstantBuffers, sizeof( m_apPSConstantBuffers ) );
	TUtil::MemClear( m_apCSConstantBuffers, sizeof( m_apCSConstantBuffers ) );
	m_uiCSConstantBufferSlots = 0;
	TUtil::MemClear( &m_oCurrent, sizeof( m_oCurrent ) );
	TUtil::MemClear( &m_oApplied, sizeof( m_oApplied ) );
	m_bStateCacheValid = TFALSE;
	m_uiVSConstantBufferSlots = 0;
	m_uiPSConstantBufferSlots = 0;
	m_pBackBufferRTView  = TNULL;
	m_oDepth             = {};
	m_pSceneTexture      = TNULL;
	m_pSceneRTView       = TNULL;
	m_pSceneSRV          = TNULL;
	m_oSceneDepth        = {};
	m_eSceneFormat       = DXGI_FORMAT_UNKNOWN;
	m_uiSceneSampleCount = 1;
	m_bSceneTargetActive = TFALSE;
	m_uiBackBufferWidth  = 0;
	m_uiBackBufferHeight = 0;
	m_bOccluded          = TFALSE;

	m_pSecondaryRenderTargetOwner = TNULL;

	m_pConstantBufferData    = TNULL;
	m_uiConstantBufferSize   = 0;
	m_bConstantBufferChanged = TFALSE;
	m_uiConstantBufferIndex  = 0;
	TUtil::MemClear( m_aConstantBuffers, sizeof( m_aConstantBuffers ) );

	m_pImmediateVertexBuffer         = TNULL;
	m_uiImmediateVertexCurrentOffset = 0;
	m_pImmediateIndexBuffer          = TNULL;
	m_uiImmediateIndexCurrentOffset  = 0;

	TUtil::MemClear( m_apSamplerStates, sizeof( m_apSamplerStates ) );
	TUtil::MemClear( m_aGammaRamp, sizeof( m_aGammaRamp ) );

	m_CurrentDepth.uiRaw                         = 0;
	m_uiCurrentStencilRef                        = 0;
	m_PreviousDepth.uiRaw                        = 0;
	m_uiPreviousStencilRef                       = 0;
	m_CurrentRasterizer.Flags.uiRaw              = 0;
	m_CurrentRasterizer.DepthBias                = 0;
	m_CurrentRasterizer.SlopeScaledDepthBias     = 0.0f;
	m_PreviousRasterizer                         = m_CurrentRasterizer;
	m_CurrentBlendState.uiRaw                    = 0;
	m_PreviousBlendState.uiRaw                   = 0;

	for ( TINT i = 0; i < 4; i++ )
	{
		m_aCurrentBlendFactor[ i ]  = 1.0f;
		m_aPreviousBlendFactor[ i ] = 1.0f;
	}

	for ( TINT i = 0; i < NUM_TEXTURE_STAGES; i++ )
	{
		m_aAddressU[ i ]         = ADDRESSINGMODE_WRAP;
		m_aAddressV[ i ]         = ADDRESSINGMODE_WRAP;
		m_aMinFilters[ i ]       = TEXTUREFILTER_POINT;
		m_aMagFilters[ i ]       = TEXTUREFILTER_POINT;
		m_aMipFilters[ i ]       = TEXTUREFILTER_NONE;
	}

	m_uiDirtySamplers      = 0;
	m_pFixedFunctionVS     = TNULL;
	m_pFixedFunctionPS     = TNULL;
	m_pFixedFunctionLayout = TNULL;
	m_pPositionColourLayout = TNULL;
	m_pZeroTexCoordBuffer   = TNULL;
	m_bFixedFunction       = TFALSE;
	m_eTextureOp           = TEXTUREOP_MODULATE;
	m_bAlphaTestEnable     = TFALSE;
	m_eAlphaFunc           = D3D11_COMPARISON_ALWAYS;
	m_uiAlphaRef           = 0;
	m_bFogEnable           = TFALSE;
	m_uiFogColour          = 0;
	m_fFogStart            = 0.0f;
	m_fFogEnd              = 1.0f;

	for ( TINT i = 0; i < TRANSFORM_NUMOF; i++ )
		m_aTransforms[ i ] = TMatrix44::IDENTITY;
}

TRenderD3DInterface::~TRenderD3DInterface()
{
	Destroy();
}

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

		auto pMode = TSTATICCAST( TD3DAdapter::Mode, m_pDevice->GetMode() );

		IDXGIDevice*   pDXGIDevice  = TNULL;
		IDXGIAdapter*  pDXGIAdapter = TNULL;
		IDXGIFactory2* pFactory     = TNULL;
		m_pDevice11->QueryInterface( __uuidof( IDXGIDevice ), (void**)&pDXGIDevice );
		pDXGIDevice->GetAdapter( &pDXGIAdapter );
		pDXGIAdapter->GetParent( __uuidof( IDXGIFactory2 ), (void**)&pFactory );

		DXGI_SWAP_CHAIN_DESC1 oSwapChainDesc = {};
		oSwapChainDesc.Width                 = pDisplayParams->uiWidth;
		oSwapChainDesc.Height                = pDisplayParams->uiHeight;
		oSwapChainDesc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
		oSwapChainDesc.SampleDesc.Count      = 1;
		oSwapChainDesc.SampleDesc.Quality    = 0;
		oSwapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		oSwapChainDesc.BufferCount           = SWAPCHAIN_BUFFER_COUNT;
		oSwapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
		oSwapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		oSwapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
		oSwapChainDesc.Flags                 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC oFullscreenDesc = {};
		oFullscreenDesc.RefreshRate                     = pMode->GetDescription().RefreshRate;
		oFullscreenDesc.ScanlineOrdering                = pMode->GetDescription().ScanlineOrdering;
		oFullscreenDesc.Scaling                         = pMode->GetDescription().Scaling;
		oFullscreenDesc.Windowed                        = TRUE;

		HRESULT hRes = pFactory->CreateSwapChainForHwnd( m_pDevice11, m_Window.GetHWND(), &oSwapChainDesc, &oFullscreenDesc, TNULL, &m_pSwapChain );

		pFactory->MakeWindowAssociation( m_Window.GetHWND(), DXGI_MWA_NO_ALT_ENTER );

		pFactory->Release();
		pDXGIAdapter->Release();
		pDXGIDevice->Release();

		if ( FAILED( hRes ) )
		{
			OnInitializationFailureDevice();
			PrintError( hRes, "Failed to create the swap chain!" );
			return TFALSE;
		}

		m_uiBackBufferWidth  = pDisplayParams->uiWidth;
		m_uiBackBufferHeight = pDisplayParams->uiHeight;

		if ( !CreateBackBufferViews() )
		{
			OnInitializationFailureDevice();
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
		if ( pMode->GetDisplayIndex() != 0 )
		{
			IDXGIOutput* pOutput = TNULL;

			if ( SUCCEEDED( m_pSwapChain->GetContainingOutput( &pOutput ) ) )
			{
				DXGI_OUTPUT_DESC oOutputDesc;
				pOutput->GetDesc( &oOutputDesc );
				pOutput->Release();

				uiWindowPosX += oOutputDesc.DesktopCoordinates.left;
				uiWindowPosY += oOutputDesc.DesktopCoordinates.top;
			}
		}

		// Set window position and size
		m_Window.SetPosition( uiWindowPosX, uiWindowPosY, pDisplayParams->uiWidth, pDisplayParams->uiHeight );

		if ( !pDisplayParams->bWindowed )
		{
			hRes = m_pSwapChain->SetFullscreenState( TRUE, TNULL );

			if ( FAILED( hRes ) )
				PrintError( hRes, "Failed to switch to fullscreen!" );
		}

		// Set cursor position to center of window
#ifdef BARNYARD_COMMUNITY_PATCH
		// Background test runs leave the user's mouse cursor alone
		if ( !TMSWindow::IsBackgroundMode() )
#endif
			SetCursorPos(
			    uiWindowPosX + pDisplayParams->uiWidth / 2,
			    uiWindowPosY + pDisplayParams->uiHeight / 2
			);

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

TBOOL TRenderD3DInterface::DestroyDisplay()
{
	TASSERT( IsDisplayCreated() );
	FlushDyingResources();

	if ( IsDisplayCreated() )
	{
		DestroyBackBufferViews();

		if ( m_pSwapChain )
		{
			m_pSwapChain->SetFullscreenState( FALSE, TNULL );
			m_pSwapChain->Release();
			m_pSwapChain = TNULL;
		}

		return TRenderInterface::DestroyDisplay();
	}
	else
	{
		return TTRUE;
	}
}

TBOOL TRenderD3DInterface::Update( TFLOAT a_fDeltaTime )
{
	TRenderInterface::Update( a_fDeltaTime );
	m_Window.Update( a_fDeltaTime );

	return !m_bExited;
}

TBOOL TRenderD3DInterface::BeginScene()
{
	TASSERT( !IsInScene() );

	if ( TRenderInterface::BeginScene() )
	{
		ResetStateCache();

		m_bSceneTargetActive = TFALSE;
		SetRenderTargets( m_pBackBufferRTView, m_oDepth.pView );

		D3D11_VIEWPORT viewport = {
			.TopLeftX = 0.0f,
			.TopLeftY = 0.0f,
			.Width    = TFLOAT( m_uiBackBufferWidth ),
			.Height   = TFLOAT( m_uiBackBufferHeight ),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f,
		};

		m_pDeviceContext->RSSetViewports( 1, &viewport );
		UpdateColourSettings();

		m_bInScene = TTRUE;
		m_BeginSceneEmitter.Throw( 0 );
	}

	return TTRUE;
}

TBOOL TRenderD3DInterface::EndScene()
{
	TASSERT( IsInScene() );

	if ( IsInScene() )
	{
		m_EndSceneEmitter.Throw( TNULL );
		TASSERT( !m_bSceneTargetActive && "EndSceneTarget wasn't called" );

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
		// Development: "-devshot" saves the finished frame (GUI included) before it's presented
		TDevScreenshot::Update( m_pDevice11, m_pDeviceContext, m_pBackBuffer );
#endif

		if ( m_pPendingCapture )
		{
			m_pPendingCapture->CaptureBackBuffer( m_pDevice11, m_pDeviceContext, m_pBackBuffer );
			m_pPendingCapture = TNULL;
		}

		HRESULT hPresentRes = m_pSwapChain->Present( m_bOccluded ? 0 : 1, m_bOccluded ? DXGI_PRESENT_TEST : 0 );
		m_bOccluded         = hPresentRes == DXGI_STATUS_OCCLUDED;

		if ( hPresentRes == DXGI_ERROR_DEVICE_REMOVED || hPresentRes == DXGI_ERROR_DEVICE_RESET )
		{
			PrintError( m_pDevice11->GetDeviceRemovedReason(), "The D3D device was removed!" );
			OnD3DDeviceLost();
		}

		m_bInScene = TFALSE;
	}

	return TTRUE;
}

TRenderAdapter::Mode::Device* TRenderD3DInterface::GetCurrentDevice()
{
	return m_pDevice;
}

TRenderInterface::DISPLAYPARAMS* TRenderD3DInterface::GetCurrentDisplayParams()
{
	return &m_oDisplayParams;
}

TBOOL TRenderD3DInterface::Create()
{
	return Create( TClassObjectName.GetName() );
}

void TRenderD3DInterface::FlushShaders()
{
	Interface()->FlushOrderTables();

	for ( auto it = TShader::sm_oShaderList.GetRootShader(); it != TNULL; it = it->GetNextShader() )
	{
		it->Flush();
	}
}

TBOOL TRenderD3DInterface::CreateVertexShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11VertexShader** a_ppVertexShader )
{
	HRESULT hRes = Interface()->GetDevice()->CreateVertexShader( a_pShaderBytecode, a_uiBytecodeLength, TNULL, a_ppVertexShader );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failure to create the vertex shader!" );
		return TFALSE;
	}

	return TTRUE;
}

TBOOL TRenderD3DInterface::CreatePixelShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11PixelShader** a_ppPixelShader )
{
	HRESULT hRes = Interface()->GetDevice()->CreatePixelShader( a_pShaderBytecode, a_uiBytecodeLength, TNULL, a_ppPixelShader );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failure to create the pixel shader!" );
		return TFALSE;
	}

	return TTRUE;
}

TBOOL TRenderD3DInterface::CreateComputeShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11ComputeShader** a_ppComputeShader )
{
	HRESULT hRes = Interface()->GetDevice()->CreateComputeShader( a_pShaderBytecode, a_uiBytecodeLength, TNULL, a_ppComputeShader );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failure to create the compute shader!" );
		return TFALSE;
	}

	return TTRUE;
}

const TCHAR* TRenderD3DInterface::GetCommonShaderSource()
{
	return s_szShaderCommon;
}

ID3DBlob* TRenderD3DInterface::CompileShader( const TCHAR* a_szSrcData, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const D3D_SHADER_MACRO* a_pDefines )
{
	ID3DBlob* pShaderBlob = TNULL;
	ID3DBlob* pErrorBlob  = TNULL;

	UINT uiFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#ifdef TOSHI_DEBUG
	uiFlags |= D3DCOMPILE_DEBUG;
#endif

	TString8 strSource = s_szShaderCommon;
	strSource += a_szSrcData;

	if ( FAILED( D3DCompile( strSource.GetString(), strSource.Length(), NULL, a_pDefines, NULL, a_szEntryPoint, a_szTarget, uiFlags, 0, &pShaderBlob, &pErrorBlob ) ) )
	{
		const TCHAR* szError = pErrorBlob ? TSTATICCAST( const TCHAR, pErrorBlob->GetBufferPointer() ) : "";
		OutputDebugStringA( "Shader compilation failed: " );
		OutputDebugStringA( szError );
		TERROR( "Shader compilation failed: %s\n", szError );

		if ( pErrorBlob )
			pErrorBlob->Release();

		TASSERT( TFALSE );
		return TNULL;
	}

	if ( pErrorBlob )
		pErrorBlob->Release();

	return pShaderBlob;
}

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
		DestroyDeviceObjects();

		m_Window.UnregisterWindowClass();
		DestroyAccelTable();

		if ( m_pDeviceContext1 )
		{
			m_pDeviceContext1->Release();
			m_pDeviceContext1 = TNULL;
		}

		if ( m_pDeviceContext )
		{
			m_pDeviceContext->ClearState();
			TUtil::MemClear( &m_oCurrent, sizeof( m_oCurrent ) );
			InvalidateStateCache();
			m_pDeviceContext->Release();
			m_pDeviceContext = TNULL;
		}

		if ( m_pDevice11 )
		{
			m_pDevice11->Release();
			m_pDevice11 = TNULL;
		}

		return TRenderInterface::Destroy();
	}
	else
	{
		return TTRUE;
	}
}

void TRenderD3DInterface::RenderIndexPrimitive( TINT a_ePrimitiveType, TINT a_iNumVertices, TINT a_iNumPrimitives, TINT a_pIndices, TINT a_pVertices, TINT a_iVertexStride )
{
	enum PRIMITIVE_
	{
		PRIMITIVE_POINTLIST,
		PRIMITIVE_LINELIST,
		PRIMITIVE_LINESTRIP,
		PRIMITIVE_TRIANGLELIST,
		PRIMITIVE_TRIANGLESTRIP,
		PRIMITIVE_TRIANGLEFAN
	};

	static const D3D11_PRIMITIVE_TOPOLOGY s_aPrimitiveTypes[] = {
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	};

	TRenderContextD3D* pContext = TRenderContextD3D::Upcast( GetCurrentContext() );

	SetTransform( TRANSFORM_VIEW, TMatrix44::IDENTITY );
	SetTransform( TRANSFORM_PROJECTION, pContext->GetProjectionMatrix() );
	SetTransform( TRANSFORM_WORLD, pContext->GetModelViewMatrix() );

	// Position and colour
	SetFixedFunctionShader();
	SetInputLayout( m_pPositionColourLayout );

	SetVertexBuffer( 1, m_pZeroTexCoordBuffer, 0, 0 );

	const TUINT16* pIndices = TREINTERPRETCAST( const TUINT16*, a_pIndices );
	UINT           uiNumIndices;

	switch ( a_ePrimitiveType )
	{
		case PRIMITIVE_POINTLIST: uiNumIndices = a_iNumPrimitives; break;
		case PRIMITIVE_LINELIST: uiNumIndices = a_iNumPrimitives * 2; break;
		case PRIMITIVE_LINESTRIP: uiNumIndices = a_iNumPrimitives + 1; break;
		case PRIMITIVE_TRIANGLELIST: uiNumIndices = a_iNumPrimitives * 3; break;
		case PRIMITIVE_TRIANGLESTRIP: uiNumIndices = a_iNumPrimitives + 2; break;
		default: uiNumIndices = a_iNumPrimitives * 3; break;
	}

	// D3D11 has no triangle fans, they become lists
	TUINT16* pFanIndices = TNULL;

	if ( a_ePrimitiveType == PRIMITIVE_TRIANGLEFAN )
	{
		pFanIndices = new TUINT16[ uiNumIndices ];

		for ( TINT i = 0; i < a_iNumPrimitives; i++ )
		{
			pFanIndices[ i * 3 + 0 ] = pIndices[ 0 ];
			pFanIndices[ i * 3 + 1 ] = pIndices[ i + 1 ];
			pFanIndices[ i * 3 + 2 ] = pIndices[ i + 2 ];
		}

		pIndices = pFanIndices;
	}

	DrawImmediately( s_aPrimitiveTypes[ a_ePrimitiveType ], uiNumIndices, pIndices, DXGI_FORMAT_R16_UINT, TREINTERPRETCAST( const void*, a_pVertices ), a_iVertexStride, a_iNumVertices );

	delete[] pFanIndices;
}

TFLOAT TRenderD3DInterface::GetPixelAspectRatio()
{
	return m_fPixelAspectRatio;
}

TBOOL TRenderD3DInterface::SetPixelAspectRatio( TFLOAT a_fPixelAspectRatio )
{
	m_fPixelAspectRatio = a_fPixelAspectRatio;
	return TTRUE;
}

void TRenderD3DInterface::FlushOrderTables()
{
	TASSERT( TTRUE == IsInScene() );

	for ( auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++ )
	{
		it->Flush();
	}
}

TRenderContext* TRenderD3DInterface::CreateRenderContext()
{
	return new TRenderContextD3D( this );
}

TRenderCapture* TRenderD3DInterface::CreateCapture()
{
	return new TRenderCaptureD3D();
}

void TRenderD3DInterface::DestroyCapture( TRenderCapture* a_pRenderCapture )
{
	if ( m_pPendingCapture == a_pRenderCapture )
		m_pPendingCapture = TNULL;

	a_pRenderCapture->Destroy();
	delete a_pRenderCapture;
}

void* TRenderD3DInterface::CreateUnknown( const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	return TNULL;
}

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

TDebugText* TRenderD3DInterface::CreateDebugText()
{
	m_pDebugText = InitDebugText( this, 0 );
	return m_pDebugText;
}

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

void TRenderD3DInterface::SetContrast( TFLOAT a_fConstrast )
{
	TMath::Clip( a_fConstrast, 0.0f, 1.0f );
	m_fContrast              = a_fConstrast;
	m_bChangedColourSettings = TTRUE;
}

void TRenderD3DInterface::SetBrightness( TFLOAT a_fBrightness )
{
	TMath::Clip( a_fBrightness, 0.0f, 1.0f );
	m_fBrightness            = a_fBrightness;
	m_bChangedColourSettings = TTRUE;
}

void TRenderD3DInterface::SetGamma( TFLOAT a_fGamma )
{
	TMath::Clip( a_fGamma, 0.0f, 1.0f );
	m_fGamma                 = a_fGamma;
	m_bChangedColourSettings = TTRUE;
}

void TRenderD3DInterface::SetSaturate( TFLOAT a_fSaturate )
{
	TMath::Clip( a_fSaturate, 0.0f, 1.0f );
	m_fSaturate              = a_fSaturate;
	m_bChangedColourSettings = TTRUE;
}

TFLOAT TRenderD3DInterface::GetContrast() const
{
	return m_fContrast;
}

TFLOAT TRenderD3DInterface::GetBrightness() const
{
	return m_fBrightness;
}

TFLOAT TRenderD3DInterface::GetGamma() const
{
	return m_fGamma;
}

TFLOAT TRenderD3DInterface::GetSaturate() const
{
	return m_fSaturate;
}

void TRenderD3DInterface::UpdateColourSettings()
{
	if ( IsColourCorrection() && m_bChangedColourSettings )
	{
		GetCurrentColourRamp();

		BOOL         bFullscreen = FALSE;
		IDXGIOutput* pOutput     = TNULL;

		if ( SUCCEEDED( m_pSwapChain->GetFullscreenState( &bFullscreen, &pOutput ) ) && bFullscreen && pOutput )
		{
			DXGI_GAMMA_CONTROL_CAPABILITIES oCaps;

			if ( SUCCEEDED( pOutput->GetGammaControlCapabilities( &oCaps ) ) && oCaps.NumGammaControlPoints > 1 )
			{
				DXGI_GAMMA_CONTROL oGammaControl = {};
				oGammaControl.Scale              = { 1.0f, 1.0f, 1.0f };
				oGammaControl.Offset             = { 0.0f, 0.0f, 0.0f };

				for ( UINT i = 0; i < oCaps.NumGammaControlPoints; i++ )
				{
					const TFLOAT fPosition = oCaps.ControlPointPositions[ i ] * 255.0f;
					const TINT   iIndex    = TMath::Min( TINT( fPosition ), 254 );
					const TFLOAT fT        = fPosition - TFLOAT( iIndex );

					auto fnSample = [ & ]( const TUINT16* a_pRamp ) {
						return ( a_pRamp[ iIndex ] + ( a_pRamp[ iIndex + 1 ] - a_pRamp[ iIndex ] ) * fT ) * ( 1.0f / 65535.0f );
					};

					oGammaControl.GammaCurve[ i ].Red   = fnSample( m_aGammaRamp[ 0 ] );
					oGammaControl.GammaCurve[ i ].Green = fnSample( m_aGammaRamp[ 1 ] );
					oGammaControl.GammaCurve[ i ].Blue  = fnSample( m_aGammaRamp[ 2 ] );
				}

				pOutput->SetGammaControl( &oGammaControl );
			}
		}

		if ( pOutput )
			pOutput->Release();
	}
}

TBOOL TRenderD3DInterface::IsCapableColourCorrection()
{
	if ( !m_bCheckedCapableColourCorrection )
	{
		m_bCheckedCapableColourCorrection = TTRUE;
		m_bCapableColourCorrection        = TTRUE;
	}

	return m_bCapableColourCorrection;
}

void TRenderD3DInterface::EnableColourCorrection( TBOOL a_bEnable )
{
	m_bEnableColourCorrection = a_bEnable && IsCapableColourCorrection();
}

void TRenderD3DInterface::ForceEnableColourCorrection( TBOOL a_bEnable )
{
	m_bEnableColourCorrection = a_bEnable;
}

TBOOL TRenderD3DInterface::IsColourCorrection()
{
	return m_bEnableColourCorrection;
}

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
	TResource::Recurse(
	    []( TResource* a_pResource, void* a_pUserData ) {
		    a_pResource->TryInvalidate();
		    return TTRUE;
	    },
	    m_Resources.GetRoot()->Child(), TTRUE, TNULL
	);

	if ( m_pDebugText )
		TSTATICCAST( TDebugD3DText, m_pDebugText )->OnDeviceLost();

	for ( TShader* pShader = TShader::sm_oShaderList.GetRootShader(); pShader != TNULL; pShader = pShader->GetNextShader() )
		pShader->TryInvalidate();

	if ( m_fnOnDeviceLostCallback )
		m_fnOnDeviceLostCallback();

	FlushDyingResources();
	g_oSystemManager.Pause( TTRUE );
}

void TRenderD3DInterface::OnD3DDeviceFound()
{
	if ( m_bFailed == TTRUE )
	{
		TResource::Recurse( []( TResource* a_pResource, void* a_pUserData ) {
			a_pResource->TryValidate();
			return TTRUE;
		},
		                    m_Resources.GetRoot()->Child(), TTRUE, TNULL );

		if ( m_pDebugText )
			TSTATICCAST( TDebugD3DText, m_pDebugText )->OnDeviceFound();

		for ( TShader* pShader = TShader::sm_oShaderList.GetRootShader(); pShader != TNULL; pShader = pShader->GetNextShader() )
			pShader->TryValidate();

		if ( m_fnOnDeviceFoundCallback )
			m_fnOnDeviceFoundCallback();

		SetDeviceDefaultStates();
		g_oSystemManager.Pause( TFALSE );
		m_bFailed = TFALSE;
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
static TINT FloatToIntTruncate( TFLOAT a_fValue )
{
	return TINT( a_fValue );
}

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
		m_aGammaRamp[ 0 ][ i ] = TUINT16( FloatToIntTruncate( aRamps[ 0 ][ i ] * 65535.0f ) );
		m_aGammaRamp[ 1 ][ i ] = TUINT16( FloatToIntTruncate( aRamps[ 1 ][ i ] * 65535.0f ) );
		m_aGammaRamp[ 2 ][ i ] = TUINT16( FloatToIntTruncate( aRamps[ 2 ][ i ] * 65535.0f ) );
	}
}

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
	pDebugText->m_pFont->InitDeviceObjects( a_pRenderer );
	pDebugText->m_pFont->RestoreDeviceObjects();

	return pDebugText;
}

void TRenderD3DInterface::SetDeviceDefaultStates()
{
	SetZEnable( TTRUE );

	// D3DCULL_CW
	m_CurrentRasterizer.Flags.Parts.bFrontCounterClockwise = FALSE;
	SetCullMode( D3D11_CULL_FRONT );

	SetTextureMinFilter( 0, TEXTUREFILTER_LINEAR );
	SetTextureMagFilter( 0, TEXTUREFILTER_LINEAR );
	SetTextureMipFilter( 0, TEXTUREFILTER_LINEAR );
	SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
}

void TRenderD3DInterface::SetTextureAddress( TINT a_iStage, ADDRESSINGMODE a_eAddressing, TEXCOORD a_eTextureCoordinate )
{
	TASSERT( a_iStage >= 0 && a_iStage < NUM_TEXTURE_STAGES );

	if ( a_eAddressing != ADDRESSINGMODE_WRAP && a_eAddressing != ADDRESSINGMODE_MIRROR &&
	     a_eAddressing != ADDRESSINGMODE_CLAMP && a_eAddressing != ADDRESSINGMODE_BORDER )
	{
		TERROR( "TRenderD3DInterface::SetTextureAddress: Invalid addressing mode specified!\n" );
		return;
	}

	// Set either U or V coordinate addressing
	if ( a_eTextureCoordinate == TEXCOORD_U )
	{
		m_aAddressU[ a_iStage ] = a_eAddressing;
	}
	else if ( a_eTextureCoordinate == TEXCOORD_V )
	{
		m_aAddressV[ a_iStage ] = a_eAddressing;
	}
	else
	{
		TASSERT( a_eTextureCoordinate == TEXCOORD_UV );
		m_aAddressU[ a_iStage ] = a_eAddressing;
		m_aAddressV[ a_iStage ] = a_eAddressing;
	}

	m_uiDirtySamplers |= 1 << a_iStage;
}

void TRenderD3DInterface::ClearRegion( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, CLEARFLAGS a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil )
{
	const TBOOL bWholeTarget = a_iX <= 0 && a_iY <= 0 && a_iX + a_iWidth >= TINT( m_uiBackBufferWidth ) && a_iY + a_iHeight >= TINT( m_uiBackBufferHeight );

	if ( a_eClearFlags & CLEARFLAGS_TARGET )
	{
		const FLOAT aColour[ 4 ] = { a_uiColorR * ( 1.0f / 255.0f ), a_uiColorG * ( 1.0f / 255.0f ), a_uiColorB * ( 1.0f / 255.0f ), 1.0f };

		if ( bWholeTarget || !m_pDeviceContext1 )
		{
			m_pDeviceContext->ClearRenderTargetView( GetRenderTargetView(), aColour );
		}
		else
		{
			const D3D11_RECT oRect = { a_iX, a_iY, a_iX + a_iWidth, a_iY + a_iHeight };
			m_pDeviceContext1->ClearView( GetRenderTargetView(), aColour, &oRect, 1 );
		}
	}

	UINT uiDepthFlags = 0;

	if ( a_eClearFlags & CLEARFLAGS_DEPTH )
	{
		uiDepthFlags |= D3D11_CLEAR_DEPTH;
		TMath::Clip( a_fZ, 0.0f, 1.0f );
	}

	if ( a_eClearFlags & CLEARFLAGS_STENCIL )
	{
		uiDepthFlags |= D3D11_CLEAR_STENCIL;
	}

	if ( uiDepthFlags != 0 && GetDepthStencilView() )
	{
		// TODO: clear only the region
		m_pDeviceContext->ClearDepthStencilView( GetDepthStencilView(), uiDepthFlags, a_fZ, TUINT8( a_uiStencil ) );
	}
}

TBOOL TRenderD3DInterface::IsTextureFormatSupported( TTEXTURERESOURCEFORMAT a_eTextureFormat )
{
	switch ( a_eTextureFormat )
	{
		case TTEXTURERESOURCEFORMAT::R8G8B8A8:
			return IsTextureFormatSupportedImpl( DXGI_FORMAT_B8G8R8A8_UNORM );
		case TTEXTURERESOURCEFORMAT::R8G8B8:
			return IsTextureFormatSupportedImpl( DXGI_FORMAT_B8G8R8X8_UNORM );
		case TTEXTURERESOURCEFORMAT::R5G5B5A1:
			return IsTextureFormatSupportedImpl( DXGI_FORMAT_B5G5R5A1_UNORM );
		case TTEXTURERESOURCEFORMAT::DDS:
			return TTRUE;
		default:
			return TFALSE;
	}
}

TBOOL TRenderD3DInterface::IsTextureFormatSupportedImpl( DXGI_FORMAT a_eFormat )
{
	UINT uiSupport = 0;
	return SUCCEEDED( m_pDevice11->CheckFormatSupport( a_eFormat, &uiSupport ) ) && HASANYFLAG( uiSupport, D3D11_FORMAT_SUPPORT_TEXTURE2D );
}

TBOOL TRenderD3DInterface::Supports32BitTextures()
{
	return IsTextureFormatSupported( TTEXTURERESOURCEFORMAT::R8G8B8A8 ) && IsTextureFormatSupported( TTEXTURERESOURCEFORMAT::R8G8B8 );
}

void TRenderD3DInterface::OnInitializationFailureDevice()
{
	CHAR caption[ 1024 ];
	CHAR text[ 1024 ];

	GetPrivateProfileStringA( "Setup", "IDS_D3DDEVICEERRORTITLE", "Initialization failure", caption, 0x400, ".\\Setup.ini" );
	GetPrivateProfileStringA( "Setup", "IDS_D3DDEVICEERROR", "Failed to initialize Direct3D. Please ensure your video card supports Direct3D 11 (feature level 10.0)", text, 0x400, ".\\Setup.ini" );
	MessageBoxA( NULL, text, caption, 0 );
}

void TRenderD3DInterface::OnInitializationFailureDisplay()
{
	CHAR caption[ 1024 ];
	CHAR text[ 1024 ];

	GetPrivateProfileStringA( "Setup", "IDS_D3DDISPLAYERRORTITLE", "Initialization failure", caption, 0x400, ".\\Setup.ini" );
	GetPrivateProfileStringA( "Setup", "IDS_D3DDISPLAYERROR", "Failed to create the display. Please run the Barnyard setup program and reconfigure", text, 0x400, ".\\Setup.ini" );
	MessageBoxA( NULL, text, caption, 0 );
}

TBOOL TRenderD3DInterface::Create( const TCHAR* a_szWindowName )
{
	TASSERT( TFALSE == IsCreated() );

	if ( TRenderInterface::Create() )
	{
		UINT uiFlags = 0;
#ifdef TOSHI_DEBUG
		uiFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hRes = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, uiFlags, s_aFeatureLevels, TARRAYSIZE( s_aFeatureLevels ), D3D11_SDK_VERSION, &m_pDevice11, TNULL, &m_pDeviceContext );

		// No SDK layers installed
		if ( FAILED( hRes ) && uiFlags != 0 )
			hRes = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, s_aFeatureLevels, TARRAYSIZE( s_aFeatureLevels ), D3D11_SDK_VERSION, &m_pDevice11, TNULL, &m_pDeviceContext );

		// The Direct3D 11.0 runtime doesn't know 11_1
		if ( hRes == E_INVALIDARG )
			hRes = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, s_aFeatureLevels + 1, TARRAYSIZE( s_aFeatureLevels ) - 1, D3D11_SDK_VERSION, &m_pDevice11, TNULL, &m_pDeviceContext );

		if ( SUCCEEDED( hRes ) )
		{
			if ( FAILED( m_pDeviceContext->QueryInterface( __uuidof( ID3D11DeviceContext1 ), (void**)&m_pDeviceContext1 ) ) )
				m_pDeviceContext1 = TNULL;

			// Initialize adapter database and create accelerator table
			BuildAdapterDatabase();
			CreateAccelTable();
			CreateDeviceObjects();

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

void TRenderD3DInterface::BuildAdapterDatabase()
{
	IDXGIFactory1* pFactory = TNULL;

	if ( FAILED( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**)&pFactory ) ) )
		return;

	IDXGIAdapter* pDXGIAdapter = TNULL;

	for ( UINT i = 0; pFactory->EnumAdapters( i, &pDXGIAdapter ) != DXGI_ERROR_NOT_FOUND; i++ )
	{
		auto pAdapter = new TD3DAdapter();
		pAdapter->SetAdapterIndex( i );

		auto& rDesc = pAdapter->GetAdapterDesc();
		pDXGIAdapter->GetDesc( &rDesc );

		TCHAR szDescription[ 128 ];
		TStringManager::StringUnicodeToChar( szDescription, rDesc.Description, -1 );

		LARGE_INTEGER oDriverVersion;
		oDriverVersion.QuadPart = 0;
		pDXGIAdapter->CheckInterfaceSupport( __uuidof( IDXGIDevice ), &oDriverVersion );

		pAdapter->SetDriver( "Direct3D 11" );
		pAdapter->SetDescription( szDescription );
		pAdapter->SetDriverVersion( oDriverVersion );

		pAdapter->EnumerateOutputs( this, pDXGIAdapter );

		GetAdapterList()->InsertTail( pAdapter );
		pDXGIAdapter->Release();
	}

	pFactory->Release();
}

void TRenderD3DInterface::DestroyAccelTable()
{
	if ( m_AcceleratorTable )
	{
		DestroyAcceleratorTable( m_AcceleratorTable );
		m_AcceleratorTable = NULL;
	}
}

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

const TCHAR* TRenderD3DInterface::GetErrorString( TINT32 a_eError )
{
	static TCHAR s_szError[ 16 ];
	TStringManager::String8Format( s_szError, sizeof( s_szError ), "0x%08X", TUINT32( a_eError ) );
	return s_szError;
}

const TCHAR* TRenderD3DInterface::GetErrorDescription( TINT32 a_eError )
{
	static TCHAR s_szDescription[ 512 ];

	if ( !FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, a_eError, 0, s_szDescription, sizeof( s_szDescription ), NULL ) )
		s_szDescription[ 0 ] = '\0';

	return s_szDescription;
}

void TRenderD3DInterface::RegisterOrderTable( TOrderTable* a_pOrderTable )
{
	m_OrderTables.Insert( a_pOrderTable );
}

TBOOL TRenderD3DInterface::CreateBackBufferViews()
{
	HRESULT hRes = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&m_pBackBuffer );

	if ( SUCCEEDED( hRes ) )
		hRes = m_pDevice11->CreateRenderTargetView( m_pBackBuffer, TNULL, &m_pBackBufferRTView );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failed to create the back buffer!" );
		return TFALSE;
	}

	if ( !CreateDepthTarget( 1, m_oDepth ) )
		return TFALSE;

	const FLOAT aBlack[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView( m_pBackBufferRTView, aBlack );
	m_pDeviceContext->ClearDepthStencilView( m_oDepth.pView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	return TTRUE;
}

TBOOL TRenderD3DInterface::CreateDepthTarget( UINT a_uiSampleCount, DEPTHTARGET& a_rTarget )
{
	// The texture is typeless so shaders can read it through a view of the matching colour format
	const DXGI_FORMAT eDepthFormat = TD3DAdapter::Mode::Device::DEPTHSTENCILFORMATS[ m_oDisplayParams.eDepthStencilFormat ];
	DXGI_FORMAT       eTypelessFormat;
	DXGI_FORMAT       eReadFormat;
	UINT              uiReadOnlyFlags = D3D11_DSV_READ_ONLY_DEPTH;

	switch ( eDepthFormat )
	{
		case DXGI_FORMAT_D16_UNORM:
			eTypelessFormat = DXGI_FORMAT_R16_TYPELESS;
			eReadFormat     = DXGI_FORMAT_R16_UNORM;
			break;
		case DXGI_FORMAT_D32_FLOAT:
			eTypelessFormat = DXGI_FORMAT_R32_TYPELESS;
			eReadFormat     = DXGI_FORMAT_R32_FLOAT;
			break;
		default:
			eTypelessFormat = DXGI_FORMAT_R24G8_TYPELESS;
			eReadFormat     = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			uiReadOnlyFlags |= D3D11_DSV_READ_ONLY_STENCIL;
			break;
	}

	const TBOOL bMultisampled = a_uiSampleCount > 1;

	D3D11_TEXTURE2D_DESC oDepthDesc = {};
	oDepthDesc.Width                = m_uiBackBufferWidth;
	oDepthDesc.Height               = m_uiBackBufferHeight;
	oDepthDesc.MipLevels            = 1;
	oDepthDesc.ArraySize            = 1;
	oDepthDesc.Format               = eTypelessFormat;
	oDepthDesc.SampleDesc.Count     = a_uiSampleCount;
	oDepthDesc.SampleDesc.Quality   = 0;
	oDepthDesc.Usage                = D3D11_USAGE_DEFAULT;
	oDepthDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* pDepthTexture = TNULL;
	HRESULT          hRes          = m_pDevice11->CreateTexture2D( &oDepthDesc, TNULL, &pDepthTexture );

	if ( SUCCEEDED( hRes ) )
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC oViewDesc = {};
		oViewDesc.Format                        = eDepthFormat;
		oViewDesc.ViewDimension                 = bMultisampled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		hRes                                    = m_pDevice11->CreateDepthStencilView( pDepthTexture, &oViewDesc, &a_rTarget.pView );

		if ( SUCCEEDED( hRes ) )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC oSRVDesc = {};
			oSRVDesc.Format                          = eReadFormat;
			oSRVDesc.ViewDimension                   = bMultisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			oSRVDesc.Texture2D.MipLevels             = 1;
			hRes                                     = m_pDevice11->CreateShaderResourceView( pDepthTexture, &oSRVDesc, &a_rTarget.pSRV );
		}

		if ( SUCCEEDED( hRes ) && m_pDevice11->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 )
		{
			oViewDesc.Flags = uiReadOnlyFlags;

			if ( FAILED( m_pDevice11->CreateDepthStencilView( pDepthTexture, &oViewDesc, &a_rTarget.pReadOnlyView ) ) )
				a_rTarget.pReadOnlyView = TNULL;
		}

		pDepthTexture->Release();
	}

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Failed to create the depth buffer!" );
		DestroyDepthTarget( a_rTarget );
		return TFALSE;
	}

	return TTRUE;
}

void TRenderD3DInterface::DestroyDepthTarget( DEPTHTARGET& a_rTarget )
{
	if ( m_oCurrent.pDepthStencilView == a_rTarget.pView || m_oCurrent.pDepthStencilView == a_rTarget.pReadOnlyView )
		m_oCurrent.pDepthStencilView = TNULL;

	if ( a_rTarget.pSRV )
	{
		InvalidateShaderResource( a_rTarget.pSRV );
		a_rTarget.pSRV->Release();
	}

	if ( a_rTarget.pReadOnlyView )
		a_rTarget.pReadOnlyView->Release();

	if ( a_rTarget.pView )
		a_rTarget.pView->Release();

	a_rTarget = {};
}

void TRenderD3DInterface::SetSceneTargetFormat( DXGI_FORMAT a_eFormat, UINT a_uiSampleCount )
{
	TASSERT( !m_bSceneTargetActive );

	if ( a_eFormat != m_eSceneFormat || a_uiSampleCount != m_uiSceneSampleCount )
	{
		m_eSceneFormat       = a_eFormat;
		m_uiSceneSampleCount = TMath::Max( a_uiSampleCount, 1u );
		DestroySceneTarget();
	}
}

TBOOL TRenderD3DInterface::BeginSceneTarget()
{
	TASSERT( IsInScene() && !m_bSceneTargetActive );

	if ( !m_pSceneTexture && !CreateSceneTarget() )
		return TFALSE;

	m_bSceneTargetActive = TTRUE;
	SetRenderTargets( m_pSceneRTView, m_oSceneDepth.pView );
	return TTRUE;
}

void TRenderD3DInterface::EndSceneTarget()
{
	TASSERT( m_bSceneTargetActive );

	m_bSceneTargetActive = TFALSE;
	SetRenderTargets( m_pBackBufferRTView, m_oDepth.pView );
}

TBOOL TRenderD3DInterface::CreateSceneTarget()
{
	D3D11_TEXTURE2D_DESC oDesc;
	m_pBackBuffer->GetDesc( &oDesc );
	oDesc.Format             = ( m_eSceneFormat != DXGI_FORMAT_UNKNOWN ) ? m_eSceneFormat : oDesc.Format;
	oDesc.SampleDesc.Count   = m_uiSceneSampleCount;
	oDesc.SampleDesc.Quality = 0;
	oDesc.Usage              = D3D11_USAGE_DEFAULT;
	oDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	oDesc.CPUAccessFlags     = 0;
	oDesc.MiscFlags          = 0;

	if ( FAILED( m_pDevice11->CreateTexture2D( &oDesc, TNULL, &m_pSceneTexture ) ) ||
	     FAILED( m_pDevice11->CreateRenderTargetView( m_pSceneTexture, TNULL, &m_pSceneRTView ) ) ||
	     FAILED( m_pDevice11->CreateShaderResourceView( m_pSceneTexture, TNULL, &m_pSceneSRV ) ) ||
	     !CreateDepthTarget( m_uiSceneSampleCount, m_oSceneDepth ) )
	{
		TERROR( "Couldn't create the scene target\n" );
		DestroySceneTarget();
		return TFALSE;
	}

	return TTRUE;
}

void TRenderD3DInterface::DestroySceneTarget()
{
	if ( m_pSceneSRV )
	{
		InvalidateShaderResource( m_pSceneSRV );
		m_pSceneSRV->Release();
		m_pSceneSRV = TNULL;
	}

	if ( m_pSceneRTView )
	{
		if ( m_oCurrent.pRenderTargetView == m_pSceneRTView )
			m_oCurrent.pRenderTargetView = TNULL;

		m_pSceneRTView->Release();
		m_pSceneRTView = TNULL;
	}

	if ( m_pSceneTexture )
	{
		m_pSceneTexture->Release();
		m_pSceneTexture = TNULL;
	}

	DestroyDepthTarget( m_oSceneDepth );
	m_bSceneTargetActive = TFALSE;
}

void TRenderD3DInterface::DestroyBackBufferViews()
{
	// The views are released right away, so they can't wait for the next draw to be unbound
	SetRenderTargets( TNULL, TNULL );
	SetSecondaryRenderTarget( TNULL, TNULL );

	if ( m_pDeviceContext )
	{
		m_pDeviceContext->OMSetRenderTargets( 0, TNULL, TNULL );
		m_oApplied.pRenderTargetView          = TNULL;
		m_oApplied.pSecondaryRenderTargetView = TNULL;
		m_oApplied.pDepthStencilView          = TNULL;
	}

	DestroySceneTarget();
	DestroyDepthTarget( m_oDepth );

	if ( m_pBackBufferRTView )
	{
		m_pBackBufferRTView->Release();
		m_pBackBufferRTView = TNULL;
	}

	if ( m_pBackBuffer )
	{
		m_pBackBuffer->Release();
		m_pBackBuffer = TNULL;
	}
}

ID3D11SamplerState* TRenderD3DInterface::GetSamplerState( TEXTUREFILTER a_eMinFilter, TEXTUREFILTER a_eMagFilter, TEXTUREFILTER a_eMipFilter, ADDRESSINGMODE a_eAddressU, ADDRESSINGMODE a_eAddressV )
{
	ID3D11SamplerState*& rpSamplerState = m_apSamplerStates[ a_eMinFilter ][ a_eMagFilter ][ a_eMipFilter ][ a_eAddressU ][ a_eAddressV ];

	if ( !rpSamplerState )
	{
		static constexpr D3D11_TEXTURE_ADDRESS_MODE s_aAddressModes[ ADDRESSINGMODE_NUMOF ] = {
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_MIRROR,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_BORDER,
		};

		auto GetFilterType = []( TEXTUREFILTER a_eFilter ) { return a_eFilter == TEXTUREFILTER_LINEAR ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT; };

		D3D11_SAMPLER_DESC oDesc = {};
		oDesc.Filter             = D3D11_FILTER( D3D11_ENCODE_BASIC_FILTER( GetFilterType( a_eMinFilter ), GetFilterType( a_eMagFilter ), GetFilterType( a_eMipFilter ), D3D11_FILTER_REDUCTION_TYPE_STANDARD ) );
		oDesc.AddressU           = s_aAddressModes[ a_eAddressU ];
		oDesc.AddressV           = s_aAddressModes[ a_eAddressV ];
		oDesc.AddressW           = s_aAddressModes[ a_eAddressU ];
		oDesc.MaxAnisotropy      = 1;
		oDesc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
		oDesc.MinLOD             = 0.0f;
		oDesc.MaxLOD             = ( a_eMipFilter == TEXTUREFILTER_NONE ) ? 0.0f : D3D11_FLOAT32_MAX;

		HRESULT hRes = m_pDevice11->CreateSamplerState( &oDesc, &rpSamplerState );
		TASSERT( SUCCEEDED( hRes ) );
	}

	return rpSamplerState;
}

void TRenderD3DInterface::CreateDeviceObjects()
{
	for ( TUINT32 i = 0; i < NUMCONSTANTBUFFERS; i++ )
		m_aConstantBuffers[ i ] = CreateBuffer( D3D11_BIND_CONSTANT_BUFFER, CONSTANT_BUFFER_SIZE, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

	m_pConstantBufferData = TSTATICCAST( TUINT8, TMemalign( CONSTANT_BUFFER_SIZE, 16 ) );
	TUtil::MemClear( m_pConstantBufferData, CONSTANT_BUFFER_SIZE );
	m_uiConstantBufferSize   = 0;
	m_bConstantBufferChanged = TFALSE;
	m_uiConstantBufferIndex  = 0;

	m_pImmediateVertexBuffer         = CreateBuffer( D3D11_BIND_VERTEX_BUFFER, IMMEDIATE_VERTEX_BUFFER_SIZE, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
	m_uiImmediateVertexCurrentOffset = 0;
	m_pImmediateIndexBuffer          = CreateBuffer( D3D11_BIND_INDEX_BUFFER, IMMEDIATE_INDEX_BUFFER_SIZE, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
	m_uiImmediateIndexCurrentOffset  = 0;

	ID3DBlob* pVSBlob = CompileShader( s_szFixedFunctionShader, "vs_main", "vs_4_0", TNULL );
	ID3DBlob* pPSBlob = CompileShader( s_szFixedFunctionShader, "ps_main", "ps_4_0", TNULL );
	CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pFixedFunctionVS );
	CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pFixedFunctionPS );

	const D3D11_INPUT_ELEMENT_DESC aFixedFunctionLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_pDevice11->CreateInputLayout( aFixedFunctionLayout, TARRAYSIZE( aFixedFunctionLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pFixedFunctionLayout );

	// Vertices without texture coordinates read (0, 0) from the second stream like on D3D8
	const D3D11_INPUT_ELEMENT_DESC aPositionColourLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 0 },
	};

	m_pDevice11->CreateInputLayout( aPositionColourLayout, TARRAYSIZE( aPositionColourLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pPositionColourLayout );

	static constexpr TFLOAT s_afZeroTexCoord[ 2 ] = { 0.0f, 0.0f };
	m_pZeroTexCoordBuffer = CreateBuffer( D3D11_BIND_VERTEX_BUFFER, sizeof( s_afZeroTexCoord ), s_afZeroTexCoord, D3D11_USAGE_IMMUTABLE, 0 );
	pVSBlob->Release();
	pPSBlob->Release();

	m_CurrentDepth.Parts.bDepthEnable            = TRUE;
	m_CurrentDepth.Parts.DepthWriteMask          = D3D11_DEPTH_WRITE_MASK_ALL;
	m_CurrentDepth.Parts.DepthFunc               = D3D11_COMPARISON_LESS_EQUAL;
	m_CurrentDepth.Parts.bStencilEnable          = FALSE;
	m_CurrentDepth.Parts.StencilReadMask         = 0xFF;
	m_CurrentDepth.Parts.StencilWriteMask        = 0xFF;
	m_CurrentDepth.Parts.FrontStencilFailOp      = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.Parts.FrontStencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.Parts.FrontStencilPassOp      = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.Parts.FrontStencilFunc        = D3D11_COMPARISON_ALWAYS;
	m_CurrentDepth.Parts.BackStencilFailOp       = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.Parts.BackStencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.Parts.BackStencilPassOp       = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.Parts.BackStencilFunc         = D3D11_COMPARISON_ALWAYS;
	m_uiCurrentStencilRef                        = 0;

	m_CurrentBlendState.Parts.BlendOp               = D3D11_BLEND_OP_ADD;
	m_CurrentBlendState.Parts.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	m_CurrentBlendState.Parts.SrcBlendAlpha         = D3D11_BLEND_ONE;
	m_CurrentBlendState.Parts.DestBlendAlpha        = D3D11_BLEND_ZERO;
	m_CurrentBlendState.Parts.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_CurrentBlendState.Parts.bBlendEnable          = FALSE;
	m_CurrentBlendState.Parts.SrcBlend              = D3D11_BLEND_ONE;
	m_CurrentBlendState.Parts.DestBlend             = D3D11_BLEND_ZERO;

	m_CurrentRasterizer.Flags.Parts.FillMode               = D3D11_FILL_SOLID;
	m_CurrentRasterizer.Flags.Parts.CullMode               = D3D11_CULL_NONE;
	m_CurrentRasterizer.Flags.Parts.bFrontCounterClockwise = FALSE;
	m_CurrentRasterizer.Flags.Parts.bDepthClipEnable       = TRUE;
	m_CurrentRasterizer.Flags.Parts.bScissorEnable         = FALSE;
	m_CurrentRasterizer.Flags.Parts.bMultisampleEnable     = FALSE;
	m_CurrentRasterizer.DepthBias                          = 0;
	m_CurrentRasterizer.SlopeScaledDepthBias               = 0.0f;

	ResetStateCache();
}

void TRenderD3DInterface::DestroyDeviceObjects()
{
	ID3D11SamplerState** ppSamplerStates = &m_apSamplerStates[ 0 ][ 0 ][ 0 ][ 0 ][ 0 ];

	for ( TUINT32 i = 0; i < sizeof( m_apSamplerStates ) / sizeof( *ppSamplerStates ); i++ )
	{
		if ( ppSamplerStates[ i ] )
		{
			ppSamplerStates[ i ]->Release();
			ppSamplerStates[ i ] = TNULL;
		}
	}

	for ( TUINT32 i = 0; i < NUMCONSTANTBUFFERS; i++ )
	{
		if ( m_aConstantBuffers[ i ] )
		{
			m_aConstantBuffers[ i ]->Release();
			m_aConstantBuffers[ i ] = TNULL;
		}
	}

	if ( m_pConstantBufferData )
	{
		TFree( m_pConstantBufferData );
		m_pConstantBufferData = TNULL;
	}

	if ( m_pImmediateVertexBuffer )
	{
		m_pImmediateVertexBuffer->Release();
		m_pImmediateVertexBuffer = TNULL;
	}

	if ( m_pFixedFunctionVS )
	{
		m_pFixedFunctionVS->Release();
		m_pFixedFunctionVS = TNULL;
	}

	if ( m_pFixedFunctionPS )
	{
		m_pFixedFunctionPS->Release();
		m_pFixedFunctionPS = TNULL;
	}

	if ( m_pFixedFunctionLayout )
	{
		m_pFixedFunctionLayout->Release();
		m_pFixedFunctionLayout = TNULL;
	}

	if ( m_pPositionColourLayout )
	{
		m_pPositionColourLayout->Release();
		m_pPositionColourLayout = TNULL;
	}

	if ( m_pZeroTexCoordBuffer )
	{
		m_pZeroTexCoordBuffer->Release();
		m_pZeroTexCoordBuffer = TNULL;
	}

	if ( m_pImmediateIndexBuffer )
	{
		m_pImmediateIndexBuffer->Release();
		m_pImmediateIndexBuffer = TNULL;
	}

	for ( auto it = m_DepthStates.Begin(); it != m_DepthStates.End(); it++ )
		it->second->Release();

	for ( auto it = m_RasterizerStates.Begin(); it != m_RasterizerStates.End(); it++ )
		it->second->Release();

	for ( auto it = m_BlendStates.Begin(); it != m_BlendStates.End(); it++ )
		it->second->Release();

	m_DepthStates.Clear();
	m_RasterizerStates.Clear();
	m_BlendStates.Clear();
}

ID3D11ShaderResourceView* TRenderD3DInterface::CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, DXGI_FORMAT a_eFormat, const void* a_pSrcData, TUINT8 a_uiFlags, D3D11_USAGE a_eUsage, TUINT32 a_uiCPUAccessFlags, TUINT32 a_uiSampleCount )
{
	ID3D11ShaderResourceView* pShaderResourceView = TNULL;

	const TBOOL bMipmapsFromData = HASANYFLAG( a_uiFlags, TEXTUREFLAGS_MIPMAPSFROMDATA );
	const TBOOL bGenerateMipmaps = !bMipmapsFromData && HASANYFLAG( a_uiFlags, TEXTUREFLAGS_GENERATEMIPMAPS );
	TUINT32     uiNumMipmaps     = 1;

	if ( bMipmapsFromData || bGenerateMipmaps )
	{
		const TUINT32 uiMaxSize = TMath::Max( a_uiWidth, a_uiHeight );
		uiNumMipmaps            = TUINT32( floor( log2( (double)uiMaxSize ) ) + 1.0 );
	}

	D3D11_TEXTURE2D_DESC oDesc = {};
	oDesc.Width                = a_uiWidth;
	oDesc.Height               = a_uiHeight;
	oDesc.MipLevels            = bGenerateMipmaps ? 0 : uiNumMipmaps;
	oDesc.ArraySize            = 1;
	oDesc.Format               = a_eFormat;
	oDesc.SampleDesc.Count     = a_uiSampleCount;
	oDesc.SampleDesc.Quality   = 0;
	oDesc.Usage                = a_eUsage;
	oDesc.BindFlags            = ( HASANYFLAG( a_uiFlags, TEXTUREFLAGS_RENDERTARGET ) || bGenerateMipmaps ) ? ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET ) : D3D11_BIND_SHADER_RESOURCE;
	oDesc.CPUAccessFlags       = a_uiCPUAccessFlags;
	oDesc.MiscFlags            = bGenerateMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	ID3D11Texture2D* pTexture = TNULL;

	if ( a_pSrcData == TNULL )
	{
		m_pDevice11->CreateTexture2D( &oDesc, TNULL, &pTexture );
	}
	else if ( bGenerateMipmaps )
	{
		m_pDevice11->CreateTexture2D( &oDesc, TNULL, &pTexture );
		if ( pTexture == TNULL ) return TNULL;

		m_pDeviceContext->UpdateSubresource( pTexture, 0, TNULL, a_pSrcData, GetTextureRowPitch( a_eFormat, a_uiWidth ), GetTextureDepthPitch( a_eFormat, a_uiWidth, a_uiHeight ) );
	}
	else
	{
		TASSERT( uiNumMipmaps <= MAXIMUM_NUMBER_OF_MIPMAPS );

		D3D11_SUBRESOURCE_DATA aSubresources[ MAXIMUM_NUMBER_OF_MIPMAPS ];
		const TUINT8*          pSrcData = TSTATICCAST( const TUINT8, a_pSrcData );
		TUINT32                uiWidth  = a_uiWidth;
		TUINT32                uiHeight = a_uiHeight;

		for ( TUINT32 i = 0; i < uiNumMipmaps; i++ )
		{
			aSubresources[ i ].pSysMem          = pSrcData;
			aSubresources[ i ].SysMemPitch      = GetTextureRowPitch( a_eFormat, uiWidth );
			aSubresources[ i ].SysMemSlicePitch = GetTextureDepthPitch( a_eFormat, uiWidth, uiHeight );
			pSrcData += aSubresources[ i ].SysMemSlicePitch;

			uiWidth  = TMath::Max( uiWidth >> 1, 1U );
			uiHeight = TMath::Max( uiHeight >> 1, 1U );
		}

		m_pDevice11->CreateTexture2D( &oDesc, aSubresources, &pTexture );
	}

	if ( pTexture != TNULL )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC oSRVDesc = {};
		oSRVDesc.Format                          = oDesc.Format;
		oSRVDesc.ViewDimension                   = ( a_uiSampleCount > 1 ) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		oSRVDesc.Texture2D.MostDetailedMip       = 0;
		oSRVDesc.Texture2D.MipLevels             = bGenerateMipmaps ? -1 : uiNumMipmaps;
		m_pDevice11->CreateShaderResourceView( pTexture, &oSRVDesc, &pShaderResourceView );

		if ( bGenerateMipmaps && pShaderResourceView )
			m_pDeviceContext->GenerateMips( pShaderResourceView );

		pTexture->Release();
	}

	return pShaderResourceView;
}

ID3D11ShaderResourceView* TRenderD3DInterface::CreateTextureFromFileInMemory( const void* a_pData, TUINT32 a_uiDataSize, D3D11_TEXTURE2D_DESC* a_pDesc )
{
	DirectX::TexMetadata  oMetadata;
	DirectX::ScratchImage oImage;
	HRESULT               hRes;

	if ( a_uiDataSize >= 4 && TUtil::MemCompare( a_pData, "DDS ", 4 ) == 0 )
		hRes = DirectX::LoadFromDDSMemory( TSTATICCAST( const uint8_t, a_pData ), a_uiDataSize, DirectX::DDS_FLAGS_EXPAND_LUMINANCE, &oMetadata, oImage );
	else
		hRes = DirectX::LoadFromTGAMemory( TSTATICCAST( const uint8_t, a_pData ), a_uiDataSize, DirectX::TGA_FLAGS_BGR, &oMetadata, oImage );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Unable to load a texture!" );
		return TNULL;
	}

	// The whole mip chain, like D3DX
	if ( oMetadata.mipLevels == 1 && !DirectX::IsCompressed( oMetadata.format ) && ( oMetadata.width > 1 || oMetadata.height > 1 ) )
	{
		DirectX::ScratchImage oMipChain;

		if ( SUCCEEDED( DirectX::GenerateMipMaps( oImage.GetImages(), oImage.GetImageCount(), oMetadata, DirectX::TEX_FILTER_DEFAULT, 0, oMipChain ) ) )
		{
			oImage    = std::move( oMipChain );
			oMetadata = oImage.GetMetadata();
		}
	}

	ID3D11ShaderResourceView* pShaderResourceView = TNULL;
	hRes                                          = DirectX::CreateShaderResourceView( m_pDevice11, oImage.GetImages(), oImage.GetImageCount(), oMetadata, &pShaderResourceView );

	if ( FAILED( hRes ) )
	{
		PrintError( hRes, "Unable to create a texture!" );
		return TNULL;
	}

	if ( a_pDesc )
	{
		ID3D11Texture2D* pTexture = TNULL;
		pShaderResourceView->GetResource( (ID3D11Resource**)&pTexture );
		pTexture->GetDesc( a_pDesc );
		pTexture->Release();
	}

	return pShaderResourceView;
}

TUINT32 TRenderD3DInterface::GetTextureRowPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth )
{
	switch ( a_eFormat )
	{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return a_uiWidth * 16;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return a_uiWidth * 8;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_D32_FLOAT:
			return a_uiWidth * 4;
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return a_uiWidth * 2;
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R8_UNORM:
			return a_uiWidth;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC4_UNORM:
			return ( ( a_uiWidth + 3 ) >> 2 ) * 8;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC5_UNORM:
			return ( ( a_uiWidth + 3 ) >> 2 ) * 16;
		default:
			TASSERT( TFALSE );
			return 0;
	}
}

TUINT32 TRenderD3DInterface::GetTextureDepthPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth, TUINT32 a_uiHeight )
{
	switch ( a_eFormat )
	{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC5_UNORM:
			return GetTextureRowPitch( a_eFormat, a_uiWidth ) * ( ( a_uiHeight + 3 ) >> 2 );
		default:
			return GetTextureRowPitch( a_eFormat, a_uiWidth ) * a_uiHeight;
	}
}

ID3D11Buffer* TRenderD3DInterface::CreateBuffer( UINT a_uiBindFlags, UINT a_uiDataSize, const void* a_pData, D3D11_USAGE a_eUsage, UINT a_uiCPUAccessFlags )
{
	D3D11_BUFFER_DESC oDesc   = {};
	oDesc.ByteWidth           = a_uiDataSize;
	oDesc.Usage               = a_eUsage;
	oDesc.BindFlags           = a_uiBindFlags;
	oDesc.CPUAccessFlags      = a_uiCPUAccessFlags;
	oDesc.MiscFlags           = 0;
	oDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA oData = {};
	oData.pSysMem                = a_pData;

	ID3D11Buffer* pBuffer = TNULL;
	HRESULT       hRes    = m_pDevice11->CreateBuffer( &oDesc, a_pData ? &oData : TNULL, &pBuffer );

	if ( FAILED( hRes ) )
		PrintError( hRes, "Failed to create a buffer!" );

	return pBuffer;
}

void TRenderD3DInterface::PrepareDraw()
{
	if ( m_bFixedFunction )
		UpdateFixedFunctionConstants();

	UpdateRenderStates();
	FlushConstantBuffers();
	ApplyDesiredState();
}

void TRenderD3DInterface::DrawIndexedInstanced( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, UINT a_uiInstanceCount, ID3D11Buffer* a_pIndexBuffer, UINT a_uiIndexBufferOffset, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset, UINT a_uiStartInstance )
{
	SetVertexBuffer( 0, a_pVertexBuffer, a_uiStride, a_uiOffset );
	SetIndexBuffer( a_pIndexBuffer, a_eIndexFormat, a_uiIndexBufferOffset );
	SetPrimitiveTopology( a_eTopology );
	PrepareDraw();
	m_pDeviceContext->DrawIndexedInstanced( a_uiIndexCount, a_uiInstanceCount, 0, 0, a_uiStartInstance );
}

void TRenderD3DInterface::DrawIndexedInstancedIndirect( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, ID3D11Buffer* a_pArgsBuffer, UINT a_uiArgsOffset )
{
	SetPrimitiveTopology( a_eTopology );
	PrepareDraw();
	m_pDeviceContext->DrawIndexedInstancedIndirect( a_pArgsBuffer, a_uiArgsOffset );
}

void TRenderD3DInterface::Dispatch( UINT a_uiThreadGroupsX, UINT a_uiThreadGroupsY, UINT a_uiThreadGroupsZ )
{
	if ( m_uiCSConstantBufferSlots )
		UploadConstantBufferSlots( m_apCSConstantBuffers, m_oCurrent.apCSConstantBuffers, m_uiCSConstantBufferSlots );

	ApplyDesiredState();
	m_pDeviceContext->Dispatch( a_uiThreadGroupsX, a_uiThreadGroupsY, a_uiThreadGroupsZ );
}

void TRenderD3DInterface::DispatchIndirect( ID3D11Buffer* a_pArgsBuffer, UINT a_uiArgsOffset )
{
	if ( m_uiCSConstantBufferSlots )
		UploadConstantBufferSlots( m_apCSConstantBuffers, m_oCurrent.apCSConstantBuffers, m_uiCSConstantBufferSlots );

	ApplyDesiredState();
	m_pDeviceContext->DispatchIndirect( a_pArgsBuffer, a_uiArgsOffset );
}

void TRenderD3DInterface::ClearUnorderedAccessViewUint( ID3D11UnorderedAccessView* a_pView, UINT a_uiValue )
{
	const UINT auiValues[ 4 ] = { a_uiValue, a_uiValue, a_uiValue, a_uiValue };
	m_pDeviceContext->ClearUnorderedAccessViewUint( a_pView, auiValues );
}

TBOOL TRenderD3DInterface::CreateStructuredBuffer( UINT a_uiStride, UINT a_uiCount, const void* a_pData, TBOOL a_bDynamic, ID3D11Buffer** a_ppBuffer, ID3D11ShaderResourceView** a_ppSRV )
{
	D3D11_BUFFER_DESC oDesc   = {};
	oDesc.ByteWidth           = a_uiStride * a_uiCount;
	oDesc.Usage               = a_bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	oDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
	oDesc.CPUAccessFlags      = a_bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	oDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	oDesc.StructureByteStride = a_uiStride;

	D3D11_SUBRESOURCE_DATA oData = { a_pData, 0, 0 };

	if ( FAILED( m_pDevice11->CreateBuffer( &oDesc, a_pData ? &oData : TNULL, a_ppBuffer ) ) )
		return TFALSE;

	D3D11_SHADER_RESOURCE_VIEW_DESC oSRVDesc = {};
	oSRVDesc.Format                          = DXGI_FORMAT_UNKNOWN;
	oSRVDesc.ViewDimension                   = D3D11_SRV_DIMENSION_BUFFER;
	oSRVDesc.Buffer.NumElements              = a_uiCount;

	if ( FAILED( m_pDevice11->CreateShaderResourceView( *a_ppBuffer, &oSRVDesc, a_ppSRV ) ) )
	{
		( *a_ppBuffer )->Release();
		*a_ppBuffer = TNULL;
		return TFALSE;
	}

	return TTRUE;
}

TBOOL TRenderD3DInterface::CreateTypedUAVBuffer( DXGI_FORMAT a_eFormat, UINT a_uiElementSize, UINT a_uiCount, UINT a_uiExtraBindFlags, ID3D11Buffer** a_ppBuffer, ID3D11UnorderedAccessView** a_ppUAV, ID3D11ShaderResourceView** a_ppSRV )
{
	D3D11_BUFFER_DESC oDesc = {};
	oDesc.ByteWidth         = a_uiElementSize * a_uiCount;
	oDesc.Usage             = D3D11_USAGE_DEFAULT;
	oDesc.BindFlags         = D3D11_BIND_UNORDERED_ACCESS | ( a_ppSRV ? D3D11_BIND_SHADER_RESOURCE : 0 ) | a_uiExtraBindFlags;

	if ( FAILED( m_pDevice11->CreateBuffer( &oDesc, TNULL, a_ppBuffer ) ) )
		return TFALSE;

	D3D11_UNORDERED_ACCESS_VIEW_DESC oUAVDesc = {};
	oUAVDesc.Format                           = a_eFormat;
	oUAVDesc.ViewDimension                    = D3D11_UAV_DIMENSION_BUFFER;
	oUAVDesc.Buffer.NumElements               = a_uiCount;

	HRESULT hRes = m_pDevice11->CreateUnorderedAccessView( *a_ppBuffer, &oUAVDesc, a_ppUAV );

	if ( SUCCEEDED( hRes ) && a_ppSRV )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC oSRVDesc = {};
		oSRVDesc.Format                          = a_eFormat;
		oSRVDesc.ViewDimension                   = D3D11_SRV_DIMENSION_BUFFER;
		oSRVDesc.Buffer.NumElements              = a_uiCount;

		hRes = m_pDevice11->CreateShaderResourceView( *a_ppBuffer, &oSRVDesc, a_ppSRV );

		if ( FAILED( hRes ) )
		{
			( *a_ppUAV )->Release();
			*a_ppUAV = TNULL;
		}
	}

	if ( FAILED( hRes ) )
	{
		( *a_ppBuffer )->Release();
		*a_ppBuffer = TNULL;
		return TFALSE;
	}

	return TTRUE;
}

TBOOL TRenderD3DInterface::CreateIndirectArgsBuffer( UINT a_uiNumDraws, const void* a_pData, ID3D11Buffer** a_ppBuffer, ID3D11UnorderedAccessView** a_ppUAV )
{
	constexpr UINT ARGS_SIZE = sizeof( D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS );

	D3D11_BUFFER_DESC oDesc = {};
	oDesc.ByteWidth         = ARGS_SIZE * a_uiNumDraws;
	oDesc.Usage             = D3D11_USAGE_DEFAULT;
	oDesc.BindFlags         = D3D11_BIND_UNORDERED_ACCESS;
	oDesc.MiscFlags         = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	D3D11_SUBRESOURCE_DATA oData = { a_pData, 0, 0 };

	if ( FAILED( m_pDevice11->CreateBuffer( &oDesc, a_pData ? &oData : TNULL, a_ppBuffer ) ) )
		return TFALSE;

	D3D11_UNORDERED_ACCESS_VIEW_DESC oUAVDesc = {};
	oUAVDesc.Format                           = DXGI_FORMAT_R32_TYPELESS;
	oUAVDesc.ViewDimension                    = D3D11_UAV_DIMENSION_BUFFER;
	oUAVDesc.Buffer.NumElements               = oDesc.ByteWidth / 4;
	oUAVDesc.Buffer.Flags                     = D3D11_BUFFER_UAV_FLAG_RAW;

	if ( FAILED( m_pDevice11->CreateUnorderedAccessView( *a_ppBuffer, &oUAVDesc, a_ppUAV ) ) )
	{
		( *a_ppBuffer )->Release();
		*a_ppBuffer = TNULL;
		return TFALSE;
	}

	return TTRUE;
}

void TRenderD3DInterface::DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, ID3D11Buffer* a_pVertexBuffer, UINT a_uiVertexCount, UINT a_uiStride, UINT a_uiStartVertex, UINT a_uiOffset )
{
	SetVertexBuffer( 0, a_pVertexBuffer, a_uiStride, a_uiOffset );
	SetPrimitiveTopology( a_eTopology );
	PrepareDraw();
	m_pDeviceContext->Draw( a_uiVertexCount, a_uiStartVertex );
}

void TRenderD3DInterface::DrawIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, ID3D11Buffer* a_pIndexBuffer, UINT a_uiIndexBufferOffset, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset )
{
	SetVertexBuffer( 0, a_pVertexBuffer, a_uiStride, a_uiOffset );
	SetIndexBuffer( a_pIndexBuffer, a_eIndexFormat, a_uiIndexBufferOffset );
	SetPrimitiveTopology( a_eTopology );
	PrepareDraw();
	m_pDeviceContext->DrawIndexed( a_uiIndexCount, 0, 0 );
}

static TUINT32 CopyToImmediateBuffer( ID3D11DeviceContext* a_pDeviceContext, ID3D11Buffer* a_pBuffer, TUINT32 a_uiBufferSize, TUINT32& a_rCurrentOffset, const void* a_pData, TUINT32 a_uiDataSize )
{
	TASSERT( a_uiDataSize <= a_uiBufferSize );

	D3D11_MAP eMapType = D3D11_MAP_WRITE_NO_OVERWRITE;

	if ( a_rCurrentOffset + a_uiDataSize > a_uiBufferSize || a_rCurrentOffset == 0 )
	{
		a_rCurrentOffset = 0;
		eMapType         = D3D11_MAP_WRITE_DISCARD;
	}

	D3D11_MAPPED_SUBRESOURCE oMapped;

	if ( SUCCEEDED( a_pDeviceContext->Map( a_pBuffer, 0, eMapType, 0, &oMapped ) ) )
	{
		TUtil::MemCopy( TSTATICCAST( TUINT8, oMapped.pData ) + a_rCurrentOffset, a_pData, a_uiDataSize );
		a_pDeviceContext->Unmap( a_pBuffer, 0 );
	}

	const TUINT32 uiOffset = a_rCurrentOffset;
	a_rCurrentOffset += a_uiDataSize;

	return uiOffset;
}

void TRenderD3DInterface::DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, const void* a_pVertices, UINT a_uiVertexCount, UINT a_uiStride )
{
	UINT uiOffset = CopyToImmediateBuffer( m_pDeviceContext, m_pImmediateVertexBuffer, IMMEDIATE_VERTEX_BUFFER_SIZE, m_uiImmediateVertexCurrentOffset, a_pVertices, a_uiStride * a_uiVertexCount );
	DrawNonIndexed( a_eTopology, m_pImmediateVertexBuffer, a_uiVertexCount, a_uiStride, 0, uiOffset );
}

void TRenderD3DInterface::DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, const void* a_pIndices, DXGI_FORMAT a_eIndexFormat, const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount )
{
	const TUINT32 uiIndexSize    = ( a_eIndexFormat == DXGI_FORMAT_R32_UINT ) ? 4 : 2;
	UINT          uiIndexOffset  = CopyToImmediateBuffer( m_pDeviceContext, m_pImmediateIndexBuffer, IMMEDIATE_INDEX_BUFFER_SIZE, m_uiImmediateIndexCurrentOffset, a_pIndices, uiIndexSize * a_uiIndexCount );
	UINT          uiVertexOffset = CopyToImmediateBuffer( m_pDeviceContext, m_pImmediateVertexBuffer, IMMEDIATE_VERTEX_BUFFER_SIZE, m_uiImmediateVertexCurrentOffset, a_pVertices, a_uiStride * a_uiVertexCount );
	DrawIndexed( a_eTopology, a_uiIndexCount, m_pImmediateIndexBuffer, uiIndexOffset, a_eIndexFormat, m_pImmediateVertexBuffer, a_uiStride, uiVertexOffset );
}

void TRenderD3DInterface::SetVec4InVSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount )
{
	const TUINT32 uiOffset = a_iIndex * sizeof( TVector4 );
	const TUINT32 uiSize   = a_iCount * sizeof( TVector4 );
	TASSERT( uiOffset + uiSize <= CONSTANT_BUFFER_SIZE );

	TUINT8* pDst = m_pConstantBufferData + uiOffset;

	if ( m_bConstantBufferChanged || TUtil::MemCompare( pDst, a_pSrc, uiSize ) != 0 )
	{
		TUtil::MemCopy( pDst, a_pSrc, uiSize );
		m_bConstantBufferChanged = TTRUE;
	}

	m_uiConstantBufferSize = TMath::Max( m_uiConstantBufferSize, uiOffset + uiSize );
}

void TRenderD3DInterface::FlushConstantBuffers()
{
	if ( m_bConstantBufferChanged )
	{
		m_uiConstantBufferIndex = ( m_uiConstantBufferIndex + 1 ) % NUMCONSTANTBUFFERS;
		ID3D11Buffer* pBuffer   = m_aConstantBuffers[ m_uiConstantBufferIndex ];

		D3D11_MAPPED_SUBRESOURCE oMapped;

		if ( SUCCEEDED( m_pDeviceContext->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped ) ) )
		{
			TUtil::MemCopy( oMapped.pData, m_pConstantBufferData, m_uiConstantBufferSize );
			m_pDeviceContext->Unmap( pBuffer, 0 );
		}

		m_bConstantBufferChanged = TFALSE;
	}

	ID3D11Buffer* pBuffer = m_aConstantBuffers[ m_uiConstantBufferIndex ];
	m_oCurrent.apVSConstantBuffers[ 0 ] = pBuffer;
	m_oCurrent.apPSConstantBuffers[ 0 ] = pBuffer;

	if ( m_uiVSConstantBufferSlots )
		UploadConstantBufferSlots( m_apVSConstantBuffers, m_oCurrent.apVSConstantBuffers, m_uiVSConstantBufferSlots );

	if ( m_uiPSConstantBufferSlots )
		UploadConstantBufferSlots( m_apPSConstantBuffers, m_oCurrent.apPSConstantBuffers, m_uiPSConstantBufferSlots );
}

void TRenderD3DInterface::UploadConstantBufferSlots( TConstantBufferGeneric** a_ppBuffers, ID3D11Buffer** a_ppCurrent, TUINT32 a_uiSlots )
{
	for ( TUINT i = 0; a_uiSlots != 0; i++ )
	{
		if ( !( a_uiSlots & ( 1 << i ) ) )
			continue;

		a_uiSlots &= ~( 1 << i );
		a_ppBuffers[ i ]->Upload();
		a_ppCurrent[ i ] = a_ppBuffers[ i ]->GetBuffer();
	}
}

void TRenderD3DInterface::SetRenderTargets( ID3D11RenderTargetView* a_pRenderTargetView, ID3D11DepthStencilView* a_pDepthStencilView )
{
	m_oCurrent.pRenderTargetView = a_pRenderTargetView;
	m_oCurrent.pDepthStencilView = a_pDepthStencilView;
}

void TRenderD3DInterface::SetVertexBuffer( UINT a_uiSlot, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset )
{
	TASSERT( a_uiSlot < MAX_VERTEX_STREAMS );

	m_oCurrent.apVertexBuffers[ a_uiSlot ]  = a_pVertexBuffer;
	m_oCurrent.auiVertexStrides[ a_uiSlot ] = a_uiStride;
	m_oCurrent.auiVertexOffsets[ a_uiSlot ] = a_uiOffset;
}

void TRenderD3DInterface::SetIndexBuffer( ID3D11Buffer* a_pIndexBuffer, DXGI_FORMAT a_eFormat, UINT a_uiOffset )
{
	m_oCurrent.pIndexBuffer  = a_pIndexBuffer;
	m_oCurrent.eIndexFormat  = a_eFormat;
	m_oCurrent.uiIndexOffset = a_uiOffset;
}

void TRenderD3DInterface::VSSetShaderResource( UINT a_uiSlot, ID3D11ShaderResourceView* a_pView )
{
	TASSERT( a_uiSlot < MAX_SRV_SLOTS );
	m_oCurrent.apVSShaderResources[ a_uiSlot ] = a_pView;
}

void TRenderD3DInterface::PSSetShaderResource( UINT a_uiSlot, ID3D11ShaderResourceView* a_pView )
{
	TASSERT( a_uiSlot < MAX_SRV_SLOTS );
	m_oCurrent.apPSShaderResources[ a_uiSlot ] = a_pView;
}

void TRenderD3DInterface::VSSetSampler( UINT a_uiSlot, ID3D11SamplerState* a_pSampler )
{
	TASSERT( a_uiSlot < MAX_SAMPLER_SLOTS );
	m_oCurrent.apVSSamplers[ a_uiSlot ] = a_pSampler;
}

void TRenderD3DInterface::PSSetSampler( UINT a_uiSlot, ID3D11SamplerState* a_pSampler )
{
	TASSERT( a_uiSlot < MAX_SAMPLER_SLOTS );
	m_oCurrent.apPSSamplers[ a_uiSlot ] = a_pSampler;

	// It stays until the texture stage changes again
	if ( a_uiSlot < NUM_TEXTURE_STAGES )
		m_uiDirtySamplers &= ~( 1u << a_uiSlot );
}

void TRenderD3DInterface::CSSetShaderResource( UINT a_uiSlot, ID3D11ShaderResourceView* a_pView )
{
	TASSERT( a_uiSlot < MAX_SRV_SLOTS );
	m_oCurrent.apCSShaderResources[ a_uiSlot ] = a_pView;
}

void TRenderD3DInterface::CSSetUnorderedAccessView( UINT a_uiSlot, ID3D11UnorderedAccessView* a_pView )
{
	TASSERT( a_uiSlot < MAX_UAV_SLOTS );
	m_oCurrent.apCSUnorderedAccessViews[ a_uiSlot ] = a_pView;
}

void TRenderD3DInterface::CSSetSampler( UINT a_uiSlot, ID3D11SamplerState* a_pSampler )
{
	TASSERT( a_uiSlot < MAX_SAMPLER_SLOTS );
	m_oCurrent.apCSSamplers[ a_uiSlot ] = a_pSampler;
}

void TRenderD3DInterface::SetCSConstantBuffer( UINT a_uiSlot, TConstantBufferGeneric* a_pBuffer )
{
	TASSERT( a_uiSlot < MAX_CONSTANT_BUFFER_SLOTS );

	m_apCSConstantBuffers[ a_uiSlot ] = a_pBuffer;

	if ( a_pBuffer )
	{
		m_uiCSConstantBufferSlots |= 1 << a_uiSlot;
	}
	else
	{
		m_uiCSConstantBufferSlots &= ~( 1 << a_uiSlot );
		m_oCurrent.apCSConstantBuffers[ a_uiSlot ] = TNULL;
	}
}

void TRenderD3DInterface::UnbindComputeResources()
{
	TUtil::MemClear( m_oCurrent.apCSShaderResources, sizeof( m_oCurrent.apCSShaderResources ) );
	TUtil::MemClear( m_oCurrent.apCSUnorderedAccessViews, sizeof( m_oCurrent.apCSUnorderedAccessViews ) );
}

void TRenderD3DInterface::InvalidateUnorderedAccessView( ID3D11UnorderedAccessView* a_pView )
{
	if ( !a_pView )
		return;

	for ( TUINT i = 0; i < MAX_UAV_SLOTS; i++ )
	{
		if ( m_oCurrent.apCSUnorderedAccessViews[ i ] == a_pView )
			m_oCurrent.apCSUnorderedAccessViews[ i ] = TNULL;
	}
}

void TRenderD3DInterface::InvalidateShaderResource( ID3D11ShaderResourceView* a_pView )
{
	if ( !a_pView )
		return;

	for ( TUINT i = 0; i < MAX_SRV_SLOTS; i++ )
	{
		if ( m_oCurrent.apVSShaderResources[ i ] == a_pView ) m_oCurrent.apVSShaderResources[ i ] = TNULL;
		if ( m_oCurrent.apPSShaderResources[ i ] == a_pView ) m_oCurrent.apPSShaderResources[ i ] = TNULL;
		if ( m_oCurrent.apCSShaderResources[ i ] == a_pView ) m_oCurrent.apCSShaderResources[ i ] = TNULL;
	}
}

// Sends the slots that differ in one call covering the first to the last of them
template <class T, class Setter>
static void MergeSlots( T* a_pApplied, T* const a_pCurrent, UINT a_uiNumSlots, TBOOL a_bForce, Setter a_fnSet )
{
	UINT uiFirst = a_uiNumSlots;
	UINT uiLast  = 0;

	for ( UINT i = 0; i < a_uiNumSlots; i++ )
	{
		if ( a_bForce || a_pApplied[ i ] != a_pCurrent[ i ] )
		{
			uiFirst       = TMath::Min( uiFirst, i );
			uiLast        = i;
			a_pApplied[ i ] = a_pCurrent[ i ];
		}
	}

	if ( uiFirst <= uiLast && uiFirst < a_uiNumSlots )
		a_fnSet( uiFirst, uiLast - uiFirst + 1, &a_pCurrent[ uiFirst ] );
}

// Unbinds the views on their way to null before anything else is bound, a resource can't
// be read and written at once and D3D11 silently drops one of the bindings otherwise
template <class T, class Setter>
static void MergeReleasedViews( T** a_ppApplied, T* const* a_ppCurrent, UINT a_uiNumSlots, Setter a_fnSet )
{
	for ( UINT i = 0; i < a_uiNumSlots; i++ )
	{
		if ( a_ppApplied[ i ] && !a_ppCurrent[ i ] )
		{
			a_ppApplied[ i ] = TNULL;
			a_fnSet( i, 1, &a_ppCurrent[ i ] );
		}
	}
}

void TRenderD3DInterface::ApplyDesiredState()
{
	ID3D11DeviceContext* pContext = m_pDeviceContext;
	const TBOOL          bForce   = !m_bStateCacheValid;
	const TBOOL          bCompute = IsComputeSupported();

	if ( !bForce )
	{
		MergeReleasedViews( m_oApplied.apVSShaderResources, m_oCurrent.apVSShaderResources, MAX_SRV_SLOTS, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11ShaderResourceView* const* a_ppViews ) { pContext->VSSetShaderResources( a_uiFirst, a_uiNum, a_ppViews ); } );
		MergeReleasedViews( m_oApplied.apPSShaderResources, m_oCurrent.apPSShaderResources, MAX_SRV_SLOTS, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11ShaderResourceView* const* a_ppViews ) { pContext->PSSetShaderResources( a_uiFirst, a_uiNum, a_ppViews ); } );
		if ( bCompute )
		{
			MergeReleasedViews( m_oApplied.apCSShaderResources, m_oCurrent.apCSShaderResources, MAX_SRV_SLOTS, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11ShaderResourceView* const* a_ppViews ) { pContext->CSSetShaderResources( a_uiFirst, a_uiNum, a_ppViews ); } );
			MergeReleasedViews( m_oApplied.apCSUnorderedAccessViews, m_oCurrent.apCSUnorderedAccessViews, MAX_UAV_SLOTS, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11UnorderedAccessView* const* a_ppViews ) { pContext->CSSetUnorderedAccessViews( a_uiFirst, a_uiNum, a_ppViews, TNULL ); } );
		}
	}

	ID3D11RenderTargetView* pSecondary = ( m_oCurrent.pRenderTargetView && m_oCurrent.pRenderTargetView == m_pSecondaryRenderTargetOwner ) ? m_oCurrent.pSecondaryRenderTargetView : TNULL;

	if ( bForce || m_oApplied.pRenderTargetView != m_oCurrent.pRenderTargetView || m_oApplied.pSecondaryRenderTargetView != pSecondary || m_oApplied.pDepthStencilView != m_oCurrent.pDepthStencilView )
	{
		m_oApplied.pRenderTargetView          = m_oCurrent.pRenderTargetView;
		m_oApplied.pSecondaryRenderTargetView = pSecondary;
		m_oApplied.pDepthStencilView          = m_oCurrent.pDepthStencilView;

		ID3D11RenderTargetView* apViews[ 2 ] = { m_oCurrent.pRenderTargetView, pSecondary };
		pContext->OMSetRenderTargets( pSecondary ? 2 : 1, apViews, m_oCurrent.pDepthStencilView );
	}

	if ( bForce || m_oApplied.pVertexShader != m_oCurrent.pVertexShader )
	{
		m_oApplied.pVertexShader = m_oCurrent.pVertexShader;
		pContext->VSSetShader( m_oCurrent.pVertexShader, TNULL, 0 );
	}

	if ( bForce || m_oApplied.pPixelShader != m_oCurrent.pPixelShader )
	{
		m_oApplied.pPixelShader = m_oCurrent.pPixelShader;
		pContext->PSSetShader( m_oCurrent.pPixelShader, TNULL, 0 );
	}

	MergeSlots( m_oApplied.apVSConstantBuffers, m_oCurrent.apVSConstantBuffers, MAX_CONSTANT_BUFFER_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11Buffer* const* a_ppBuffers ) { pContext->VSSetConstantBuffers( a_uiFirst, a_uiNum, a_ppBuffers ); } );
	MergeSlots( m_oApplied.apPSConstantBuffers, m_oCurrent.apPSConstantBuffers, MAX_CONSTANT_BUFFER_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11Buffer* const* a_ppBuffers ) { pContext->PSSetConstantBuffers( a_uiFirst, a_uiNum, a_ppBuffers ); } );
	MergeSlots( m_oApplied.apVSShaderResources, m_oCurrent.apVSShaderResources, MAX_SRV_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11ShaderResourceView* const* a_ppViews ) { pContext->VSSetShaderResources( a_uiFirst, a_uiNum, a_ppViews ); } );
	MergeSlots( m_oApplied.apPSShaderResources, m_oCurrent.apPSShaderResources, MAX_SRV_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11ShaderResourceView* const* a_ppViews ) { pContext->PSSetShaderResources( a_uiFirst, a_uiNum, a_ppViews ); } );
	MergeSlots( m_oApplied.apVSSamplers, m_oCurrent.apVSSamplers, MAX_SAMPLER_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11SamplerState* const* a_ppSamplers ) { pContext->VSSetSamplers( a_uiFirst, a_uiNum, a_ppSamplers ); } );
	MergeSlots( m_oApplied.apPSSamplers, m_oCurrent.apPSSamplers, MAX_SAMPLER_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11SamplerState* const* a_ppSamplers ) { pContext->PSSetSamplers( a_uiFirst, a_uiNum, a_ppSamplers ); } );

	if ( bCompute )
	{
		if ( bForce || m_oApplied.pComputeShader != m_oCurrent.pComputeShader )
		{
			m_oApplied.pComputeShader = m_oCurrent.pComputeShader;
			pContext->CSSetShader( m_oCurrent.pComputeShader, TNULL, 0 );
		}

		MergeSlots( m_oApplied.apCSConstantBuffers, m_oCurrent.apCSConstantBuffers, MAX_CONSTANT_BUFFER_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11Buffer* const* a_ppBuffers ) { pContext->CSSetConstantBuffers( a_uiFirst, a_uiNum, a_ppBuffers ); } );
		MergeSlots( m_oApplied.apCSShaderResources, m_oCurrent.apCSShaderResources, MAX_SRV_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11ShaderResourceView* const* a_ppViews ) { pContext->CSSetShaderResources( a_uiFirst, a_uiNum, a_ppViews ); } );
		MergeSlots( m_oApplied.apCSUnorderedAccessViews, m_oCurrent.apCSUnorderedAccessViews, MAX_UAV_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11UnorderedAccessView* const* a_ppViews ) { pContext->CSSetUnorderedAccessViews( a_uiFirst, a_uiNum, a_ppViews, TNULL ); } );
		MergeSlots( m_oApplied.apCSSamplers, m_oCurrent.apCSSamplers, MAX_SAMPLER_SLOTS, bForce, [ pContext ]( UINT a_uiFirst, UINT a_uiNum, ID3D11SamplerState* const* a_ppSamplers ) { pContext->CSSetSamplers( a_uiFirst, a_uiNum, a_ppSamplers ); } );
	}

	if ( bForce || m_oApplied.pInputLayout != m_oCurrent.pInputLayout )
	{
		m_oApplied.pInputLayout = m_oCurrent.pInputLayout;
		pContext->IASetInputLayout( m_oCurrent.pInputLayout );
	}

	UINT uiFirstStream = MAX_VERTEX_STREAMS;
	UINT uiLastStream  = 0;

	for ( UINT i = 0; i < MAX_VERTEX_STREAMS; i++ )
	{
		if ( bForce || m_oApplied.apVertexBuffers[ i ] != m_oCurrent.apVertexBuffers[ i ] || m_oApplied.auiVertexStrides[ i ] != m_oCurrent.auiVertexStrides[ i ] || m_oApplied.auiVertexOffsets[ i ] != m_oCurrent.auiVertexOffsets[ i ] )
		{
			uiFirstStream                     = TMath::Min( uiFirstStream, i );
			uiLastStream                      = i;
			m_oApplied.apVertexBuffers[ i ]  = m_oCurrent.apVertexBuffers[ i ];
			m_oApplied.auiVertexStrides[ i ] = m_oCurrent.auiVertexStrides[ i ];
			m_oApplied.auiVertexOffsets[ i ] = m_oCurrent.auiVertexOffsets[ i ];
		}
	}

	if ( uiFirstStream <= uiLastStream && uiFirstStream < MAX_VERTEX_STREAMS )
		pContext->IASetVertexBuffers( uiFirstStream, uiLastStream - uiFirstStream + 1, &m_oCurrent.apVertexBuffers[ uiFirstStream ], &m_oCurrent.auiVertexStrides[ uiFirstStream ], &m_oCurrent.auiVertexOffsets[ uiFirstStream ] );

	if ( bForce || m_oApplied.pIndexBuffer != m_oCurrent.pIndexBuffer || m_oApplied.eIndexFormat != m_oCurrent.eIndexFormat || m_oApplied.uiIndexOffset != m_oCurrent.uiIndexOffset )
	{
		m_oApplied.pIndexBuffer  = m_oCurrent.pIndexBuffer;
		m_oApplied.eIndexFormat  = m_oCurrent.eIndexFormat;
		m_oApplied.uiIndexOffset = m_oCurrent.uiIndexOffset;
		pContext->IASetIndexBuffer( m_oCurrent.pIndexBuffer, m_oCurrent.eIndexFormat, m_oCurrent.uiIndexOffset );
	}

	if ( bForce || m_oApplied.eTopology != m_oCurrent.eTopology )
	{
		m_oApplied.eTopology = m_oCurrent.eTopology;
		pContext->IASetPrimitiveTopology( m_oCurrent.eTopology );
	}

	m_bStateCacheValid = TTRUE;
}

void TRenderD3DInterface::SetVSConstantBuffer( UINT a_uiSlot, TConstantBufferGeneric* a_pBuffer )
{
	TASSERT( a_uiSlot != 0 && a_uiSlot < MAX_CONSTANT_BUFFER_SLOTS );

	m_apVSConstantBuffers[ a_uiSlot ] = a_pBuffer;

	if ( a_pBuffer )
	{
		m_uiVSConstantBufferSlots |= 1 << a_uiSlot;
	}
	else
	{
		m_uiVSConstantBufferSlots &= ~( 1 << a_uiSlot );
		m_oCurrent.apVSConstantBuffers[ a_uiSlot ] = TNULL;
	}
}

void TRenderD3DInterface::SetPSConstantBuffer( UINT a_uiSlot, TConstantBufferGeneric* a_pBuffer )
{
	TASSERT( a_uiSlot != 0 && a_uiSlot < MAX_CONSTANT_BUFFER_SLOTS );

	m_apPSConstantBuffers[ a_uiSlot ] = a_pBuffer;

	if ( a_pBuffer )
	{
		m_uiPSConstantBufferSlots |= 1 << a_uiSlot;
	}
	else
	{
		m_uiPSConstantBufferSlots &= ~( 1 << a_uiSlot );
		m_oCurrent.apPSConstantBuffers[ a_uiSlot ] = TNULL;
	}
}

void TRenderD3DInterface::UnbindConstantBuffer( TConstantBufferGeneric* a_pBuffer )
{
	for ( TUINT i = 1; i < MAX_CONSTANT_BUFFER_SLOTS; i++ )
	{
		if ( m_apVSConstantBuffers[ i ] == a_pBuffer )
			SetVSConstantBuffer( i, TNULL );

		if ( m_apPSConstantBuffers[ i ] == a_pBuffer )
			SetPSConstantBuffer( i, TNULL );
	}

	for ( TUINT i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++ )
	{
		if ( m_apCSConstantBuffers[ i ] == a_pBuffer )
			SetCSConstantBuffer( i, TNULL );
	}
}

void TRenderD3DInterface::UpdateRenderStates()
{
	UpdateSamplerStates();

	if ( m_CurrentDepth.uiRaw != m_PreviousDepth.uiRaw || m_uiCurrentStencilRef != m_uiPreviousStencilRef )
	{
		ID3D11DepthStencilState* pState = TNULL;
		auto                     it     = m_DepthStates.Find( m_CurrentDepth.uiRaw );

		if ( it == m_DepthStates.End() )
		{
			const auto& rParts = m_CurrentDepth.Parts;

			D3D11_DEPTH_STENCIL_DESC oDesc;
			oDesc.DepthEnable                  = rParts.bDepthEnable;
			oDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK( rParts.DepthWriteMask );
			oDesc.DepthFunc                    = D3D11_COMPARISON_FUNC( rParts.DepthFunc );
			oDesc.StencilEnable                = rParts.bStencilEnable;
			oDesc.StencilReadMask              = TUINT8( rParts.StencilReadMask );
			oDesc.StencilWriteMask             = TUINT8( rParts.StencilWriteMask );
			oDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP( rParts.FrontStencilFailOp );
			oDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP( rParts.FrontStencilDepthFailOp );
			oDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP( rParts.FrontStencilPassOp );
			oDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_FUNC( rParts.FrontStencilFunc );
			oDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP( rParts.BackStencilFailOp );
			oDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP( rParts.BackStencilDepthFailOp );
			oDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP( rParts.BackStencilPassOp );
			oDesc.BackFace.StencilFunc         = D3D11_COMPARISON_FUNC( rParts.BackStencilFunc );

			HRESULT hRes = m_pDevice11->CreateDepthStencilState( &oDesc, &pState );
			TASSERT( SUCCEEDED( hRes ) );

			m_DepthStates.Insert( m_CurrentDepth.uiRaw, pState );
		}
		else
		{
			pState = it->second;
		}

		m_pDeviceContext->OMSetDepthStencilState( pState, m_uiCurrentStencilRef );
		m_PreviousDepth        = m_CurrentDepth;
		m_uiPreviousStencilRef = m_uiCurrentStencilRef;
	}

	if ( m_CurrentRasterizer != m_PreviousRasterizer )
	{
		ID3D11RasterizerState* pState = TNULL;
		auto                   it     = m_RasterizerStates.Find( m_CurrentRasterizer );

		if ( it == m_RasterizerStates.End() )
		{
			const auto& rParts = m_CurrentRasterizer.Flags.Parts;

			D3D11_RASTERIZER_DESC oDesc;
			oDesc.FillMode              = D3D11_FILL_MODE( rParts.FillMode );
			oDesc.CullMode              = D3D11_CULL_MODE( rParts.CullMode );
			oDesc.FrontCounterClockwise = rParts.bFrontCounterClockwise;
			oDesc.DepthBias             = m_CurrentRasterizer.DepthBias;
			oDesc.DepthBiasClamp        = 0.0f;
			oDesc.SlopeScaledDepthBias  = m_CurrentRasterizer.SlopeScaledDepthBias;
			oDesc.DepthClipEnable       = rParts.bDepthClipEnable;
			oDesc.ScissorEnable         = rParts.bScissorEnable;
			oDesc.MultisampleEnable     = rParts.bMultisampleEnable;
			oDesc.AntialiasedLineEnable = FALSE;

			HRESULT hRes = m_pDevice11->CreateRasterizerState( &oDesc, &pState );
			TASSERT( SUCCEEDED( hRes ) );

			m_RasterizerStates.Insert( m_CurrentRasterizer, pState );
		}
		else
		{
			pState = it->second;
		}

		m_pDeviceContext->RSSetState( pState );
		m_PreviousRasterizer = m_CurrentRasterizer;
	}

	// The destination alpha of SetDstAlpha needs blending
	if ( m_CurrentBlendState.Parts.SrcBlendAlpha == D3D11_BLEND_BLEND_FACTOR && !m_CurrentBlendState.Parts.bBlendEnable )
	{
		m_CurrentBlendState.Parts.bBlendEnable = TRUE;
		m_CurrentBlendState.Parts.BlendOp      = D3D11_BLEND_OP_ADD;
		m_CurrentBlendState.Parts.SrcBlend     = D3D11_BLEND_ONE;
		m_CurrentBlendState.Parts.DestBlend    = D3D11_BLEND_ZERO;
	}

	if ( m_CurrentBlendState.uiRaw != m_PreviousBlendState.uiRaw || TUtil::MemCompare( m_aCurrentBlendFactor, m_aPreviousBlendFactor, sizeof( m_aCurrentBlendFactor ) ) != 0 )
	{
		ID3D11BlendState* pState = TNULL;
		auto              it     = m_BlendStates.Find( m_CurrentBlendState.uiRaw );

		if ( it == m_BlendStates.End() )
		{
			const auto& rParts = m_CurrentBlendState.Parts;

			D3D11_BLEND_DESC oDesc                      = {};
			oDesc.AlphaToCoverageEnable                 = FALSE;
			oDesc.IndependentBlendEnable                = FALSE;
			oDesc.RenderTarget[ 0 ].BlendEnable         = rParts.bBlendEnable;
			oDesc.RenderTarget[ 0 ].SrcBlend            = D3D11_BLEND( rParts.SrcBlend );
			oDesc.RenderTarget[ 0 ].DestBlend           = D3D11_BLEND( rParts.DestBlend );
			oDesc.RenderTarget[ 0 ].BlendOp             = D3D11_BLEND_OP( rParts.BlendOp );
			oDesc.RenderTarget[ 0 ].SrcBlendAlpha       = D3D11_BLEND( rParts.SrcBlendAlpha );
			oDesc.RenderTarget[ 0 ].DestBlendAlpha      = D3D11_BLEND( rParts.DestBlendAlpha );
			oDesc.RenderTarget[ 0 ].BlendOpAlpha        = D3D11_BLEND_OP( rParts.BlendOpAlpha );
			oDesc.RenderTarget[ 0 ].RenderTargetWriteMask = TUINT8( rParts.RenderTargetWriteMask );

			HRESULT hRes = m_pDevice11->CreateBlendState( &oDesc, &pState );
			TASSERT( SUCCEEDED( hRes ) );

			m_BlendStates.Insert( m_CurrentBlendState.uiRaw, pState );
		}
		else
		{
			pState = it->second;
		}

		m_pDeviceContext->OMSetBlendState( pState, m_aCurrentBlendFactor, 0xFFFFFFFF );
		m_PreviousBlendState = m_CurrentBlendState;
		TUtil::MemCopy( m_aPreviousBlendFactor, m_aCurrentBlendFactor, sizeof( m_aPreviousBlendFactor ) );
	}
}

void TRenderD3DInterface::ResetStateCache()
{
	m_PreviousDepth.uiRaw            = ~m_CurrentDepth.uiRaw;
	m_PreviousRasterizer.Flags.uiRaw = ~m_CurrentRasterizer.Flags.uiRaw;
	m_PreviousBlendState.uiRaw       = ~m_CurrentBlendState.uiRaw;
	m_uiDirtySamplers                = ( 1 << NUM_TEXTURE_STAGES ) - 1;

	InvalidateStateCache();

	m_uiImmediateVertexCurrentOffset = 0;
	m_uiImmediateIndexCurrentOffset  = 0;
}

// The alpha channel blends the same way unless SetDstAlpha replaces it
static TBOOL IsDstAlpha( const TRenderD3DInterface::BLENDSTATE& a_rState )
{
	return a_rState.Parts.SrcBlendAlpha == D3D11_BLEND_BLEND_FACTOR;
}

// Alpha blend factors can't use the colour ones
static D3D11_BLEND GetAlphaBlend( D3D11_BLEND a_eBlend )
{
	switch ( a_eBlend )
	{
		case D3D11_BLEND_SRC_COLOR: return D3D11_BLEND_SRC_ALPHA;
		case D3D11_BLEND_INV_SRC_COLOR: return D3D11_BLEND_INV_SRC_ALPHA;
		case D3D11_BLEND_DEST_COLOR: return D3D11_BLEND_DEST_ALPHA;
		case D3D11_BLEND_INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_ALPHA;
		default: return a_eBlend;
	}
}

void TRenderD3DInterface::SetSrcBlend( D3D11_BLEND a_eBlend )
{
	m_CurrentBlendState.Parts.SrcBlend = a_eBlend;

	if ( !IsDstAlpha( m_CurrentBlendState ) )
		m_CurrentBlendState.Parts.SrcBlendAlpha = GetAlphaBlend( a_eBlend );
}

void TRenderD3DInterface::SetDestBlend( D3D11_BLEND a_eBlend )
{
	m_CurrentBlendState.Parts.DestBlend = a_eBlend;

	if ( !IsDstAlpha( m_CurrentBlendState ) )
		m_CurrentBlendState.Parts.DestBlendAlpha = GetAlphaBlend( a_eBlend );
}

void TRenderD3DInterface::SetBlendOp( D3D11_BLEND_OP a_eBlendOp )
{
	m_CurrentBlendState.Parts.BlendOp = a_eBlendOp;

	if ( !IsDstAlpha( m_CurrentBlendState ) )
		m_CurrentBlendState.Parts.BlendOpAlpha = a_eBlendOp;
}

TVector4 TRenderD3DInterface::GetAlphaTestParams() const
{
	return TVector4( m_bAlphaTestEnable ? 1.0f : 0.0f, m_uiAlphaRef / 255.0f, TFLOAT( m_eAlphaFunc ), 0.0f );
}

void TRenderD3DInterface::GetFogParams( TVector4& a_rParams, TVector4& a_rColour ) const
{
	a_rParams = TVector4( m_bFogEnable ? 1.0f : 0.0f, m_fFogStart, m_fFogEnd, 0.0f );
	a_rColour = TVector4( ( ( m_uiFogColour >> 16 ) & 0xFF ) / 255.0f, ( ( m_uiFogColour >> 8 ) & 0xFF ) / 255.0f, ( m_uiFogColour & 0xFF ) / 255.0f, 1.0f );
}

void TRenderD3DInterface::SetFixedFunctionShader()
{
	m_bFixedFunction = TTRUE;
	m_oCurrent.pVertexShader = m_pFixedFunctionVS;
	m_oCurrent.pPixelShader  = m_pFixedFunctionPS;
	m_oCurrent.pInputLayout  = m_pFixedFunctionLayout;
}

void TRenderD3DInterface::SetShaders( ID3D11VertexShader* a_pVertexShader, ID3D11PixelShader* a_pPixelShader, ID3D11InputLayout* a_pInputLayout )
{
	m_bFixedFunction = TFALSE;
	m_oCurrent.pVertexShader = a_pVertexShader;
	m_oCurrent.pPixelShader  = a_pPixelShader;
	m_oCurrent.pInputLayout  = a_pInputLayout;
}

void TRenderD3DInterface::SetTransform( TRANSFORM a_eTransform, const TMatrix44& a_rMatrix )
{
	m_aTransforms[ a_eTransform ] = a_rMatrix;
}

void TRenderD3DInterface::SetTexture( UINT a_uiStage, ID3D11ShaderResourceView* a_pTexture )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_oCurrent.apPSShaderResources[ a_uiStage ] = a_pTexture;
}

void TRenderD3DInterface::SetTextureMinFilter( UINT a_uiStage, TEXTUREFILTER a_eFilter )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_aMinFilters[ a_uiStage ] = a_eFilter;
	m_uiDirtySamplers |= 1 << a_uiStage;
}

void TRenderD3DInterface::SetTextureMagFilter( UINT a_uiStage, TEXTUREFILTER a_eFilter )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_aMagFilters[ a_uiStage ] = a_eFilter;
	m_uiDirtySamplers |= 1 << a_uiStage;
}

void TRenderD3DInterface::SetTextureMipFilter( UINT a_uiStage, TEXTUREFILTER a_eFilter )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_aMipFilters[ a_uiStage ] = a_eFilter;
	m_uiDirtySamplers |= 1 << a_uiStage;
}

void TRenderD3DInterface::CaptureStateBlock( STATEBLOCK& a_rStateBlock ) const
{
	a_rStateBlock.oDepth           = m_CurrentDepth;
	a_rStateBlock.oRasterizer      = m_CurrentRasterizer;
	a_rStateBlock.oBlend           = m_CurrentBlendState;
	a_rStateBlock.bAlphaTestEnable = m_bAlphaTestEnable;
	a_rStateBlock.eAlphaFunc       = m_eAlphaFunc;
	a_rStateBlock.uiAlphaRef       = m_uiAlphaRef;
	a_rStateBlock.bFogEnable       = m_bFogEnable;
	a_rStateBlock.eTextureOp       = m_eTextureOp;
	a_rStateBlock.pTexture         = m_oCurrent.apPSShaderResources[ 0 ];
	a_rStateBlock.eMinFilter       = m_aMinFilters[ 0 ];
	a_rStateBlock.eMagFilter       = m_aMagFilters[ 0 ];
	a_rStateBlock.eMipFilter       = m_aMipFilters[ 0 ];
}

void TRenderD3DInterface::ApplyStateBlock( const STATEBLOCK& a_rStateBlock )
{
	m_CurrentDepth      = a_rStateBlock.oDepth;
	m_CurrentRasterizer = a_rStateBlock.oRasterizer;
	m_CurrentBlendState = a_rStateBlock.oBlend;
	m_bAlphaTestEnable  = a_rStateBlock.bAlphaTestEnable;
	m_eAlphaFunc        = a_rStateBlock.eAlphaFunc;
	m_uiAlphaRef        = a_rStateBlock.uiAlphaRef;
	m_bFogEnable        = a_rStateBlock.bFogEnable;
	m_eTextureOp        = a_rStateBlock.eTextureOp;

	SetTexture( 0, a_rStateBlock.pTexture );
	SetTextureMinFilter( 0, a_rStateBlock.eMinFilter );
	SetTextureMagFilter( 0, a_rStateBlock.eMagFilter );
	SetTextureMipFilter( 0, a_rStateBlock.eMipFilter );
}

void TRenderD3DInterface::UpdateSamplerStates()
{
	for ( TUINT32 i = 0; m_uiDirtySamplers != 0; i++ )
	{
		if ( !( m_uiDirtySamplers & ( 1 << i ) ) )
			continue;

		m_uiDirtySamplers &= ~( 1 << i );

		ID3D11SamplerState* pSamplerState = GetSamplerState( m_aMinFilters[ i ], m_aMagFilters[ i ], m_aMipFilters[ i ], m_aAddressU[ i ], m_aAddressV[ i ] );
		m_oCurrent.apPSSamplers[ i ] = pSamplerState;
	}
}

void TRenderD3DInterface::UpdateFixedFunctionConstants()
{
	TMatrix44 oWorldView;
	oWorldView.Multiply( m_aTransforms[ TRANSFORM_VIEW ], m_aTransforms[ TRANSFORM_WORLD ] );

	TMatrix44 oWorldViewProjection;
	oWorldViewProjection.Multiply( m_aTransforms[ TRANSFORM_PROJECTION ], oWorldView );

	TVector4 vParams = GetAlphaTestParams();
	vParams.w        = ( m_eTextureOp == TEXTUREOP_MODULATE && m_oCurrent.apPSShaderResources[ 0 ] ) ? 1.0f : 0.0f;

	SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );
	SetVec4InVSBuffer( 4, &vParams );
}

void TRenderD3DInterface::SetStencil( TBOOL a_bEnable, D3D11_COMPARISON_FUNC a_eFunc, UINT a_uiStencilRef, TUINT8 a_uiMask, D3D11_STENCIL_OP a_eFailOp, D3D11_STENCIL_OP a_eDepthFailOp, D3D11_STENCIL_OP a_ePassOp )
{
	auto& rParts = m_CurrentDepth.Parts;

	rParts.bStencilEnable          = a_bEnable;
	rParts.StencilReadMask         = a_uiMask;
	rParts.StencilWriteMask        = a_uiMask;
	rParts.FrontStencilFunc        = a_eFunc;
	rParts.FrontStencilFailOp      = a_eFailOp;
	rParts.FrontStencilDepthFailOp = a_eDepthFailOp;
	rParts.FrontStencilPassOp      = a_ePassOp;
	rParts.BackStencilFunc         = a_eFunc;
	rParts.BackStencilFailOp       = a_eFailOp;
	rParts.BackStencilDepthFailOp  = a_eDepthFailOp;
	rParts.BackStencilPassOp       = a_ePassOp;
	m_uiCurrentStencilRef          = a_uiStencilRef;
}

void TRenderD3DInterface::SetDepthBias( INT a_iDepthBias, FLOAT a_fSlopeScaledDepthBias )
{
	m_CurrentRasterizer.DepthBias            = a_iDepthBias;
	m_CurrentRasterizer.SlopeScaledDepthBias = a_fSlopeScaledDepthBias;
}

void TRenderD3DInterface::SetColorUpdate( TBOOL a_bUpdate )
{
	if ( a_bUpdate )
		m_CurrentBlendState.Parts.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
	else
		m_CurrentBlendState.Parts.RenderTargetWriteMask &= ~( D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE );
}

void TRenderD3DInterface::SetAlphaUpdate( TBOOL a_bUpdate )
{
	if ( a_bUpdate )
		m_CurrentBlendState.Parts.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	else
		m_CurrentBlendState.Parts.RenderTargetWriteMask &= ~D3D11_COLOR_WRITE_ENABLE_ALPHA;
}

void TRenderD3DInterface::SetDstAlpha( TFLOAT a_fAlpha )
{
	if ( a_fAlpha >= 0.0f )
	{
		m_aCurrentBlendFactor[ 3 ]               = a_fAlpha;
		m_CurrentBlendState.Parts.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
		m_CurrentBlendState.Parts.SrcBlendAlpha  = D3D11_BLEND_BLEND_FACTOR;
		m_CurrentBlendState.Parts.DestBlendAlpha = D3D11_BLEND_ZERO;
	}
	else
	{
		m_CurrentBlendState.Parts.BlendOpAlpha   = m_CurrentBlendState.Parts.BlendOp;
		m_CurrentBlendState.Parts.SrcBlendAlpha  = m_CurrentBlendState.Parts.SrcBlend;
		m_CurrentBlendState.Parts.DestBlendAlpha = m_CurrentBlendState.Parts.DestBlend;
	}
}

TOSHI_NAMESPACE_END
