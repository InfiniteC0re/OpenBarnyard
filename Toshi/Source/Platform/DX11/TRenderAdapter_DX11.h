#pragma once
#include "Toshi/TString8.h"
#include "Render/TRenderInterface.h"
#include "Render/TRenderAdapter.h"

#include <d3d11.h>
#include <dxgi.h>

TOSHI_NAMESPACE_START

class MODAPI TD3DAdapter : public TRenderAdapter
{
public:
	class Mode : public TRenderAdapter::Mode
	{
	public:
		static constexpr TUINT32 NUMSUPPORTEDDEVICES = 2;
		static constexpr TUINT32 DISPLAYNAMESIZE     = 32;

		class Device : public TRenderAdapter::Mode::Device
		{
		public:
			friend TD3DAdapter;
			friend Mode;

			static constexpr const TCHAR* DEVICETYPESSTRINGS[ NUMSUPPORTEDDEVICES ] = {
				"HAL",
				"REF"
			};

			// D16, D15S1, D24X8, D24S8, D32
			static constexpr DXGI_FORMAT DEPTHSTENCILFORMATS[] = {
				DXGI_FORMAT_D16_UNORM,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				DXGI_FORMAT_D32_FLOAT
			};

			static constexpr TUINT32 NUMDEPTHSTENCILFORMATS = TARRAYSIZE( DEPTHSTENCILFORMATS );

		public:
			Device();

		public:
			virtual TRenderAdapter::Mode* GetMode() const override;
			virtual TUINT32               GetDeviceIndex() const override;
			virtual const TCHAR*          GetTypeString() const override;
			virtual TBOOL                 IsSoftware() const override;
			virtual TBOOL                 CanRenderWindowed() const override;
			virtual TBOOL                 SupportsHardwareTransfomations() const override;
			virtual TBOOL                 IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const override;
			virtual TBOOL                 SupportsPureDevice() const;

			void SetOwnerMode( Mode* a_pMode ) { m_pOwnerMode = a_pMode; }
			void SetDeviceIndex( TUINT32 a_uiIndex ) { m_uiDeviceIndex = a_uiIndex; }

			TBOOL SupportsVertexShaders() const { return TTRUE; }
			TBOOL SupportsPixelShader14() const { return TTRUE; }
			TBOOL SupportsPixelShaders() const { return TTRUE; }

		private:
			Mode*   m_pOwnerMode;
			TUINT32 m_uiDeviceIndex;
			TBOOL   m_bIsSoftware;
			TBOOL   m_bCanRenderWindowed;
			TBOOL   m_bSupportsTransformation;
			TBOOL   m_bSupportsPureDevice;
			TBOOL   m_aSupportedDSFormats[ NUMDEPTHSTENCILFORMATS ];
		};

	public:
		virtual ~Mode() = default;

		virtual TRenderAdapter*               GetAdapter() const override;
		virtual size_t                        GetModeIndex() const override;
		virtual TUINT32                       GetWidth() const override;
		virtual TUINT32                       GetHeight() const override;
		virtual TBOOL                         Is32Bit() const override;
		virtual TBOOL                         Is16Bit() const override;
		virtual TUINT32                       GetRefreshRate() const override;
		virtual TRenderAdapter::Mode::Device* GetDevice( TUINT32 a_iDevice ) override;

		Mode();

		void SetOwnerAdapter( TD3DAdapter* a_pOwnerAdapter ) { m_pOwnerAdapter = a_pOwnerAdapter; }
		void SetModeIndex( TUINT32 a_uiModeIndex ) { m_uiModeIndex = a_uiModeIndex; }
		void SetDisplayIndex( TUINT32 a_uiDisplayIndex ) { m_uiDisplayIndex = a_uiDisplayIndex; }

		void                  SetDescription( const DXGI_MODE_DESC& a_rDescription ) { m_Description = a_rDescription; }
		DXGI_MODE_DESC&       GetDescription() { return m_Description; }
		const DXGI_MODE_DESC& GetDescription() const { return m_Description; }

		TUINT32      GetDisplayIndex() const { return m_uiDisplayIndex; }
		const WCHAR* GetDisplayName() const { return m_wszDisplayName; }
		void         SetDisplayName( const WCHAR* a_wszDisplayName );

		static void GetDisplayMode( IDXGIOutput* a_pOutput, DXGI_MODE_DESC* a_pModeDesc );

	private:
		TD3DAdapter*   m_pOwnerAdapter;
		DXGI_MODE_DESC m_Description;
		TUINT32        m_uiModeIndex;
		Device         m_aDevices[ NUMSUPPORTEDDEVICES ];
		TUINT32        m_uiDisplayIndex;
		WCHAR          m_wszDisplayName[ DISPLAYNAMESIZE ];
	};

public:
	TD3DAdapter();
	~TD3DAdapter();

	virtual TUINT32         GetAdapterIndex() const override;
	virtual const TString8& GetDriver() const override;
	virtual const TString8& GetDriverDescription() const override;
	virtual TUINT16         GetProductID() const override;
	virtual TUINT16         GetVersion() const override;
	virtual TUINT16         GetSubVersion() const override;
	virtual TUINT16         GetBuild() const override;
	virtual const void*     GetDeviceIdentifier() const override;
	virtual TUINT32         GetNumSupportedDevices() const override;

	void SetAdapterIndex( TUINT32 a_uiAdapterIndex ) { m_uiAdapterIndex = a_uiAdapterIndex; }
	void SetDriver( const TString8& a_rDriver ) { m_Driver = a_rDriver; }
	void SetDescription( const TString8& a_rDescription ) { m_Description = a_rDescription; }
	void SetDriverVersion( const LARGE_INTEGER& a_rDriverVersion ) { m_DriverVersion = a_rDriverVersion; }

	Mode&       GetMode() { return m_Mode; }
	const Mode& GetMode() const { return m_Mode; }

	DXGI_ADAPTER_DESC&       GetAdapterDesc() { return m_AdapterDesc; }
	const DXGI_ADAPTER_DESC& GetAdapterDesc() const { return m_AdapterDesc; }

	void EnumerateOutputs( TRenderInterface* a_pRenderer, IDXGIAdapter* a_pDXGIAdapter );

private:
	DXGI_ADAPTER_DESC m_AdapterDesc;
	Mode              m_Mode;
	TUINT32           m_uiAdapterIndex;
	TString8          m_Driver;
	TString8          m_Description;
	LARGE_INTEGER     m_DriverVersion;
};

TOSHI_NAMESPACE_END
