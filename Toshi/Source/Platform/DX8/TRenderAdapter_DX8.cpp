#include "ToshiPCH.h"
#include "TRenderAdapter_DX8.h"
#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TD3DAdapter::TD3DAdapter()
{
}

// $Barnyard: FUNCTION 006c7070
// $Barnyard: FUNCTION 006d6940
TD3DAdapter::~TD3DAdapter()
{
}

// $Barnyard: FUNCTION 006d66c0
TUINT32 TD3DAdapter::GetAdapterIndex() const
{
	return m_uiAdapterIndex;
}

// $Barnyard: FUNCTION 006d66d0
const TString8& TD3DAdapter::GetDriver() const
{
	return m_Driver;
}

// $Barnyard: FUNCTION 006d66e0
const TString8& TD3DAdapter::GetDriverDescription() const
{
	return m_Description;
}

// $Barnyard: FUNCTION 006d66f0
TUINT16 TD3DAdapter::GetProductID() const
{
	return m_DriverVersionHighPart >> 16;
}

// $Barnyard: FUNCTION 006d6700
TUINT16 TD3DAdapter::GetVersion() const
{
	return m_DriverVersionHighPart & 0xFFFF;
}

// $Barnyard: FUNCTION 006d6710
TUINT16 TD3DAdapter::GetSubVersion() const
{
	return m_DriverVersionLowPart >> 16;
}

// $Barnyard: FUNCTION 006d6720
TUINT16 TD3DAdapter::GetBuild() const
{
	return m_DriverVersionLowPart & 0xFFFF;
}

// $Barnyard: FUNCTION 006d6730
const GUID& TD3DAdapter::GetDeviceIdentifier() const
{
	return m_DeviceIdentifier;
}

// $Barnyard: FUNCTION 006d47b0
TUINT32 TD3DAdapter::GetNumSupportedDevices() const
{
	return Mode::NUMSUPPORTEDDEVICES;
}

// $Barnyard: FUNCTION 006d69a0
void TD3DAdapter::EnumerateOutputs( TRenderInterface* a_pRenderer )
{
	TRenderD3DInterface* pRenderer = TSTATICCAST( TRenderD3DInterface, a_pRenderer );

	TUINT32 uiAdapterIndex        = GetAdapterIndex();
	TUINT32 uiNumSupportedDevices = GetNumSupportedDevices();
	TUINT32 uiAdapterModeCount    = pRenderer->GetDirect3D()->GetAdapterModeCount( uiAdapterIndex );

	for ( TUINT32 i = 0; i < uiAdapterModeCount; i++ )
	{
		auto pMode = new Mode();

		pMode->SetOwnerAdapter( this );
		pMode->SetModeIndex( i );

		pRenderer->GetDirect3D()->EnumAdapterModes( uiAdapterIndex, i, &pMode->GetD3DDisplayMode() );

		for ( TUINT32 k = 0; k < uiNumSupportedDevices; k++ )
		{
			auto pDevice = TSTATICCAST( TD3DAdapter::Mode::Device, pMode->GetDevice( k ) );

			pDevice->SetOwnerMode( pMode );
			pDevice->SetDeviceIndex( k );

			pRenderer->GetDirect3D()->GetDeviceCaps( uiAdapterIndex, Mode::Device::DEVICETYPES[ k ], &pDevice->GetD3DCaps() );
			HRESULT hRes = pRenderer->GetDirect3D()->CheckDeviceType(
			    uiAdapterIndex,
			    Mode::Device::DEVICETYPES[ k ],
			    pMode->GetD3DDisplayMode().Format,
			    pMode->GetD3DDisplayMode().Format,
			    FALSE );

			if ( SUCCEEDED( hRes ) )
			{
				auto& caps = pDevice->GetD3DCaps();

				if ( Mode::Device::DEVICETYPES[ k ] == D3DDEVTYPE_REF )
				{
					pDevice->m_bIsSoftware = TTRUE;
				}

				if ( caps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
				{
					pDevice->m_bCanRenderWindowed = TTRUE;
				}

				if ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
				{
					pDevice->m_bSupportsTransformation = TTRUE;
					pDevice->m_bSupportsPureDevice     = caps.DevCaps & D3DDEVCAPS_PUREDEVICE;
				}

				if ( caps.DevCaps & D3DDEVCAPS_NPATCHES )
				{
					pDevice->m_bSupportsNPatches = TTRUE;
				}

				if ( pDevice->SupportsHardwareTransfomations() )
				{
					if ( pDevice->SupportsPureDevice() )
					{
						pDevice->m_eFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
					}
					else
					{
						pDevice->m_eFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
					}
				}
				else
				{
					pDevice->m_eFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				}

				for ( TINT j = 0; j < Mode::Device::NUMDEPTHSTENCILFORMATS; j++ )
				{
					if ( Mode::Device::DEPTHSTENCILFORMATS[ j ] != D3DFMT_UNKNOWN )
					{
						HRESULT hCheck = pRenderer->GetDirect3D()->CheckDeviceFormat(
						    uiAdapterIndex,
						    Mode::Device::DEVICETYPES[ k ],
						    pMode->GetD3DDisplayMode().Format,
						    D3DUSAGE_DEPTHSTENCIL,
						    D3DRTYPE_SURFACE,
						    Mode::Device::DEPTHSTENCILFORMATS[ j ] );

						if ( SUCCEEDED( hCheck ) )
						{
							HRESULT hMatch = pRenderer->GetDirect3D()->CheckDepthStencilMatch(
							    uiAdapterIndex,
							    Mode::Device::DEVICETYPES[ k ],
							    pMode->GetD3DDisplayMode().Format,
							    pMode->GetD3DDisplayMode().Format,
							    Mode::Device::DEPTHSTENCILFORMATS[ j ] );

							if ( SUCCEEDED( hMatch ) )
							{
								pDevice->m_aSupportedDSFormats[ j ] = TTRUE;
							}
						}
					}
				}
			}
		}

		GetModeList()->InsertTail( pMode );
	}
}

// $Barnyard: FUNCTION 004ffb50
TRenderAdapter* TD3DAdapter::Mode::GetAdapter() const
{
	return m_pOwnerAdapter;
}

// $Barnyard: FUNCTION 004ffe40
TSIZE TD3DAdapter::Mode::GetModeIndex() const
{
	return m_uiModeIndex;
}

// $Barnyard: FUNCTION 006d6760
TUINT32 TD3DAdapter::Mode::GetWidth() const
{
	return m_DisplayMode.Width;
}

// $Barnyard: FUNCTION 005f7a30
TUINT32 TD3DAdapter::Mode::GetHeight() const
{
	return m_DisplayMode.Height;
}

// $Barnyard: FUNCTION 006d6770
TBOOL TD3DAdapter::Mode::Is32Bit() const
{
	return m_DisplayMode.Format == D3DFMT_X8R8G8B8;
}

// $Barnyard: FUNCTION 006d6780
TBOOL TD3DAdapter::Mode::Is16Bit() const
{
	return m_DisplayMode.Format == D3DFMT_R5G6B5 ||
	    m_DisplayMode.Format == D3DFMT_X1R5G5B5 ||
	    m_DisplayMode.Format == D3DFMT_X4R4G4B4;
}

// $Barnyard: FUNCTION 006d67a0
TUINT32 TD3DAdapter::Mode::GetRefreshRate() const
{
	return m_DisplayMode.RefreshRate;
}

// $Barnyard: FUNCTION 006d67b0
TRenderAdapter::Mode::Device* TD3DAdapter::Mode::GetDevice( TUINT32 a_iDevice )
{
	if ( a_iDevice >= 0 && a_iDevice < NUMSUPPORTEDDEVICES )
	{
		return &m_aDevices[ a_iDevice ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006d67f0
D3DFORMAT TD3DAdapter::Mode::GetBackBufferFormat( TUINT32 a_uiColourDepth )
{
	if ( a_uiColourDepth == 16 )
	{
		return D3DFORMAT( ( -(TUINT32)( ( *(TUINT8*)&( m_DisplayMode ).Format & 0x17 ) != 0 ) & 0xfffffff9 ) + D3DFMT_X4R4G4B4 );
	}

	return D3DFORMAT( ( a_uiColourDepth != 32 ) - 1 & 0x16 );
}

// $Barnyard: FUNCTION 006d6820
TRenderAdapter::Mode* TD3DAdapter::Mode::Device::GetMode() const
{
	return m_pOwnerMode;
}

// $Barnyard: FUNCTION 006d6830
TUINT32 TD3DAdapter::Mode::Device::GetDeviceIndex() const
{
	return m_uiDeviceIndex;
}

// $Barnyard: FUNCTION 006d6840
const TCHAR* TD3DAdapter::Mode::Device::GetTypeString() const
{
	return DEVICETYPESSTRINGS[ m_uiDeviceIndex ];
}

// $Barnyard: FUNCTION 006d6850
TBOOL TD3DAdapter::Mode::Device::IsSoftware() const
{
	return m_bIsSoftware;
}

// $Barnyard: FUNCTION 006d6860
TBOOL TD3DAdapter::Mode::Device::CanRenderWindowed() const
{
	return m_bCanRenderWindowed;
}

// $Barnyard: FUNCTION 006d6870
TBOOL TD3DAdapter::Mode::Device::SupportsHardwareTransfomations() const
{
	return m_bSupportsTransformation;
}

// $Barnyard: FUNCTION 006d6890
TBOOL TD3DAdapter::Mode::Device::IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const
{
	return m_aSupportedDSFormats[ a_iIndex ];
}

// $Barnyard: FUNCTION 006d6880
TBOOL TD3DAdapter::Mode::Device::SupportsPureDevice() const
{
	return m_bSupportsPureDevice;
}

TOSHI_NAMESPACE_END
