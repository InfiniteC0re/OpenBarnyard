#include "ToshiPCH.h"
#include "TInputDeviceController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TInputDeviceController );

// $Barnyard: FUNCTION 006c48d0
TInputDeviceController::TInputDeviceController()
{
	m_CurrentState = new TBOOL[ NUM_BUTTONS ];
	m_OldState     = new TBOOL[ NUM_BUTTONS ];
	TUtil::MemClear( m_CurrentState, sizeof( *m_CurrentState ) * NUM_BUTTONS );
	TUtil::MemClear( m_OldState, sizeof( *m_OldState ) * NUM_BUTTONS );
}

// $Barnyard: FUNCTION 006c4ea0
TInputDeviceController::~TInputDeviceController()
{
}

// $Barnyard: FUNCTION 006c4990
TBOOL TInputDeviceController::GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const
{
	if ( a_iDoodad < 0x10000 || 0x1001b < a_iDoodad )
	{
		return TFALSE;
	}

	if ( a_iDoodad != 0x10019 && a_iDoodad != 0x1001a && a_iDoodad != 0x1001b )
	{
		a_rProperties.m_iUnk  = 0;
		a_rProperties.m_bFlag = TFALSE;
		return TTRUE;
	}

	a_rProperties.m_iUnk  = 2;
	a_rProperties.m_bFlag = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006c4d30
const TCHAR* TInputDeviceController::GetButtonFromDoodad( Doodad a_iDoodad ) const
{
	// TODO: Create constants for doodads

	switch ( a_iDoodad )
	{
		case 0x10001:
			return "BUTTON_DPAD_UP";
		case 0x10002:
			return "BUTTON_DPAD_RIGHT";
		case 0x10003:
			return "BUTTON_DPAD_DOWN";
		case 0x10004:
			return "BUTTON_DPAD_LEFT";
		case 0x10005:
			return "BUTTON_SELECT";
		case 0x10006:
			return "BUTTON_START";
		case 0x10007:
			return "BUTTON_STICK_LEFT";
		case 0x10008:
			return "BUTTON_STICK_RIGHT";
		case 0x10009:
			return "BUTTON_CONTEXT_1";
		case 0x1000a:
			return "BUTTON_CONTEXT_2";
		case 0x1000b:
			return "BUTTON_CONTEXT_3";
		case 0x1000c:
			return "BUTTON_CONTEXT_4";
		case 0x1000d:
			return "BUTTON_SHOULDER_LEFT_1";
		case 0x1000e:
			return "BUTTON_SHOULDER_LEFT_2";
		case 0x1000f:
			return "BUTTON_SHOULDER_RIGHT_1";
		case 0x10010:
			return "BUTTON_SHOULDER_RIGHT_2";
		case 0x10011:
			return "VIRTBTN_STICK_LEFT_UP";
		case 0x10012:
			return "VIRTBTN_STICK_LEFT_RIGHT";
		case 0x10013:
			return "VIRTBTN_STICK_LEFT_DOWN";
		case 0x10014:
			return "VIRTBTN_STICK_LEFT_LEFT";
		case 0x10015:
			return "VIRTBTN_STICK_RIGHT_UP";
		case 0x10016:
			return "VIRTBTN_STICK_RIGHT_RIGHT";
		case 0x10017:
			return "VIRTBTN_STICK_RIGHT_DOWN";
		case 0x10018:
			return "VIRTBTN_STICK_RIGHT_LEFT";
		case 0x10019:
			return "STICK_LEFT";
		case 0x1001a:
			return "STICK_RIGHT";
		case 0x1001b:
			return "VIRTSTK_DPAD";
		default:
			return "UNKNOWN";
	}
}

// $Barnyard: FUNCTION 006c49f0
TBOOL TInputDeviceController::IsDown( Doodad a_iDoodad ) const
{
	switch ( a_iDoodad )
	{
		case 0x10011:
			return m_CurrentState[ 0 ];
		case 0x10012:
			return m_CurrentState[ 1 ];
		case 0x10013:
			return m_CurrentState[ 2 ];
		case 0x10014:
			return m_CurrentState[ 3 ];
		case 0x10015:
			return m_CurrentState[ 4 ];
		case 0x10016:
			return m_CurrentState[ 5 ];
		case 0x10017:
			return m_CurrentState[ 6 ];
		case 0x10018:
			return m_CurrentState[ 7 ];
		default:
			return TFALSE;
	}
}

void TInputDeviceController::SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue )
{
}

// $Barnyard: FUNCTION 006c4a80
TBOOL TInputDeviceController::WasDown( Doodad a_iDoodad ) const
{
	switch ( a_iDoodad )
	{
		case 0x10011:
			return m_OldState[ 0 ];
		case 0x10012:
			return m_OldState[ 1 ];
		case 0x10013:
			return m_OldState[ 2 ];
		case 0x10014:
			return m_OldState[ 3 ];
		case 0x10015:
			return m_OldState[ 4 ];
		case 0x10016:
			return m_OldState[ 5 ];
		case 0x10017:
			return m_OldState[ 6 ];
		case 0x10018:
			return m_OldState[ 7 ];
		default:
			return TFALSE;
	}
}

TBOOL TInputDeviceController::Unknown2()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 006c4ee0
TINT TInputDeviceController::ProcessVirtualButtons( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = 0;

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
	{
		if ( m_CurrentState[ i ] != m_OldState[ i ] )
		{
			a_rEmitter.Throw(
			    TInputInterface::InputEvent(
			        this,
			        0x10011 + i,
			        m_CurrentState[ i ] == TTRUE ?
			            TInputInterface::EVENT_TYPE_GONE_DOWN :
			            TInputInterface::EVENT_TYPE_GONE_UP ) );

			iNumProcessed += 1;
		}
	}

	a_rEmitter.Throw(
	    TInputInterface::InputEvent(
	        this,
	        0x1001B,
	        TInputInterface::EVENT_TYPE_VIRTUAL,
	        m_fMag1,
	        m_fMag2 ) );

	return iNumProcessed + 1;
}

TOSHI_NAMESPACE_END
