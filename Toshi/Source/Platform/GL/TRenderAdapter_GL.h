#pragma once
#include "Toshi/TString8.h"
#include "Render/TRenderInterface.h"
#include "Render/TRenderAdapter.h"

TOSHI_NAMESPACE_START

class MODAPI TGLAdapter : public TRenderAdapter
{
public:
	class Mode : public TRenderAdapter::Mode
	{
	public:
		static constexpr TUINT32 NUMSUPPORTEDDEVICES    = 2;
		static constexpr TUINT32 NUMDEPTHSTENCILFORMATS = 5;

		class Device : public TRenderAdapter::Mode::Device
		{
		public:
			friend TGLAdapter;
			friend Mode;

			static constexpr const TCHAR* DEVICETYPESSTRINGS[ NUMSUPPORTEDDEVICES ] = {
				"HAL",
				"REF"
			};

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

		void Set( TGLAdapter* a_pOwnerAdapter, TUINT32 a_uiModeIndex, TUINT32 a_uiWidth, TUINT32 a_uiHeight, TUINT32 a_uiRefreshRate );

	private:
		TGLAdapter* m_pOwnerAdapter;
		TUINT32     m_uiModeIndex;
		TUINT32     m_uiWidth;
		TUINT32     m_uiHeight;
		TUINT32     m_uiRefreshRate;
		Device      m_aDevices[ NUMSUPPORTEDDEVICES ];
	};

public:
	TGLAdapter();
	~TGLAdapter();

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

	Mode&       GetMode() { return m_Mode; }
	const Mode& GetMode() const { return m_Mode; }

	void EnumerateOutputs( TRenderInterface* a_pRenderer );

private:
	Mode     m_Mode;
	TUINT32  m_uiAdapterIndex;
	TString8 m_Driver;
	TString8 m_Description;
};

TOSHI_NAMESPACE_END
