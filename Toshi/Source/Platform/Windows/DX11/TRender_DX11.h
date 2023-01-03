#pragma once
#include "Toshi/Render/TRender.h"
#include "Platform/Windows/TMSWindow.h"

#include <d3d11.h>

namespace Toshi
{
	class TRenderDX11 : public TRender
	{
	private:
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
		
		
	public:
		TRenderDX11() { }
		~TRenderDX11() { }

		TMSWindow* GetMSWindow() { return &m_Window; }
		
	public:
		virtual bool CreateDisplay(DisplayParams* params) override { return true; }
		virtual bool DestroyDisplay() override { return true; }
		virtual void* GetCurrentDevice() override { return TNULL; }
		virtual DisplayParams* GetCurrentDisplayParams() override { return TNULL; }
		virtual bool Create() override { return Create("de Blob"); }
		
		bool Create(LPCSTR a_name);
		void Update();

		static const char* GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel);

	private:
		void BuildAdapterDatabase();

	private:
		ID3D11DeviceContext* m_pDeviceContext; // 0x68C
		ID3D11Device* m_pDevice;               // 0x690
		HACCEL m_hAccel;                       // 0x694
		TMSWindow m_Window;                    // 0x6B0
	};
}