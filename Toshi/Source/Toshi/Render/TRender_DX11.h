#pragma once
#include "Toshi/Render/TRender.h"

namespace Toshi
{
	class TRenderDX11 //: TRenderInterface
	{

		static constexpr D3D_DRIVER_TYPE m_scpDriverTypes[3]
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE
		};

		static constexpr D3D_FEATURE_LEVEL m_scpFeatureLevels[4]
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		bool m_bIsCreated; //0xA

		ID3D11DeviceContext* m_pDeviceContext; // 0x68C
		ID3D11Device* m_pDevice; // 0x690

		inline bool IsCreated() const { return m_bIsCreated; }

		bool Create(LPCSTR a_name);
		const char* GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel);
	};
}