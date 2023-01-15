#include "ToshiPCH.h"
#include "TRender_DX11.h"
#include "TRenderContext_DX11.h"

namespace Toshi
{
	TRenderContext* TRender::CreateRenderContext()
	{
		return new TRenderContextDX11(*this);
	}

	UINT TRenderDX11::s_QualityLevel = 1;

	TRenderAdapter* TD3DAdapter::Mode::GetAdapter() const
	{
		return m_Adapter;
	}

	size_t TD3DAdapter::Mode::GetModeIndex() const
	{
		return m_ModeIndex;
	}

	uint32_t TD3DAdapter::Mode::GetWidth() const
	{
		return m_Description.Width;
	}

	uint32_t TD3DAdapter::Mode::GetHeight() const
	{
		return m_Description.Height;
	}

	bool TD3DAdapter::Mode::SomeCheck1() const
	{
		DXGI_FORMAT format = m_Description.Format;

		return (
			format == DXGI_FORMAT_B8G8R8X8_UNORM ||
			format == DXGI_FORMAT_R8G8B8A8_UNORM
		);
	}

	bool TD3DAdapter::Mode::SomeCheck2() const
	{
		DXGI_FORMAT format = m_Description.Format;

		return (
			format == DXGI_FORMAT_B5G6R5_UNORM   ||
			format == DXGI_FORMAT_B5G5R5A1_UNORM ||
			format == DXGI_FORMAT_B4G4R4A4_UNORM
		);
	}

	float TD3DAdapter::Mode::GetRefreshRate() const
	{
		return m_Description.RefreshRate.Numerator / m_Description.RefreshRate.Denominator;
	}

	TRenderAdapter::Mode::Device* TD3DAdapter::Mode::GetDevice(int device)
	{
		TASSERT(device > 0 && device < NUMSUPPORTEDDEVICES);
		return &m_Devices[device];
	}

	void TD3DAdapter::Mode::GetDisplayMode(IDXGIOutput* dxgiOutput, DXGI_MODE_DESC* modeDesc)
	{
		DXGI_OUTPUT_DESC outputDesc;
		dxgiOutput->GetDesc(&outputDesc);

		MONITORINFOEXA monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfoA(outputDesc.Monitor, &monitorInfo);

		DEVMODEA displaySettings = { };
		displaySettings.dmSize = sizeof(displaySettings);
		EnumDisplaySettingsA(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &displaySettings);

		bool defaultRefreshRate = false;

		if (displaySettings.dmDisplayFrequency == 1 || displaySettings.dmDisplayFrequency == 0)
		{
			defaultRefreshRate = true;
			displaySettings.dmDisplayFrequency = 0;
		}

		DXGI_MODE_DESC matchMode;
		matchMode.Width = displaySettings.dmPelsWidth;
		matchMode.Height = displaySettings.dmPelsHeight;
		matchMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		matchMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		matchMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		matchMode.RefreshRate.Denominator = (defaultRefreshRate == true) ? 0 : 1;
		matchMode.RefreshRate.Numerator = displaySettings.dmDisplayFrequency;

		dxgiOutput->FindClosestMatchingMode(&matchMode, modeDesc, NULL);
	}

	bool TRenderDX11::CreateDisplay(DisplayParams* pDisplayParams)
	{
		TASSERT(IsCreated() == TTRUE);
		TASSERT(IsDisplayCreated() == TFALSE);

		if (TRender::CreateDisplay() == TFALSE)
		{
			ShowDisplayError();
			return false;
		}

		m_DisplayParams = *pDisplayParams;

		auto adapterList = GetAdapterList();
		auto adapter = adapterList->Head()->As<TD3DAdapter>();
		auto mode = adapter->GetMode();

		if (!m_DisplayParams.Unk5 && !m_DisplayParams.IsFullscreen)
		{
			m_DisplayParams.Width = mode->GetWidth();
			m_DisplayParams.Height = mode->GetHeight();
		}

		IDXGIDevice* pDevice;
		IDXGIAdapter* pAdapter;
		IDXGIFactory1* pFactory1;
		m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDevice);
		pDevice->GetAdapter(&pAdapter);
		pAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory1);

		DisplayParams* pCurrDisplayParams = GetCurrentDisplayParams();

		DXGI_MODE_DESC foundMode;
		if (pCurrDisplayParams->Unk5 == TFALSE)
		{
			IDXGIOutput* pOutput;
			pAdapter->EnumOutputs(0, &pOutput);

			TD3DAdapter::Mode::GetDisplayMode(pOutput, &foundMode);
			m_DisplayParams.Width = foundMode.Width;
			m_DisplayParams.Height = foundMode.Height;
		}

		pAdapter->Release();
		pDevice->Release();

		Toshi::TUtil::MemClear(&m_SwapChainDesc, sizeof(m_SwapChainDesc));

		DXGI_MODE_DESC* pSelectedMode;

		if (m_DisplayParams.IsFullscreen == false)
		{
			pSelectedMode = mode->GetDescription();
			foundMode.Format = pSelectedMode->Format;
		}
		else
		{
			pSelectedMode = &foundMode;
		}

		m_SwapChainDesc.BufferCount = 1;
		m_SwapChainDesc.BufferDesc.Width = m_DisplayParams.Width;
		m_SwapChainDesc.BufferDesc.Height = m_DisplayParams.Height;
		m_SwapChainDesc.BufferDesc.Format = pSelectedMode->Format;
		m_SwapChainDesc.BufferDesc.RefreshRate = pSelectedMode->RefreshRate;
		m_SwapChainDesc.OutputWindow = m_Window.GetHWND();
		m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_SwapChainDesc.SampleDesc.Count = 1;
		m_SwapChainDesc.SampleDesc.Quality = 0;
		m_SwapChainDesc.Windowed = TRUE;
		m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		m_SwapChainDesc.Flags = m_DisplayParams.IsFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

		pFactory1->CreateSwapChain(m_pDevice, &m_SwapChainDesc, &m_SwapChain);
		pFactory1->MakeWindowAssociation(m_Window.GetHWND(), DXGI_MWA_NO_ALT_ENTER);
		pFactory1->Release();

		if (m_pDeviceContext == TNULL || m_pDevice == TNULL || m_SwapChain == TNULL)
		{
			ShowDeviceError();
		}

		UINT refreshRateNum = m_SwapChainDesc.BufferDesc.RefreshRate.Numerator;
		UINT refreshRateDenom = m_SwapChainDesc.BufferDesc.RefreshRate.Denominator;
		double refreshRate = (double)refreshRateNum / refreshRateDenom;

		TOSHI_CORE_TRACE(L"CurrentDisplay: {0}", mode->GetDisplayName());
		pSelectedMode = mode->GetDescription();
		TOSHI_CORE_TRACE("CurrentDisplayMode = {0} x {1} ({2:.4f} Hz) = [{3} / {4}]", pSelectedMode->Width, pSelectedMode->Height, refreshRate, refreshRateNum, refreshRateDenom);
		
		if (m_DisplayParams.IsFullscreen)
		{
			TOSHI_CORE_TRACE("Set Fullscreen {0} x {1} ({2:.4f} Hz) = [{3} / {4}]", m_DisplayParams.Width, m_DisplayParams.Height, refreshRate, refreshRateNum, refreshRateDenom);
			m_Window.SetFullscreen();
		}
		else
		{
			TOSHI_CORE_TRACE("Set Windowed {0} x {1}", m_DisplayParams.Width, m_DisplayParams.Height);
			m_Window.SetWindowed();
		}

		UINT centerX = 0;
		UINT centerY = 0;

		if (!m_DisplayParams.IsFullscreen && !m_DisplayParams.Unk5)
		{
			LONG style = GetWindowLongA(m_Window.GetHWND(), GWL_STYLE);
			SetWindowLongA(m_Window.GetHWND(), GWL_STYLE, style & (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_DISABLED | WS_GROUP | WS_HSCROLL | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_POPUP | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL));
			SetWindowPos(m_Window.GetHWND(), HWND_TOP, 0, 0, m_DisplayParams.Width, m_DisplayParams.Height, 0);
		}
		else
		{
			centerX = (mode->GetWidth() - m_DisplayParams.Width) / 2;
			centerY = (mode->GetHeight() - m_DisplayParams.Height) / 2;
			m_Window.SetPosition(centerX, centerY, m_DisplayParams.Width, m_DisplayParams.Height);
		}

		m_DisplayHeight = m_DisplayParams.Height;
		m_DisplayWidth = m_DisplayParams.Width;
		m_IsWidescreen = true;

		// expectedWidth is the width expected for 16:9 screen
		UINT expectedWidth = (m_DisplayHeight / 9) * 16;
		
		if (expectedWidth != m_DisplayWidth)
		{
			// The aspect ratio is not 16:9
			if (expectedWidth < m_DisplayWidth)
			{
				m_DisplayParams.Width = expectedWidth;
				m_SwapChainDesc.BufferDesc.Width = expectedWidth;
			}
			else
			{
				UINT expectedHeight = (m_DisplayWidth / 16) * 9;
				m_DisplayParams.Height = expectedHeight;
				m_SwapChainDesc.BufferDesc.Height = expectedHeight;
			}
		}

		if (s_QualityLevel == 1)
		{
			UINT width = m_DisplayParams.Width / 2;
			UINT height = m_DisplayParams.Height / 2;

			if (width > 1920)
			{
				width = 1920;
				height = 1080;
			}

			if (width < 1280)
			{
				width = 1280;
			}

			if (height < 720)
			{
				height = 720;
			}

			m_DisplayParams.Height = height;
			m_DisplayParams.Width = width;
		}
		else if (s_QualityLevel > 0)
		{
			m_DisplayParams.Width = 1280;
			m_DisplayParams.Height = 720;
		}

		m_DisplayParams.Width = TMath::Max(m_DisplayParams.Width, 1280);
		m_DisplayParams.Height = TMath::Max(m_DisplayParams.Height, 720);

		m_SwapChainDesc.BufferDesc.Width = m_DisplayParams.Width;
		m_SwapChainDesc.BufferDesc.Height = m_DisplayParams.Height;
		uint32_t& qualityLevel = m_DisplayParams.MultisampleQualityLevel;

		if (qualityLevel < 2)
		{
			qualityLevel = 1;
		}
		else
		{
			if (qualityLevel != 2 && qualityLevel != 4 && qualityLevel != 8)
			{
				qualityLevel = 4;
			}

			bool isMultisampleSet = false;
			for (UINT level = qualityLevel; 1 < qualityLevel; qualityLevel /= 2)
			{
				UINT numQualityLevels;
				HRESULT hRes = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, level, &numQualityLevels);
				
				if (hRes != S_OK && numQualityLevels != 0)
				{
					m_DisplayParams.MultisampleQualityLevel = level;
					isMultisampleSet = true;
					break;
				}
			}

			if (!isMultisampleSet)
			{
				// Default value
				m_DisplayParams.MultisampleQualityLevel = 1;
			}
		}

		HRESULT hRes1 = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_Texture2D1);

		if (hRes1 == S_OK)
		{
			HRESULT hRes2 = m_pDevice->CreateRenderTargetView(m_Texture2D1, NULL, &m_RTView1);

			if (hRes2 == S_OK)
			{
				TOSHI_CORE_TRACE("Creating Main RT {0} x {1}", m_DisplayParams.Width, m_DisplayParams.Height);
				TTODO("Toshi::TRenderDX11::FUN_006a6b60");
			}
		}

		SetCursorPos(centerX + m_DisplayParams.Width / 2, centerY + m_DisplayParams.Height / 2);

		if (m_DisplayParams.Unk5 == false)
		{
			ShowCursor(false);
		}

		TTODO("Toshi::TRenderDX11::CreateSamplerStates");
		TTODO("Create and compile shaders");

		return true;
	}

	bool TRenderDX11::RecreateDisplay(DisplayParams* pDisplayParams)
	{
		return false;
	}

	void TRenderDX11::ShowDeviceError()
	{
		CHAR caption[1024] = { 0 };
		CHAR text[1024] = { 0 };

		GetPrivateProfileStringA("Setup", "IDS_D3DDEVICEERRORTITLE", "Initialization failure", caption, sizeof(caption), ".\\Setup.ini");
		GetPrivateProfileStringA("Setup", "IDS_D3DDEVICEERROR", "Failed to create D3D11 device", caption, sizeof(caption), ".\\Setup.ini");

		MessageBoxA(NULL, text, caption, MB_OK);
	}

	void TRenderDX11::ShowDisplayError()
	{
		CHAR caption[1024] = { 0 };
		CHAR text[1024] = { 0 };

		GetPrivateProfileStringA("Setup", "IDS_D3DDISPLAYERRORTITLE", "Initialization failure", caption, sizeof(caption), ".\\Setup.ini");
		GetPrivateProfileStringA("Setup", "IDS_D3DDISPLAYERROR", "Failed to create the display. Please run the Barnyard setup program and reconfigure", text, sizeof(text), ".\\Setup.ini");
		
		MessageBoxA(NULL, text, caption, MB_OK);
	}

	bool TRenderDX11::Create(LPCSTR a_name)
	{
		// 006a5e30
		TASSERT(TFALSE == IsCreated());
		bool bResult = TRender::Create();

		if (bResult)
		{
			TOSHI_INFO("Creating TRenderDX11");
			TIMPLEMENT_D("FUN_0065d750");

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_3;

			for (size_t i = 0; i < sizeof(m_scpDriverTypes) / sizeof(*m_scpDriverTypes); i++)
			{
				if (D3D11CreateDevice(NULL, m_scpDriverTypes[i], NULL, 0, m_scpFeatureLevels, 4, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext) == S_OK) break;
				if (D3D11CreateDevice(NULL, m_scpDriverTypes[i], NULL, 0, &m_scpFeatureLevels[1], 3, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext) == S_OK) break;
			}

			TOSHI_INFO("FeatureLevel : {0}", GetFeatureLevel(featureLevel));

			BuildAdapterDatabase();

			if (m_hAccel != NULL)
			{
				DestroyAcceleratorTable(m_hAccel);
				m_hAccel = NULL;
			}

			ACCEL accels[2];
			accels[0].fVirt = true;
			accels[0].key = VK_ESCAPE;

			accels[1].fVirt = FALT;
			accels[1].key = VK_RETURN;

			m_hAccel = CreateAcceleratorTableA(accels, sizeof(accels) / sizeof(*accels));

			if (m_Window.RegisterWindowClass(this, a_name))
			{
				m_Unk2 = 1;
				return true;
			}

			TOSHI_INFO("Failed to create Window");
		}

		return false;
	}

	const char* TRenderDX11::GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel)
	{
		// 006a9470
		switch (a_featureLevel)
		{
		case D3D_FEATURE_LEVEL_9_1:
			return "D3D_FEATURE_LEVEL_9_1";
		case D3D_FEATURE_LEVEL_9_2:
			return "D3D_FEATURE_LEVEL_9_2";
		case D3D_FEATURE_LEVEL_9_3:
			return "D3D_FEATURE_LEVEL_9_3";
		case D3D_FEATURE_LEVEL_10_0:
			return "D3D_FEATURE_LEVEL_10_0";
		case D3D_FEATURE_LEVEL_10_1:
			return "D3D_FEATURE_LEVEL_10_1";
		case D3D_FEATURE_LEVEL_11_0:
			return "D3D_FEATURE_LEVEL_11_0";
		case D3D_FEATURE_LEVEL_11_1:
			return "D3D_FEATURE_LEVEL_11_1";
		default:
			return "unknown";
		}
	}

	void TRenderDX11::BuildAdapterDatabase()
	{
		// 006a7df0
		IDXGIFactory1* pFactory;

		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
		
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* dxgiAdapter;

			int adapterIndex = 0;
			HRESULT enumResult = pFactory->EnumAdapters(adapterIndex, &dxgiAdapter);
			
			while (enumResult != DXGI_ERROR_NOT_FOUND)
			{
				TD3DAdapter* d3dAdapter = new TD3DAdapter;
				auto adapterDesc = d3dAdapter->GetAdapterDesc();
				dxgiAdapter->GetDesc(adapterDesc);
				
				d3dAdapter->SetAdapterIndex(adapterIndex);
				d3dAdapter->UpdateAdapterInfo();

				TOSHI_CORE_TRACE(L"Adapter: {0}", adapterDesc->Description);
				TOSHI_CORE_TRACE("Vendor: {0}, Device: {1}, Revision: {2}", adapterDesc->VendorId, adapterDesc->DeviceId, adapterDesc->Revision);
				TOSHI_CORE_TRACE("DedicatedSystemMemory: {0:.2f} MB", (double)adapterDesc->DedicatedSystemMemory / 1024 / 1024);
				TOSHI_CORE_TRACE("SharedSystemMemory: {0:.2f} MB", (double)adapterDesc->SharedSystemMemory / 1024 / 1024);
				d3dAdapter->EnumerateOutputs(this, dxgiAdapter);

				GetAdapterList()->InsertTail(d3dAdapter);

				dxgiAdapter->Release();
				enumResult = pFactory->EnumAdapters(++adapterIndex, &dxgiAdapter);
			}
		}
	}

	void TD3DAdapter::EnumerateOutputs(TRenderDX11* render, IDXGIAdapter* dxgiAdapter)
	{
		IDXGIOutput* dxgiOutput;
		bool shouldSaveMonitorData = true;
		GetModeList()->DeleteAll();

		int displayIndex = 0;
		HRESULT enumResult = dxgiAdapter->EnumOutputs(displayIndex, &dxgiOutput);

		DXGI_MODE_DESC modeDesc;
		DXGI_OUTPUT_DESC outputDesc;
		while (enumResult != DXGI_ERROR_NOT_FOUND)
		{
			dxgiOutput->GetDesc(&outputDesc);
			LONG deviceHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
			LONG deviceWidth = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
		
			TOSHI_CORE_TRACE(L"Display[{0}]: {1} ({2}x{3})", displayIndex, outputDesc.DeviceName, deviceWidth, deviceHeight);
			
			UINT numModes = 0;
			dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
			
			if (numModes > 0)
			{
				TD3DAdapter::Mode::GetDisplayMode(dxgiOutput, &modeDesc);

				if (shouldSaveMonitorData == true)
				{
					m_Mode.SetAdapter(this);
					m_Mode.SetDescription(modeDesc);
					m_Mode.SetName(outputDesc.DeviceName);
					shouldSaveMonitorData = false;
				}

				DXGI_MODE_DESC* descriptions = new DXGI_MODE_DESC[numModes];
				DXGI_MODE_DESC* currentDesc = descriptions;

				dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, descriptions);
				
				for (size_t i = 0; i < numModes; i++)
				{
					Mode* mode = new Mode;
					mode->SetModeIndex(i);
					mode->SetDisplayIndex(displayIndex);
					mode->SetAdapter(this);
					mode->SetDescription(currentDesc[i]);
					mode->SetName(outputDesc.DeviceName);

					GetModeList()->InsertTail(mode);
				}

				delete[] descriptions;
			}

			displayIndex += 1;
			enumResult = dxgiAdapter->EnumOutputs(displayIndex, &dxgiOutput);
		}
	}
}