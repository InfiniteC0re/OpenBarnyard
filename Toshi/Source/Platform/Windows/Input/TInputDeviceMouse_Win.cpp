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
	return false;
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
		TTODO("this + 100();");
	}

	TIMPLEMENT();
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
			TTODO("this + 100();");
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

		HRESULT hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

		if (hr != DI_OK)
		{
			return false;
		}

	}



	return 0;
}

bool const Toshi::TInputDXDeviceMouse::BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, bool exclusive)
{
	TASSERT(a_poDeviceInstance != NULL);
	TASSERT(a_poDXInputDevice != NULL);

	Release();

	TIMPLEMENT_D("Create Device?");

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
