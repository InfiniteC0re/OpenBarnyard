#pragma once
#include "Toshi/TString8.h"
#include "Render/TRenderInterface.h"
#include "Render/TRenderAdapter.h"

#include <d3d8.h>

TOSHI_NAMESPACE_START

class TD3DAdapter : public TRenderAdapter
{
public:
	class Mode : public TRenderAdapter::Mode
	{
	public:
		static constexpr TUINT32 NUMSUPPORTEDDEVICES = 2;

		class Device : public TRenderAdapter::Mode::Device
		{
		public:
			friend TD3DAdapter;
			friend Mode;

			static constexpr D3DDEVTYPE DEVICETYPES[ NUMSUPPORTEDDEVICES ] = {
				D3DDEVTYPE_HAL,
				D3DDEVTYPE_REF
			};

			static constexpr const TCHAR* DEVICETYPESSTRINGS[ NUMSUPPORTEDDEVICES ] = {
				"HAL",
				"REF"
			};

			static constexpr D3DFORMAT DEPTHSTENCILFORMATS[] = {
				D3DFMT_D16,
				D3DFMT_D15S1,
				D3DFMT_D24X8,
				D3DFMT_D24S8,
				D3DFMT_D32
			};

			static constexpr TUINT32 NUMDEPTHSTENCILFORMATS = TARRAYSIZE( DEPTHSTENCILFORMATS );

		public:
			virtual TRenderAdapter::Mode* GetMode() const override;
			virtual TUINT32				  GetDeviceIndex() const override;
			virtual const TCHAR*		  GetTypeString() const override;
			virtual TBOOL				  IsSoftware() const override;
			virtual TBOOL				  CanRenderWindowed() const override;
			virtual TBOOL				  SupportsHardwareTransfomations() const override;
			virtual TBOOL				  IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const override;
			virtual TBOOL				  SupportsPureDevice() const;

			void SetD3DCaps( const D3DCAPS8& a_rCaps ) { m_Caps = a_rCaps; }
			void SetOwnerMode( Mode* a_pMode ) { m_pOwnerMode = a_pMode; }
			void SetDeviceIndex( TUINT32 a_uiIndex ) { m_uiDeviceIndex = a_uiIndex; }

			TUINT32 GetD3DDeviceFlags() const { return m_eFlags; }

			D3DCAPS8&		GetD3DCaps() { return m_Caps; }
			const D3DCAPS8& GetD3DCaps() const { return m_Caps; }

		private:
			D3DCAPS8 m_Caps;
			TUINT32	 m_eFlags;
			Mode*	 m_pOwnerMode;
			TUINT32	 m_uiDeviceIndex;
			TBOOL	 m_bIsSoftware;
			TBOOL	 m_bCanRenderWindowed;
			TBOOL	 m_bSupportsTransformation;
			TBOOL	 m_bSupportsPureDevice;
			TBOOL	 m_bSupportsNPatches;
			TBOOL	 m_aSupportedDSFormats[ NUMDEPTHSTENCILFORMATS ];
		};

	public:
		virtual TRenderAdapter*				  GetAdapter() const override;
		virtual size_t						  GetModeIndex() const override;
		virtual TUINT32						  GetWidth() const override;
		virtual TUINT32						  GetHeight() const override;
		virtual TBOOL						  Is32Bit() const override;
		virtual TBOOL						  Is16Bit() const override;
		virtual TUINT32						  GetRefreshRate() const override;
		virtual TRenderAdapter::Mode::Device* GetDevice( TUINT32 a_iDevice ) override;

		D3DFORMAT GetBackBufferFormat( TUINT32 a_uiColourDepth );

		void SetOwnerAdapter( TD3DAdapter* a_pOwnerAdapter ) { m_pOwnerAdapter = a_pOwnerAdapter; }
		void SetModeIndex( TUINT32 a_uiModeIndex ) { m_uiModeIndex = a_uiModeIndex; }

		void			SetD3DDisplayMode( const D3DDISPLAYMODE& a_rDisplayMode ) { m_DisplayMode = a_rDisplayMode; }
		D3DDISPLAYMODE& GetD3DDisplayMode() { return m_DisplayMode; }

	private:
		TD3DAdapter*   m_pOwnerAdapter;
		D3DDISPLAYMODE m_DisplayMode;
		TUINT32		   m_uiModeIndex;
		Device		   m_aDevices[ NUMSUPPORTEDDEVICES ];
	};

public:
	virtual TUINT32			GetAdapterIndex() const override;
	virtual const TString8& GetDriver() const override;
	virtual const TString8& GetDriverDescription() const override;
	virtual TUINT16			GetProductID() const override;
	virtual TUINT16			GetVersion() const override;
	virtual TUINT16			GetSubVersion() const override;
	virtual TUINT16			GetBuild() const override;
	virtual const GUID&		GetDeviceIdentifier() const override;
	virtual TUINT32			GetNumSupportedDevices() const override;

	void SetAdapterIndex( TUINT32 a_uiAdapterIndex ) { m_uiAdapterIndex = a_uiAdapterIndex; }
	void SetDriver( const TString8& a_rDriver ) { m_Driver = a_rDriver; }
	void SetDescription( const TString8& a_rDescription ) { m_Description = a_rDescription; }
	void SetDriverVersionLowPart( DWORD a_uiDriverVersionLowPart ) { m_DriverVersionLowPart = a_uiDriverVersionLowPart; }
	void SetDriverVersionHighPart( DWORD a_uiDriverVersionHighPart ) { m_DriverVersionHighPart = a_uiDriverVersionHighPart; }
	void SetVendorId( DWORD a_uiVendorId ) { m_VendorId = a_uiVendorId; }
	void SetDeviceId( DWORD a_uiDeviceId ) { m_DeviceId = a_uiDeviceId; }
	void SetSubSysId( DWORD a_uiSubSysId ) { m_SubSysId = a_uiSubSysId; }
	void SetRevision( DWORD a_uiRevision ) { m_Revision = a_uiRevision; }
	void SetDeviceIdentifier( const GUID& a_rDeviceIdentifier ) { m_DeviceIdentifier = a_rDeviceIdentifier; }

	Mode&		GetMode() { return m_Mode; }
	const Mode& GetMode() const { return m_Mode; }

	D3DADAPTER_IDENTIFIER8*		  GetD3DIdentifier8() { return &m_Identifier; }
	const D3DADAPTER_IDENTIFIER8* GetD3DIdentifier8() const { return &m_Identifier; }

	void EnumerateOutputs( TRenderInterface* a_pRenderer );

private:
	D3DADAPTER_IDENTIFIER8 m_Identifier;
	Mode				   m_Mode;
	TUINT32				   m_uiAdapterIndex;
	TString8			   m_Driver;
	TString8			   m_Description;
	DWORD				   m_DriverVersionLowPart;
	DWORD				   m_DriverVersionHighPart;
	DWORD				   m_VendorId;
	DWORD				   m_DeviceId;
	DWORD				   m_SubSysId;
	DWORD				   m_Revision;
	GUID				   m_DeviceIdentifier;
};

TOSHI_NAMESPACE_END
