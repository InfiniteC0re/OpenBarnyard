#include "ToshiPCH.h"
#include "TRender_DX11.h"

namespace Toshi
{
	bool TRenderDX11::Create(LPCSTR a_name)
	{
		TASSERT(TFALSE == IsCreated(), "TRenderDX11 already created");
		bool bResult = TRenderInterface::Create();
		if (bResult)
		{
			TOSHI_INFO("Creating TRenderDX11");
			TIMPLEMENT("FUN_0065d750");

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_3;

			for (size_t i = 0; i < 3; i++)
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

			m_hAccel = CreateAcceleratorTableA(accels, 2);

			if (GetMSWindow()->Create(this, a_name))
			{
				return true;
			}
			TOSHI_INFO("Failed to create Window");
		}
		return false;
	}

	const char* TRenderDX11::GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel)
	{
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
		IDXGIFactory1* pFactory;

		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
		
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter1* pAdapter;

			while (pFactory->EnumAdapters1(0, &pAdapter) != DXGI_ERROR_NOT_FOUND)
			{
				TTODO("Implement iterating through adapters");
			}

		}
	}

	void TRenderDX11::Update()
	{
		MSG msg;
		if (m_bIsEnabled)
		{
			while (GetMessageA(&msg, NULL, 0, 0) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
				if (PeekMessageA(&msg, NULL, 0, 0, 0) == 0)
				{
					return;
				}
			}
			m_bIsEnabled = true;
		}
	}
}