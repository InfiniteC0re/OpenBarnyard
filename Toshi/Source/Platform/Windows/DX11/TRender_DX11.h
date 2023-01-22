#pragma once
#include "Toshi/Render/TRender.h"
#include "Toshi/Render/TRenderAdapter.h"
#include "Toshi/Render/Shaders/TToneMap.h"
#include "Toshi/Render/Shaders/TFXAA.h"
#include "Platform/Windows/TMSWindow.h"
#include "Includes.h"

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
		friend class ARenderer;

		static constexpr size_t HEAPSIZE = 0x10000;
		static constexpr size_t VERTEX_CONSTANT_BUFFER_SIZE = 0x1000;
		static constexpr size_t PIXEL_CONSTANT_BUFFER_SIZE = 0x400;
		static constexpr size_t NUMBUFFERS = 16;
		static constexpr size_t VERTEX_BUFFER_SIZE = 0x100000;
		static constexpr size_t INDEX_BUFFER_SIZE = 0x10000;

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
		
		enum Flags
		{
			Flags_Unk = 1,
			Flags_Unk2,
			Flags_Unk3,
			Flags_Unk4,
			Flags_Unk5,
			Flags_Unk6,
			Flags_Unk7,
			Flags_Unk8,
		};

		static inline float s_vertexData[] =
		{
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 
			0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, 0.0f, 0.0f, 0.0f, 0.0f
		};


		static inline const char* s_defaultVertexShader = " float4 ScaleTranslate : register(c0);               float4 uvST : register(c1);                                                             struct VS_IN                                                                                {                                                 float4 ObjPos   : POSITION;                   float2 UV\t\t  : TEXCOORD0;\t\t\t    };                                                                                          struct VS_OUT                                 {                                                 float4 ProjPos  : SV_POSITION;                   float2 UV\t\t  : TEXCOORD0;\t\t\t    };                                                                                          VS_OUT main( VS_IN In )                       {                                                 VS_OUT Out;                                   float4 scaledvert = In.ObjPos.xyzw * ScaleTranslate.xyzw;         scaledvert = scaledvert.xyzw + ScaleTranslate.zwzw;\t\t\t\t        scaledvert.zw = float2(0.0,1.0);\t\t\t\t\t\t\t\t     Out.ProjPos = scaledvert;\t\t\t\t        Out.UV = (In.UV*uvST.xy) + uvST.zw;           return Out;                               }                                            ";
		static inline const char* s_defaultPixelShader = " struct PS_IN                                  {                                            \t\tfloat4 Position\t\t\t: SV_POSITION; \t\tfloat2 Tex0             : TEXCOORD0;    };                                                                                          sampler2D   diffuse_texture     : register(s0) = sampler_state { MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; };   float4 main( PS_IN In )  : COLOR\t\t\t    {                                                return tex2D( diffuse_texture, In.Tex0 );        }                                            ";
		static inline const char* s_defaultPixelShader2 = " struct PS_IN                                  {                                            \t\tfloat4 Position\t\t\t: SV_POSITION; \t\tfloat2 Tex0             : TEXCOORD0;    };                                                                                          sampler2D   diffuse_texture     : register(s0) = sampler_state { MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; };   float4 main( PS_IN In )  : COLOR\t\t\t    {                                                return float4(tex2D( diffuse_texture, In.Tex0 ).xyz, 1.0);        }                                            ";

	public:
		TRenderDX11();
		~TRenderDX11() = default;

		TMSWindow* GetMSWindow()
		{
			return &m_Window;
		}

		DXGI_SWAP_CHAIN_DESC* GetSwapChainDesc()
		{
			return &m_SwapChainDesc;
		}

		ID3D11DeviceContext* GetDeviceContext() const
		{
			return m_pDeviceContext;
		}

		static TRenderDX11* Interface()
		{
			return static_cast<TRenderDX11*>(TRender::GetSingletonWeak());
		}
		
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

		static int GetTextureRowPitch(DXGI_FORMAT format, int width);
		static int GetTextureDepthPitch(DXGI_FORMAT format, int width, int height);
		static const char* GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel);
		static ID3DBlob* CompileShader(const char* srcData, LPCSTR pEntrypoint, LPCSTR pTarget, const D3D_SHADER_MACRO* pDefines);

		void CreateVSPS();
		bool Create(LPCSTR a_name);
		void CopyToVertexConstantBuffer(int index, const void* src, int count);
		void CopyToPixelConstantBuffer(int index, const void* src, int count);
		HRESULT CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11PixelShader** ppPixelShader);
		HRESULT CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11VertexShader** ppVertexShader);
		ID3D11ShaderResourceView* CreateTexture(UINT width, UINT height, DXGI_FORMAT format, void* srcData, uint8_t flags, D3D11_USAGE usage, uint32_t cpuAccessFlags, uint32_t sampleDescCount);
		ID3D11RenderTargetView* CreateRenderTargetView(ID3D11ShaderResourceView* pShaderResourceView);
		ID3D11SamplerState* CreateSamplerStateAutoAnisotropy(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, FLOAT mipLODBias, uint32_t borderColor, FLOAT minLOD, FLOAT maxLOD);
		ID3D11SamplerState* CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, FLOAT mipLODBias, uint32_t borderColor, FLOAT minLOD, FLOAT maxLOD, UINT maxAnisotropy);
		void SomeFlagsShinanigans(uint8_t flag1, uint32_t flag2, uint32_t flag3, uint32_t flag4);
		void FUN_006a8d30();
		void UnsetConstantBuffers();
	private:


		void BuildAdapterDatabase();

		void CreateSamplerStates();

	public:
		static UINT s_QualityLevel;
		static bool s_bPresentTest;
		static TMemoryHeap* s_pMemHeap;
		static ID3D11ShaderResourceView* s_pShaderResourceView;

	public:
		DXGI_SWAP_CHAIN_DESC m_SwapChainDesc;      // 0x61C
		ID3D11Texture2D* m_Texture2D1;             // 0x658
		ID3D11RenderTargetView* m_RTView1;         // 0x65C
		ID3D11DepthStencilView* m_StencilView;     // 0x660
		IDXGISwapChain* m_SwapChain;               // 0x664
		UINT m_DisplayWidth;                       // 0x668
		UINT m_DisplayHeight;                      // 0x66C
		ID3D11Texture2D* m_SRView1Texture;         // 0x670
		ID3D11RenderTargetView* m_RTView2;         // 0x674
		ID3D11ShaderResourceView* m_SRView1;       // 0x678
		ID3D11Texture2D* m_SRView2Texture;         // 0x67C
		ID3D11ShaderResourceView* m_SRView2;       // 0x680
		ID3D11ShaderResourceView* m_StencilTexSR;  // 0x684
		bool m_IsWidescreen;                       // 0x688
		ID3D11DeviceContext* m_pDeviceContext;     // 0x68C
		ID3D11Device* m_pDevice;                   // 0x690
		HACCEL m_hAccel;                           // 0x694
		DisplayParams m_DisplayParams;             // 0x698
		TMSWindow m_Window;                        // 0x6B0
		ID3D11VertexShader* m_pVertexShader;       // 0x6D8
		ID3D11PixelShader* m_pPixelShader1;        // 0x6DC
		ID3D11PixelShader* m_pPixelShader2;        // 0x6E0
		ID3D11InputLayout* m_pInputLayout;         // 0x6E4
		ID3D11Buffer* m_pSomeBuffer;               // 0x6E8
		FLOAT m_ClearColor[4];                     // 0x6EC
		TToneMap* m_pToneMap;                      // 0x71C
		TFXAA* m_pFXAA;                            // 0x724
		size_t m_NumDrawnFrames;                   // 0x72C
		ID3D11SamplerState* m_SamplerState1;       // 0x73C
		ID3D11SamplerState* m_SamplerState2;       // 0x740
		ID3D11SamplerState* m_SamplerState3;       // 0x744
		ID3D11SamplerState* m_SamplerState4;       // 0x748
		ID3D11SamplerState* m_SamplerState5;       // 0x74C
		ID3D11SamplerState* m_SamplerState6;       // 0x750
		ID3D11SamplerState* m_SamplerState7;       // 0x754
		ID3D11SamplerState* m_SamplerState8;       // 0x758
		ID3D11SamplerState* m_SamplerState9;       // 0x75C
		ID3D11SamplerState* m_SamplerState10;      // 0x760
		ID3D11SamplerState* m_SamplerState11;      // 0x764
		ID3D11SamplerState* m_SamplerState12;      // 0x768
		void* m_pVertexConstantBuffer;             // 0x76C
		bool m_IsVertexConstantBufferSet;          // 0x770
		ID3D11Buffer* m_VertexBuffers[NUMBUFFERS]; // 0x774
		size_t m_Unk1;                             // 0x7B4
		void* m_pPixelConstantBuffer;              // 0x7B8
		bool m_IsPixelConstantBufferSet;           // 0x7BC
		ID3D11Buffer* m_PixelBuffers[NUMBUFFERS];  // 0x7C0
		size_t m_Unk2;                             // 0x800
		ID3D11Buffer* m_MainVertexBuffer;          // 0x804
		size_t m_Unk3;                             // 0x808
		ID3D11Buffer* m_MainIndexBuffer;           // 0x80C
		size_t m_Unk4;                             // 0x810
		uint16_t m_Flags;                          // 0x820
		uint16_t m_Flags2;                         // 0x858
		uint16_t m_someFlags2;                     // 0x88C
		uint16_t m_someFlags1;                     // 0x88E
	};
}