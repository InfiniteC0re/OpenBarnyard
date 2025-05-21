#pragma once
#include "Includes.h"
#include "Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDXInterface : public TInputInterface
{
public:
	TDECLARE_CLASS( TInputDXInterface, TInputInterface );

public:
	TInputDXInterface()
	{
		m_poDirectInput8 = NULL;
		m_hMainWindow    = NULL;
		m_bExclusive     = TTRUE;
	}

	// $Barnyard: FUNCTION 006d7430
	virtual ~TInputDXInterface() override
	{
		ReleaseAllDevices();

		if ( m_poDirectInput8 )
		{
			m_poDirectInput8->Release();
			m_poDirectInput8 = TNULL;
		}
	}

	virtual TBOOL Initialise() override;
	virtual TBOOL Deinitialise() override;
	virtual void  SetExclusiveMode( TBOOL a_bIsExclusive ) override;
	virtual void  RefreshDirect() {}

	TBOOL LostDevice();

	HWND GetMainWindow() const
	{
		return m_hMainWindow;
	}

	void SetMainWindow( HWND a_hMainWindow )
	{
		m_hMainWindow = a_hMainWindow;
	}

private:
	static BOOL CALLBACK EnumerateDeviceCallback( LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface );

	LPDIRECTINPUT8A GetDirectInput() const
	{
		return m_poDirectInput8;
	}

private:
	LPDIRECTINPUT8A m_poDirectInput8; // 0x30
	HWND            m_hMainWindow;    // 0x34
	TBOOL           m_bExclusive;     // 0x38
};

TOSHI_NAMESPACE_END
