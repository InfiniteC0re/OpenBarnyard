#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"

void Toshi::TInputDXDeviceMouse::Release()
{
}

bool const Toshi::TInputDXDeviceMouse::BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice)
{
	TASSERT(a_poDeviceInstance != NULL);
	TASSERT(a_poDXInputDevice != NULL);

	Release();

	return true;
}
