#pragma once
#include "Toshi/Render/TRender.h"
<<<<<<< HEAD:Toshi/Source/Toshi/Render/TRender_DX11.h
#include "Toshi/Core/TMSWindow.h"
=======
#include "Platform/Windows/TMSWindow.h"
>>>>>>> b19878c8db5405a44256abff728c3dce985f73ec:Toshi/Source/Platform/Windows/DX11/TRender_DX11.h
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
<<<<<<< HEAD:Toshi/Source/Toshi/Render/TRender_DX11.h

		TMSWindow* m_pWindow;
		ID3D11DeviceContext* m_pDeviceContext; // 0x68C
		ID3D11Device* m_pDevice; // 0x690
		HACCEL m_hAccel; // 0x694

		void BuildAdapterDatabase();
		
	public:
		TMSWindow* GetMSWindow() { return m_pWindow; }
=======
		
	public:
		virtual bool Create() override { return Create("de Blob"); }
		
>>>>>>> b19878c8db5405a44256abff728c3dce985f73ec:Toshi/Source/Platform/Windows/DX11/TRender_DX11.h
		bool Create(LPCSTR a_name);
		void Update();

		static const char* GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel);

	private:
		void BuildAdapterDatabase();

	private:
		ID3D11DeviceContext* m_pDeviceContext; // 0x68C
		ID3D11Device* m_pDevice;               // 0x690
		TMSWindow m_Window;                    // 0x6B0
	};
}