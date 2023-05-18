#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"

void Toshi::TInputDXDeviceMouse::Release()
{
	if (m_poDXInputDevice)
	{
		m_poDXInputDevice->Release();
		m_poDXInputDevice = NULL;
	}
}

bool Toshi::TInputDXDeviceMouse::Initialise()
{
	TIMPLEMENT();
	GetCursorPos(&m_CursorPos);

	DIPROPDWORD dwordProperty{};
	dwordProperty.diph.dwSize = sizeof(dwordProperty);
	dwordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dwordProperty.diph.dwObj = 8;
	dwordProperty.diph.dwHow = DIPH_BYOFFSET;

	HRESULT hr = m_poDXInputDevice->GetProperty(DIPROP_GRANULARITY, &dwordProperty.diph);

	TTODO("Which DIERR is 0x80070020?");
	if (hr != 0x80070020)
	{
		m_bInitiliased = hr == DI_OK;
		return m_bInitiliased;
	}
	m_bInitiliased = false;
	return true;
}

bool Toshi::TInputDXDeviceMouse::Deinitialise()
{
	Release();
	return true;
}

bool Toshi::TInputDXDeviceMouse::Acquire()
{
	HRESULT hr = m_poDXInputDevice->Acquire();
	if (FAILED(hr))
	{
		m_bIsAquired = false;
		return false;
	}

	m_bIsAquired = true;

	if (hr != S_FALSE)
	{
		RefreshDirect();
	}

	return false;
}

bool Toshi::TInputDXDeviceMouse::Unacquire()
{
	m_bIsAquired = false;
	HRESULT hr = m_poDXInputDevice->Unacquire();
	return SUCCEEDED(hr);
}

bool Toshi::TInputDXDeviceMouse::Flush()
{
	DWORD dwItems = INFINITE;
	if (m_bIsAquired)
	{
		HRESULT hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwItems, 0);
		if (SUCCEEDED(hr))
		{
			RefreshDirect();
			return true;
		}
	}
	return false;
}

int Toshi::TInputDXDeviceMouse::ProcessEvents(TGenericEmitter& emitter, float flt)
{
	DIDEVICEOBJECTDATA dod[32];
	DWORD dwItems = 32;

	TUtil::MemClear(dod, sizeof(dod));

	HRESULT hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

	TTODO("Which DIERR is 0x8007001E?");
	if (FAILED(hr) && hr == 0x8007001E)
	{
		Unacquire();
		Acquire();

		hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

		if (hr != DI_OK)
		{
			return false;
		}

	}



	return 0;
}

void Toshi::TInputDXDeviceMouse::RefreshDirect()
{
	if (m_bIsAquired)
	{
		HRESULT hr = m_poDXInputDevice->Poll();
		if (SUCCEEDED(hr))
		{
			DIMOUSESTATE mouseState;
			hr = m_poDXInputDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
			if (SUCCEEDED(hr))
			{
				TTODO("Everything here");
			}
		}
	}
}

bool const Toshi::TInputDXDeviceMouse::BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, bool exclusive)
{
	TASSERT(a_poDeviceInstance != NULL);
	TASSERT(a_poDXInputDevice != NULL);

	Release();

	TIMPLEMENT_D("Weird for loop");

	m_poDXInputDevice = a_poDXInputDevice;
	m_DIDevCaps.dwSize = sizeof(DIDEVCAPS);

	m_poDXInputDevice->GetCapabilities(&m_DIDevCaps);
	HRESULT hr = m_poDXInputDevice->SetDataFormat(&c_dfDIMouse);

	if (hr != DI_OK)
	{
		return false;
	}

	if (a_mainWindow)
	{
		m_poDXInputDevice->SetCooperativeLevel(a_mainWindow, DISCL_FOREGROUND | (exclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE));
	}

	DIPROPDWORD dipdw{};
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 1;

	m_poDXInputDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	return true;
}

int Toshi::TInputDXDeviceMouse::GetAxisInt(int doodad, Coord coord) const
{
	if (doodad == 0x3000b)
	{
		if (coord == X)
		{
			return m_aAxis.m_iX;
		}
		if (coord == Y)
		{
			return m_aAxis.m_iY;
		}
	}
	return 0;
}

float Toshi::TInputDXDeviceMouse::GetAxisFloat(int doodad, Coord coord) const
{
	if (doodad == 0x3000b)
	{
		if (coord == X)
		{
			return m_aAxis.m_fX;
		}
		if (coord == Y)
		{
			return m_aAxis.m_fY;
		}
	}
	else if (doodad == 0x3000c)
	{
		return m_fWheelAxis;
	}
	return 0;
}
