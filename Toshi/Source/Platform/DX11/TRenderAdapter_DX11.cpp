#include "ToshiPCH.h"
#include "TRenderAdapter_DX11.h"
#include "TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TD3DAdapter::TD3DAdapter()
{
	TUtil::MemClear( &m_AdapterDesc, sizeof( m_AdapterDesc ) );
	m_uiAdapterIndex         = 0;
	m_DriverVersion.QuadPart = 0;
}

TD3DAdapter::~TD3DAdapter()
{
}

TUINT32 TD3DAdapter::GetAdapterIndex() const
{
	return m_uiAdapterIndex;
}

const TString8& TD3DAdapter::GetDriver() const
{
	return m_Driver;
}

const TString8& TD3DAdapter::GetDriverDescription() const
{
	return m_Description;
}

TUINT16 TD3DAdapter::GetProductID() const
{
	return HIWORD( m_DriverVersion.HighPart );
}

TUINT16 TD3DAdapter::GetVersion() const
{
	return LOWORD( m_DriverVersion.HighPart );
}

TUINT16 TD3DAdapter::GetSubVersion() const
{
	return HIWORD( m_DriverVersion.LowPart );
}

TUINT16 TD3DAdapter::GetBuild() const
{
	return LOWORD( m_DriverVersion.LowPart );
}

const void* TD3DAdapter::GetDeviceIdentifier() const
{
	return &m_AdapterDesc.AdapterLuid;
}

TUINT32 TD3DAdapter::GetNumSupportedDevices() const
{
	return Mode::NUMSUPPORTEDDEVICES;
}

void TD3DAdapter::EnumerateOutputs( TRenderInterface* a_pRenderer, IDXGIAdapter* a_pDXGIAdapter )
{
	GetModeList()->DeleteAll();

	TBOOL        bFirstOutput   = TTRUE;
	IDXGIOutput* pOutput        = TNULL;
	TUINT32      uiDisplayIndex = 0;

	while ( a_pDXGIAdapter->EnumOutputs( uiDisplayIndex, &pOutput ) != DXGI_ERROR_NOT_FOUND )
	{
		DXGI_OUTPUT_DESC oOutputDesc;
		pOutput->GetDesc( &oOutputDesc );

		UINT uiNumModes = 0;
		pOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &uiNumModes, NULL );

		if ( uiNumModes != 0 )
		{
			if ( bFirstOutput )
			{
				DXGI_MODE_DESC oDesktopMode;
				Mode::GetDisplayMode( pOutput, &oDesktopMode );

				m_Mode.SetOwnerAdapter( this );
				m_Mode.SetDescription( oDesktopMode );
				m_Mode.SetDisplayName( oOutputDesc.DeviceName );
				bFirstOutput = TFALSE;
			}

			DXGI_MODE_DESC* pModes = new DXGI_MODE_DESC[ uiNumModes ];
			pOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &uiNumModes, pModes );

			for ( UINT i = 0; i < uiNumModes; i++ )
			{
				Mode* pMode = new Mode();
				pMode->SetOwnerAdapter( this );
				pMode->SetModeIndex( i );
				pMode->SetDisplayIndex( uiDisplayIndex );
				pMode->SetDescription( pModes[ i ] );
				pMode->SetDisplayName( oOutputDesc.DeviceName );

				for ( TUINT32 k = 0; k < Mode::NUMSUPPORTEDDEVICES; k++ )
				{
					auto pDevice = TSTATICCAST( Mode::Device, pMode->GetDevice( k ) );
					pDevice->SetOwnerMode( pMode );
					pDevice->SetDeviceIndex( k );

					if ( k == 1 )
					{
						pDevice->m_bIsSoftware = TTRUE;
						continue;
					}

					pDevice->m_bCanRenderWindowed      = TTRUE;
					pDevice->m_bSupportsTransformation = TTRUE;
					pDevice->m_bSupportsPureDevice     = TTRUE;

					for ( TUINT32 j = 0; j < Mode::Device::NUMDEPTHSTENCILFORMATS; j++ )
						pDevice->m_aSupportedDSFormats[ j ] = TTRUE;
				}

				GetModeList()->InsertTail( pMode );
			}

			delete[] pModes;
		}

		pOutput->Release();
		uiDisplayIndex++;
	}
}

TRenderAdapter* TD3DAdapter::Mode::GetAdapter() const
{
	return m_pOwnerAdapter;
}

TSIZE TD3DAdapter::Mode::GetModeIndex() const
{
	return m_uiModeIndex;
}

TUINT32 TD3DAdapter::Mode::GetWidth() const
{
	return m_Description.Width;
}

TUINT32 TD3DAdapter::Mode::GetHeight() const
{
	return m_Description.Height;
}

TBOOL TD3DAdapter::Mode::Is32Bit() const
{
	return m_Description.Format == DXGI_FORMAT_B8G8R8X8_UNORM || m_Description.Format == DXGI_FORMAT_R8G8B8A8_UNORM;
}

TBOOL TD3DAdapter::Mode::Is16Bit() const
{
	return m_Description.Format == DXGI_FORMAT_B5G6R5_UNORM || m_Description.Format == DXGI_FORMAT_B5G5R5A1_UNORM || m_Description.Format == DXGI_FORMAT_B4G4R4A4_UNORM;
}

TUINT32 TD3DAdapter::Mode::GetRefreshRate() const
{
	if ( m_Description.RefreshRate.Denominator == 0 )
		return 0;

	return TUINT32( TFLOAT( m_Description.RefreshRate.Numerator ) / TFLOAT( m_Description.RefreshRate.Denominator ) + 0.5f );
}

TRenderAdapter::Mode::Device* TD3DAdapter::Mode::GetDevice( TUINT32 a_iDevice )
{
	if ( a_iDevice >= 0 && a_iDevice < NUMSUPPORTEDDEVICES )
	{
		return &m_aDevices[ a_iDevice ];
	}

	return TNULL;
}

void TD3DAdapter::Mode::SetDisplayName( const WCHAR* a_wszDisplayName )
{
	wcsncpy_s( m_wszDisplayName, a_wszDisplayName, _TRUNCATE );
}

void TD3DAdapter::Mode::GetDisplayMode( IDXGIOutput* a_pOutput, DXGI_MODE_DESC* a_pModeDesc )
{
	DXGI_OUTPUT_DESC oOutputDesc;
	a_pOutput->GetDesc( &oOutputDesc );

	MONITORINFOEXA oMonitorInfo;
	oMonitorInfo.cbSize = sizeof( oMonitorInfo );
	GetMonitorInfoA( oOutputDesc.Monitor, &oMonitorInfo );

	DEVMODEA oDisplaySettings = {};
	oDisplaySettings.dmSize   = sizeof( oDisplaySettings );
	EnumDisplaySettingsA( oMonitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &oDisplaySettings );

	// 0 and 1 mean the hardware's default refresh rate
	const TBOOL bDefaultRefreshRate = oDisplaySettings.dmDisplayFrequency == 1 || oDisplaySettings.dmDisplayFrequency == 0;

	DXGI_MODE_DESC oMatchMode;
	oMatchMode.Width                   = oDisplaySettings.dmPelsWidth;
	oMatchMode.Height                  = oDisplaySettings.dmPelsHeight;
	oMatchMode.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	oMatchMode.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	oMatchMode.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	oMatchMode.RefreshRate.Numerator   = bDefaultRefreshRate ? 0 : oDisplaySettings.dmDisplayFrequency;
	oMatchMode.RefreshRate.Denominator = bDefaultRefreshRate ? 0 : 1;

	a_pOutput->FindClosestMatchingMode( &oMatchMode, a_pModeDesc, NULL );
}

TD3DAdapter::Mode::Mode()
{
	m_pOwnerAdapter  = TNULL;
	m_uiModeIndex    = 0;
	m_uiDisplayIndex = 0;
	TUtil::MemClear( &m_Description, sizeof( m_Description ) );
	m_wszDisplayName[ 0 ] = L'\0';
}

TD3DAdapter::Mode::Device::Device()
{
	m_pOwnerMode              = TNULL;
	m_uiDeviceIndex           = 0;
	m_bIsSoftware             = TFALSE;
	m_bCanRenderWindowed      = TFALSE;
	m_bSupportsTransformation = TFALSE;
	m_bSupportsPureDevice     = TFALSE;

	for ( TUINT32 i = 0; i < NUMDEPTHSTENCILFORMATS; i++ )
		m_aSupportedDSFormats[ i ] = TFALSE;
}

TRenderAdapter::Mode* TD3DAdapter::Mode::Device::GetMode() const
{
	return m_pOwnerMode;
}

TUINT32 TD3DAdapter::Mode::Device::GetDeviceIndex() const
{
	return m_uiDeviceIndex;
}

const TCHAR* TD3DAdapter::Mode::Device::GetTypeString() const
{
	return DEVICETYPESSTRINGS[ m_uiDeviceIndex ];
}

TBOOL TD3DAdapter::Mode::Device::IsSoftware() const
{
	return m_bIsSoftware;
}

TBOOL TD3DAdapter::Mode::Device::CanRenderWindowed() const
{
	return m_bCanRenderWindowed;
}

TBOOL TD3DAdapter::Mode::Device::SupportsHardwareTransfomations() const
{
	return m_bSupportsTransformation;
}

TBOOL TD3DAdapter::Mode::Device::IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const
{
	return m_aSupportedDSFormats[ a_iIndex ];
}

TBOOL TD3DAdapter::Mode::Device::SupportsPureDevice() const
{
	return m_bSupportsPureDevice;
}

TOSHI_NAMESPACE_END
