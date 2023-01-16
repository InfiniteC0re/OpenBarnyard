#pragma once
#include "Toshi/Render/TRender.h"
#include "Toshi/Render/TRenderAdapter.h"
#include "Platform/Windows/TMSWindow.h"

#include <d3d11.h>

namespace Toshi
{
	class TD3DAdapter : public TRenderAdapter
	{
	public:
		class Mode : public TRenderAdapter::Mode
		{
		public:
			static constexpr size_t NUMSUPPORTEDDEVICES = 2;
			static constexpr size_t DISPLAYNAMESIZE = 45;

			class Device : public TRenderAdapter::Mode::Device
			{
			public:
				Device() = default;
				~Device() = default;
			};

		public:
			Mode() : m_Devices() { }
			~Mode() = default;

			virtual TRenderAdapter* GetAdapter() const override;
			virtual size_t GetModeIndex() const override;
			virtual uint32_t GetWidth() const override;
			virtual uint32_t GetHeight() const override;
			virtual bool SomeCheck1() const override;
			virtual bool SomeCheck2() const override;
			virtual float GetRefreshRate() const override;
			virtual TRenderAdapter::Mode::Device* GetDevice(int device) override;

			static void GetDisplayMode(IDXGIOutput* dxgiOutput, DXGI_MODE_DESC* modeDesc);

			DXGI_MODE_DESC* GetDescription()
			{
				return &m_Description;
			}

			void SetDescription(DXGI_MODE_DESC& description)
			{
				m_Description = description;
			}

			void SetAdapter(TD3DAdapter* adapter)
			{
				m_Adapter = adapter;
			}

			WCHAR* GetDisplayName()
			{
				return m_DisplayName;
			}

			void SetModeIndex(size_t index)
			{
				m_ModeIndex = index;
			}

			void SetDisplayIndex(size_t index)
			{
				m_DisplayIndex = index;
			}

			void SetName(WCHAR* name)
			{
				TStringManager::String16Copy(GetDisplayName(), name);
			}

		private:
			TD3DAdapter* m_Adapter;
			DXGI_MODE_DESC m_Description;
			size_t m_ModeIndex;
			Device m_Devices[NUMSUPPORTEDDEVICES];
			size_t m_DisplayIndex;
			WCHAR m_DisplayName[DISPLAYNAMESIZE + 1];
		};

	public:
		TD3DAdapter() { }

		void EnumerateOutputs(class TRenderDX11* render, IDXGIAdapter* dxgiAdapter);

		void UpdateAdapterInfo()
		{
			m_VendorId = m_AdapterDesc.VendorId;
			m_DeviceId = m_AdapterDesc.DeviceId;
			m_SubSysId = m_AdapterDesc.SubSysId;
			m_Revision = m_AdapterDesc.Revision;
		}

		void SetAdapterIndex(size_t index)
		{
			m_AdapterIndex = index;
		}

		DXGI_ADAPTER_DESC* GetAdapterDesc()
		{
			return &m_AdapterDesc;
		}

		Mode* GetMode()
		{
			return &m_Mode;
		}

	private:
		DXGI_ADAPTER_DESC m_AdapterDesc; // 0x20
		void* m_Unk1;                    // 0x138
		void* m_Unk2;                    // 0x13C
		void* m_Unk3;                    // 0x140
		Mode m_Mode;                     // 0x144
		size_t m_AdapterIndex;           // 0x228
		UINT m_VendorId;                 // 0x634
		UINT m_DeviceId;                 // 0x638
		UINT m_SubSysId;                 // 0x63C
		UINT m_Revision;                 // 0x640
	};

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
		TRenderDX11();
		~TRenderDX11() { }

		TMSWindow* GetMSWindow() { return &m_Window; }
		
	public:
		virtual bool CreateDisplay(DisplayParams* params) override;
		virtual bool DestroyDisplay() override { return true; }
		virtual void Update(float deltaTime) override;
		virtual void BeginScene() override;
		virtual void EndScene() override;
		virtual void* GetCurrentDevice() override { return TNULL; }
		virtual DisplayParams* GetCurrentDisplayParams() override { return &m_DisplayParams; }
		virtual bool Create() override { return Create("de Blob"); }
		
		virtual bool RecreateDisplay(DisplayParams* pDisplayParams);
		virtual void ShowDeviceError();
		virtual void ShowDisplayError();

		bool Create(LPCSTR a_name);
		
		static int GetTextureRowPitch(DXGI_FORMAT format, int width);
		static int GetTextureDepthPitch(DXGI_FORMAT format, int width, int height);
		static const char* GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel);

		ID3D11ShaderResourceView* CreateTexture(UINT width, UINT height, DXGI_FORMAT format, void* srcData, uint8_t flags, D3D11_USAGE usage, uint32_t cpuAccessFlags, uint32_t sampleDescCount);
		
		ID3D11RenderTargetView* CreateRenderTargetView(ID3D11ShaderResourceView* pShaderResourceView);

	private:
		void BuildAdapterDatabase();

	public:
		static UINT s_QualityLevel;
		static bool s_bPresentTest;

	private:
		DXGI_SWAP_CHAIN_DESC m_SwapChainDesc;     // 0x61C
		ID3D11Texture2D* m_Texture2D1;            // 0x658
		ID3D11RenderTargetView* m_RTView1;        // 0x65C
		ID3D11DepthStencilView* m_StencilView;    // 0x660
		IDXGISwapChain* m_SwapChain;              // 0x664
		UINT m_DisplayWidth;                      // 0x668
		UINT m_DisplayHeight;                     // 0x66C
		ID3D11Texture2D* m_SRView1Texture;        // 0x670
		ID3D11RenderTargetView* m_RTView2;        // 0x674
		ID3D11ShaderResourceView* m_SRView1;      // 0x678
		ID3D11Texture2D* m_SRView2Texture;        // 0x67C
		ID3D11ShaderResourceView* m_SRView2;      // 0x680
		ID3D11ShaderResourceView* m_StencilTexSR; // 0x684
		bool m_IsWidescreen;                      // 0x688
		ID3D11DeviceContext* m_pDeviceContext;    // 0x68C
		ID3D11Device* m_pDevice;                  // 0x690
		HACCEL m_hAccel;                          // 0x694
		DisplayParams m_DisplayParams;            // 0x698
		TMSWindow m_Window;                       // 0x6B0
		FLOAT m_ClearColor[4];                    // 0x6EC
		size_t m_NumDrawnFrames;                  // 0x72C
	};
}