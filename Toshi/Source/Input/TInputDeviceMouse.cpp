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
// $Barnyard: FUNCTION 006c4210
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
			props.m_iNumAxes = 2;
			props.m_bFlag    = TFALSE;
			break;
		case AXIS_WHEEL:
			props.m_iNumAxes = 1;
			props.m_bFlag    = TFALSE;
			break;
		default:
			props.m_iNumAxes = 0;
			props.m_bFlag    = TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c4160
const TCHAR* TInputDeviceMouse::GetButtonFromDoodad( Doodad a_iDoodad ) const
{
	switch ( a_iDoodad )
	{
		case BUTTON_1:
			return "BUTTON_1";
		case BUTTON_2:
			return "BUTTON_2";
		case BUTTON_3:
			return "BUTTON_3";
		case BUTTON_4:
			return "BUTTON_4";
		case BUTTON_5:
			return "BUTTON_5";
		case BUTTON_6:
			return "BUTTON_6";
		case BUTTON_7:
			return "BUTTON_7";
		case BUTTON_8:
			return "BUTTON_8";
		case BUTTON_WHEEL_FORWARD:
			return "BUTTON_WHEEL_FORWARD";
		case BUTTON_WHEEL_BACKWARD:
			return "BUTTON_WHEEL_BACKWARD";
		case AXIS_CURSOR:
			return "AXIS_CURSOR";
		case AXIS_WHEEL:
			return "AXIS_WHEEL";
		default:
			return "UNKNOWN";
	}
}

TOSHI_NAMESPACE_END
