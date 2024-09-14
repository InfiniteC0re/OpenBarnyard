#include "ToshiPCH.h"
#include "TInputDeviceMouse.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TInputDeviceMouse );

// $Barnyard: FUNCTION 006c40d0
TInputDeviceMouse::TInputDeviceMouse()
{
}

// $Barnyard: FUNCTION 006c40f0
TInputDeviceMouse::~TInputDeviceMouse()
{
}

// $Barnyard: FUNCTION 006c4100
TBOOL TInputDeviceMouse::GetDoodadProperties( Doodad a_iDoodad, TInputDevice::DoodadProperties& props ) const
{
	if ( a_iDoodad < 0x30000 || AXIS_WHEEL < a_iDoodad )
	{
		return TFALSE;
	}

	switch ( a_iDoodad )
	{
		case AXIS_CURSOR:
			props.m_iUnk  = 2;
			props.m_bFlag = TFALSE;
			break;
		case AXIS_WHEEL:
			props.m_iUnk  = 1;
			props.m_bFlag = TFALSE;
			break;
		default:
			props.m_iUnk  = 0;
			props.m_bFlag = TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c4160
const TCHAR* TInputDeviceMouse::GetButtonFromDoodad( Doodad a_iDoodad ) const
{
	switch ( a_iDoodad )
	{
		case 0x30001:
			return "BUTTON_1";
		case 0x30002:
			return "BUTTON_2";
		case 0x30003:
			return "BUTTON_3";
		case 0x30004:
			return "BUTTON_4";
		case 0x30005:
			return "BUTTON_5";
		case 0x30006:
			return "BUTTON_6";
		case 0x30007:
			return "BUTTON_7";
		case 0x30008:
			return "BUTTON_8";
		case 0x30009:
			return "BUTTON_WHEEL_FORWARD";
		case 0x3000A:
			return "BUTTON_WHEEL_BACKWARD";
		case 0x3000B:
			return "AXIS_CURSOR";
		case 0x3000C:
			return "AXIS_WHEEL";
		default:
			return "UNKNOWN";
	}
}

TOSHI_NAMESPACE_END
