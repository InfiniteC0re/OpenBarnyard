#include "ToshiPCH.h"
#include "TRenderCapture_DX8.h"
#include "TRenderContext_DX8.h"
#include "TRenderAdapter_DX8.h"
#include "TRenderInterface_DX8.h"
#include "TTextureFactoryHAL_DX8.h"
#include "TTextureResourceHAL_DX8.h"

#include "Toshi/Render/TShader.h"

namespace Toshi {
	
	TRenderD3DInterface::TRenderD3DInterface()
	{
		m_pDirect3D = TNULL;
		m_pDirectDevice = TNULL;
		m_fPixelAspectRatio = 1.0f;
		m_AcceleratorTable = NULL;
		m_pDevice = TNULL;
		m_oDisplayParams.uiWidth = 640;
		m_oDisplayParams.uiHeight = 480;
		m_oDisplayParams.uiColourDepth = 32;
		m_oDisplayParams.eDepthStencilFormat = 0;
		m_oDisplayParams.bWindowed = TTRUE;
		m_fBrightness = 0.5f;
		m_fSaturate = 0.5f;
		m_bExited = TFALSE;
		m_bCheckedCapableColourCorrection = TFALSE;
		m_bCapableColourCorrection = TFALSE;
		m_bFailed = TFALSE;
		m_Unk1 = TNULL;
		m_Unk2 = TNULL;
		m_fContrast = 0.583012f;
		m_fGamma = 0.420849f;
		m_bChangedColourSettings = TTRUE;
		m_bEnableColourCorrection = TTRUE;
	}

	TRenderD3DInterface::~TRenderD3DInterface()
	{
		Destroy();
	}

	TBOOL TRenderD3DInterface::CreateDisplay(const DISPLAYPARAMS& a_rParams)
	{
		if (!TRenderInterface::CreateDisplay())
		{
			OnInitializationFailureDisplay();
			return TFALSE;
		}

		m_pDevice = TSTATICCAST(TD3DAdapter::Mode::Device*, FindDevice(a_rParams));
		m_oDisplayParams = a_rParams;

		if (m_pDevice)
		{
			auto pDisplayParams = GetCurrentDisplayParams();

			RECT clientRect;
			GetClientRect(GetDesktopWindow(), &clientRect);

			if (2000 < clientRect.right)
			{
				clientRect.right /= 2;
			}

			TUINT32 uiWindowPosX = 0;
			TUINT32 uiWindowPosY = 0;

			if (pDisplayParams->bWindowed)
			{
				auto pMode = GetCurrentDevice()->GetMode();
				uiWindowPosX = (clientRect.right - pMode->GetWidth()) / 2;
				uiWindowPosY = (clientRect.bottom - pMode->GetHeight()) / 2;
			}

			TUtil::MemClear(&m_PresentParams, sizeof(m_PresentParams));
			m_PresentParams.Windowed = pDisplayParams->bWindowed;
			m_PresentParams.BackBufferCount = 1;
			m_PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
			m_PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
			m_PresentParams.EnableAutoDepthStencil = TRUE;
			m_PresentParams.hDeviceWindow = m_Window.GetHWND();
			m_PresentParams.AutoDepthStencilFormat = TD3DAdapter::Mode::Device::DEPTHSTENCILFORMATS[pDisplayParams->eDepthStencilFormat];
			m_PresentParams.BackBufferWidth = pDisplayParams->uiWidth;
			m_PresentParams.BackBufferHeight = pDisplayParams->uiHeight;

			auto pDevice = TSTATICCAST(TD3DAdapter::Mode::Device*, GetCurrentDevice());
			auto pMode = TSTATICCAST(TD3DAdapter::Mode*, pDevice->GetMode());
			auto pAdapter = TSTATICCAST(TD3DAdapter*, pMode->GetAdapter());
			auto uiAdapterIndex = pAdapter->GetAdapterIndex();

			if (pDisplayParams->bWindowed)
			{
				m_PresentParams.BackBufferFormat = pMode->GetD3DDisplayMode().Format;
			}
			else
			{
				m_PresentParams.BackBufferFormat = pMode->GetBackBufferFormat(pDisplayParams->uiColourDepth);
			}

			HRESULT hRes = m_pDirect3D->CreateDevice(
				uiAdapterIndex,
				TD3DAdapter::Mode::Device::DEVICETYPES[pDevice->GetDeviceIndex()],
				m_Window.GetHWND(),
				pDevice->GetD3DDeviceFlags(),
				&m_PresentParams,
				&m_pDirectDevice
			);

			if (FAILED(hRes))
			{
				OnInitializationFailureDevice();
				PrintError(hRes, "Failed to create D3D Device!");

				return TFALSE;
			}

			SetDeviceDefaultStates();

			if (pDisplayParams->bWindowed)
			{
				m_Window.SetWindowed();
			}
			else
			{
				m_Window.SetFullscreen();
			}

			if (uiAdapterIndex != 0)
			{
				HMONITOR hMonitor = m_pDirect3D->GetAdapterMonitor(uiAdapterIndex);

				MONITORINFO monitorInfo = { .cbSize = sizeof(monitorInfo) };
				GetMonitorInfoA(hMonitor, &monitorInfo);

				uiWindowPosX += monitorInfo.rcMonitor.left;
				uiWindowPosY += monitorInfo.rcMonitor.right;
			}

			m_Window.SetPosition(uiWindowPosX, uiWindowPosY, pDisplayParams->uiWidth, pDisplayParams->uiHeight);
			
			IDirect3DSurface8* pSurface;
			m_pDirectDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
			pSurface->GetDesc(&m_SurfaceDesk);
			pSurface->Release();

			SetCursorPos(
				uiWindowPosX + pDisplayParams->uiWidth / 2,
				uiWindowPosY + pDisplayParams->uiHeight / 2
			);

			m_pDirectDevice->ShowCursor(TRUE);

			TUINT invalidTextureData[32];
			for (int i = 0; i < 32; i++)
			{
				invalidTextureData[i] = 0xff0fff0f;
			}

			auto pTextureFactory = TSTATICCAST(TTextureFactoryHAL*, GetSystemResource(SYSRESOURCE_TEXTUREFACTORY));
			m_pInvalidTexture = pTextureFactory->CreateTextureFromMemory(invalidTextureData, sizeof(invalidTextureData), 0x11, 8, 8);

			EnableColourCorrection(TTRUE);
			m_bDisplayCreated = TTRUE;

			return TTRUE;
		}

		OnInitializationFailureDisplay();
		return TFALSE;
	}

	TBOOL TRenderD3DInterface::DestroyDisplay()
	{
		TASSERT(IsDisplayCreated());
		FlushDyingResources();

		if (IsDisplayCreated())
		{
			if (m_pDirectDevice)
			{
				m_pDirectDevice->Release();
			}

			m_bDisplayCreated = TFALSE;
			return TTRUE;
		}
		else
		{
			return TTRUE;
		}
	}

	TBOOL TRenderD3DInterface::Update(float a_fDeltaTime)
	{
		FlushDyingResources();
		m_Window.Update(a_fDeltaTime);

		return !m_bExited;
	}

	TBOOL TRenderD3DInterface::BeginScene()
	{
		TASSERT(!IsInScene());

		if (TRenderInterface::BeginScene())
		{
			HRESULT hRes = m_pDirectDevice->BeginScene();

			if (SUCCEEDED(hRes))
			{
				RECT windowRect;
				GetWindowRect(m_Window.GetHWND(), &windowRect);

				D3DVIEWPORT8 viewport = {
					.X = 0,
					.Y = 0,
					.Width = DWORD(windowRect.right - windowRect.left),
					.Height = DWORD(windowRect.bottom - windowRect.top),
					.MinZ = 0.0f,
					.MaxZ = 1.0f,
				};

				m_pDirectDevice->SetViewport(&viewport);
				UpdateColourSettings();
				
				m_bInScene = TTRUE;
				m_BeginSceneEmitter.Throw(0);
			}
			else
			{
				return TFALSE;
			}
		}

		return TTRUE;
	}

	TBOOL TRenderD3DInterface::EndScene()
	{
		TASSERT(IsInScene());

		if (IsInScene())
		{
			m_EndSceneEmitter.Throw(TNULL);
			HRESULT hRes = m_pDirectDevice->EndScene();

			if (SUCCEEDED(hRes))
			{
				HRESULT hPresentRes = m_pDirectDevice->Present(NULL, NULL, NULL, NULL);

				if (hPresentRes == D3DERR_DEVICELOST)
				{
					HRESULT hCooperativeLevel = m_pDirectDevice->TestCooperativeLevel();

					if (hCooperativeLevel == D3DERR_DEVICELOST)
					{
						OnD3DDeviceLost();
					}
					else if (hCooperativeLevel == D3DERR_DEVICENOTRESET)
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
		return Create(s_Class.GetName());
	}

	void TRenderD3DInterface::FlushShaders()
	{
		GetSingleton()->FlushOrderTables();

		for (auto it = TShader::sm_oShaderList.GetRootShader(); it != TNULL; it = it->GetNextShader())
		{
			it->Flush();
		}
	}

	void TRenderD3DInterface::PrintError(TINT32 a_eError, const char* a_szInfo)
	{
		if (!a_szInfo)
		{
			a_szInfo = "D3D Error";
		}

		const char* errString = GetErrorString(a_eError);
		const char* errDescription = GetErrorDescription(a_eError);

		TString8 string;
		string.Format("> %s: D3D Error [%s] : Description [%s] !\n", a_szInfo, errString, errDescription);
		OutputDebugStringA(string);
		
		TASSERT(TFALSE, string);
	}

	TBOOL TRenderD3DInterface::Destroy()
	{
		TASSERT(TTRUE == IsCreated());

		if (IsCreated())
		{
			FlushDyingResources();
			DestroyAllShaderResources();
			FlushDyingResources();

			DestroySystemResources();
			DestroyDisplay();

			m_Window.UnregisterWindowClass();
			DestroyAccelTable();

			if (m_pDirect3D)
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

	void TRenderD3DInterface::RenderIndexPrimitive(int param_2, int param_3, int param_4, int param_5, int param_6, int param_7)
	{
		TIMPLEMENT();
	}

	float TRenderD3DInterface::GetPixelAspectRatio()
	{
		return m_fPixelAspectRatio;
	}

	TBOOL TRenderD3DInterface::SetPixelAspectRatio(float a_fPixelAspectRatio)
	{
		m_fPixelAspectRatio = a_fPixelAspectRatio;
		return TTRUE;
	}

	void TRenderD3DInterface::FlushOrderTables()
	{
		TASSERT(TTRUE == IsInScene());

		for (auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++)
		{
			it->Flush();
		}
	}

	TRenderContext* TRenderD3DInterface::CreateRenderContext()
	{
		return new TRenderContextD3D(this);
	}

	TRenderCapture* TRenderD3DInterface::CreateCapture()
	{
		return new TRenderCaptureD3D();
	}

	void TRenderD3DInterface::DestroyCapture(TRenderCapture* a_pRenderCapture)
	{
		TTODO("a_pRenderCapture->vftable + 8");
		delete a_pRenderCapture;
	}

	void* TRenderD3DInterface::CreateUnknown(const char* a_szName, int a_iUnk1, int a_iUnk2, int a_iUnk3)
	{
		return TNULL;
	}

	TModel* TRenderD3DInterface::CreateModel1(void* a_Unk1, void* a_Unk2)
	{
		TIMPLEMENT();
		return TNULL;
	}

	TModel* TRenderD3DInterface::CreateModel2(void* a_Unk1, void* a_Unk2)
	{
		TIMPLEMENT();
		return TNULL;
	}

	TModel* TRenderD3DInterface::CreateModel3(void* a_Unk1, void* a_Unk2, void* a_Unk3, void* a_Unk4)
	{
		TIMPLEMENT();
		return TNULL;
	}

	TDebugText* TRenderD3DInterface::CreateDebugText()
	{
		m_pDebugText = InitDebugText(0);
		return m_pDebugText;
	}

	void TRenderD3DInterface::DestroyDebugText()
	{
		if (m_pDebugText)
		{
			delete m_pDebugText;
			m_pDebugText = TNULL;
		}
	}

	TBOOL TRenderD3DInterface::RecreateDisplay(const DISPLAYPARAMS& a_rDisplayParams)
	{
		if (IsCreated())
		{
			if (IsDisplayCreated())
			{
				OnD3DDeviceLost();
				DestroyDisplay();

				if (CreateDisplay(a_rDisplayParams))
				{
					OnD3DDeviceFound();
					return TTRUE;
				}
			}
		}

		return TFALSE;
	}

	void TRenderD3DInterface::SetContrast(TFLOAT a_fConstrast)
	{
		TMath::Clip(a_fConstrast, 0.0f, 1.0f);
		m_fContrast = a_fConstrast;
		m_bChangedColourSettings = TTRUE;
	}

	void TRenderD3DInterface::SetBrightness(TFLOAT a_fBrightness)
	{
		TMath::Clip(a_fBrightness, 0.0f, 1.0f);
		m_fBrightness = a_fBrightness;
		m_bChangedColourSettings = TTRUE;
	}

	void TRenderD3DInterface::SetGamma(TFLOAT a_fGamma)
	{
		TMath::Clip(a_fGamma, 0.0f, 1.0f);
		m_fGamma = a_fGamma;
		m_bChangedColourSettings = TTRUE;
	}

	void TRenderD3DInterface::SetSaturate(TFLOAT a_fSaturate)
	{
		TMath::Clip(a_fSaturate, 0.0f, 1.0f);
		m_fSaturate = a_fSaturate;
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
		if (IsColourCorrection())
		{
			GetCurrentColourRamp();
			m_pDirectDevice->SetGammaRamp(0, &m_GammaRamp);
		}
	}

	TBOOL TRenderD3DInterface::IsCapableColourCorrection()
	{
		if (!m_bCheckedCapableColourCorrection)
		{
			D3DCAPS8 caps;
			HRESULT hRes = m_pDirectDevice->GetDeviceCaps(&caps);
			m_bCapableColourCorrection = SUCCEEDED(hRes) && HASFLAG(caps.AdapterOrdinal & 0x20000);
		}

		return m_bCapableColourCorrection;
	}

	void TRenderD3DInterface::EnableColourCorrection(TBOOL a_bEnable)
	{
		m_bEnableColourCorrection = a_bEnable && IsCapableColourCorrection();
	}

	void TRenderD3DInterface::ForceEnableColourCorrection(TBOOL a_bEnable)
	{
		m_bEnableColourCorrection = a_bEnable;
	}

	TBOOL TRenderD3DInterface::IsColourCorrection()
	{
		return m_bEnableColourCorrection;
	}

	void TRenderD3DInterface::OnD3DDeviceLost()
	{
		TIMPLEMENT();
	}

	void TRenderD3DInterface::OnD3DDeviceFound()
	{
		TIMPLEMENT();
	}

	void TRenderD3DInterface::GetCurrentColourRamp()
	{
		TIMPLEMENT();

		if (m_bChangedColourSettings)
		{
			m_bChangedColourSettings = TFALSE;
		}
	}

	TDebugD3DText* TRenderD3DInterface::InitDebugText(TINT a_iBufferSize)
	{
		TIMPLEMENT();
		return TNULL;
	}

	void TRenderD3DInterface::SetDeviceDefaultStates()
	{
		m_pDirectDevice->SetRenderState(D3DRS_ZENABLE, 1);
		m_pDirectDevice->SetRenderState(D3DRS_CULLMODE, 2);
		m_pDirectDevice->SetRenderState(D3DRS_LIGHTING, 0);
		m_pDirectDevice->SetTextureStageState(0, D3DTSS_MINFILTER, 2);
		m_pDirectDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, 2);
		m_pDirectDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, 2);
	}

	void TRenderD3DInterface::SetTextureStageState(DWORD a_iStage, TINT a_eType, TINT a_iUnk)
	{
		// TODO: Refactor

		DWORD DVar1;

		if (a_eType == 0) {
			DVar1 = 1;
			if (a_iUnk == 0) goto LAB_006c618c;
			if (a_iUnk == 1) goto LAB_006c61da;
			m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSU, 1);
			DVar1 = 1;
		}
		else if (a_eType == 2) {
			DVar1 = 3;
			if (a_iUnk == 0) goto LAB_006c618c;
			if (a_iUnk == 1) goto LAB_006c61da;
			m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSU, 3);
			DVar1 = 3;
		}
		else if (a_eType == 1) {
			DVar1 = 2;
			if (a_iUnk == 0) {
			LAB_006c618c:
				m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSU, DVar1);
				return;
			}
			if (a_iUnk == 1) {
			LAB_006c61da:
				m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSV, DVar1);
				return;
			}
			m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSU, 2);
			DVar1 = 2;
		}
		else {
			if (a_eType != 3) {
				return;
			}
			DVar1 = 4;
			if (a_iUnk == 0) goto LAB_006c618c;
			if (a_iUnk == 1) goto LAB_006c61da;
			m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSU, 4);
			DVar1 = 4;
		}

		m_pDirectDevice->SetTextureStageState(a_iStage, D3DTSS_ADDRESSV, DVar1);
	}

	void TRenderD3DInterface::BeginEndScene()
	{
		if (S_OK == m_pDirectDevice->BeginScene())
		{
			m_pDirectDevice->EndScene();
		}
	}

	TBOOL TRenderD3DInterface::IsTextureFormatSupported(int a_eTextureFormat)
	{
		switch (a_eTextureFormat) {
		case 1:
			return IsTextureFormatSupportedImpl(D3DFMT_A8R8G8B8);
		case 2:
			return IsTextureFormatSupportedImpl(D3DFMT_X8R8G8B8);
		case 3:
			return IsTextureFormatSupportedImpl(D3DFMT_A1R5G5B5);
		case 4:
			return TTRUE;
		default:
			return TFALSE;
		}
	}

	TBOOL TRenderD3DInterface::IsTextureFormatSupportedImpl(D3DFORMAT a_eFormat)
	{
		auto pDevice = GetCurrentDevice();

		return SUCCEEDED(
			m_pDirect3D->CheckDeviceFormat(
				pDevice->GetMode()->GetAdapter()->GetAdapterIndex(),
				TD3DAdapter::Mode::Device::DEVICETYPES[pDevice->GetDeviceIndex()],
				m_PresentParams.BackBufferFormat,
				0,
				D3DRTYPE_TEXTURE,
				a_eFormat
			)
		);
	}

	TBOOL TRenderD3DInterface::Supports32BitTextures()
	{
		return IsTextureFormatSupported(1) && IsTextureFormatSupported(2);
	}

	void TRenderD3DInterface::OnInitializationFailureDevice()
	{
		CHAR caption[1024];
		CHAR text[1024];

		GetPrivateProfileStringA("Setup", "IDS_D3DDEVICEERRORTITLE", "Initialization failure", caption, 0x400, ".\\Setup.ini");
		GetPrivateProfileStringA("Setup", "IDS_D3DDEVICEERROR", "Failed to initialize Direct3D. Please ensure DirectX8.1b is installed AND DirectX8.1b drivers for your video card", text, 0x400, ".\\Setup.ini");
		MessageBoxA(NULL, text, caption, 0);
	}

	void TRenderD3DInterface::OnInitializationFailureDisplay()
	{
		CHAR caption[1024];
		CHAR text[1024];

		GetPrivateProfileStringA("Setup", "IDS_D3DDISPLAYERRORTITLE", "Initialization failure", caption, 0x400, ".\\Setup.ini");
		GetPrivateProfileStringA("Setup", "IDS_D3DDISPLAYERROR", "Failed to create the display. Please run the Barnyard setup program and reconfigure", text, 0x400, ".\\Setup.ini");
		MessageBoxA(NULL, text, caption, 0);
	}

	TBOOL TRenderD3DInterface::Create(const char* a_szWindowName)
	{
		TASSERT(TFALSE == IsCreated());

		if (TRenderInterface::Create())
		{
			m_pDirect3D = Direct3DCreate8(D3D_SDK_VERSION);

			if (m_pDirect3D)
			{
				BuildAdapterDatabase();
				CreateAccelTable();

				if (m_Window.Create(this, a_szWindowName))
				{
					if (m_bCreateSystemResources)
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
		UINT uiAdapterCount = m_pDirect3D->GetAdapterCount();

		for (UINT i = 0; i < uiAdapterCount; i++)
		{
			auto pAdapter = new TD3DAdapter();
			pAdapter->SetAdapterIndex(i);

			D3DDISPLAYMODE displayMode;
			auto pIdentifier = pAdapter->GetD3DIdentifier8();
			m_pDirect3D->GetAdapterIdentifier(i, D3DENUM_NO_WHQL_LEVEL, pIdentifier);
			m_pDirect3D->GetAdapterDisplayMode(i, &displayMode);
			
			pAdapter->SetDriver(pIdentifier->Driver);
			pAdapter->SetDescription(pIdentifier->Description);
			pAdapter->SetDriverVersionLowPart(pIdentifier->DriverVersion.LowPart);
			pAdapter->SetDriverVersionHighPart(pIdentifier->DriverVersion.HighPart);
			pAdapter->SetDeviceId(pIdentifier->DeviceId);
			pAdapter->SetVendorId(pIdentifier->VendorId);
			pAdapter->SetSubSysId(pIdentifier->SubSysId);
			pAdapter->SetRevision(pIdentifier->Revision);
			pAdapter->SetDeviceIdentifier(pIdentifier->DeviceIdentifier);

			pAdapter->GetMode().SetD3DDisplayMode(displayMode);
			pAdapter->EnumerateOutputs(this);

			GetAdapterList()->InsertTail(*pAdapter);
		}
	}

	void TRenderD3DInterface::DestroyAccelTable()
	{
		if (m_AcceleratorTable)
		{
			DestroyAcceleratorTable(m_AcceleratorTable);
			m_AcceleratorTable = NULL;
		}
	}

	void TRenderD3DInterface::CreateAccelTable()
	{
		DestroyAccelTable();

		ACCEL accel[2];
		accel[0].fVirt = 1;
		accel[0].key = 27;
		accel[0].cmd = 0;
		accel[1].fVirt = 16;
		accel[1].key = 13;
		accel[1].cmd = 0;
		m_AcceleratorTable = CreateAcceleratorTableA(accel, 2);
	}

	const char* TRenderD3DInterface::GetErrorString(TINT32 a_eError)
	{
		const char* pcVar1;

		if (a_eError < -0x7789feb0) {
			if (a_eError == -0x7789feb1) {
				return "DDERR_NOVSYNCHW";
			}
			if (a_eError < -0x7ff8fb7e) {
				if (a_eError == -0x7ff8fb7f) {
					return "DIERR_BETADIRECTINPUTVERSION";
				}
				if (a_eError < -0x7ffbfdb7) {
					if (a_eError == -0x7ffbfdb8) {
						return "VFW_E_INVALID_MEDIA_TYPE";
					}
					if (a_eError < -0x7ffbfde5) {
						if (a_eError == -0x7ffbfde6) {
							return "VFW_E_NO_COLOR_KEY_SET";
						}
						if (a_eError < -0x7ffbfdf8) {
							if (a_eError == -0x7ffbfdf9) {
								return "DIERR_NOTBUFFERED & VFW_E_NO_ACCEPTABLE_TYPES";
							}
							if (a_eError < -0x7ffbfe0f) {
								if (a_eError == -0x7ffbfe10) {
									return "CO_E_NOTINITIALIZED";
								}
								if (a_eError < -0x7fffbffb) {
									if (a_eError == -0x7fffbffc) {
										return "E_ABORT";
									}
									if (a_eError == -0x7ffffff6) {
										return "E_PENDING";
									}
									if (a_eError == -0x7fffbfff) {
										return "E_NOTIMPL";
									}
									if (a_eError == -0x7fffbffe) {
										return "E_NOINTERFACE";
									}
									if (a_eError == -0x7fffbffd) {
										return "E_POINTER";
									}
								}
								else {
									if (a_eError == -0x7fffbffb) {
										return "E_FAIL";
									}
									if (a_eError == -0x7fff0001) {
										return "E_UNEXPECTED";
									}
									if (a_eError == -0x7ffbfef0) {
										return "CLASS_E_NOAGGREGATION";
									}
									if (a_eError == -0x7ffbfeac) {
										return "REGDB_E_CLASSNOTREG";
									}
								}
							}
							else if (a_eError < -0x7ffbfdfc) {
								if (a_eError == -0x7ffbfdfd) {
									return "DIERR_NOTDOWNLOADED & VFW_E_ENUM_OUT_OF_SYNC & DMO_E_TYPE_NOT_SET";
								}
								if (a_eError == -0x7ffbfe0f) {
									return "CO_E_ALREADYINITIALIZED";
								}
								if (a_eError == -0x7ffbfe00) {
									return "DIERR_INSUFFICIENTPRIVS & VFW_E_INVALIDMEDIATYPE";
								}
								if (a_eError == -0x7ffbfdff) {
									return "DIERR_DEVICEFULL & VFW_E_INVALIDSUBTYPE & DMO_E_INVALIDSTREAMINDEX";
								}
								if (a_eError == -0x7ffbfdfe) {
									return "DIERR_MOREDATA & VFW_E_NEED_OWNER & DMO_E_INVALIDTYPE";
								}
							}
							else {
								if (a_eError == -0x7ffbfdfc) {
									return "DIERR_HASEFFECTS & VFW_E_ALREADY_CONNECTED & DMO_E_NOTACCEPTING";
								}
								if (a_eError == -0x7ffbfdfb) {
									return "DIERR_NOTEXCLUSIVEACQUIRED & VFW_E_FILTER_ACTIVE & DMO_E_TYPE_NOT_ACCEPTED";
								}
								if (a_eError == -0x7ffbfdfa) {
									return "DIERR_INCOMPLETEEFFECT & VFW_E_NO_TYPES & DMO_E_NO_MORE_ITEMS";
								}
							}
						}
						else {
							switch (a_eError) {
							case -0x7ffbfdf8:
								return "DIERR_EFFECTPLAYING & VFW_E_INVALID_DIRECTION";
							case -0x7ffbfdf7:
								return "DIERR_UNPLUGGED & VFW_E_NOT_CONNECTED";
							case -0x7ffbfdf6:
								return "DIERR_REPORTFULL & VFW_E_NO_ALLOCATOR";
							case -0x7ffbfdf5:
								return "DIERR_MAPFILEFAIL & VFW_E_RUNTIME_ERROR";
							case -0x7ffbfdf4:
								return "VFW_E_BUFFER_NOTSET";
							case -0x7ffbfdf3:
								return "VFW_E_BUFFER_OVERFLOW";
							case -0x7ffbfdf2:
								return "VFW_E_BADALIGN";
							case -0x7ffbfdf1:
								return "VFW_E_ALREADY_COMMITTED";
							case -0x7ffbfdf0:
								return "VFW_E_BUFFERS_OUTSTANDING";
							case -0x7ffbfdef:
								return "VFW_E_NOT_COMMITTED";
							case -0x7ffbfdee:
								return "VFW_E_SIZENOTSET";
							case -0x7ffbfded:
								return "VFW_E_NO_CLOCK";
							case -0x7ffbfdec:
								return "VFW_E_NO_SINK";
							case -0x7ffbfdeb:
								return "VFW_E_NO_INTERFACE";
							case -0x7ffbfdea:
								return "VFW_E_NOT_FOUND";
							case -0x7ffbfde9:
								return "VFW_E_CANNOT_CONNECT";
							case -0x7ffbfde8:
								return "VFW_E_CANNOT_RENDER";
							case -0x7ffbfde7:
								return "VFW_E_CHANGING_FORMAT";
							}
						}
					}
					else {
						switch (a_eError) {
						case -0x7ffbfde5:
							return "VFW_E_NOT_OVERLAY_CONNECTION";
						case -0x7ffbfde4:
							return "VFW_E_NOT_SAMPLE_CONNECTION";
						case -0x7ffbfde3:
							return "VFW_E_PALETTE_SET";
						case -0x7ffbfde2:
							return "VFW_E_COLOR_KEY_SET";
						case -0x7ffbfde1:
							return "VFW_E_NO_COLOR_KEY_FOUND";
						case -0x7ffbfde0:
							return "VFW_E_NO_PALETTE_AVAILABLE";
						case -0x7ffbfddf:
							return "VFW_E_NO_DISPLAY_PALETTE";
						case -0x7ffbfdde:
							return "VFW_E_TOO_MANY_COLORS";
						case -0x7ffbfddd:
							return "VFW_E_STATE_CHANGED";
						case -0x7ffbfddc:
							return "VFW_E_NOT_STOPPED";
						case -0x7ffbfddb:
							return "VFW_E_NOT_PAUSED";
						case -0x7ffbfdda:
							return "VFW_E_NOT_RUNNING";
						case -0x7ffbfdd9:
							return "VFW_E_WRONG_STATE";
						case -0x7ffbfdd8:
							return "VFW_E_START_TIME_AFTER_END";
						case -0x7ffbfdd7:
							return "VFW_E_INVALID_RECT";
						case -0x7ffbfdd6:
							return "VFW_E_TYPE_NOT_ACCEPTED";
						case -0x7ffbfdd5:
							return "VFW_E_SAMPLE_REJECTED";
						case -0x7ffbfdd4:
							return "VFW_E_SAMPLE_REJECTED_EOS";
						case -0x7ffbfdd3:
							return "VFW_E_DUPLICATE_NAME";
						case -0x7ffbfdd2:
							return "VFW_E_TIMEOUT";
						case -0x7ffbfdd1:
							return "VFW_E_INVALID_FILE_FORMAT";
						case -0x7ffbfdd0:
							return "VFW_E_ENUM_OUT_OF_RANGE";
						case -0x7ffbfdcf:
							return "VFW_E_CIRCULAR_GRAPH";
						case -0x7ffbfdce:
							return "VFW_E_NOT_ALLOWED_TO_SAVE";
						case -0x7ffbfdcd:
							return "VFW_E_TIME_ALREADY_PASSED";
						case -0x7ffbfdcc:
							return "VFW_E_ALREADY_CANCELLED";
						case -0x7ffbfdcb:
							return "VFW_E_CORRUPT_GRAPH_FILE";
						case -0x7ffbfdca:
							return "VFW_E_ADVISE_ALREADY_SET";
						case -0x7ffbfdc8:
							return "VFW_E_NO_MODEX_AVAILABLE";
						case -0x7ffbfdc7:
							return "VFW_E_NO_ADVISE_SET";
						case -0x7ffbfdc6:
							return "VFW_E_NO_FULLSCREEN";
						case -0x7ffbfdc5:
							return "VFW_E_IN_FULLSCREEN_MODE";
						case -0x7ffbfdc0:
							return "VFW_E_UNKNOWN_FILE_TYPE";
						case -0x7ffbfdbf:
							return "VFW_E_CANNOT_LOAD_SOURCE_FILTER";
						case -0x7ffbfdbd:
							return "VFW_E_FILE_TOO_SHORT";
						case -0x7ffbfdbc:
							return "VFW_E_INVALID_FILE_VERSION";
						case -0x7ffbfdb9:
							return "VFW_E_INVALID_CLSID";
						}
					}
				}
				else {
					if (a_eError < -0x7ffbfd79) {
						switch (a_eError) {
						case -0x7ffbfdb7:
							return "VFW_E_SAMPLE_TIME_NOT_SET";
						case -0x7ffbfdb6:
						case -0x7ffbfdb5:
						case -0x7ffbfdb4:
						case -0x7ffbfdb3:
						case -0x7ffbfdb2:
						case -0x7ffbfdb1:
						case -0x7ffbfdb0:
						case -0x7ffbfdac:
						case -0x7ffbfda9:
						case -0x7ffbfda8:
						case -0x7ffbfda6:
						case -0x7ffbfda0:
						case -0x7ffbfd9d:
						case -0x7ffbfd99:
						case -0x7ffbfd98:
						case -0x7ffbfd96:
						case -0x7ffbfd95:
						case -0x7ffbfd94:
						case -0x7ffbfd93:
						case -0x7ffbfd92:
						case -0x7ffbfd91:
						case -0x7ffbfd90:
						case -0x7ffbfd82:
						case -0x7ffbfd80:
							goto switchD_006e0d79_caseD_7;
						case -0x7ffbfdaf:
							return "VFW_E_MEDIA_TIME_NOT_SET";
						case -0x7ffbfdae:
							return "VFW_E_NO_TIME_FORMAT_SET";
						case -0x7ffbfdad:
							return "VFW_E_MONO_AUDIO_HW";
						case -0x7ffbfdab:
							return "VFW_E_NO_DECOMPRESSOR";
						case -0x7ffbfdaa:
							return "VFW_E_NO_AUDIO_HARDWARE";
						case -0x7ffbfda7:
							return "VFW_E_RPZA";
						case -0x7ffbfda5:
							return "VFW_E_PROCESSOR_NOT_SUITABLE";
						case -0x7ffbfda4:
							return "VFW_E_UNSUPPORTED_AUDIO";
						case -0x7ffbfda3:
							return "VFW_E_UNSUPPORTED_VIDEO";
						case -0x7ffbfda2:
							return "VFW_E_MPEG_NOT_CONSTRAINED";
						case -0x7ffbfda1:
							return "VFW_E_NOT_IN_GRAPH";
						case -0x7ffbfd9f:
							return "VFW_E_NO_TIME_FORMAT";
						case -0x7ffbfd9e:
							return "VFW_E_READ_ONLY";
						case -0x7ffbfd9c:
							return "VFW_E_BUFFER_UNDERFLOW";
						case -0x7ffbfd9b:
							return "VFW_E_UNSUPPORTED_STREAM";
						case -0x7ffbfd9a:
							return "VFW_E_NO_TRANSPORT";
						case -0x7ffbfd97:
							return "VFW_E_BAD_VIDEOCD";
						case -0x7ffbfd8f:
							return "VFW_E_OUT_OF_VIDEO_MEMORY";
						case -0x7ffbfd8e:
							return "VFW_E_VP_NEGOTIATION_FAILED";
						case -0x7ffbfd8d:
							return "VFW_E_DDRAW_CAPS_NOT_SUITABLE";
						case -0x7ffbfd8c:
							return "VFW_E_NO_VP_HARDWARE";
						case -0x7ffbfd8b:
							return "VFW_E_NO_CAPTURE_HARDWARE";
						case -0x7ffbfd8a:
							return "VFW_E_DVD_OPERATION_INHIBITED";
						case -0x7ffbfd89:
							return "VFW_E_DVD_INVALIDDOMAIN";
						case -0x7ffbfd88:
							return "VFW_E_DVD_NO_BUTTON";
						case -0x7ffbfd87:
							return "VFW_E_DVD_GRAPHNOTREADY";
						case -0x7ffbfd86:
							return "VFW_E_DVD_RENDERFAIL";
						case -0x7ffbfd85:
							return "VFW_E_DVD_DECNOTENOUGH";
						case -0x7ffbfd84:
							return "VFW_E_DDRAW_VERSION_NOT_SUITABLE";
						case -0x7ffbfd83:
							return "VFW_E_COPYPROT_FAILED";
						case -0x7ffbfd81:
							return "VFW_E_TIME_EXPIRED";
						case -0x7ffbfd7f:
							return "VFW_E_DVD_WRONG_SPEED";
						case -0x7ffbfd7e:
							return "VFW_E_DVD_MENU_DOES_NOT_EXIST";
						case -0x7ffbfd7d:
							return "VFW_E_DVD_CMD_CANCELLED";
						case -0x7ffbfd7c:
							return "VFW_E_DVD_STATE_WRONG_VERSION";
						case -0x7ffbfd7b:
							return "VFW_E_DVD_STATE_CORRUPT";
						default:
							return "VFW_E_DVD_STATE_WRONG_DISC";
						}
					}
					if (a_eError < -0x7ffbfcff) {
						if (a_eError == -0x7ffbfd00) {
							return "DIERR_DRIVERFIRST";
						}
						switch (a_eError) {
						case -0x7ffbfd79:
							return "VFW_E_DVD_INCOMPATIBLE_REGION";
						case -0x7ffbfd78:
							return "VFW_E_DVD_NO_ATTRIBUTES";
						case -0x7ffbfd77:
							return "VFW_E_DVD_NO_GOUP_PGC";
						case -0x7ffbfd76:
							return "VFW_E_DVD_LOW_PARENTAL_LEVEL";
						case -0x7ffbfd75:
							return "VFW_E_DVD_NOT_IN_KARAOKE_MODE";
						case -0x7ffbfd72:
							return "VFW_E_FRAME_STEP_UNSUPPORTED";
						case -0x7ffbfd71:
							return "VFW_E_DVD_STREAM_DISABLED";
						case -0x7ffbfd70:
							return "VFW_E_DVD_TITLE_UNKNOWN";
						case -0x7ffbfd6f:
							return "VFW_E_DVD_INVALID_DISC";
						case -0x7ffbfd6e:
							return "VFW_E_DVD_NO_RESUME_INFORMATION";
						case -0x7ffbfd6d:
							return "VFW_E_PIN_ALREADY_BLOCKED_ON_THIS_THREAD";
						case -0x7ffbfd6c:
							return "VFW_E_PIN_ALREADY_BLOCKED";
						case -0x7ffbfd6b:
							return "VFW_E_CERTIFICATION_FAILURE";
						}
					}
					else if (a_eError < -0x7ff8fffa) {
						if (a_eError == -0x7ff8fffb) {
							return "E_ACCESSDENIED";
						}
						if (a_eError < -0x7ffbfc0d) {
							if (a_eError == -0x7ffbfc0e) {
								return "VFW_E_BAD_KEY";
							}
							if (a_eError == -0x7ffbfcff) {
								return "DIERR_DRIVERFIRST+1";
							}
							if (a_eError == -0x7ffbfcfe) {
								return "DIERR_DRIVERFIRST+2";
							}
							if (a_eError == -0x7ffbfcfd) {
								return "DIERR_DRIVERFIRST+3";
							}
							if (a_eError == -0x7ffbfcfc) {
								return "DIERR_DRIVERFIRST+4";
							}
							if (a_eError == -0x7ffbfcfb) {
								return "DIERR_DRIVERFIRST+5";
							}
						}
						else {
							if (a_eError == -0x7ffbfc01) {
								return "DIERR_DRIVERLAST";
							}
							if (a_eError == -0x7ffbfc00) {
								return "DIERR_INVALIDCLASSINSTALLER";
							}
							if (a_eError == -0x7ffbfbff) {
								return "DIERR_CANCELLED";
							}
							if (a_eError == -0x7ffbfbfe) {
								return "DIERR_BADINF";
							}
							if (a_eError == -0x7ff8fffe) goto switchD_006e0d79_caseD_2;
						}
					}
					else if (a_eError < -0x7ff8ffa8) {
						if (a_eError == -0x7ff8ffa9) {
							return "E_INVALIDARG";
						}
						if (a_eError == -0x7ff8fffa) {
							return "E_HANDLE";
						}
						if (a_eError == -0x7ff8fff4) {
							return "DIERR_NOTACQUIRED";
						}
						if (a_eError == -0x7ff8fff2) {
							return "E_OUTOFMEMORY";
						}
						if (a_eError == -0x7ff8ffeb) {
							return "DIERR_NOTINITIALIZED";
						}
						if (a_eError == -0x7ff8ffe2) {
							return "DIERR_INPUTLOST";
						}
					}
					else {
						if (a_eError == -0x7ff8ff89) {
							return "DIERR_BADDRIVERVER";
						}
						if (a_eError == -0x7ff8ff56) {
							return "DIERR_ACQUIRED";
						}
						if (a_eError == -0x7ff8fefd) {
							return "DIERR_NOMOREITEMS";
						}
						if (a_eError == -0x7ff8fb82) {
							return "DIERR_OLDDIRECTINPUTVERSION";
						}
					}
				}
			}
			else if (a_eError < -0x7fea7d6f) {
				if (a_eError == -0x7fea7d70) {
					return "DPNERR_INVALIDADDRESSFORMAT";
				}
				if (a_eError < -0x7feafe78) {
					if (a_eError == -0x7feafe79) {
						return "DVERR_INITIALIZED";
					}
					if (a_eError < -0x7feafe8f) {
						if (a_eError == -0x7feafe90) {
							return "DVERR_INVALIDTARGET";
						}
						if (a_eError < -0x7feaff69) {
							if (a_eError == -0x7feaff6a) {
								return "DVERR_INVALIDHANDLE";
							}
							if (a_eError < -0x7feaffb5) {
								if (a_eError == -0x7feaffb6) {
									return "DVERR_EXCEPTION";
								}
								if (a_eError == -0x7ff8fb70) {
									return "E_PROP_ID_UNSUPPORTED";
								}
								if (a_eError == -0x7ff8fb6e) {
									return "E_PROP_SET_UNSUPPORTED";
								}
								if (a_eError == -0x7ff8fb21) {
									return "DIERR_ALREADYINITIALIZED";
								}
								if (a_eError == -0x7feaffe2) {
									return "DVERR_BUFFERTOOSMALL";
								}
							}
							else {
								if (a_eError == -0x7feaff88) {
									return "DVERR_INVALIDFLAGS";
								}
								if (a_eError == -0x7feaff7e) {
									return "DVERR_INVALIDOBJECT";
								}
								if (a_eError == -0x7feaff79) {
									return "DVERR_INVALIDPLAYER";
								}
								if (a_eError == -0x7feaff6f) {
									return "DVERR_INVALIDGROUP";
								}
							}
						}
						else if (a_eError < -0x7feafe95) {
							if (a_eError == -0x7feafe96) {
								return "DVERR_CONNECTED";
							}
							if (a_eError == -0x7feafed4) {
								return "DVERR_SESSIONLOST";
							}
							if (a_eError == -0x7feafed2) {
								return "DVERR_NOVOICESESSION";
							}
							if (a_eError == -0x7feafe98) {
								return "DVERR_CONNECTIONLOST";
							}
							if (a_eError == -0x7feafe97) {
								return "DVERR_NOTINITIALIZED";
							}
						}
						else {
							if (a_eError == -0x7feafe95) {
								return "DVERR_NOTCONNECTED";
							}
							if (a_eError == -0x7feafe92) {
								return "DVERR_CONNECTABORTING";
							}
							if (a_eError == -0x7feafe91) {
								return "DVERR_NOTALLOWED";
							}
						}
					}
					else {
						switch (a_eError) {
						case -0x7feafe8f:
							return "DVERR_TRANSPORTNOTHOST";
						case -0x7feafe8e:
							return "DVERR_COMPRESSIONNOTSUPPORTED";
						case -0x7feafe8d:
							return "DVERR_ALREADYPENDING";
						case -0x7feafe8c:
							return "DVERR_SOUNDINITFAILURE";
						case -0x7feafe8b:
							return "DVERR_TIMEOUT";
						case -0x7feafe8a:
							return "DVERR_CONNECTABORTED";
						case -0x7feafe89:
							return "DVERR_NO3DSOUND";
						case -0x7feafe88:
							return "DVERR_ALREADYBUFFERED";
						case -0x7feafe87:
							return "DVERR_NOTBUFFERED";
						case -0x7feafe86:
							return "DVERR_HOSTING";
						case -0x7feafe85:
							return "DVERR_NOTHOSTING";
						case -0x7feafe84:
							return "DVERR_INVALIDDEVICE";
						case -0x7feafe83:
							return "DVERR_RECORDSYSTEMERROR";
						case -0x7feafe82:
							return "DVERR_PLAYBACKSYSTEMERROR";
						case -0x7feafe81:
							return "DVERR_SENDERROR";
						case -0x7feafe80:
							return "DVERR_USERCANCEL";
						case -0x7feafe7d:
							return "DVERR_RUNSETUP";
						case -0x7feafe7c:
							return "DVERR_INCOMPATIBLEVERSION";
						}
					}
				}
				else if (a_eError < -0x7fea7fcf) {
					if (a_eError == -0x7fea7fd0) {
						return "DPNERR_ABORTED";
					}
					switch (a_eError) {
					case -0x7feafe78:
						return "DVERR_NOTRANSPORT";
					case -0x7feafe77:
						return "DVERR_NOCALLBACK";
					case -0x7feafe76:
						return "DVERR_TRANSPORTNOTINIT";
					case -0x7feafe75:
						return "DVERR_TRANSPORTNOSESSION";
					case -0x7feafe74:
						return "DVERR_TRANSPORTNOPLAYER";
					case -0x7feafe73:
						return "DVERR_USERBACK";
					case -0x7feafe72:
						return "DVERR_NORECVOLAVAILABLE";
					case -0x7feafe71:
						return "DVERR_INVALIDBUFFER";
					case -0x7feafe70:
						return "DVERR_LOCKEDBUFFER";
					}
				}
				else if (a_eError < -0x7fea7e8f) {
					if (a_eError == -0x7fea7e90) {
						return "DPNERR_CONVERSION";
					}
					if (a_eError < -0x7fea7eff) {
						if (a_eError == -0x7fea7f00) {
							return "DPNERR_BUFFERTOOSMALL";
						}
						if (a_eError == -0x7fea7fc0) {
							return "DPNERR_ADDRESSING";
						}
						if (a_eError == -0x7fea7fb0) {
							return "DPNERR_ALREADYCLOSING";
						}
						if (a_eError == -0x7fea7fa0) {
							return "DPNERR_ALREADYCONNECTED";
						}
						if (a_eError == -0x7fea7f90) {
							return "DPNERR_ALREADYDISCONNECTING";
						}
						if (a_eError == -0x7fea7f80) {
							return "DPNERR_ALREADYINITIALIZED";
						}
						if (a_eError == -0x7fea7f70) {
							return "DPNERR_ALREADYREGISTERED";
						}
					}
					else {
						if (a_eError == -0x7fea7ef0) {
							return "DPNERR_CANNOTCANCEL";
						}
						if (a_eError == -0x7fea7ee0) {
							return "DPNERR_CANTCREATEGROUP";
						}
						if (a_eError == -0x7fea7ed0) {
							return "DPNERR_CANTCREATEPLAYER";
						}
						if (a_eError == -0x7fea7ec0) {
							return "DPNERR_CANTLAUNCHAPPLICATION";
						}
						if (a_eError == -0x7fea7eb0) {
							return "DPNERR_CONNECTING";
						}
						if (a_eError == -0x7fea7ea0) {
							return "DPNERR_CONNECTIONLOST";
						}
					}
				}
				else if (a_eError < -0x7fea7dcf) {
					if (a_eError == -0x7fea7dd0) {
						return "DPNERR_EXCEPTION";
					}
					if (a_eError == -0x7fea7e8b) {
						return "DPNERR_DATATOOLARGE";
					}
					if (a_eError == -0x7fea7e80) {
						return "DPNERR_DOESNOTEXIST";
					}
					if (a_eError == -0x7fea7e70) {
						return "DPNERR_DUPLICATECOMMAND";
					}
					if (a_eError == -0x7fea7e00) {
						return "DPNERR_ENDPOINTNOTRECEIVING";
					}
					if (a_eError == -0x7fea7df0) {
						return "DPNERR_ENUMQUERYTOOLARGE";
					}
					if (a_eError == -0x7fea7de0) {
						return "DPNERR_ENUMRESPONSETOOLARGE";
					}
				}
				else {
					if (a_eError == -0x7fea7dc0) {
						return "DPNERR_GROUPNOTEMPTY";
					}
					if (a_eError == -0x7fea7db0) {
						return "DPNERR_HOSTING";
					}
					if (a_eError == -0x7fea7da0) {
						return "DPNERR_HOSTREJECTEDCONNECTION";
					}
					if (a_eError == -0x7fea7d90) {
						return "DPNERR_HOSTTERMINATEDSESSION";
					}
					if (a_eError == -0x7fea7d80) {
						return "DPNERR_INCOMPLETEADDRESS";
					}
				}
			}
			else if (a_eError < -0x7789fff5) {
				if (a_eError == -0x7789fff6) {
					return "DDERR_CANNOTATTACHSURFACE";
				}
				if (a_eError < -0x7fea7b7f) {
					if (a_eError == -0x7fea7b80) {
						return "DPNERR_NOCONNECTION";
					}
					if (a_eError < -0x7fea7c6f) {
						if (a_eError == -0x7fea7c70) {
							return "DPNERR_INVALIDINTERFACE";
						}
						if (a_eError < -0x7fea7cbf) {
							if (a_eError == -0x7fea7cc0) {
								return "DPNERR_INVALIDFLAGS";
							}
							if (a_eError == -0x7fea7d00) {
								return "DPNERR_INVALIDAPPLICATION";
							}
							if (a_eError == -0x7fea7cf0) {
								return "DPNERR_INVALIDCOMMAND";
							}
							if (a_eError == -0x7fea7ce0) {
								return "DPNERR_INVALIDDEVICEADDRESS";
							}
							if (a_eError == -0x7fea7cd0) {
								return "DPNERR_INVALIDENDPOINT";
							}
						}
						else {
							if (a_eError == -0x7fea7cb0) {
								return "DPNERR_INVALIDGROUP";
							}
							if (a_eError == -0x7fea7ca0) {
								return "DPNERR_INVALIDHANDLE";
							}
							if (a_eError == -0x7fea7c90) {
								return "DPNERR_INVALIDHOSTADDRESS";
							}
							if (a_eError == -0x7fea7c80) {
								return "DPNERR_INVALIDINSTANCE";
							}
						}
					}
					else if (a_eError < -0x7fea7bbf) {
						if (a_eError == -0x7fea7bc0) {
							return "DPNERR_INVALIDSTRING";
						}
						if (a_eError == -0x7fea7c00) {
							return "DPNERR_INVALIDOBJECT";
						}
						if (a_eError == -0x7fea7bf0) {
							return "DPNERR_INVALIDPASSWORD";
						}
						if (a_eError == -0x7fea7be0) {
							return "DPNERR_INVALIDPLAYER";
						}
						if (a_eError == -0x7fea7bd0) {
							return "DPNERR_INVALIDPRIORITY";
						}
					}
					else {
						if (a_eError == -0x7fea7bb0) {
							return "DPNERR_INVALIDURL";
						}
						if (a_eError == -0x7fea7ba0) {
							return "DPNERR_INVALIDVERSION";
						}
						if (a_eError == -0x7fea7b90) {
							return "DPNERR_NOCAPS";
						}
					}
				}
				else if (a_eError < -0x7fea7a7f) {
					if (a_eError == -0x7fea7a80) {
						return "DPNERR_PLAYERNOTINGROUP";
					}
					if (a_eError < -0x7fea7acf) {
						if (a_eError == -0x7fea7ad0) {
							return "DPNERR_NOTHOST";
						}
						if (a_eError == -0x7fea7b70) {
							return "DPNERR_NOHOSTPLAYER";
						}
						if (a_eError == -0x7fea7b00) {
							return "DPNERR_NOMOREADDRESSCOMPONENTS";
						}
						if (a_eError == -0x7fea7af0) {
							return "DPNERR_NORESPONSE";
						}
						if (a_eError == -0x7fea7ae0) {
							return "DPNERR_NOTALLOWED";
						}
					}
					else {
						if (a_eError == -0x7fea7ac0) {
							return "DPNERR_NOTREADY";
						}
						if (a_eError == -0x7fea7ab0) {
							return "DPNERR_NOTREGISTERED";
						}
						if (a_eError == -0x7fea7aa0) {
							return "DPNERR_PLAYERALREADYINGROUP";
						}
						if (a_eError == -0x7fea7a90) {
							return "DPNERR_PLAYERLOST";
						}
					}
				}
				else if (a_eError < -0x7fea79cf) {
					if (a_eError == -0x7fea79d0) {
						return "DPNERR_TIMEDOUT";
					}
					if (a_eError == -0x7fea7a70) {
						return "DPNERR_PLAYERNOTREACHABLE";
					}
					if (a_eError == -0x7fea7a00) {
						return "DPNERR_SENDTOOLARGE";
					}
					if (a_eError == -0x7fea79f0) {
						return "DPNERR_SESSIONFULL";
					}
					if (a_eError == -0x7fea79e0) {
						return "DPNERR_TABLEFULL";
					}
				}
				else {
					if (a_eError == -0x7fea79c0) {
						return "DPNERR_UNINITIALIZED";
					}
					if (a_eError == -0x7fea79b0) {
						return "DPNERR_USERCANCEL";
					}
					if (a_eError == -0x7789fffb) {
						return "DDERR_ALREADYINITIALIZED";
					}
				}
			}
			else if (a_eError < -0x7789ff2b) {
				if (a_eError == -0x7789ff2c) {
					return "DDERR_NOCOOPERATIVELEVELSET";
				}
				if (a_eError < -0x7789ff6e) {
					if (a_eError == -0x7789ff6f) {
						return "DDERR_INVALIDPIXELFORMAT";
					}
					if (a_eError < -0x7789ffa0) {
						if (a_eError == -0x7789ffa1) {
							return "DDERR_INCOMPATIBLEPRIMARY";
						}
						if (a_eError == -0x7789ffec) {
							return "DDERR_CANNOTDETACHSURFACE";
						}
						if (a_eError == -0x7789ffd8) {
							return "DDERR_CURRENTLYNOTAVAIL";
						}
						if (a_eError == -0x7789ffc9) {
							return "DDERR_EXCEPTION";
						}
						if (a_eError == -0x7789ffa6) {
							return "DDERR_HEIGHTALIGN";
						}
					}
					else {
						if (a_eError == -0x7789ff9c) {
							return "DDERR_INVALIDCAPS";
						}
						if (a_eError == -0x7789ff92) {
							return "DDERR_INVALIDCLIPLIST";
						}
						if (a_eError == -0x7789ff88) {
							return "DDERR_INVALIDMODE";
						}
						if (a_eError == -0x7789ff7e) {
							return "DDERR_INVALIDOBJECT";
						}
					}
				}
				else if (a_eError < -0x7789ff4a) {
					if (a_eError == -0x7789ff4b) {
						return "DDERR_NOSTEREOHARDWARE";
					}
					if (a_eError == -0x7789ff6a) {
						return "DDERR_INVALIDRECT";
					}
					if (a_eError == -0x7789ff60) {
						return "DDERR_LOCKEDSURFACES";
					}
					if (a_eError == -0x7789ff56) {
						return "DDERR_NO3D";
					}
					if (a_eError == -0x7789ff4c) {
						return "DDERR_NOALPHAHW";
					}
				}
				else {
					if (a_eError == -0x7789ff4a) {
						return "DDERR_NOSURFACELEFT";
					}
					if (a_eError == -0x7789ff33) {
						return "DDERR_NOCLIPLIST";
					}
					if (a_eError == -0x7789ff2e) {
						return "DDERR_NOCOLORCONVHW";
					}
				}
			}
			else if (a_eError < -0x7789fefb) {
				if (a_eError == -0x7789fefc) {
					return "DDERR_NOOVERLAYHW";
				}
				if (a_eError < -0x7789ff19) {
					if (a_eError == -0x7789ff1a) {
						return "DDERR_NOFLIPHW";
					}
					if (a_eError == -0x7789ff29) {
						return "DDERR_NOCOLORKEY";
					}
					if (a_eError == -0x7789ff24) {
						return "DDERR_NOCOLORKEYHW";
					}
					if (a_eError == -0x7789ff22) {
						return "DDERR_NODIRECTDRAWSUPPORT";
					}
					if (a_eError == -0x7789ff1f) {
						return "DDERR_NOEXCLUSIVEMODE";
					}
				}
				else {
					if (a_eError == -0x7789ff10) {
						return "DDERR_NOGDI";
					}
					if (a_eError == -0x7789ff06) {
						return "DDERR_NOMIRRORHW";
					}
					if (a_eError == -0x7789ff01) {
						return "DDERR_NOTFOUND";
					}
				}
			}
			else if (a_eError < -0x7789fec3) {
				if (a_eError == -0x7789fec4) {
					return "DDERR_NOT4BITCOLOR";
				}
				if (a_eError == -0x7789fef2) {
					return "DDERR_OVERLAPPINGRECTS";
				}
				if (a_eError == -0x7789fee8) {
					return "DDERR_NORASTEROPHW";
				}
				if (a_eError == -0x7789fede) {
					return "DDERR_NOROTATIONHW";
				}
				if (a_eError == -0x7789feca) {
					return "DDERR_NOSTRETCHHW";
				}
			}
			else {
				if (a_eError == -0x7789fec3) {
					return "DDERR_NOT4BITCOLORINDEX";
				}
				if (a_eError == -0x7789fec0) {
					return "DDERR_NOT8BITCOLOR";
				}
				if (a_eError == -0x7789feb6) {
					return "DDERR_NOTEXTUREHW";
				}
			}
			goto switchD_006e0d79_caseD_7;
		}
		if (a_eError < -0x7787eef6) {
			if (a_eError == -0x7787eef7) {
				return "DMUS_E_INVALIDOFFSET";
			}
			if (-0x7789fd45 < a_eError) {
				if (a_eError < -0x7789f7e7) {
					if (a_eError == -0x7789f7e8) {
						return "D3DERR_WRONGTEXTUREFORMAT";
					}
					switch (a_eError) {
					case -0x7789fcae:
						return "DXFILEERR_BADOBJECT";
					case -0x7789fcad:
						return "DXFILEERR_BADVALUE";
					case -0x7789fcac:
						return "DXFILEERR_BADTYPE";
					case -0x7789fcab:
						return "DXFILEERR_BADSTREAMHANDLE";
					case -0x7789fcaa:
						return "DXFILEERR_BADALLOC";
					case -0x7789fca9:
						return "DXFILEERR_NOTFOUND";
					case -0x7789fca8:
						return "DXFILEERR_NOTDONEYET";
					case -0x7789fca7:
						return "DXFILEERR_FILENOTFOUND";
					case -0x7789fca6:
						return "DXFILEERR_RESOURCENOTFOUND";
					case -0x7789fca5:
						return "DXFILEERR_URLNOTFOUND";
					case -0x7789fca4:
						return "DXFILEERR_BADRESOURCE";
					case -0x7789fca3:
						return "DXFILEERR_BADFILETYPE";
					case -0x7789fca2:
						return "DXFILEERR_BADFILEVERSION";
					case -0x7789fca1:
						return "DXFILEERR_BADFILEFLOATSIZE";
					case -0x7789fca0:
						return "DXFILEERR_BADFILECOMPRESSIONTYPE";
					case -0x7789fc9f:
						return "DXFILEERR_BADFILE";
					case -0x7789fc9e:
						return "DXFILEERR_PARSEERROR";
					case -0x7789fc9d:
						return "DXFILEERR_NOTEMPLATE";
					case -0x7789fc9c:
						return "DXFILEERR_BADARRAYSIZE";
					case -0x7789fc9b:
						return "DXFILEERR_BADDATAREFERENCE";
					case -0x7789fc9a:
						return "DXFILEERR_INTERNALERROR";
					case -0x7789fc99:
						return "DXFILEERR_NOMOREOBJECTS";
					case -0x7789fc98:
						return "DXFILEERR_BADINTRINSICS";
					case -0x7789fc97:
						return "DXFILEERR_NOMORESTREAMHANDLES";
					case -0x7789fc96:
						return "DXFILEERR_NOMOREDATA";
					case -0x7789fc95:
						return "DXFILEERR_BADCACHEFILE";
					case -0x7789fc94:
						return "DXFILEERR_NOINTERNET";
					}
				}
				else if (a_eError < -0x7789f799) {
					if (a_eError == -0x7789f79a) {
						return "D3DERR_NOTFOUND";
					}
					switch (a_eError) {
					case -0x7789f7e7:
						return "D3DERR_UNSUPPORTEDCOLOROPERATION";
					case -0x7789f7e6:
						return "D3DERR_UNSUPPORTEDCOLORARG";
					case -0x7789f7e5:
						return "D3DERR_UNSUPPORTEDALPHAOPERATION";
					case -0x7789f7e4:
						return "D3DERR_UNSUPPORTEDALPHAARG";
					case -0x7789f7e3:
						return "D3DERR_TOOMANYOPERATIONS";
					case -0x7789f7e2:
						return "D3DERR_CONFLICTINGTEXTUREFILTER";
					case -0x7789f7e1:
						return "D3DERR_UNSUPPORTEDFACTORVALUE";
					case -0x7789f7df:
						return "D3DERR_CONFLICTINGRENDERSTATE";
					case -0x7789f7de:
						return "D3DERR_UNSUPPORTEDTEXTUREFILTER";
					case -0x7789f7da:
						return "D3DERR_CONFLICTINGTEXTUREPALETTE";
					case -0x7789f7d9:
						return "D3DERR_DRIVERINTERNALERROR";
					}
				}
				else if (a_eError < -0x7787ff9b) {
					if (a_eError == -0x7787ff9c) {
						return "DSERR_BADFORMAT";
					}
					if (a_eError < -0x7789f4aa) {
						if (a_eError == -0x7789f4ab) {
							return "D3DXERR_INVALIDMESH";
						}
						if (a_eError < -0x7789f794) {
							if (a_eError == -0x7789f795) {
								return "D3DERR_INVALIDDEVICE";
							}
							if (a_eError == -0x7789f799) {
								return "D3DERR_MOREDATA";
							}
							if (a_eError == -0x7789f798) {
								return "D3DERR_DEVICELOST";
							}
							if (a_eError == -0x7789f797) {
								return "D3DERR_DEVICENOTRESET";
							}
							if (a_eError == -0x7789f796) {
								return "D3DERR_NOTAVAILABLE";
							}
						}
						else {
							if (a_eError == -0x7789f794) {
								return "D3DERR_INVALIDCALL";
							}
							if (a_eError == -0x7789f793) {
								return "D3DERR_DRIVERINVALIDCALL";
							}
							if (a_eError == -0x7789f4ac) {
								return "D3DXERR_CANNOTMODIFYINDEXBUFFER";
							}
						}
					}
					else if (a_eError < -0x7787fff5) {
						if (a_eError == -0x7787fff6) {
							return "DSERR_ALLOCATED";
						}
						if (a_eError == -0x7789f4aa) {
							return "D3DXERR_CANNOTATTRSORT";
						}
						if (a_eError == -0x7789f4a9) {
							return "D3DXERR_SKINNINGNOTSUPPORTED";
						}
						if (a_eError == -0x7789f4a8) {
							return "D3DXERR_TOOMANYINFLUENCES";
						}
						if (a_eError == -0x7789f4a7) {
							return "D3DXERR_INVALIDDATA";
						}
					}
					else {
						if (a_eError == -0x7787ffe2) {
							return "DSERR_CONTROLUNAVAIL";
						}
						if (a_eError == -0x7787ffce) {
							return "DSERR_INVALIDCALL";
						}
						if (a_eError == -0x7787ffba) {
							return "DSERR_PRIOLEVELNEEDED";
						}
					}
				}
				else if (a_eError < -0x7787ff2d) {
					if (a_eError == -0x7787ff2e) {
						return "DSERR_BADSENDBUFFERGUID";
					}
					if (a_eError < -0x7787ff55) {
						if (a_eError == -0x7787ff56) {
							return "DSERR_UNINITIALIZED";
						}
						if (a_eError == -0x7787ff88) {
							return "DSERR_NODRIVER";
						}
						if (a_eError == -0x7787ff7e) {
							return "DSERR_ALREADYINITIALIZED";
						}
						if (a_eError == -0x7787ff6a) {
							return "DSERR_BUFFERLOST";
						}
						if (a_eError == -0x7787ff60) {
							return "DSERR_OTHERAPPHASPRIO";
						}
					}
					else {
						if (a_eError == -0x7787ff4c) {
							return "DSERR_BUFFERTOOSMALL";
						}
						if (a_eError == -0x7787ff42) {
							return "DSERR_DS8_REQUIRED";
						}
						if (a_eError == -0x7787ff38) {
							return "DSERR_SENDLOOP";
						}
					}
				}
				else {
					if (a_eError == -0x7787eeff) {
						return "DMUS_E_DRIVER_FAILED";
					}
					if (a_eError == -0x7787eefe) {
						return "DMUS_E_PORTS_OPEN";
					}
					if (a_eError == -0x7787eefd) {
						return "DMUS_E_DEVICE_IN_USE";
					}
					if (a_eError == -0x7787eefc) {
						return "DMUS_E_INSUFFICIENTBUFFER";
					}
					if (a_eError == -0x7787eefb) {
						return "DMUS_E_BUFFERNOTSET";
					}
					if (a_eError == -0x7787eefa) {
						return "DMUS_E_BUFFERNOTAVAILABLE";
					}
					if (a_eError == -0x7787eef8) {
						return "DMUS_E_NOTADLSCOL";
					}
				}
				goto switchD_006e0d79_caseD_7;
			}
			if (a_eError == -0x7789fd45) {
				return "DDERR_DEVICEDOESNTOWNSURFACE";
			}
			switch (a_eError) {
			case -0x7789fdc2:
				pcVar1 = "DDERR_BLTFASTCANTCLIP";
				break;
			case -0x7789fdc1:
				pcVar1 = "DDERR_NOBLTHW";
				break;
			case -0x7789fdc0:
				pcVar1 = "DDERR_NODDROPSHW";
				break;
			case -0x7789fdbf:
				pcVar1 = "DDERR_OVERLAYNOTVISIBLE";
				break;
			case -0x7789fdbe:
				pcVar1 = "DDERR_NOOVERLAYDEST";
				break;
			case -0x7789fdbd:
				pcVar1 = "DDERR_INVALIDPOSITION";
				break;
			case -0x7789fdbc:
				pcVar1 = "DDERR_NOTAOVERLAYSURFACE";
				break;
			case -0x7789fdbb:
				pcVar1 = "DDERR_EXCLUSIVEMODEALREADYSET";
				break;
			case -0x7789fdba:
				pcVar1 = "DDERR_NOTFLIPPABLE";
				break;
			case -0x7789fdb9:
				pcVar1 = "DDERR_CANTDUPLICATE";
				break;
			case -0x7789fdb8:
				pcVar1 = "DDERR_NOTLOCKED";
				break;
			case -0x7789fdb7:
				pcVar1 = "DDERR_CANTCREATEDC";
				break;
			case -0x7789fdb6:
				pcVar1 = "DDERR_NODC";
				break;
			case -0x7789fdb5:
				pcVar1 = "DDERR_WRONGMODE";
				break;
			case -0x7789fdb4:
				pcVar1 = "DDERR_IMPLICITLYCREATED";
				break;
			case -0x7789fdb3:
				pcVar1 = "DDERR_NOTPALETTIZED";
				break;
			case -0x7789fdb2:
				pcVar1 = "DDERR_UNSUPPORTEDMODE";
				break;
			case -0x7789fdb1:
				pcVar1 = "DDERR_NOMIPMAPHW";
				break;
			case -0x7789fdb0:
				pcVar1 = "DDERR_INVALIDSURFACETYPE";
				break;
			case -0x7789fdaf:
			case -0x7789fdae:
			case -0x7789fdad:
			case -0x7789fdac:
			case -0x7789fdab:
			case -0x7789fdaa:
			case -0x7789fda9:
			case -0x7789fda4:
			case -0x7789fda3:
			case -0x7789fda2:
			case -0x7789fda1:
			case -0x7789fda0:
			case -0x7789fd9f:
			case -0x7789fd9e:
			case -0x7789fd9d:
			case -0x7789fd9c:
			case -0x7789fd9b:
			case -0x7789fd9a:
			case -0x7789fd99:
			case -0x7789fd98:
			case -0x7789fd97:
			case -0x7789fd96:
			case -0x7789fd95:
			case -0x7789fd93:
			case -0x7789fd92:
			case -0x7789fd91:
			case -0x7789fd90:
			case -0x7789fd8f:
			case -0x7789fd8e:
			case -0x7789fd8d:
			case -0x7789fd8c:
			case -0x7789fd8b:
			case -0x7789fd89:
			case -0x7789fd88:
			case -0x7789fd87:
			case -0x7789fd86:
			case -0x7789fd85:
			case -0x7789fd84:
			case -0x7789fd83:
			case -0x7789fd82:
			case -0x7789fd81:
			case -0x7789fd7f:
			case -0x7789fd7e:
			case -0x7789fd7d:
			case -0x7789fd7c:
			case -0x7789fd7b:
			case -0x7789fd7a:
			case -0x7789fd79:
			case -0x7789fd78:
			case -0x7789fd77:
			case -0x7789fd76:
			case -0x7789fd75:
			case -0x7789fd74:
			case -0x7789fd73:
			case -0x7789fd72:
			case -0x7789fd71:
			case -0x7789fd70:
			case -0x7789fd6f:
			case -0x7789fd6e:
			case -0x7789fd6d:
			case -0x7789fd6b:
			case -0x7789fd6a:
			case -0x7789fd69:
			case -0x7789fd68:
			case -0x7789fd67:
			case -0x7789fd66:
			case -0x7789fd65:
			case -0x7789fd64:
			case -0x7789fd63:
			case -0x7789fd62:
			case -0x7789fd61:
			case -0x7789fd60:
			case -0x7789fd5f:
			case -0x7789fd5e:
			case -0x7789fd5d:
			case -0x7789fd5c:
			case -0x7789fd5b:
			case -0x7789fd5a:
			case -0x7789fd59:
			case -0x7789fd57:
			case -0x7789fd56:
			case -0x7789fd55:
			case -0x7789fd54:
			case -0x7789fd53:
			case -0x7789fd52:
			case -0x7789fd51:
			case -0x7789fd50:
			case -0x7789fd4f:
				goto switchD_006e0d79_caseD_7;
			case -0x7789fda8:
				pcVar1 = "DDERR_NOOPTIMIZEHW";
				break;
			case -0x7789fda7:
				pcVar1 = "DDERR_NOTLOADED";
				break;
			case -0x7789fda6:
				pcVar1 = "DDERR_NOFOCUSWINDOW";
				break;
			case -0x7789fda5:
				pcVar1 = "DDERR_NOTONMIPMAPSUBLEVEL";
				break;
			case -0x7789fd94:
				pcVar1 = "DDERR_DCALREADYCREATED";
				break;
			case -0x7789fd8a:
				pcVar1 = "DDERR_NONONLOCALVIDMEM";
				break;
			case -0x7789fd80:
				pcVar1 = "DDERR_CANTPAGELOCK";
				break;
			case -0x7789fd6c:
				pcVar1 = "DDERR_CANTPAGEUNLOCK";
				break;
			case -0x7789fd58:
				pcVar1 = "DDERR_NOTPAGELOCKED";
				break;
			case -0x7789fd4e:
				pcVar1 = "DDERR_MOREDATA";
				break;
			case -0x7789fd4d:
				pcVar1 = "DDERR_EXPIRED";
				break;
			case -0x7789fd4c:
				pcVar1 = "DDERR_TESTFINISHED";
				break;
			case -0x7789fd4b:
				pcVar1 = "DDERR_NEWMODE";
				break;
			case -0x7789fd4a:
				pcVar1 = "DDERR_D3DNOTINITIALIZED";
				break;
			case -0x7789fd49:
				pcVar1 = "DDERR_VIDEONOTACTIVE";
				break;
			case -0x7789fd48:
				pcVar1 = "DDERR_NOMONITORINFORMATION";
				break;
			case -0x7789fd47:
				pcVar1 = "DDERR_NODRIVERSUPPORT";
				break;
			default:
				if (a_eError == -0x7789fdc3) {
					return "DDERR_NOPALETTEHW";
				}
				switch (a_eError) {
				case -0x7789fdf8:
					pcVar1 = "DDERR_UNSUPPORTEDMASK";
					break;
				case -0x7789fdf7:
					pcVar1 = "DDERR_INVALIDSTREAM";
					break;
				case -0x7789fdf6:
				case -0x7789fdf5:
				case -0x7789fdf4:
				case -0x7789fdf3:
				case -0x7789fdf2:
				case -0x7789fdf1:
				case -0x7789fdf0:
				case -0x7789fdef:
				case -0x7789fdee:
				case -0x7789fded:
				case -0x7789fdec:
				case -0x7789fdeb:
				case -0x7789fdea:
				case -0x7789fde9:
				case -0x7789fde8:
				case -0x7789fde6:
				case -0x7789fde5:
				case -0x7789fde3:
				case -0x7789fde1:
				case -0x7789fde0:
				case -0x7789fddf:
				case -0x7789fdde:
				case -0x7789fddd:
				case -0x7789fddc:
				case -0x7789fddb:
				case -0x7789fdda:
				case -0x7789fdd9:
				case -0x7789fdd8:
				case -0x7789fdd7:
				case -0x7789fdd6:
				case -0x7789fdd5:
				case -0x7789fdd4:
				case -0x7789fdd3:
				case -0x7789fdd2:
				case -0x7789fdd1:
					goto switchD_006e0d79_caseD_7;
				case -0x7789fde7:
					pcVar1 = "DDERR_VERTICALBLANKINPROGRESS";
					break;
				case -0x7789fde4:
					pcVar1 = "DDERR_WASSTILLDRAWING";
					break;
				case -0x7789fde2:
					pcVar1 = "DDERR_DDSCAPSCOMPLEXREQUIRED";
					break;
				case -0x7789fdd0:
					pcVar1 = "DDERR_XALIGN";
					break;
				case -0x7789fdcf:
					pcVar1 = "DDERR_INVALIDDIRECTDRAWGUID";
					break;
				case -0x7789fdce:
					pcVar1 = "DDERR_DIRECTDRAWALREADYCREATED";
					break;
				case -0x7789fdcd:
					pcVar1 = "DDERR_NODIRECTDRAWHW";
					break;
				case -0x7789fdcc:
					pcVar1 = "DDERR_PRIMARYSURFACEALREADYEXISTS";
					break;
				case -0x7789fdcb:
					pcVar1 = "DDERR_NOEMULATION";
					break;
				case -0x7789fdca:
					pcVar1 = "DDERR_REGIONTOOSMALL";
					break;
				case -0x7789fdc9:
					pcVar1 = "DDERR_CLIPPERISUSINGHWND";
					break;
				case -0x7789fdc8:
					pcVar1 = "DDERR_NOCLIPPERATTACHED";
					break;
				case -0x7789fdc7:
					pcVar1 = "DDERR_NOHWND";
					break;
				case -0x7789fdc6:
					pcVar1 = "DDERR_HWNDSUBCLASSED";
					break;
				case -0x7789fdc5:
					pcVar1 = "DDERR_HWNDALREADYSET";
					break;
				case -0x7789fdc4:
					pcVar1 = "DDERR_NOPALETTEATTACHED";
					break;
				default:
					if (a_eError == -0x7789fe02) {
						return "DDERR_UNSUPPORTEDFORMAT";
					}
					if (a_eError < -0x7789fe5b) {
						if (a_eError == -0x7789fe5c) {
							return "DDERR_SURFACEALREADYDEPENDENT";
						}
						if (a_eError < -0x7789fe81) {
							if (a_eError == -0x7789fe82) {
								return "DDERR_OVERLAYCANTCLIP";
							}
							if (a_eError == -0x7789feac) {
								return "DDERR_NOZBUFFERHW";
							}
							if (a_eError == -0x7789fea2) {
								return "DDERR_NOZOVERLAYHW";
							}
							if (a_eError == -0x7789fe98) {
								return "DDERR_OUTOFCAPS";
							}
							if (a_eError == -0x7789fe84) {
								return "D3DERR_OUTOFVIDEOMEMORY";
							}
						}
						else {
							if (a_eError == -0x7789fe80) {
								return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
							}
							if (a_eError == -0x7789fe7d) {
								return "DDERR_PALETTEBUSY";
							}
							if (a_eError == -0x7789fe70) {
								return "DDERR_COLORKEYNOTSET";
							}
							if (a_eError == -0x7789fe66) {
								return "DDERR_SURFACEALREADYATTACHED";
							}
						}
					}
					else if (a_eError < -0x7789fe33) {
						if (a_eError == -0x7789fe34) {
							return "DDERR_SURFACENOTATTACHED";
						}
						if (a_eError == -0x7789fe52) {
							return "DDERR_SURFACEBUSY";
						}
						if (a_eError == -0x7789fe4d) {
							return "DDERR_CANTLOCKSURFACE";
						}
						if (a_eError == -0x7789fe48) {
							return "DDERR_SURFACEISOBSCURED";
						}
						if (a_eError == -0x7789fe3e) {
							return "DDERR_SURFACELOST";
						}
					}
					else {
						if (a_eError == -0x7789fe2a) {
							return "DDERR_TOOBIGHEIGHT";
						}
						if (a_eError == -0x7789fe20) {
							return "DDERR_TOOBIGSIZE";
						}
						if (a_eError == -0x7789fe16) {
							return "DDERR_TOOBIGWIDTH";
						}
					}
					goto switchD_006e0d79_caseD_7;
				}
			}
		}
		else {
			if (a_eError < -0x7787ee8f) {
				switch (a_eError) {
				case -0x7787eeef:
					return "DMUS_E_ALREADY_LOADED";
				case -0x7787eeee:
				case -0x7787eee8:
				case -0x7787eee6:
				case -0x7787eee5:
				case -0x7787eee4:
				case -0x7787eee3:
				case -0x7787eee2:
				case -0x7787eee1:
				case -0x7787eec0:
				case -0x7787eebf:
				case -0x7787eebe:
				case -0x7787eebd:
				case -0x7787eebc:
				case -0x7787eebb:
				case -0x7787eeba:
				case -0x7787eeb9:
				case -0x7787eeb8:
				case -0x7787eeb7:
				case -0x7787eeb6:
				case -0x7787eeb5:
				case -0x7787eeb4:
				case -0x7787eeb3:
				case -0x7787eeb2:
				case -0x7787eeb1:
				case -0x7787eead:
				case -0x7787eeac:
				case -0x7787eea6:
				case -0x7787eea5:
				case -0x7787eea4:
				case -0x7787eea3:
				case -0x7787eea2:
				case -0x7787eea1:
					goto switchD_006e0d79_caseD_7;
				case -0x7787eeed:
					return "DMUS_E_INVALIDPOS";
				case -0x7787eeec:
					return "DMUS_E_INVALIDPATCH";
				case -0x7787eeeb:
					return "DMUS_E_CANNOTSEEK";
				case -0x7787eeea:
					return "DMUS_E_CANNOTWRITE";
				case -0x7787eee9:
					return "DMUS_E_CHUNKNOTFOUND";
				case -0x7787eee7:
					return "DMUS_E_INVALID_DOWNLOADID";
				case -0x7787eee0:
					return "DMUS_E_NOT_DOWNLOADED_TO_PORT";
				case -0x7787eedf:
					return "DMUS_E_ALREADY_DOWNLOADED";
				case -0x7787eede:
					return "DMUS_E_UNKNOWN_PROPERTY";
				case -0x7787eedd:
					return "DMUS_E_SET_UNSUPPORTED";
				case -0x7787eedc:
					return "DMUS_E_GET_UNSUPPORTED";
				case -0x7787eedb:
					return "DMUS_E_NOTMONO";
				case -0x7787eeda:
					return "DMUS_E_BADARTICULATION";
				case -0x7787eed9:
					return "DMUS_E_BADINSTRUMENT";
				case -0x7787eed8:
					return "DMUS_E_BADWAVELINK";
				case -0x7787eed7:
					return "DMUS_E_NOARTICULATION";
				case -0x7787eed6:
					return "DMUS_E_NOTPCM";
				case -0x7787eed5:
					return "DMUS_E_BADWAVE";
				case -0x7787eed4:
					return "DMUS_E_BADOFFSETTABLE";
				case -0x7787eed3:
					return "DMUS_E_UNKNOWNDOWNLOAD";
				case -0x7787eed2:
					return "DMUS_E_NOSYNTHSINK";
				case -0x7787eed1:
					return "DMUS_E_ALREADYOPEN";
				case -0x7787eed0:
					return "DMUS_E_ALREADYCLOSED";
				case -0x7787eecf:
					return "DMUS_E_SYNTHNOTCONFIGURED";
				case -0x7787eece:
					return "DMUS_E_SYNTHACTIVE";
				case -0x7787eecd:
					return "DMUS_E_CANNOTREAD";
				case -0x7787eecc:
					return "DMUS_E_DMUSIC_RELEASED";
				case -0x7787eecb:
					return "DMUS_E_BUFFER_EMPTY";
				case -0x7787eeca:
					return "DMUS_E_BUFFER_FULL";
				case -0x7787eec9:
					return "DMUS_E_PORT_NOT_CAPTURE";
				case -0x7787eec8:
					return "DMUS_E_PORT_NOT_RENDER";
				case -0x7787eec7:
					return "DMUS_E_DSOUND_NOT_SET";
				case -0x7787eec6:
					return "DMUS_E_ALREADY_ACTIVATED";
				case -0x7787eec5:
					return "DMUS_E_INVALIDBUFFER";
				case -0x7787eec4:
					return "DMUS_E_WAVEFORMATNOTSUPPORTED";
				case -0x7787eec3:
					return "DMUS_E_SYNTHINACTIVE";
				case -0x7787eec2:
					return "DMUS_E_DSOUND_ALREADY_SET";
				case -0x7787eec1:
					return "DMUS_E_INVALID_EVENT";
				case -0x7787eeb0:
					return "DMUS_E_UNSUPPORTED_STREAM";
				case -0x7787eeaf:
					return "DMUS_E_ALREADY_INITED";
				case -0x7787eeae:
					return "DMUS_E_INVALID_BAND";
				case -0x7787eeab:
					return "DMUS_E_TRACK_HDR_NOT_FIRST_CK";
				case -0x7787eeaa:
					return "DMUS_E_TOOL_HDR_NOT_FIRST_CK";
				case -0x7787eea9:
					return "DMUS_E_INVALID_TRACK_HDR";
				case -0x7787eea8:
					return "DMUS_E_INVALID_TOOL_HDR";
				case -0x7787eea7:
					return "DMUS_E_ALL_TOOLS_FAILED";
				case -0x7787eea0:
					return "DMUS_E_ALL_TRACKS_FAILED";
				case -0x7787ee9f:
					return "DSERR_OBJECTNOTFOUND";
				case -0x7787ee9e:
					return "DMUS_E_NOT_INIT";
				case -0x7787ee9d:
					return "DMUS_E_TYPE_DISABLED";
				case -0x7787ee9c:
					return "DMUS_E_TYPE_UNSUPPORTED";
				case -0x7787ee9b:
					return "DMUS_E_TIME_PAST";
				case -0x7787ee9a:
					return "DMUS_E_TRACK_NOT_FOUND";
				case -0x7787ee99:
					return "DMUS_E_TRACK_NO_CLOCKTIME_SUPPORT";
				default:
					return "DMUS_E_NO_MASTER_CLOCK";
				}
			}
			if (1 < a_eError) {
				if (a_eError < 0x40104) {
					if (a_eError == 0x40103) {
						return "VFW_S_NO_MORE_ITEMS";
					}
					switch (a_eError) {
					case 2:
					switchD_006e0d79_caseD_2:
						return "ERROR_FILE_NOT_FOUND";
					case 3:
						return "ERROR_PATH_NOT_FOUND";
					case 4:
						return "ERROR_TOO_MANY_OPEN_FILES";
					case 5:
						return "ERROR_ACCESS_DENIED";
					case 6:
						return "ERROR_INVALID_HANDLE";
					case 8:
						return "ERROR_NOT_ENOUGH_MEMORY";
					case 9:
						return "ERROR_INVALID_BLOCK";
					case 10:
						return "ERROR_BAD_ENVIRONMENT";
					case 0xb:
						return "ERROR_BAD_FORMAT";
					case 0xe:
						return "ERROR_OUTOFMEMORY";
					}
				}
				else if (a_eError < 0x4028d) {
					if (a_eError == 0x4028c) {
						return "VFW_S_DVD_CHANNEL_CONTENTS_NOT_AVAILABLE";
					}
					if (a_eError < 0x40259) {
						if (a_eError == 0x40258) {
							return "VFW_S_AUDIO_NOT_RENDERED";
						}
						if (a_eError < 0x40247) {
							if (a_eError == 0x40246) {
								return "VFW_S_CONNECTIONS_DEFERRED";
							}
							if (a_eError == 0x4022d) {
								return "VFW_S_DUPLICATE_NAME";
							}
							if (a_eError == 0x40237) {
								return "VFW_S_STATE_INTERMEDIATE";
							}
							if (a_eError == 0x40242) {
								return "VFW_S_PARTIAL_RENDER";
							}
							if (a_eError == 0x40245) {
								return "VFW_S_SOME_DATA_IGNORED";
							}
						}
						else {
							if (a_eError == 0x40250) {
								return "VFW_S_RESOURCE_NOT_NEEDED";
							}
							if (a_eError == 0x40254) {
								return "VFW_S_MEDIA_TYPE_IGNORED";
							}
							if (a_eError == 0x40257) {
								return "VFW_S_VIDEO_NOT_RENDERED";
							}
						}
					}
					else if (a_eError < 0x40269) {
						if (a_eError == 0x40268) {
							return "VFW_S_CANT_CUE";
						}
						if (a_eError == 0x4025a) {
							return "VFW_S_RPZA";
						}
						if (a_eError == 0x40260) {
							return "VFW_S_ESTIMATED";
						}
						if (a_eError == 0x40263) {
							return "VFW_S_RESERVED";
						}
						if (a_eError == 0x40267) {
							return "VFW_S_STREAM_OFF";
						}
					}
					else {
						if (a_eError == 0x40270) {
							return "VFW_S_NO_STOP_TIME";
						}
						if (a_eError == 0x4027e) {
							return "VFW_S_NOPREVIEWPIN";
						}
						if (a_eError == 0x40280) {
							return "VFW_S_DVD_NON_ONE_SEQUENTIAL";
						}
					}
				}
				else if (a_eError < 0x8781202) {
					if (a_eError == 0x8781201) {
						return "DMUS_S_FREE";
					}
					if (a_eError < 0x878000b) {
						if (a_eError == 0x878000a) {
							return "DS_NO_VIRTUALIZATION";
						}
						if (a_eError == 0x4028d) {
							return "VFW_S_DVD_NOT_ACCURATE";
						}
						if (a_eError == 0x150005) {
							return "DV_FULLDUPLEX";
						}
						if (a_eError == 0x15000a) {
							return "DV_HALFDUPLEX";
						}
						if (a_eError == 0x150010) {
							return "DV_PENDING";
						}
					}
					else {
						if (a_eError == 0x8781091) {
							return "DMUS_S_PARTIALLOAD";
						}
						if (a_eError == 0x8781092) {
							return "DMUS_S_PARTIALDOWNLOAD";
						}
						if (a_eError == 0x8781200) {
							return "DMUS_S_REQUEUE";
						}
					}
				}
				else if (a_eError < 0x8781214) {
					if (a_eError == 0x8781213) {
						return "DMUS_S_UP_OCTAVE";
					}
					if (a_eError == 0x8781202) {
						return "DMUS_S_END";
					}
					if (a_eError == 0x8781210) {
						return "DMUS_S_STRING_TRUNCATED";
					}
					if (a_eError == 0x8781211) {
						return "DMUS_S_LAST_TOOL";
					}
					if (a_eError == 0x8781212) {
						return "DMUS_S_OVER_CHORD";
					}
				}
				else {
					if (a_eError == 0x8781214) {
						return "DMUS_S_DOWN_OCTAVE";
					}
					if (a_eError == 0x8781215) {
						return "DMUS_S_NOBUFFERCONTROL";
					}
					if (a_eError == 0x8781216) {
						return "DMUS_S_GARBAGE_COLLECTED";
					}
				}
				goto switchD_006e0d79_caseD_7;
			}
			if (a_eError == 1) {
				return "S_FALSE";
			}
			if (-0x7787ede7 < a_eError) {
				if (a_eError < 1) {
					if (a_eError == 0) {
						return "S_OK";
					}
					switch (a_eError) {
					case -0x7787ede6:
						return "DMUS_E_SCRIPT_VARIABLE_NOT_FOUND";
					case -0x7787ede5:
						return "DMUS_E_SCRIPT_ROUTINE_NOT_FOUND";
					case -0x7787ede4:
						return "DMUS_E_SCRIPT_CONTENT_READONLY";
					case -0x7787ede3:
						return "DMUS_E_SCRIPT_NOT_A_REFERENCE";
					case -0x7787ede2:
						return "DMUS_E_SCRIPT_VALUE_NOT_SUPPORTED";
					case -0x7787ede0:
						return "DMUS_E_INVALID_SEGMENTTRIGGERTRACK";
					case -0x7787eddf:
						return "DMUS_E_INVALID_LYRICSTRACK";
					case -0x7787edde:
						return "DMUS_E_INVALID_PARAMCONTROLTRACK";
					case -0x7787eddd:
						return "DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR";
					case -0x7787eddc:
						return "DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR";
					case -0x7787eddb:
						return "DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE";
					case -0x7787edda:
						return "DMUS_E_AUDIOPATHS_NOT_VALID";
					case -0x7787edd9:
						return "DMUS_E_AUDIOPATHS_IN_USE";
					case -0x7787edd8:
						return "DMUS_E_NO_AUDIOPATH_CONFIG";
					case -0x7787edd7:
						return "DMUS_E_AUDIOPATH_INACTIVE";
					case -0x7787edd6:
						return "DMUS_E_AUDIOPATH_NOBUFFER";
					case -0x7787edd5:
						return "DMUS_E_AUDIOPATH_NOPORT";
					case -0x7787edd4:
						return "DMUS_E_NO_AUDIOPATH";
					case -0x7787edd3:
						return "DMUS_E_INVALIDCHUNK";
					case -0x7787edd2:
						return "DMUS_E_AUDIOPATH_NOGLOBALFXBUFFER";
					case -0x7787edd1:
						return "DMUS_E_INVALID_CONTAINER_OBJECT";
					}
				}
				goto switchD_006e0d79_caseD_7;
			}
			if (a_eError == -0x7787ede7) {
				return "DMUS_E_INVALID_SCRIPTTRACK";
			}
			switch (a_eError) {
			case -0x7787edfb:
				pcVar1 = "DMUS_E_CANNOT_FREE";
				break;
			case -0x7787edfa:
				pcVar1 = "DMUS_E_CANNOT_OPEN_PORT";
				break;
			case -0x7787edf9:
				pcVar1 = "DMUS_E_CANNOT_CONVERT";
				break;
			case -0x7787edf0:
				pcVar1 = "DMUS_E_DESCEND_CHUNK_FAIL";
				break;
			case -0x7787edef:
				pcVar1 = "DMUS_E_NOT_LOADED";
				break;
			case -0x7787eded:
				pcVar1 = "DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE";
				break;
			case -0x7787edec:
				pcVar1 = "DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE";
				break;
			case -0x7787edeb:
				pcVar1 = "DMUS_E_SCRIPT_ERROR_IN_SCRIPT";
				break;
			case -0x7787edea:
				pcVar1 = "DMUS_E_SCRIPT_CANTLOAD_OLEAUT32";
				break;
			case -0x7787ede9:
				pcVar1 = "DMUS_E_SCRIPT_LOADSCRIPT_ERROR";
				break;
			case -0x7787ede8:
				pcVar1 = "DMUS_E_SCRIPT_INVALID_FILE";
				break;
			default:
				if (a_eError == -0x7787edfc) {
					return "DMUS_E_ALREADY_SENT";
				}
				if (a_eError < -0x7787ee7a) {
					if (a_eError == -0x7787ee7b) {
						return "DMUS_E_LOADER_OBJECTNOTFOUND";
					}
					if (a_eError == -0x7787ee80) {
						return "DMUS_E_LOADER_NOCLASSID";
					}
					if (a_eError == -0x7787ee7f) {
						return "DMUS_E_LOADER_BADPATH";
					}
					if (a_eError == -0x7787ee7e) {
						return "DMUS_E_LOADER_FAILEDOPEN";
					}
					if (a_eError == -0x7787ee7d) {
						return "DMUS_E_LOADER_FORMATNOTSUPPORTED";
					}
					if (a_eError == -0x7787ee7c) {
						return "DMUS_E_LOADER_FAILEDCREATE";
					}
				}
				else {
					if (a_eError == -0x7787ee7a) {
						return "DMUS_E_LOADER_NOFILENAME";
					}
					if (a_eError == -0x7787ee00) {
						return "DMUS_E_INVALIDFILE";
					}
					if (a_eError == -0x7787edff) {
						return "DMUS_E_ALREADY_EXISTS";
					}
					if (a_eError == -0x7787edfe) {
						return "DMUS_E_OUT_OF_RANGE";
					}
					if (a_eError == -0x7787edfd) {
						return "DMUS_E_SEGMENT_INIT_FAILED";
					}
				}
			case -0x7787edf8:
			case -0x7787edf7:
			case -0x7787edf6:
			case -0x7787edf5:
			case -0x7787edf4:
			case -0x7787edf3:
			case -0x7787edf2:
			case -0x7787edf1:
			case -0x7787edee:
			switchD_006e0d79_caseD_7:
				pcVar1 = "Unknown";
			}
		}
		return pcVar1;
	}

	const char* TRenderD3DInterface::GetErrorDescription(TINT32 a_eError)
	{
		const char* pcVar1;
		bool bVar2;

		if (a_eError < -0x7789feb0) {
			if (a_eError == -0x7789feb1) {
				return "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations.";
			}
			if (a_eError < -0x7ff8fb7e) {
				if (a_eError == -0x7ff8fb7f) {
					return "The application was written for an unsupported prerelease version of DirectInput.";
				}
				if (a_eError < -0x7ffbfdb7) {
					if (a_eError == -0x7ffbfdb8) {
						return "This file is corrupt: it contains an invalid media type.";
					}
					if (a_eError < -0x7ffbfde5) {
						if (a_eError == -0x7ffbfde6) {
							return "No color key has been set.";
						}
						if (a_eError < -0x7ffbfdf8) {
							if (a_eError == -0x7ffbfdf9) {
								return "Attempted to read buffered device data from a device that is not buffered. & There is no common media type between these pins.";
							}
							if (a_eError < -0x7ffbfe0f) {
								if (a_eError == -0x7ffbfe10) {
									return "CoInitialize has not been called.";
								}
								if (a_eError < -0x7fffbffb) {
									if (a_eError == -0x7fffbffc) {
										return "Operation aborted";
									}
									if (a_eError == -0x7ffffff6) {
										return "The data necessary to complete this operation is not yet available.";
									}
									if (a_eError == -0x7fffbfff) {
										return "The function called is not supported at this time";
									}
									if (a_eError == -0x7fffbffe) {
										return "The requested COM interface is not available";
									}
									if (a_eError == -0x7fffbffd) {
										return "Invalid pointer";
									}
								}
								else {
									if (a_eError == -0x7fffbffb) {
										return "An undetermined error occurred";
									}
									if (a_eError == -0x7fff0001) {
										return "Catastrophic failure";
									}
									if (a_eError == -0x7ffbfef0) {
										return "This object does not support aggregation";
									}
									if (a_eError == -0x7ffbfeac) {
										return "Class not registered";
									}
								}
							}
							else if (a_eError < -0x7ffbfdfc) {
								if (a_eError == -0x7ffbfdfd) {
									return "The effect is not downloaded. & The enumerator has become invalid.";
								}
								if (a_eError == -0x7ffbfe0f) {
									return "CoInitialize has already been called.";
								}
								if (a_eError == -0x7ffbfe00) {
									return "Unable to IDirectInputJoyConfig_Acquire because the user does not have sufficient privileges to change the joystick configuration. & An invalid media type was specified";
								}
								if (a_eError == -0x7ffbfdff) {
									return "The device is full. & An invalid media subtype was specified.";
								}
								if (a_eError == -0x7ffbfdfe) {
									return "Not all the requested information fit into the buffer. & This object can only be created as an aggregated object.";
								}
							}
							else {
								if (a_eError == -0x7ffbfdfc) {
									return "The device cannot be reinitialized because there are still effects attached to it. & At least one of the pins involved in the operation is already connected.";
								}
								if (a_eError == -0x7ffbfdfb) {
									return "The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode. & This operation cannot be performed because the filter is active.";
								}
								if (a_eError == -0x7ffbfdfa) {
									return "The effect could not be downloaded because essential information is missing.  For example, no axes have been associated with the effect, or no type-specific information has been created. & One of the specified pins supports no media types.";
								}
							}
						}
						else {
							switch (a_eError) {
							case -0x7ffbfdf8:
								return "An attempt was made to modify parameters of an effect while it is playing.  Not all hardware devices support altering the parameters of an effect while it is playing. & Two pins of the same direction cannot be connected together.";
							case -0x7ffbfdf7:
								return "The operation could not be completed because the device is not plugged in. & The operation cannot be performed because the pins are not connected.";
							case -0x7ffbfdf6:
								return
									"SendDeviceData failed because more information was requested to be sent than can be sent to the device.  Some devices have restrictions on how much data can be sent to them.  (For example, there might be a limit on the number of buttons that can be pressed at once.) & No sample buffer  allocator is available."
									;
							case -0x7ffbfdf5:
								return "A mapper file function failed because reading or writing the user or IHV settings file failed. & A run-time error occurred.";
							case -0x7ffbfdf4:
								return "No buffer space has been set";
							case -0x7ffbfdf3:
								return "The buffer is not big enough.";
							case -0x7ffbfdf2:
								return "An invalid alignment was specified.";
							case -0x7ffbfdf1:
								return "Cannot change allocated memory while the filter is active.";
							case -0x7ffbfdf0:
								return "One or more buffers are still active.";
							case -0x7ffbfdef:
								return "Cannot allocate a sample when the allocator is not active.";
							case -0x7ffbfdee:
								return "Cannot allocate memory because no size has been set.";
							case -0x7ffbfded:
								return "Cannot lock for synchronization because no clock has been defined.";
							case -0x7ffbfdec:
								return "Quality messages could not be sent because no quality sink has been defined.";
							case -0x7ffbfdeb:
								return "A required interface has not been implemented.";
							case -0x7ffbfdea:
								return "An object or name was not found.";
							case -0x7ffbfde9:
								return "No combination of intermediate filters could be found to make the connection.";
							case -0x7ffbfde8:
								return "No combination of filters could be found to render the stream.";
							case -0x7ffbfde7:
								return "Could not change formats dynamically.";
							}
						}
					}
					else {
						switch (a_eError) {
						case -0x7ffbfde5:
							return "Current pin connection is not using the IOverlay transport.";
						case -0x7ffbfde4:
							return "Current pin connection is not using the IMemInputPin transport.";
						case -0x7ffbfde3:
							return "Setting a color key would conflict with the palette already set.";
						case -0x7ffbfde2:
							return "Setting a palette would conflict with the color key already set.";
						case -0x7ffbfde1:
							return "No matching color key is available.";
						case -0x7ffbfde0:
							return "No palette is available.";
						case -0x7ffbfddf:
							return "Display does not use a palette.";
						case -0x7ffbfdde:
							return "Too many colors for the current display settings.";
						case -0x7ffbfddd:
							return "The state changed while waiting to process the sample.";
						case -0x7ffbfddc:
							return "The operation could not be performed because the filter is not stopped.";
						case -0x7ffbfddb:
							return "The operation could not be performed because the filter is not paused.";
						case -0x7ffbfdda:
							return "The operation could not be performed because the filter is not running.";
						case -0x7ffbfdd9:
							return "The operation could not be performed because the filter is in the wrong state.";
						case -0x7ffbfdd8:
							return "The sample start time is after the sample end time.";
						case -0x7ffbfdd7:
							return "The supplied rectangle is invalid.";
						case -0x7ffbfdd6:
							return "This pin cannot use the supplied media type.";
						case -0x7ffbfdd5:
							return "This sample cannot be rendered.";
						case -0x7ffbfdd4:
							return "This sample cannot be rendered because the end of the stream has been reached.";
						case -0x7ffbfdd3:
							return "An attempt to add a filter with a duplicate name failed.";
						case -0x7ffbfdd2:
							return "A time-out has expired.";
						case -0x7ffbfdd1:
							return "The file format is invalid.";
						case -0x7ffbfdd0:
							return "The list has already been exhausted.";
						case -0x7ffbfdcf:
							return "The filter graph is circular.";
						case -0x7ffbfdce:
							return "Updates are not allowed in this state.";
						case -0x7ffbfdcd:
							return "An attempt was made to queue a command for a time in the past.";
						case -0x7ffbfdcc:
							return "The queued command has already been canceled.";
						case -0x7ffbfdcb:
							return "Cannot render the file because it is corrupt.";
						case -0x7ffbfdca:
							return "An overlay advise link already exists.";
						case -0x7ffbfdc8:
							return "No full-screen modes are available.";
						case -0x7ffbfdc7:
							return "This Advise cannot be canceled because it was not successfully set.";
						case -0x7ffbfdc6:
							return "A full-screen mode is not available.";
						case -0x7ffbfdc5:
							return "Cannot call IVideoWindow methods while in full-screen mode.";
						case -0x7ffbfdc0:
							return "The media type of this file is not recognized.";
						case -0x7ffbfdbf:
							return "The source filter for this file could not be loaded.";
						case -0x7ffbfdbd:
							return "A file appeared to be incomplete.";
						case -0x7ffbfdbc:
							return "The version number of the file is invalid.";
						case -0x7ffbfdb9:
							return "This file is corrupt: it contains an invalid class identifier.";
						}
					}
				}
				else {
					if (a_eError < -0x7ffbfd79) {
						switch (a_eError) {
						case -0x7ffbfdb7:
							return "No time stamp has been set for this sample.";
						case -0x7ffbfdb6:
						case -0x7ffbfdb5:
						case -0x7ffbfdb4:
						case -0x7ffbfdb3:
						case -0x7ffbfdb2:
						case -0x7ffbfdb1:
						case -0x7ffbfdb0:
						case -0x7ffbfdac:
						case -0x7ffbfda9:
						case -0x7ffbfda8:
						case -0x7ffbfda6:
						case -0x7ffbfda0:
						case -0x7ffbfd9d:
						case -0x7ffbfd99:
						case -0x7ffbfd98:
						case -0x7ffbfd96:
						case -0x7ffbfd95:
						case -0x7ffbfd94:
						case -0x7ffbfd93:
						case -0x7ffbfd92:
						case -0x7ffbfd91:
						case -0x7ffbfd90:
						case -0x7ffbfd82:
						case -0x7ffbfd80:
							goto switchD_006e3696_caseD_7;
						case -0x7ffbfdaf:
							return "No media time stamp has been set for this sample.";
						case -0x7ffbfdae:
							return "No media time format has been selected.";
						case -0x7ffbfdad:
							return "Cannot change balance because audio device is mono only.";
						case -0x7ffbfdab:
						switchD_006e1b3f_caseD_80040255:
							return "Cannot play back the video stream: no suitable decompressor could be found.";
						case -0x7ffbfdaa:
							return "Cannot play back the audio stream: no audio hardware is available, or the hardware is not responding.";
						case -0x7ffbfda7:
						switchD_006e1b3f_caseD_80040259:
							return "Cannot play back the video stream: format \'RPZA\' is not supported.";
						case -0x7ffbfda5:
							return "ActiveMovie cannot play MPEG movies on this processor.";
						case -0x7ffbfda4:
							return "Cannot play back the audio stream: the audio format is not supported.";
						case -0x7ffbfda3:
							return "Cannot play back the video stream: the video format is not supported.";
						case -0x7ffbfda2:
							return "ActiveMovie cannot play this video stream because it falls outside the constrained standard.";
						case -0x7ffbfda1:
							return "Cannot perform the requested function on an object that is not in the filter graph.";
						case -0x7ffbfd9f:
							return "Cannot get or set time related information on an object that is using a time format of TIME_FORMAT_NONE.";
						case -0x7ffbfd9e:
							return "The connection cannot be made because the stream is read only and the filter alters the data.";
						case -0x7ffbfd9c:
							return "The buffer is not full enough.";
						case -0x7ffbfd9b:
							return "Cannot play back the file.  The format is not supported.";
						case -0x7ffbfd9a:
							return "Pins cannot connect due to not supporting the same transport.";
						case -0x7ffbfd97:
							return "The Video CD can\'t be read correctly by the device or is the data is corrupt.";
						case -0x7ffbfd8f:
							return "There is not enough Video Memory at this display resolution and number of colors. Reducing resolution might help.";
						case -0x7ffbfd8e:
							return "The VideoPort connection negotiation process has failed.";
						case -0x7ffbfd8d:
							return "Either DirectDraw has not been installed or the Video Card capabilities are not suitable. Make sure the display is not in 16 color mode.";
						case -0x7ffbfd8c:
							return "No VideoPort hardware is available, or the hardware is not responding.";
						case -0x7ffbfd8b:
							return "No Capture hardware is available, or the hardware is not responding.";
						case -0x7ffbfd8a:
							return "This User Operation is inhibited by DVD Content at this time.";
						case -0x7ffbfd89:
							return "This Operation is not permitted in the current domain.";
						case -0x7ffbfd88:
							return "The specified button is invalid or is not present at the current time, or there is no button present at the specified location.";
						case -0x7ffbfd87:
							return "DVD-Video playback graph has not been built yet.";
						case -0x7ffbfd86:
							return "DVD-Video playback graph building failed.";
						case -0x7ffbfd85:
							return "DVD-Video playback graph could not be built due to insufficient decoders.";
						case -0x7ffbfd84:
							return "Version number of DirectDraw not suitable. Make sure to install dx5 or higher version.";
						case -0x7ffbfd83:
							return "Copy protection cannot be enabled. Please make sure any other copy protected content is not being shown now.";
						case -0x7ffbfd81:
							return "This object cannot be used anymore as its time has expired.";
						case -0x7ffbfd7f:
							return "The operation cannot be performed at the current playback speed.";
						case -0x7ffbfd7e:
							return "The specified menu doesn\'t exist.";
						case -0x7ffbfd7d:
							return "The specified command was either cancelled or no longer exists.";
						case -0x7ffbfd7c:
							return "The data did not contain a recognized version.";
						case -0x7ffbfd7b:
							return "The state data was corrupt.";
						default:
							return "The state data is from a different disc.";
						}
					}
					if (a_eError < -0x7ffbfcff) {
						if (a_eError == -0x7ffbfd00) {
							return "Device driver-specific codes. Unless the specific driver has been precisely identified, no meaning should be attributed to these values other than that the driver originated the error.";
						}
						switch (a_eError) {
						case -0x7ffbfd79:
							return "The region was not compatible with the current drive.";
						case -0x7ffbfd78:
							return "The requested DVD stream attribute does not exist.";
						case -0x7ffbfd77:
							return "Currently there is no GoUp (Annex J user function) program chain (PGC).";
						case -0x7ffbfd76:
							return "The current parental level was too low.";
						case -0x7ffbfd75:
							return "The current audio is not karaoke content.";
						case -0x7ffbfd72:
							return "Frame step is not supported on this configuration.";
						case -0x7ffbfd71:
							return "The specified stream is disabled and cannot be selected.";
						case -0x7ffbfd70:
							return "The operation depends on the current title number, however the navigator has not yet entered the VTSM or the title domains, so the \'current\' title index is unknown.";
						case -0x7ffbfd6f:
							return "The specified path does not point to a valid DVD disc.";
						case -0x7ffbfd6e:
							return "There is currently no resume information.";
						case -0x7ffbfd6d:
							return "This thread has already blocked this output pin.  There is no need to call IPinFlowControl::Block() again.";
						case -0x7ffbfd6c:
							return "IPinFlowControl::Block() has been called on another thread.  The current thread cannot make any assumptions about this pin\'s block state.";
						case -0x7ffbfd6b:
							return "An operation failed due to a certification failure.";
						}
					}
					else if (a_eError < -0x7ff8fffa) {
						if (a_eError == -0x7ff8fffb) {
							return "Access is denied";
						}
						if (a_eError < -0x7ffbfc0d) {
							if (a_eError == -0x7ffbfc0e) {
								return "A registry entry is corrupt.";
							}
							if (a_eError == -0x7ffbfcff) {
								return "DIERR_DRIVERFIRST+1";
							}
							if (a_eError == -0x7ffbfcfe) {
								return "DIERR_DRIVERFIRST+2";
							}
							if (a_eError == -0x7ffbfcfd) {
								return "DIERR_DRIVERFIRST+3";
							}
							if (a_eError == -0x7ffbfcfc) {
								return "DIERR_DRIVERFIRST+4";
							}
							if (a_eError == -0x7ffbfcfb) {
								return "DIERR_DRIVERFIRST+5";
							}
						}
						else {
							if (a_eError == -0x7ffbfc01) {
								return "Device installer errors.";
							}
							if (a_eError == -0x7ffbfc00) {
								return "Registry entry or DLL for class installer invalid or class installer not found.";
							}
							if (a_eError == -0x7ffbfbff) {
								return "The user cancelled the install operation.";
							}
							if (a_eError == -0x7ffbfbfe) {
								return "The INF file for the selected device could not be found or is invalid or is damaged.";
							}
							if (a_eError == -0x7ff8fffe) goto switchD_006e3696_caseD_2;
						}
					}
					else if (a_eError < -0x7ff8ffa8) {
						if (a_eError == -0x7ff8ffa9) {
							return "An invalid parameter was passed to the returning function";
						}
						if (a_eError == -0x7ff8fffa) goto LAB_006e243b;
						if (a_eError == -0x7ff8fff4) {
							return "The operation cannot be performed unless the device is acquired.";
						}
						if (a_eError == -0x7ff8fff2) {
							return "Ran out of memory";
						}
						if (a_eError == -0x7ff8ffeb) goto LAB_006e30af;
						if (a_eError == -0x7ff8ffe2) {
							return "Access to the device has been lost.  It must be re-acquired.";
						}
					}
					else {
						if (a_eError == -0x7ff8ff89) {
							return "The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.";
						}
						if (a_eError == -0x7ff8ff56) {
							return "The operation cannot be performed while the device is acquired.";
						}
						if (a_eError == -0x7ff8fefd) {
							return "No more items.";
						}
						if (a_eError == -0x7ff8fb82) {
							return "The application requires a newer version of DirectInput.";
						}
					}
				}
			}
			else if (a_eError < -0x7fea7d6f) {
				if (a_eError == -0x7fea7d70) {
					return "Invalid address format";
				}
				if (a_eError < -0x7feafe78) {
					if (a_eError == -0x7feafe79) {
						return "Initialized";
					}
					if (a_eError < -0x7feafe8f) {
						if (a_eError == -0x7feafe90) {
							return "Invalid target";
						}
						if (a_eError < -0x7feaff69) {
							if (a_eError == -0x7feaff6a) {
							LAB_006e243b:
								return "Invalid handle";
							}
							if (a_eError < -0x7feaffb5) {
								if (a_eError == -0x7feaffb6) goto LAB_006e2302;
								if (a_eError == -0x7ff8fb70) {
									return "The specified property ID is not supported for the specified property set.";
								}
								if (a_eError == -0x7ff8fb6e) {
									return "The Specified property set is not supported.";
								}
								if (a_eError == -0x7ff8fb21) goto LAB_006e2623;
								if (a_eError == -0x7feaffe2) goto LAB_006e1f75;
							}
							else {
								if (a_eError == -0x7feaff88) goto LAB_006e23f9;
								if (a_eError == -0x7feaff7e) goto LAB_006e2498;
								if (a_eError == -0x7feaff79) goto LAB_006e2484;
								if (a_eError == -0x7feaff6f) goto LAB_006e1fbf;
							}
						}
						else if (a_eError < -0x7feafe95) {
							if (a_eError == -0x7feafe96) {
								return "Connected";
							}
							if (a_eError == -0x7feafed4) {
								return "Session lost";
							}
							if (a_eError == -0x7feafed2) {
								return "No voice session";
							}
							if (a_eError == -0x7feafe98) {
							LAB_006e2247:
								return "Connection lost";
							}
							if (a_eError == -0x7feafe97) {
								return "Not initialized";
							}
						}
						else {
							if (a_eError == -0x7feafe95) {
								return "Not connected";
							}
							if (a_eError == -0x7feafe92) {
								return "Connect aborting";
							}
							bVar2 = a_eError == -0x7feafe91;
						LAB_006e2523:
							if (bVar2) {
								return "Not allowed";
							}
						}
					}
					else {
						switch (a_eError) {
						case -0x7feafe8f:
							return "Transport not host";
						case -0x7feafe8e:
							return "Compression not supported";
						case -0x7feafe8d:
							return "Already pending";
						case -0x7feafe8c:
							return "Sound init failure";
						case -0x7feafe8b:
							return "Time out";
						case -0x7feafe8a:
							return "Connect aborted";
						case -0x7feafe89:
							return "No 3d sound";
						case -0x7feafe88:
							return "Already buffered";
						case -0x7feafe87:
							return "Not buffered";
						case -0x7feafe86:
						switchD_006e2064_caseD_8015017a:
							return "Hosting";
						case -0x7feafe85:
							return "Not hosting";
						case -0x7feafe84:
						switchD_006e2064_caseD_8015017c:
							return "Invalid device";
						case -0x7feafe83:
							return "Record system error";
						case -0x7feafe82:
							return "Playback system error";
						case -0x7feafe81:
							return "Send error";
						case -0x7feafe80:
						switchD_006e2064_caseD_80150180:
							return "User cancel";
						case -0x7feafe7d:
							return "Run setup";
						case -0x7feafe7c:
							return "Incompatible version";
						}
					}
				}
				else if (a_eError < -0x7fea7fcf) {
					if (a_eError == -0x7fea7fd0) {
						return "Aborted";
					}
					switch (a_eError) {
					case -0x7feafe78:
						return "No transport";
					case -0x7feafe77:
						return "No callback";
					case -0x7feafe76:
						return "Transport not init";
					case -0x7feafe75:
						return "Transport no session";
					case -0x7feafe74:
						return "Transport no player";
					case -0x7feafe73:
						return "User back";
					case -0x7feafe72:
						return "No rec vol available";
					case -0x7feafe71:
						return "Invalid buffer";
					case -0x7feafe70:
						return "Locked buffer";
					}
				}
				else if (a_eError < -0x7fea7e8f) {
					if (a_eError == -0x7fea7e90) {
						return "Conversion";
					}
					if (a_eError < -0x7fea7eff) {
						if (a_eError == -0x7fea7f00) {
						LAB_006e1f75:
							return "Buffer too small";
						}
						if (a_eError == -0x7fea7fc0) {
							return "Addressing";
						}
						if (a_eError == -0x7fea7fb0) {
							return "Already closing";
						}
						if (a_eError == -0x7fea7fa0) {
							return "Already connected";
						}
						if (a_eError == -0x7fea7f90) {
							return "Already disconnecting";
						}
						if (a_eError == -0x7fea7f80) {
							return "Already initialized";
						}
						if (a_eError == -0x7fea7f70) {
							return "Already registered";
						}
					}
					else {
						if (a_eError == -0x7fea7ef0) {
							return "Can not cancel";
						}
						if (a_eError == -0x7fea7ee0) {
							return "Cant create group";
						}
						if (a_eError == -0x7fea7ed0) {
							return "Cant create player";
						}
						if (a_eError == -0x7fea7ec0) {
							return "Cant launch application";
						}
						if (a_eError == -0x7fea7eb0) {
							return "Connecting";
						}
						if (a_eError == -0x7fea7ea0) goto LAB_006e2247;
					}
				}
				else if (a_eError < -0x7fea7dcf) {
					if (a_eError == -0x7fea7dd0) {
					LAB_006e2302:
						return "Exception";
					}
					if (a_eError == -0x7fea7e8b) {
						return "Data too large";
					}
					if (a_eError == -0x7fea7e80) {
						return "Does not exist";
					}
					if (a_eError == -0x7fea7e70) {
						return "Duplicate command";
					}
					if (a_eError == -0x7fea7e00) {
						return "End point not receiving";
					}
					if (a_eError == -0x7fea7df0) {
						return "Enum query too large";
					}
					if (a_eError == -0x7fea7de0) {
						return "Enum response too large";
					}
				}
				else {
					if (a_eError == -0x7fea7dc0) {
						return "Group not empty";
					}
					if (a_eError == -0x7fea7db0) goto switchD_006e2064_caseD_8015017a;
					if (a_eError == -0x7fea7da0) {
						return "Host rejected connection";
					}
					if (a_eError == -0x7fea7d90) {
						return "Host terminated session";
					}
					if (a_eError == -0x7fea7d80) {
						return "Incomplete address";
					}
				}
			}
			else if (a_eError < -0x7789fff5) {
				if (a_eError == -0x7789fff6) {
					return "This surface can not be attached to the requested surface.";
				}
				if (a_eError < -0x7fea7b7f) {
					if (a_eError == -0x7fea7b80) {
						return "No connection";
					}
					if (a_eError < -0x7fea7c6f) {
						if (a_eError == -0x7fea7c70) {
							return "Invalid interface";
						}
						if (a_eError < -0x7fea7cbf) {
							if (a_eError == -0x7fea7cc0) {
							LAB_006e23f9:
								return "Invalid flags";
							}
							if (a_eError == -0x7fea7d00) {
								return "Invalid application";
							}
							if (a_eError == -0x7fea7cf0) {
								return "Invalid command";
							}
							if (a_eError == -0x7fea7ce0) {
								return "Invalid device address";
							}
							if (a_eError == -0x7fea7cd0) {
								return "Invalid end point";
							}
						}
						else {
							if (a_eError == -0x7fea7cb0) {
							LAB_006e1fbf:
								return "Invalid group";
							}
							if (a_eError == -0x7fea7ca0) goto LAB_006e243b;
							if (a_eError == -0x7fea7c90) {
								return "Invalid host address";
							}
							if (a_eError == -0x7fea7c80) {
								return "Invalid instance";
							}
						}
					}
					else if (a_eError < -0x7fea7bbf) {
						if (a_eError == -0x7fea7bc0) {
							return "Invalid string";
						}
						if (a_eError == -0x7fea7c00) {
						LAB_006e2498:
							return "Invalid object";
						}
						if (a_eError == -0x7fea7bf0) {
							return "Invalid password";
						}
						if (a_eError == -0x7fea7be0) {
						LAB_006e2484:
							return "Invalid player";
						}
						if (a_eError == -0x7fea7bd0) {
							return "Invalid priority";
						}
					}
					else {
						if (a_eError == -0x7fea7bb0) {
							return "Invalid url";
						}
						if (a_eError == -0x7fea7ba0) {
							return "Invalid version";
						}
						if (a_eError == -0x7fea7b90) {
							return "No caps";
						}
					}
				}
				else if (a_eError < -0x7fea7a7f) {
					if (a_eError == -0x7fea7a80) {
						return "Player not in group";
					}
					if (a_eError < -0x7fea7acf) {
						if (a_eError == -0x7fea7ad0) {
							return "Not host";
						}
						if (a_eError == -0x7fea7b70) {
							return "No host player";
						}
						if (a_eError == -0x7fea7b00) {
							return "No more address components";
						}
						if (a_eError == -0x7fea7af0) {
							return "No response";
						}
						bVar2 = a_eError == -0x7fea7ae0;
						goto LAB_006e2523;
					}
					if (a_eError == -0x7fea7ac0) {
						return "Not ready";
					}
					if (a_eError == -0x7fea7ab0) {
						return "Not registered";
					}
					if (a_eError == -0x7fea7aa0) {
						return "Player already in group";
					}
					if (a_eError == -0x7fea7a90) {
						return "Player lost";
					}
				}
				else if (a_eError < -0x7fea79cf) {
					if (a_eError == -0x7fea79d0) {
						return "Timed out";
					}
					if (a_eError == -0x7fea7a70) {
						return "Player not reachable";
					}
					if (a_eError == -0x7fea7a00) {
						return "Send too large";
					}
					if (a_eError == -0x7fea79f0) {
						return "Session full";
					}
					if (a_eError == -0x7fea79e0) {
						return "Table full";
					}
				}
				else {
					if (a_eError == -0x7fea79c0) {
						return "Uninitialized";
					}
					if (a_eError == -0x7fea79b0) goto switchD_006e2064_caseD_80150180;
					if (a_eError == -0x7789fffb) {
					LAB_006e2623:
						return "This object is already initialized";
					}
				}
			}
			else if (a_eError < -0x7789ff2b) {
				if (a_eError == -0x7789ff2c) {
					return "Create function called without DirectDraw object method SetCooperativeLevel being called.";
				}
				if (a_eError < -0x7789ff6e) {
					if (a_eError == -0x7789ff6f) {
						return "pixel format was invalid as specified";
					}
					if (a_eError < -0x7789ffa0) {
						if (a_eError == -0x7789ffa1) {
							return "Unable to match primary surface creation request with existing primary surface.";
						}
						if (a_eError == -0x7789ffec) {
							return "This surface can not be detached from the requested surface.";
						}
						if (a_eError == -0x7789ffd8) {
							return "Support is currently not available.";
						}
						if (a_eError == -0x7789ffc9) {
							return "An exception was encountered while performing the requested operation";
						}
						if (a_eError == -0x7789ffa6) {
							return "Height of rectangle provided is not a multiple of reqd alignment";
						}
					}
					else {
						if (a_eError == -0x7789ff9c) {
							return "One or more of the caps bits passed to the callback are incorrect.";
						}
						if (a_eError == -0x7789ff92) {
							return "DirectDraw does not support provided Cliplist.";
						}
						if (a_eError == -0x7789ff88) {
							return "DirectDraw does not support the requested mode";
						}
						if (a_eError == -0x7789ff7e) {
							return "DirectDraw received a pointer that was an invalid DIRECTDRAW object.";
						}
					}
				}
				else if (a_eError < -0x7789ff4a) {
					if (a_eError == -0x7789ff4b) {
						return "Operation could not be carried out because there is no stereo hardware present or available.";
					}
					if (a_eError == -0x7789ff6a) {
						return "Rectangle provided was invalid.";
					}
					if (a_eError == -0x7789ff60) {
						return "Operation could not be carried out because one or more surfaces are locked";
					}
					if (a_eError == -0x7789ff56) {
						return "There is no 3D present.";
					}
					if (a_eError == -0x7789ff4c) {
						return "Operation could not be carried out because there is no alpha accleration hardware present or available.";
					}
				}
				else {
					if (a_eError == -0x7789ff4a) {
						return "Operation could not be carried out because there is no hardware present which supports stereo surfaces";
					}
					if (a_eError == -0x7789ff33) {
						return "no clip list available";
					}
					if (a_eError == -0x7789ff2e) {
						return "Operation could not be carried out because there is no color conversion hardware present or available.";
					}
				}
			}
			else if (a_eError < -0x7789fefb) {
				if (a_eError == -0x7789fefc) {
					return "Operation could not be carried out because there is no overlay hardware present or available.";
				}
				if (a_eError < -0x7789ff19) {
					if (a_eError == -0x7789ff1a) {
						return "Flipping visible surfaces is not supported.";
					}
					if (a_eError == -0x7789ff29) {
						return "Surface doesn\'t currently have a color key";
					}
					if (a_eError == -0x7789ff24) {
						return "Operation could not be carried out because there is no hardware support of the dest color key.";
					}
					if (a_eError == -0x7789ff22) {
						return "No DirectDraw support possible with current display driver";
					}
					if (a_eError == -0x7789ff1f) {
						return "Operation requires the application to have exclusive mode but the application does not have exclusive mode.";
					}
				}
				else {
					if (a_eError == -0x7789ff10) {
						return "There is no GDI present.";
					}
					if (a_eError == -0x7789ff06) {
						return "Operation could not be carried out because there is no hardware present or available.";
					}
					if (a_eError == -0x7789ff01) {
						return "Requested item was not found";
					}
				}
			}
			else if (a_eError < -0x7789fec3) {
				if (a_eError == -0x7789fec4) {
					return "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.";
				}
				if (a_eError == -0x7789fef2) {
					return "Operation could not be carried out because the source and destination rectangles are on the same surface and overlap each other.";
				}
				if (a_eError == -0x7789fee8) {
					return "Operation could not be carried out because there is no appropriate raster op hardware present or available.";
				}
				if (a_eError == -0x7789fede) {
					return "Operation could not be carried out because there is no rotation hardware present or available.";
				}
				if (a_eError == -0x7789feca) {
					return "Operation could not be carried out because there is no hardware support for stretching";
				}
			}
			else {
				if (a_eError == -0x7789fec3) {
					return "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette.";
				}
				if (a_eError == -0x7789fec0) {
					return "DirectDraw Surface is not in 8 bit color mode and the requested operation requires 8 bit color.";
				}
				if (a_eError == -0x7789feb6) {
					return "Operation could not be carried out because there is no texture mapping hardware present or available.";
				}
			}
			goto switchD_006e3696_caseD_7;
		}
		if (a_eError < -0x7787eef6) {
			if (a_eError == -0x7787eef7) {
				return "Wave chunks in DLS collection file are at incorrect offsets.";
			}
			if (-0x7789fd45 < a_eError) {
				if (a_eError < -0x7789f7e7) {
					if (a_eError == -0x7789f7e8) {
						return "Wrong texture format";
					}
					switch (a_eError) {
					case -0x7789fcae:
						return "Bad object";
					case -0x7789fcad:
						return "Bad value";
					case -0x7789fcac:
						return "Bad type";
					case -0x7789fcab:
						return "Bad stream handle";
					case -0x7789fcaa:
						return "Bad alloc";
					case -0x7789fca9:
					switchD_006e2d3a_caseD_88760357:
						return "Not found";
					case -0x7789fca8:
						return "Not done yet";
					case -0x7789fca7:
						return "File not found";
					case -0x7789fca6:
						return "Resource not found";
					case -0x7789fca5:
						return "Url not found";
					case -0x7789fca4:
						return "Bad resource";
					case -0x7789fca3:
						return "Bad file type";
					case -0x7789fca2:
						return "Bad file version";
					case -0x7789fca1:
						return "Bad file float size";
					case -0x7789fca0:
						return "Bad file compression type";
					case -0x7789fc9f:
						return "Bad file";
					case -0x7789fc9e:
						return "Parse error";
					case -0x7789fc9d:
						return "No template";
					case -0x7789fc9c:
						return "Bad array size";
					case -0x7789fc9b:
						return "Bad data reference";
					case -0x7789fc9a:
						return "Internal error";
					case -0x7789fc99:
						return "No more objects";
					case -0x7789fc98:
						return "Bad intrinsics";
					case -0x7789fc97:
						return "No more stream handles";
					case -0x7789fc96:
						return "No more data";
					case -0x7789fc95:
						return "Bad cache file";
					case -0x7789fc94:
						return "No internet";
					}
				}
				else if (a_eError < -0x7789f799) {
					if (a_eError == -0x7789f79a) goto switchD_006e2d3a_caseD_88760357;
					switch (a_eError) {
					case -0x7789f7e7:
						return "Unsupported color operation";
					case -0x7789f7e6:
						return "Unsupported color arg";
					case -0x7789f7e5:
						return "Unsupported alpha operation";
					case -0x7789f7e4:
						return "Unsupported alpha arg";
					case -0x7789f7e3:
						return "Too many operations";
					case -0x7789f7e2:
						return "Conflicting texture filter";
					case -0x7789f7e1:
						return "Unsupported factor value";
					case -0x7789f7df:
						return "Conflicting render state";
					case -0x7789f7de:
						return "Unsupported texture filter";
					case -0x7789f7da:
						return "Conflicting texture palette";
					case -0x7789f7d9:
						return "Driver internal error";
					}
				}
				else if (a_eError < -0x7787ff9b) {
					if (a_eError == -0x7787ff9c) {
						return "The specified WAVE format is not supported";
					}
					if (a_eError < -0x7789f4aa) {
						if (a_eError == -0x7789f4ab) {
							return "Invalid mesh";
						}
						if (a_eError < -0x7789f794) {
							if (a_eError == -0x7789f795) goto switchD_006e2064_caseD_8015017c;
							if (a_eError == -0x7789f799) {
								return "More data";
							}
							if (a_eError == -0x7789f798) {
								return "Device lost";
							}
							if (a_eError == -0x7789f797) {
								return "Device not reset";
							}
							if (a_eError == -0x7789f796) {
								return "Not available";
							}
						}
						else {
							if (a_eError == -0x7789f794) {
								return "Invalid call";
							}
							if (a_eError == -0x7789f793) {
								return "Driver invalid call";
							}
							if (a_eError == -0x7789f4ac) {
								return "Can not modify index buffer";
							}
						}
					}
					else if (a_eError < -0x7787fff5) {
						if (a_eError == -0x7787fff6) {
							return "The call failed because resources (such as a priority level) were already being used by another caller";
						}
						if (a_eError == -0x7789f4aa) {
							return "Cannot attr sort";
						}
						if (a_eError == -0x7789f4a9) {
							return "Skinning not supported";
						}
						if (a_eError == -0x7789f4a8) {
							return "Too many influences";
						}
						if (a_eError == -0x7789f4a7) {
							return "Invalid data";
						}
					}
					else {
						if (a_eError == -0x7787ffe2) {
							return "The control (vol, pan, etc.) requested by the caller is not available";
						}
						if (a_eError == -0x7787ffce) {
							return "This call is not valid for the current state of this object";
						}
						if (a_eError == -0x7787ffba) {
							return "The caller does not have the priority level required for the function to succeed";
						}
					}
				}
				else if (a_eError < -0x7787ff2d) {
					if (a_eError == -0x7787ff2e) {
						return "The GUID specified in an audiopath file does not match a valid MIXIN buffer";
					}
					if (a_eError < -0x7787ff55) {
						if (a_eError == -0x7787ff56) {
						LAB_006e30af:
							return "This object has not been initialized";
						}
						if (a_eError == -0x7787ff88) {
							return "No sound driver is available for use";
						}
						if (a_eError == -0x7787ff7e) goto LAB_006e2623;
						if (a_eError == -0x7787ff6a) {
							return "The buffer memory has been lost, and must be restored";
						}
						if (a_eError == -0x7787ff60) {
							return "Another app has a higher priority level, preventing this call from succeeding";
						}
					}
					else {
						if (a_eError == -0x7787ff4c) {
							return "Tried to create a DSBCAPS_CTRLFX buffer shorter than DSBSIZE_FX_MIN milliseconds";
						}
						if (a_eError == -0x7787ff42) {
							return "Attempt to use DirectSound 8 functionality on an older DirectSound object";
						}
						if (a_eError == -0x7787ff38) {
							return "A circular loop of send effects was detected";
						}
					}
				}
				else {
					if (a_eError == -0x7787eeff) {
						return "An unexpected error was returned from a device driver, indicating possible failure of the driver or hardware.";
					}
					if (a_eError == -0x7787eefe) {
						return "The requested operation cannot be performed while there are  instantiated ports in any process in the system.";
					}
					if (a_eError == -0x7787eefd) {
						return "The requested device is already in use (possibly by a non-DirectMusic client) and cannot be opened again.";
					}
					if (a_eError == -0x7787eefc) {
						return "Buffer is not large enough for requested operation.";
					}
					if (a_eError == -0x7787eefb) {
						return "No buffer was prepared for the download data.";
					}
					if (a_eError == -0x7787eefa) {
						return "Download failed due to inability to access or create download buffer.";
					}
					if (a_eError == -0x7787eef8) {
						return "Error parsing DLS collection. File is corrupt.";
					}
				}
				goto switchD_006e3696_caseD_7;
			}
			if (a_eError == -0x7789fd45) {
				return "Surfaces created by one direct draw device cannot be used directly by another direct draw device.";
			}
			switch (a_eError) {
			case -0x7789fdc2:
				pcVar1 = "If a clipper object is attached to the source surface passed into a BltFast call.";
				break;
			case -0x7789fdc1:
				pcVar1 = "No blter.";
				break;
			case -0x7789fdc0:
				pcVar1 = "No DirectDraw ROP hardware.";
				break;
			case -0x7789fdbf:
				pcVar1 = "returned when GetOverlayPosition is called on a hidden overlay";
				break;
			case -0x7789fdbe:
				pcVar1 = "returned when GetOverlayPosition is called on a overlay that UpdateOverlay has never been called on to establish a destionation.";
				break;
			case -0x7789fdbd:
				pcVar1 = "returned when the position of the overlay on the destionation is no longer legal for that destionation.";
				break;
			case -0x7789fdbc:
				pcVar1 = "returned when an overlay member is called for a non-overlay surface";
				break;
			case -0x7789fdbb:
				pcVar1 = "An attempt was made to set the cooperative level when it was already set to exclusive.";
				break;
			case -0x7789fdba:
				pcVar1 = "An attempt has been made to flip a surface that is not flippable.";
				break;
			case -0x7789fdb9:
				pcVar1 = "Can\'t duplicate primary & 3D surfaces, or surfaces that are implicitly created.";
				break;
			case -0x7789fdb8:
				pcVar1 = "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted.";
				break;
			case -0x7789fdb7:
				pcVar1 = "Windows can not create any more DCs, or a DC was requested for a paltte-indexed surface when the surface had no palette AND the display mode was not palette-indexed (in this case DirectDraw cannot select a proper palette into the DC)";
				break;
			case -0x7789fdb6:
				pcVar1 = "No DC was ever created for this surface.";
				break;
			case -0x7789fdb5:
				pcVar1 = "This surface can not be restored because it was created in a different mode.";
				break;
			case -0x7789fdb4:
				pcVar1 = "This surface can not be restored because it is an implicitly created surface.";
				break;
			case -0x7789fdb3:
				pcVar1 = "The surface being used is not a palette-based surface";
				break;
			case -0x7789fdb2:
				pcVar1 = "The display is currently in an unsupported mode";
				break;
			case -0x7789fdb1:
				pcVar1 = "Operation could not be carried out because there is no mip-map texture mapping hardware present or available.";
				break;
			case -0x7789fdb0:
				pcVar1 = "The requested action could not be performed because the surface was of the wrong type.";
				break;
			case -0x7789fdaf:
			case -0x7789fdae:
			case -0x7789fdad:
			case -0x7789fdac:
			case -0x7789fdab:
			case -0x7789fdaa:
			case -0x7789fda9:
			case -0x7789fda4:
			case -0x7789fda3:
			case -0x7789fda2:
			case -0x7789fda1:
			case -0x7789fda0:
			case -0x7789fd9f:
			case -0x7789fd9e:
			case -0x7789fd9d:
			case -0x7789fd9c:
			case -0x7789fd9b:
			case -0x7789fd9a:
			case -0x7789fd99:
			case -0x7789fd98:
			case -0x7789fd97:
			case -0x7789fd96:
			case -0x7789fd95:
			case -0x7789fd93:
			case -0x7789fd92:
			case -0x7789fd91:
			case -0x7789fd90:
			case -0x7789fd8f:
			case -0x7789fd8e:
			case -0x7789fd8d:
			case -0x7789fd8c:
			case -0x7789fd8b:
			case -0x7789fd89:
			case -0x7789fd88:
			case -0x7789fd87:
			case -0x7789fd86:
			case -0x7789fd85:
			case -0x7789fd84:
			case -0x7789fd83:
			case -0x7789fd82:
			case -0x7789fd81:
			case -0x7789fd7f:
			case -0x7789fd7e:
			case -0x7789fd7d:
			case -0x7789fd7c:
			case -0x7789fd7b:
			case -0x7789fd7a:
			case -0x7789fd79:
			case -0x7789fd78:
			case -0x7789fd77:
			case -0x7789fd76:
			case -0x7789fd75:
			case -0x7789fd74:
			case -0x7789fd73:
			case -0x7789fd72:
			case -0x7789fd71:
			case -0x7789fd70:
			case -0x7789fd6f:
			case -0x7789fd6e:
			case -0x7789fd6d:
			case -0x7789fd6b:
			case -0x7789fd6a:
			case -0x7789fd69:
			case -0x7789fd68:
			case -0x7789fd67:
			case -0x7789fd66:
			case -0x7789fd65:
			case -0x7789fd64:
			case -0x7789fd63:
			case -0x7789fd62:
			case -0x7789fd61:
			case -0x7789fd60:
			case -0x7789fd5f:
			case -0x7789fd5e:
			case -0x7789fd5d:
			case -0x7789fd5c:
			case -0x7789fd5b:
			case -0x7789fd5a:
			case -0x7789fd59:
			case -0x7789fd57:
			case -0x7789fd56:
			case -0x7789fd55:
			case -0x7789fd54:
			case -0x7789fd53:
			case -0x7789fd52:
			case -0x7789fd51:
			case -0x7789fd50:
			case -0x7789fd4f:
				goto switchD_006e3696_caseD_7;
			case -0x7789fda8:
				pcVar1 = "Device does not support optimized surfaces, therefore no video memory optimized surfaces";
				break;
			case -0x7789fda7:
				pcVar1 = "Surface is an optimized surface, but has not yet been allocated any memory";
				break;
			case -0x7789fda6:
				pcVar1 = "Attempt was made to create or set a device window without first setting the focus window";
				break;
			case -0x7789fda5:
				pcVar1 = "Attempt was made to set a palette on a mipmap sublevel";
				break;
			case -0x7789fd94:
				pcVar1 = "A DC has already been returned for this surface. Only one DC can be retrieved per surface.";
				break;
			case -0x7789fd8a:
				pcVar1 = "An attempt was made to allocate non-local video memory from a device that does not support non-local video memory.";
				break;
			case -0x7789fd80:
				pcVar1 = "The attempt to page lock a surface failed.";
				break;
			case -0x7789fd6c:
				pcVar1 = "The attempt to page unlock a surface failed.";
				break;
			case -0x7789fd58:
				pcVar1 = "An attempt was made to page unlock a surface with no outstanding page locks.";
				break;
			case -0x7789fd4e:
				pcVar1 = "There is more data available than the specified buffer size could hold";
				break;
			case -0x7789fd4d:
				pcVar1 = "The data has expired and is therefore no longer valid.";
				break;
			case -0x7789fd4c:
				pcVar1 = "The mode test has finished executing.";
				break;
			case -0x7789fd4b:
				pcVar1 = "The mode test has switched to a new mode.";
				break;
			case -0x7789fd4a:
				pcVar1 = "D3D has not yet been initialized.";
				break;
			case -0x7789fd49:
				pcVar1 = "The video port is not active";
				break;
			case -0x7789fd48:
				pcVar1 = "The monitor does not have EDID data.";
				break;
			case -0x7789fd47:
				pcVar1 = "The driver does not enumerate display mode refresh rates.";
				break;
			default:
				if (a_eError == -0x7789fdc3) {
					return "No hardware support for 16 or 256 color palettes.";
				}
				switch (a_eError) {
				case -0x7789fdf8:
					pcVar1 = "Bitmask in the pixel format requested is unsupported by DirectDraw";
					break;
				case -0x7789fdf7:
					pcVar1 = "The specified stream contains invalid data";
					break;
				case -0x7789fdf6:
				case -0x7789fdf5:
				case -0x7789fdf4:
				case -0x7789fdf3:
				case -0x7789fdf2:
				case -0x7789fdf1:
				case -0x7789fdf0:
				case -0x7789fdef:
				case -0x7789fdee:
				case -0x7789fded:
				case -0x7789fdec:
				case -0x7789fdeb:
				case -0x7789fdea:
				case -0x7789fde9:
				case -0x7789fde8:
				case -0x7789fde6:
				case -0x7789fde5:
				case -0x7789fde3:
				case -0x7789fde1:
				case -0x7789fde0:
				case -0x7789fddf:
				case -0x7789fdde:
				case -0x7789fddd:
				case -0x7789fddc:
				case -0x7789fddb:
				case -0x7789fdda:
				case -0x7789fdd9:
				case -0x7789fdd8:
				case -0x7789fdd7:
				case -0x7789fdd6:
				case -0x7789fdd5:
				case -0x7789fdd4:
				case -0x7789fdd3:
				case -0x7789fdd2:
				case -0x7789fdd1:
					goto switchD_006e3696_caseD_7;
				case -0x7789fde7:
					pcVar1 = "vertical blank is in progress";
					break;
				case -0x7789fde4:
					pcVar1 = "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete.";
					break;
				case -0x7789fde2:
					pcVar1 = "The specified surface type requires specification of the COMPLEX flag";
					break;
				case -0x7789fdd0:
					pcVar1 = "Rectangle provided was not horizontally aligned on reqd. boundary";
					break;
				case -0x7789fdcf:
					pcVar1 = "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.";
					break;
				case -0x7789fdce:
					pcVar1 = "A DirectDraw object representing this driver has already been created for this process.";
					break;
				case -0x7789fdcd:
					pcVar1 = "A hardware only DirectDraw object creation was attempted but the driver did not support any hardware.";
					break;
				case -0x7789fdcc:
					pcVar1 = "this process already has created a primary surface";
					break;
				case -0x7789fdcb:
					pcVar1 = "software emulation not available.";
					break;
				case -0x7789fdca:
					pcVar1 = "region passed to Clipper::GetClipList is too small.";
					break;
				case -0x7789fdc9:
					pcVar1 = "an attempt was made to set a clip list for a clipper objec that is already monitoring an hwnd.";
					break;
				case -0x7789fdc8:
					pcVar1 = "No clipper object attached to surface object";
					break;
				case -0x7789fdc7:
					pcVar1 = "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.";
					break;
				case -0x7789fdc6:
					pcVar1 = "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state.";
					break;
				case -0x7789fdc5:
					pcVar1 = "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created.";
					break;
				case -0x7789fdc4:
					pcVar1 = "No palette object attached to this surface.";
					break;
				default:
					if (a_eError == -0x7789fe02) {
						return "Pixel format requested is unsupported by DirectDraw";
					}
					if (a_eError < -0x7789fe5b) {
						if (a_eError == -0x7789fe5c) {
							return "This surface is already a dependency of the surface it is being made a dependency of.";
						}
						if (a_eError < -0x7789fe81) {
							if (a_eError == -0x7789fe82) {
								return "hardware does not support clipped overlays";
							}
							if (a_eError == -0x7789feac) {
								return "Operation could not be carried out because there is no hardware support for zbuffer blting.";
							}
							if (a_eError == -0x7789fea2) {
								return "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays.";
							}
							if (a_eError == -0x7789fe98) {
								return "The hardware needed for the requested operation has already been allocated.";
							}
							if (a_eError == -0x7789fe84) {
								return "Out of video memory";
							}
						}
						else {
							if (a_eError == -0x7789fe80) {
								return "Can only have ony color key active at one time for overlays";
							}
							if (a_eError == -0x7789fe7d) {
								return "Access to this palette is being refused because the palette is already locked by another thread.";
							}
							if (a_eError == -0x7789fe70) {
								return "No src color key specified for this operation.";
							}
							if (a_eError == -0x7789fe66) {
								return "This surface is already attached to the surface it is being attached to.";
							}
						}
					}
					else if (a_eError < -0x7789fe33) {
						if (a_eError == -0x7789fe34) {
							return "The requested surface is not attached.";
						}
						if (a_eError == -0x7789fe52) {
							return "Access to this surface is being refused because the surface is already locked by another thread.";
						}
						if (a_eError == -0x7789fe4d) {
							return "Access to this surface is being refused because no driver exists which can supply a pointer to the surface. This is most likely to happen when attempting to lock the primary surface when no DCI provider is present. Will also happen on attempts to lock an optimized surface.";
						}
						if (a_eError == -0x7789fe48) {
							return "Access to Surface refused because Surface is obscured.";
						}
						if (a_eError == -0x7789fe3e) {
							return "Access to this surface is being refused because the surface is gone. The DIRECTDRAWSURFACE object representing this surface should have Restore called on it.";
						}
					}
					else {
						if (a_eError == -0x7789fe2a) {
							return "Height requested by DirectDraw is too large.";
						}
						if (a_eError == -0x7789fe20) {
							return "Size requested by DirectDraw is too large --  The individual height and width are OK.";
						}
						if (a_eError == -0x7789fe16) {
							return "Width requested by DirectDraw is too large.";
						}
					}
					goto switchD_006e3696_caseD_7;
				}
			}
		}
		else {
			if (a_eError < -0x7787ee8f) {
				switch (a_eError) {
				case -0x7787eeef:
					return "Second attempt to load a DLS collection that is currently open. ";
				case -0x7787eeee:
				case -0x7787eee8:
				case -0x7787eee6:
				case -0x7787eee5:
				case -0x7787eee4:
				case -0x7787eee3:
				case -0x7787eee2:
				case -0x7787eee1:
				case -0x7787eec0:
				case -0x7787eebf:
				case -0x7787eebe:
				case -0x7787eebd:
				case -0x7787eebc:
				case -0x7787eebb:
				case -0x7787eeba:
				case -0x7787eeb9:
				case -0x7787eeb8:
				case -0x7787eeb7:
				case -0x7787eeb6:
				case -0x7787eeb5:
				case -0x7787eeb4:
				case -0x7787eeb3:
				case -0x7787eeb2:
				case -0x7787eeb1:
				case -0x7787eead:
				case -0x7787eeac:
				case -0x7787eea6:
				case -0x7787eea5:
				case -0x7787eea4:
				case -0x7787eea3:
				case -0x7787eea2:
				case -0x7787eea1:
					goto switchD_006e3696_caseD_7;
				case -0x7787eeed:
					return "Error reading wave data from DLS collection. Indicates bad file.";
				case -0x7787eeec:
					return "There is no instrument in the collection that matches patch number.";
				case -0x7787eeeb:
					return "The IStream* doesn\'t support Seek().";
				case -0x7787eeea:
					return "The IStream* doesn\'t support Write().";
				case -0x7787eee9:
					return "The RIFF parser doesn\'t contain a required chunk while parsing file.";
				case -0x7787eee7:
					return "Invalid download id was used in the process of creating a download buffer.";
				case -0x7787eee0:
					return "Tried to unload an object that was not downloaded or previously unloaded.";
				case -0x7787eedf:
					return "Buffer was already downloaded to synth.";
				case -0x7787eede:
					return "The specified property item was not recognized by the target object.";
				case -0x7787eedd:
					return "The specified property item may not be set on the target object.";
				case -0x7787eedc:
					return "* The specified property item may not be retrieved from the target object.";
				case -0x7787eedb:
					return "Wave chunk has more than one interleaved channel. DLS format requires MONO.";
				case -0x7787eeda:
					return "Invalid articulation chunk in DLS collection.";
				case -0x7787eed9:
					return "Invalid instrument chunk in DLS collection.";
				case -0x7787eed8:
					return "Wavelink chunk in DLS collection points to invalid wave.";
				case -0x7787eed7:
					return "Articulation missing from instrument in DLS collection.";
				case -0x7787eed6:
					return "Downoaded DLS wave is not in PCM format. ";
				case -0x7787eed5:
					return "Bad wave chunk in DLS collection";
				case -0x7787eed4:
					return "Offset Table for download buffer has errors. ";
				case -0x7787eed3:
					return "Attempted to download unknown data type.";
				case -0x7787eed2:
					return "The operation could not be completed because no sink was connected to the synthesizer.";
				case -0x7787eed1:
					return "An attempt was made to open the software synthesizer while it was already  open.";
				case -0x7787eed0:
					return "An attempt was made to close the software synthesizer while it was already  open.";
				case -0x7787eecf:
					return "The operation could not be completed because the software synth has not  yet been fully configured.";
				case -0x7787eece:
					return "The operation cannot be carried out while the synthesizer is active.";
				case -0x7787eecd:
					return "An error occurred while attempting to read from the IStream* object.";
				case -0x7787eecc:
					return "The operation cannot be performed because the final instance of the DirectMusic object was released. Ports cannot be used after final  release of the DirectMusic object.";
				case -0x7787eecb:
					return "There was no data in the referenced buffer.";
				case -0x7787eeca:
					return "There is insufficient space to insert the given event into the buffer.";
				case -0x7787eec9:
					return "The given operation could not be carried out because the port is a capture port.";
				case -0x7787eec8:
					return "The given operation could not be carried out because the port is a render port.";
				case -0x7787eec7:
					return "The port could not be created because no DirectSound has been specified. Specify a DirectSound interface via the IDirectMusic::SetDirectSound method; pass NULL to have DirectMusic manage usage of DirectSound.";
				case -0x7787eec6:
					return "The operation cannot be carried out while the port is active.";
				case -0x7787eec5:
					return "Invalid DirectSound buffer was handed to port. ";
				case -0x7787eec4:
					return "Invalid buffer format was handed to the synth sink.";
				case -0x7787eec3:
					return "The operation cannot be carried out while the synthesizer is inactive.";
				case -0x7787eec2:
					return "IDirectMusic::SetDirectSound has already been called. It may not be changed while in use.";
				case -0x7787eec1:
					return "The given event is invalid (either it is not a valid MIDI message or it makes use of running status). The event cannot be packed into the buffer.";
				case -0x7787eeb0:
					return "The IStream* object does not contain data supported by the loading object.";
				case -0x7787eeaf:
					return "The object has already been initialized.";
				case -0x7787eeae:
					return "The file does not contain a valid band.";
				case -0x7787eeab:
					return "The IStream* object\'s data does not have a track header as the first chunk, and therefore can not be read by the segment object.";
				case -0x7787eeaa:
					return "The IStream* object\'s data does not have a tool header as the first chunk, and therefore can not be read by the graph object.";
				case -0x7787eea9:
					return "The IStream* object\'s data contains an invalid track header (ckid is 0 and fccType is NULL,) and therefore can not be read by the segment object.";
				case -0x7787eea8:
					return "The IStream* object\'s data contains an invalid tool header (ckid is 0 and fccType is NULL,) and therefore can not be read by the graph object.";
				case -0x7787eea7:
					return "The graph object was unable to load all tools from the IStream* object data. This may be due to errors in the stream, or the tools being incorrectly registered on the client.";
				case -0x7787eea0:
					return "The segment object was unable to load all tracks from the IStream* object data. This may be due to errors in the stream, or the tracks being incorrectly registered on the client.";
				case -0x7787ee9f:
					return "The object requested was not found (numerically equal to DMUS_E_NOT_FOUND)";
				case -0x7787ee9e:
					return "A required object is not initialized or failed to initialize.";
				case -0x7787ee9d:
					return "The requested parameter type is currently disabled. Parameter types may be enabled and disabled by certain calls to SetParam().";
				case -0x7787ee9c:
					return "The requested parameter type is not supported on the object.";
				case -0x7787ee9b:
					return "The time is in the past, and the operation can not succeed.";
				case -0x7787ee9a:
					return "The requested track is not contained by the segment.";
				case -0x7787ee99:
					return "The track does not support clock time playback or getparam.";
				default:
					return "There is no master clock in the performance. Be sure to call IDirectMusicPerformance::Init().";
				}
			}
			if (1 < a_eError) {
				if (a_eError < 0x40104) {
					if (a_eError == 0x40103) {
						return "The end of the list has been reached.";
					}
					switch (a_eError) {
					case 2:
					switchD_006e3696_caseD_2:
						return "The system cannot find the file specified.";
					case 3:
						return "The system cannot find the path specified.";
					case 4:
						return "The system cannot open the file.";
					case 5:
						return "Access is denied.";
					case 6:
						return "The handle is invalid.";
					case 8:
						return "Not enough storage is available to process this command.";
					case 9:
						return "The storage control block address is invalid.";
					case 10:
						return "The environment is incorrect.";
					case 0xb:
						return "An attempt was made to load a program with an incorrect format.";
					case 0xe:
						return "The system cannot find the drive specified.";
					}
				}
				else if (a_eError < 0x4028d) {
					if (a_eError == 0x4028c) {
						return "The audio stream did not contain sufficient information to determine the contents of each channel.";
					}
					if (a_eError < 0x40259) {
						if (a_eError == 0x40258) {
							return "Cannot play back the audio stream: no audio hardware is available.";
						}
						if (a_eError < 0x40247) {
							if (a_eError == 0x40246) {
								return "Some connections have failed and have been deferred.";
							}
							if (a_eError == 0x4022d) {
								return "An attempt to add a filter with a duplicate name succeeded with a modified name.";
							}
							if (a_eError == 0x40237) {
								return "The state transition has not completed.";
							}
							if (a_eError == 0x40242) {
								return "Some of the streams in this movie are in an unsupported format.";
							}
							if (a_eError == 0x40245) {
								return "The file contained some property settings that were not used.";
							}
						}
						else {
							if (a_eError == 0x40250) {
								return "The resource specified is no longer needed.";
							}
							if (a_eError == 0x40254) {
								return "A connection could not be made with the media type in the persistent graph, but has been made with a negotiated media type.";
							}
							if (a_eError == 0x40257) goto switchD_006e1b3f_caseD_80040255;
						}
					}
					else if (a_eError < 0x40269) {
						if (a_eError == 0x40268) {
							return "The graph can\'t be cued because of lack of or corrupt data.";
						}
						if (a_eError == 0x4025a) goto switchD_006e1b3f_caseD_80040259;
						if (a_eError == 0x40260) {
							return "The value returned had to be estimated.  It\'s accuracy can not be guaranteed.";
						}
						if (a_eError == 0x40263) {
							return "This success code is reserved for internal purposes within ActiveMovie.";
						}
						if (a_eError == 0x40267) {
							return "The stream has been turned off.";
						}
					}
					else {
						if (a_eError == 0x40270) {
							return "The stop time for the sample was not set.";
						}
						if (a_eError == 0x4027e) {
							return "There was no preview pin available, so the capture pin output is being split to provide both capture and preview.";
						}
						if (a_eError == 0x40280) {
							return "The current title was not a sequential set of chapters (PGC), and the returned timing information might not be continuous.";
						}
					}
				}
				else if (a_eError < 0x8781202) {
					if (a_eError == 0x8781201) {
						return "Return value from IDirectMusicTool::ProcessPMsg() which indicates to the performance that it should free the PMsg automatically.";
					}
					if (a_eError < 0x878000b) {
						if (a_eError == 0x878000a) {
							return "The call succeeded, but we had to substitute the 3D algorithm";
						}
						if (a_eError == 0x4028d) {
							return "The seek into the movie was not frame accurate.";
						}
						if (a_eError == 0x150005) {
							return "Full duplex";
						}
						if (a_eError == 0x15000a) {
							return "Half duplex";
						}
						if (a_eError == 0x150010) {
							return "Pending";
						}
					}
					else {
						if (a_eError == 0x8781091) {
							return
								"The object could only load partially. This can happen if some components are not registered properly, such as embedded tracks and tools. This can also happen if some content is missing. For example, if a segment uses a DLS collection that is not in the loader\'s current search directory ."
								;
						}
						if (a_eError == 0x8781092) {
							return "Return value from IDirectMusicBand::Download() which indicates that some of the instruments safely downloaded, but others failed. This usually occurs when some instruments are on PChannels not supported by the performance or port.";
						}
						if (a_eError == 0x8781200) {
							return "Return value from IDirectMusicTool::ProcessPMsg() which indicates to the performance that it should cue the PMsg again automatically.";
						}
					}
				}
				else if (a_eError < 0x8781214) {
					if (a_eError == 0x8781213) {
						return
							"Returned from IDirectMusicPerformance::MIDIToMusic(),  and IDirectMusicPerformance::MusicToMIDI(), this indicates  that the note conversion generated a note value that is below 0,  so it has been bumped up one or more octaves to be in the proper MIDI range of 0 through 127.  Note that thi s is valid for MIDIToMusic() when using play modes DMUS_PLAYMODE_FIXEDTOCHORD and DMUS_PLAYMODE_FIXEDTOKEY, both of which store MIDI values in wMusicValue. With MusicToMIDI(), it is valid for all play modes. Ofcourse, DMUS_PLAYMODE_FIXED will never return this success code."
							;
					}
					if (a_eError == 0x8781202) {
						return "Return value from IDirectMusicTrack::Play() which indicates to the segment that the track has no more data after mtEnd.";
					}
					if (a_eError == 0x8781210) {
						return "Returned string has been truncated to fit the buffer size.";
					}
					if (a_eError == 0x8781211) {
						return "Returned from IDirectMusicGraph::StampPMsg(), this indicates that the PMsg is already stamped with the last tool in the graph. The returned PMsg\'s tool pointer is now NULL.";
					}
					if (a_eError == 0x8781212) {
						return
							"Returned from IDirectMusicPerformance::MusicToMIDI(), this indicates  that no note has been calculated because the music value has the note  at a position higher than the top note of the chord. This applies only to DMUS_PLAYMODE_NORMALCHORD play mode. This success code indicates that the  caller should not do anything with the note. It is not meant to be played against this chord."
							;
					}
				}
				else {
					if (a_eError == 0x8781214) {
						return
							"Returned from IDirectMusicPerformance::MIDIToMusic(),  and IDirectMusicPerformance::MusicToMIDI(), this indicates  that the note conversion generated a note value that is above 127, so it has been bumped down one or more octaves to be in the proper MIDI range of 0 through 127.  Note that  this is valid for MIDIToMusic() when using play modes DMUS_PLAYMODE_FIXEDTOCHORD and DMUS_PLAYMODE_FIXEDTOKEY, both of which store MIDI values in wMusicValue. With MusicToMIDI(), it is valid for all play modes. Ofcourse, DMUS_PLAYMODE_FIXED will never return this success code."
							;
					}
					if (a_eError == 0x8781215) {
						return "Although the audio output from the port will be routed to the same device as the given DirectSound buffer, buffer controls such as pan and volume will not affect the output.";
					}
					if (a_eError == 0x8781216) {
						return "The requested operation was not performed because during CollectGarbage the loader determined that the object had been released.";
					}
				}
				goto switchD_006e3696_caseD_7;
			}
			if (a_eError == 1) {
				return "Call successful, but returned FALSE";
			}
			if (-0x7787ede7 < a_eError) {
				if (a_eError < 1) {
					if (a_eError == 0) {
						return "The function completed successfully";
					}
					switch (a_eError) {
					case -0x7787ede6:
						return "The script does not contain a variable with the specified name.";
					case -0x7787ede5:
						return "The script does not contain a routine with the specified name.";
					case -0x7787ede4:
						return "Scripts variables for content referenced or embedded in a script cannot be set.";
					case -0x7787ede3:
						return "Attempt was made to set a script\'s variable by reference to a value that was not an object type.";
					case -0x7787ede2:
						return "Attempt was made to set a script\'s variable by value to an object that does not support a default value property.";
					case -0x7787ede0:
						return "The file contains an invalid segment trigger track.";
					case -0x7787eddf:
						return "The file contains an invalid lyrics track.";
					case -0x7787edde:
						return "The file contains an invalid parameter control track.";
					case -0x7787eddd:
						return "A script written in AudioVBScript could not be read because it contained a statement that is not allowed by the AudioVBScript language.";
					case -0x7787eddc:
						return "A script routine written in AudioVBScript failed because an invalid operation occurred.  For example, adding the number 3 to a segment object would produce this error.  So would attempting to call a routine that doesn\'t exist.";
					case -0x7787eddb:
						return "A script routine written in AudioVBScript failed because a function outside of a script failed to complete. For example, a call to PlaySegment that fails to play because of low memory would return this error.";
					case -0x7787edda:
						return "The Performance has set up some PChannels using the AssignPChannel command, which makes it not capable of supporting audio paths.";
					case -0x7787edd9:
						return "This is the inverse of the previous error. The Performance has set up some audio paths, which makes is incompatible with the calls to allocate pchannels, etc. ";
					case -0x7787edd8:
						return "A segment or song was asked for its embedded audio path configuration, but there isn\'t any. ";
					case -0x7787edd7:
						return "An audiopath is inactive, perhaps because closedown was called.";
					case -0x7787edd6:
						return "An audiopath failed to create because a requested buffer could not be created.";
					case -0x7787edd5:
						return "An audiopath could not be used for playback because it lacked port assignments.";
					case -0x7787edd4:
						return "Attempt was made to play segment in audiopath mode and there was no audiopath.";
					case -0x7787edd3:
						return "Invalid data was found in a RIFF file chunk.";
					case -0x7787edd2:
						return "Attempt was made to create an audiopath that sends to a global effects buffer which did not exist.";
					case -0x7787edd1:
						return "The file does not contain a valid container object.";
					}
				}
				goto switchD_006e3696_caseD_7;
			}
			if (a_eError == -0x7787ede7) {
				return "The file contains an invalid script track.";
			}
			switch (a_eError) {
			case -0x7787edfb:
				pcVar1 = "The DMUS_PMSG was either not allocated by the performance via IDirectMusicPerformance::AllocPMsg(), or it was already freed via IDirectMusicPerformance::FreePMsg().";
				break;
			case -0x7787edfa:
				pcVar1 = "The default system port could not be opened.";
				break;
			case -0x7787edf9:
				pcVar1 = "A call to MIDIToMusic() or MusicToMIDI() resulted in an error because the requested conversion could not happen. This usually occurs when the provided DMUS_CHORD_KEY structure has an invalid chord or scale pattern.";
				break;
			case -0x7787edf0:
				pcVar1 = "DMUS_E_DESCEND_CHUNK_FAIL is returned when the end of the file  was reached before the desired chunk was found.";
				break;
			case -0x7787edef:
				pcVar1 = "An attempt to use this object failed because it first needs to be loaded.";
				break;
			case -0x7787eded:
				pcVar1 = "The activeX scripting engine for the script\'s language is not compatible with DirectMusic.";
				break;
			case -0x7787edec:
				pcVar1 = "A varient was used that had a type that is not supported by DirectMusic.";
				break;
			case -0x7787edeb:
				pcVar1 = "An error was encountered while parsing or executing the script. The pErrorInfo parameter (if supplied) was filled with information about the error.";
				break;
			case -0x7787edea:
				pcVar1 = "Loading of oleaut32.dll failed.  VBScript and other activeX scripting languages require use of oleaut32.dll.  On platforms where oleaut32.dll is not present, only the DirectMusicScript language, which doesn\'t require oleaut32.dll can be used.";
				break;
			case -0x7787ede9:
				pcVar1 = "An error occured while parsing a script loaded using LoadScript.  The script that was loaded contains an error.";
				break;
			case -0x7787ede8:
				pcVar1 = "The script file is invalid.";
				break;
			default:
				if (a_eError == -0x7787edfc) {
					return "The DMUS_PMSG has already been sent to the performance object via IDirectMusicPerformance::SendPMsg().";
				}
				if (a_eError < -0x7787ee7a) {
					if (a_eError == -0x7787ee7b) {
						return "Object was not found.";
					}
					if (a_eError == -0x7787ee80) {
						return "The class id field is required and missing in the DMUS_OBJECTDESC.";
					}
					if (a_eError == -0x7787ee7f) {
						return "The requested file path is invalid.";
					}
					if (a_eError == -0x7787ee7e) {
						return "File open failed - either file doesn\'t exist or is locked.";
					}
					if (a_eError == -0x7787ee7d) {
						return "Search data type is not supported.";
					}
					if (a_eError == -0x7787ee7c) {
						return "Unable to find or create object.";
					}
				}
				else {
					if (a_eError == -0x7787ee7a) {
						return "The file name is missing from the DMUS_OBJECTDESC.";
					}
					if (a_eError == -0x7787ee00) {
						return "The file requested is not a valid file.";
					}
					if (a_eError == -0x7787edff) {
						return "The tool is already contained in the graph. Create a new instance.";
					}
					if (a_eError == -0x7787edfe) {
						return "Value is out of range, for instance the requested length is longer than the segment.";
					}
					if (a_eError == -0x7787edfd) {
						return "Segment initialization failed, most likely due to a critical memory situation.";
					}
				}
			case -0x7787edf8:
			case -0x7787edf7:
			case -0x7787edf6:
			case -0x7787edf5:
			case -0x7787edf4:
			case -0x7787edf3:
			case -0x7787edf2:
			case -0x7787edf1:
			case -0x7787edee:
			switchD_006e3696_caseD_7:
				pcVar1 = "n/a";
			}
		}
		return pcVar1;

	}
}
