#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"

void Toshi::TInputDXDeviceMouse::Release()
{
	TIMPLEMENT();
}

bool Toshi::TInputDXDeviceMouse::Initialise()
{
	TIMPLEMENT();
	return false;
}

bool Toshi::TInputDXDeviceMouse::Acquire()
{
	TIMPLEMENT();
	return false;
}

bool Toshi::TInputDXDeviceMouse::Unacquire()
{
	m_bIsAquired = false;
	TIMPLEMENT();
	return false;
}

bool const Toshi::TInputDXDeviceMouse::BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice)
{
	TASSERT(a_poDeviceInstance != NULL);
	TASSERT(a_poDXInputDevice != NULL);

	Release();

	TIMPLEMENT();
	return true;
}
