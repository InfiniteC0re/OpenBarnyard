#pragma once
#include "Toshi/Render/TRender.h"
#include <d3d11.h>

namespace Toshi
{
	class TRenderDX11 : TRenderInterface
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

		ID3D11DeviceContext* m_pDeviceContext; // 0x68C
		ID3D11Device* m_pDevice; // 0x690

		void BuildAdapterDatabase();
		
	public:

		bool Create(LPCSTR a_name);
		virtual bool Create() { return Create("de Blob"); }
		const char* GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel);
		void Update();
	};
}