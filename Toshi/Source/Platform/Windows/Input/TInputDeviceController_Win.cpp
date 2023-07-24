#include "ToshiPCH.h"
#include "TInputDeviceController_Win.h"

namespace Toshi {

	TBOOL TInputDXDeviceController::IsDirectInputController(LPCDIDEVICEINSTANCEA a_poDeviceInstance)
	{
		TASSERT(a_poDeviceInstance != NULL);
		char fmtStr[37];

		T2String8::Format(fmtStr, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
			a_poDeviceInstance->guidProduct.Data1, a_poDeviceInstance->guidProduct.Data2,
			a_poDeviceInstance->guidProduct.Data3, a_poDeviceInstance->guidProduct.Data4[0],
			a_poDeviceInstance->guidProduct.Data4[1], a_poDeviceInstance->guidProduct.Data4[2],
			a_poDeviceInstance->guidProduct.Data4[3], a_poDeviceInstance->guidProduct.Data4[4],
			a_poDeviceInstance->guidProduct.Data4[5], a_poDeviceInstance->guidProduct.Data4[6],
			a_poDeviceInstance->guidProduct.Data4[7]);

		if (TStringManager::String8FindString(fmtStr, (char*)"09cc054c-0000-0000-0000-504944564944") == TNULL
			&& TStringManager::String8FindString(fmtStr, (char*)"05C4054C-0000-0000-0000-504944564944") == TNULL
			&& TStringManager::String8FindString(fmtStr, (char*)"00050925-0000-0000-0000-504944564944") == TNULL
			&& TStringManager::String8FindString(fmtStr, (char*)"0268054C-0000-0000-0000-504944564944") == TNULL
			&& TStringManager::String8FindString(fmtStr, (char*)"03088888-0000-0000-0000-504944564944") == TNULL)
		{
			return TFALSE;
		}

		return TTRUE;
	}

	TBOOL const TInputDXDeviceController::BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice)
	{
		TASSERT(a_poDeviceInstance != NULL);
		TASSERT(a_poDXInputDevice != NULL);
		m_bIsXInput = IsDirectInputController(a_poDeviceInstance);
		Initialise();

		m_pDXInputDevice = a_poDXInputDevice;
		m_pDXDiDevCaps = DIDEVCAPS();
		m_pDXInputDevice->GetCapabilities(&m_pDXDiDevCaps);
		HRESULT hr = m_pDXInputDevice->SetDataFormat(&c_dfDIJoystick);

		if (hr != DI_OK)
		{
			return TFALSE;
		}

		m_pDXInputDevice->EnumObjects(EnumerateObjectCallback, this, DIDFT_BUTTON | DIDFT_AXIS);
		TTODO("field_0x38C = 0");
		return TTRUE;
	}

}
