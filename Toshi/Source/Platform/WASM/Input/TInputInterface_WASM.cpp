#include "ToshiPCH.h"
#include "TInputInterface_WASM.h"
#include "TInputDeviceKeyboard_WASM.h"
#include "TInputDeviceMouse_WASM.h"
#include "TInputDeviceController_WASM.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TInputWASMInterface );

TInputWASMInterface::TInputWASMInterface()
{
}

TInputWASMInterface::~TInputWASMInterface()
{
	ReleaseAllDevices();
}

TBOOL TInputWASMInterface::Initialise()
{
	AddDevice( new TInputWASMDeviceKeyboard() );
	AddDevice( new TInputWASMDeviceMouse() );
	AddDevice( new TInputWASMDeviceController() );

	AcquireAll();

	return TTRUE;
}

TBOOL TInputWASMInterface::Deinitialise()
{
	UnacquireAll();
	ReleaseAllDevices();

	return TTRUE;
}

TOSHI_NAMESPACE_END
