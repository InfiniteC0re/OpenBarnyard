#include "ToshiPCH.h"
#include "TRenderAdapter_GL.h"


//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TGLAdapter::TGLAdapter()
{
	m_uiAdapterIndex = 0;
}

TGLAdapter::~TGLAdapter()
{
}

TUINT32 TGLAdapter::GetAdapterIndex() const
{
	return m_uiAdapterIndex;
}

const TString8& TGLAdapter::GetDriver() const
{
	return m_Driver;
}

const TString8& TGLAdapter::GetDriverDescription() const
{
	return m_Description;
}

TUINT16 TGLAdapter::GetProductID() const
{
	return 0;
}

TUINT16 TGLAdapter::GetVersion() const
{
	return 0;
}

TUINT16 TGLAdapter::GetSubVersion() const
{
	return 0;
}

TUINT16 TGLAdapter::GetBuild() const
{
	return 0;
}

const void* TGLAdapter::GetDeviceIdentifier() const
{
	return &m_uiAdapterIndex;
}

TUINT32 TGLAdapter::GetNumSupportedDevices() const
{
	return Mode::NUMSUPPORTEDDEVICES;
}

void TGLAdapter::EnumerateOutputs( TRenderInterface* a_pRenderer )
{
	GetModeList()->DeleteAll();

	TUINT32 uiNumModes = 0;

	auto fnAddMode = [ & ]( TUINT32 a_uiWidth, TUINT32 a_uiHeight, TUINT32 a_uiRefreshRate ) {
		Mode* pMode = new Mode();
		pMode->Set( this, uiNumModes++, a_uiWidth, a_uiHeight, a_uiRefreshRate );
		GetModeList()->InsertTail( pMode );
	};

#ifdef TOSHI_SKU_WINDOWS
	DEVMODEA oDesktopMode = {};
	oDesktopMode.dmSize   = sizeof( oDesktopMode );
	EnumDisplaySettingsA( NULL, ENUM_CURRENT_SETTINGS, &oDesktopMode );
	m_Mode.Set( this, 0, oDesktopMode.dmPelsWidth, oDesktopMode.dmPelsHeight, oDesktopMode.dmDisplayFrequency );

	DEVMODEA oMode = {};
	oMode.dmSize   = sizeof( oMode );

	for ( DWORD i = 0; EnumDisplaySettingsA( NULL, i, &oMode ); i++ )
	{
		if ( oMode.dmBitsPerPel == 32 )
			fnAddMode( oMode.dmPelsWidth, oMode.dmPelsHeight, oMode.dmDisplayFrequency );
	}
#else
	// The canvas can have any size, the page tells which one fits
	TINT iWidth  = 0;
	TINT iHeight = 0;
	TWindow_WASM::GetPageSize( iWidth, iHeight );
	m_Mode.Set( this, 0, iWidth, iHeight, 60 );

	fnAddMode( iWidth, iHeight, 60 );
	fnAddMode( 800, 600, 60 );
	fnAddMode( 640, 480, 60 );
#endif
}

TGLAdapter::Mode::Mode()
{
	m_pOwnerAdapter = TNULL;
	m_uiModeIndex   = 0;
	m_uiWidth       = 0;
	m_uiHeight      = 0;
	m_uiRefreshRate = 0;
}

void TGLAdapter::Mode::Set( TGLAdapter* a_pOwnerAdapter, TUINT32 a_uiModeIndex, TUINT32 a_uiWidth, TUINT32 a_uiHeight, TUINT32 a_uiRefreshRate )
{
	m_pOwnerAdapter = a_pOwnerAdapter;
	m_uiModeIndex   = a_uiModeIndex;
	m_uiWidth       = a_uiWidth;
	m_uiHeight      = a_uiHeight;
	m_uiRefreshRate = a_uiRefreshRate;

	for ( TUINT32 i = 0; i < NUMSUPPORTEDDEVICES; i++ )
	{
		m_aDevices[ i ].SetOwnerMode( this );
		m_aDevices[ i ].SetDeviceIndex( i );
		m_aDevices[ i ].m_bIsSoftware = i == 1;
	}
}

TRenderAdapter* TGLAdapter::Mode::GetAdapter() const
{
	return m_pOwnerAdapter;
}

TSIZE TGLAdapter::Mode::GetModeIndex() const
{
	return m_uiModeIndex;
}

TUINT32 TGLAdapter::Mode::GetWidth() const
{
	return m_uiWidth;
}

TUINT32 TGLAdapter::Mode::GetHeight() const
{
	return m_uiHeight;
}

TBOOL TGLAdapter::Mode::Is32Bit() const
{
	return TTRUE;
}

TBOOL TGLAdapter::Mode::Is16Bit() const
{
	return TFALSE;
}

TUINT32 TGLAdapter::Mode::GetRefreshRate() const
{
	return m_uiRefreshRate;
}

TRenderAdapter::Mode::Device* TGLAdapter::Mode::GetDevice( TUINT32 a_iDevice )
{
	if ( a_iDevice < NUMSUPPORTEDDEVICES )
	{
		return &m_aDevices[ a_iDevice ];
	}

	return TNULL;
}

TGLAdapter::Mode::Device::Device()
{
	m_pOwnerMode    = TNULL;
	m_uiDeviceIndex = 0;
	m_bIsSoftware   = TFALSE;
}

TRenderAdapter::Mode* TGLAdapter::Mode::Device::GetMode() const
{
	return m_pOwnerMode;
}

TUINT32 TGLAdapter::Mode::Device::GetDeviceIndex() const
{
	return m_uiDeviceIndex;
}

const TCHAR* TGLAdapter::Mode::Device::GetTypeString() const
{
	return DEVICETYPESSTRINGS[ m_uiDeviceIndex ];
}

TBOOL TGLAdapter::Mode::Device::IsSoftware() const
{
	return m_bIsSoftware;
}

TBOOL TGLAdapter::Mode::Device::CanRenderWindowed() const
{
	return !m_bIsSoftware;
}

TBOOL TGLAdapter::Mode::Device::SupportsHardwareTransfomations() const
{
	return !m_bIsSoftware;
}

TBOOL TGLAdapter::Mode::Device::IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const
{
	return !m_bIsSoftware && a_iIndex < NUMDEPTHSTENCILFORMATS;
}

TBOOL TGLAdapter::Mode::Device::SupportsPureDevice() const
{
	return !m_bIsSoftware;
}

TOSHI_NAMESPACE_END
