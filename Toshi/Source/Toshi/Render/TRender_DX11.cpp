#include "TRender_DX11.h"
#include "ToshiPCH.h"

namespace Toshi
{
	bool TRenderDX11::Create(LPCSTR a_name)
	{
		TASSERT(TFALSE == IsCreated(), "TRenderDX11 already created");
		//bool bResult = TRenderInterface::Create();
		//if (bResult)
		{
			TOSHI_INFO("Creating TRenderDX11");
			TIMPLEMENT("FUN_0065d750");

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_3;

			for (size_t i = 0; i < 3; i++)
			{
				if (D3D11CreateDevice(NULL, m_scpDriverTypes[i], NULL, 0, m_scpFeatureLevels, 4, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext) == S_OK) break;
				if (D3D11CreateDevice(NULL, m_scpDriverTypes[i], NULL, 0, m_scpFeatureLevels + 4, 3, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext) == S_OK) break;
			}

			TOSHI_INFO("FeatureLevel : %s", GetFeatureLevel(featureLevel));
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
}