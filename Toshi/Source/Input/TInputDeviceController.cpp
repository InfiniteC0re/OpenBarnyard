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

// $Barnyard: FUNCTION 006c4960
// $Barnyard: FUNCTION 006c4ea0
TInputDeviceController::~TInputDeviceController()
{
	delete[] m_CurrentState;
	delete[] m_OldState;
}

// $Barnyard: FUNCTION 006c4b10
void TInputDeviceController::UpdateVirtualButtons()
{
	TBOOL* pOldState = m_OldState;
	m_OldState       = m_CurrentState;
	m_CurrentState   = pOldState;

	TFLOAT afX[ 2 ];
	TFLOAT afY[ 2 ];
	afX[ 0 ] = GetAxisFloat( STICK_LEFT, 0 );
	afY[ 0 ] = GetAxisFloat( STICK_LEFT, 1 );
	afX[ 1 ] = GetAxisFloat( STICK_RIGHT, 0 );
	afY[ 1 ] = GetAxisFloat( STICK_RIGHT, 1 );

	// Hysteresis: a direction goes down past 0.75 and up again below 0.5
	for ( TINT i = 0; i < 2; i++ )
	{
		TBOOL* pOld = m_OldState + i * 4;
		TBOOL* pNew = m_CurrentState + i * 4;

		if ( pOld[ 3 ] == TFALSE && afX[ i ] < -0.75f )
			pNew[ 3 ] = TTRUE;
		else if ( pOld[ 3 ] == TTRUE && afX[ i ] > -0.5f )
			pNew[ 3 ] = TFALSE;
		else
			pNew[ 3 ] = pOld[ 3 ];

		if ( pOld[ 1 ] == TFALSE && afX[ i ] > 0.75f )
			pNew[ 1 ] = TTRUE;
		else if ( pOld[ 1 ] == TTRUE && afX[ i ] < 0.5f )
			pNew[ 1 ] = TFALSE;
		else
			pNew[ 1 ] = pOld[ 1 ];

		if ( pOld[ 0 ] == TFALSE && afY[ i ] < -0.75f )
			pNew[ 0 ] = TTRUE;
		else if ( pOld[ 0 ] == TTRUE && afY[ i ] > -0.5f )
			pNew[ 0 ] = TFALSE;
		else
			pNew[ 0 ] = pOld[ 0 ];

		if ( pOld[ 2 ] == TFALSE && afY[ i ] > 0.75f )
			pNew[ 2 ] = TTRUE;
		else if ( pOld[ 2 ] == TTRUE && afY[ i ] < 0.5f )
			pNew[ 2 ] = TFALSE;
		else
			pNew[ 2 ] = pOld[ 2 ];
	}

	m_fOldMag1 = m_fMag1;
	m_fOldMag2 = m_fMag2;

	// D-pad
	if ( IsDown( BUTTON_DPAD_LEFT ) )
		m_fMag1 = -1.0f;
	else if ( IsDown( BUTTON_DPAD_RIGHT ) )
		m_fMag1 = 1.0f;
	else
		m_fMag1 = 0.0f;

	if ( IsDown( BUTTON_DPAD_UP ) )
		m_fMag2 = -1.0f;
	else if ( IsDown( BUTTON_DPAD_DOWN ) )
		m_fMag2 = 1.0f;
	else
		m_fMag2 = 0.0f;
}

// $Barnyard: FUNCTION 006c4990
TBOOL TInputDeviceController::GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const
{
	if ( a_iDoodad < 0x10000 || VIRTSTK_DPAD < a_iDoodad )
	{
		return TFALSE;
	}

	if ( a_iDoodad != STICK_LEFT && a_iDoodad != STICK_RIGHT && a_iDoodad != VIRTSTK_DPAD )
	{
		a_rProperties.m_iNumAxes = 0;
		a_rProperties.m_bFlag    = TFALSE;
		return TTRUE;
	}

	a_rProperties.m_iNumAxes = 2;
	a_rProperties.m_bFlag    = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006c4d30
const TCHAR* TInputDeviceController::GetButtonFromDoodad( Doodad a_iDoodad ) const
{
	switch ( a_iDoodad )
	{
		case BUTTON_DPAD_UP:
			return "BUTTON_DPAD_UP";
		case BUTTON_DPAD_RIGHT:
			return "BUTTON_DPAD_RIGHT";
		case BUTTON_DPAD_DOWN:
			return "BUTTON_DPAD_DOWN";
		case BUTTON_DPAD_LEFT:
			return "BUTTON_DPAD_LEFT";
		case BUTTON_SELECT:
			return "BUTTON_SELECT";
		case BUTTON_START:
			return "BUTTON_START";
		case BUTTON_STICK_LEFT:
			return "BUTTON_STICK_LEFT";
		case BUTTON_STICK_RIGHT:
			return "BUTTON_STICK_RIGHT";
		case BUTTON_CONTEXT_1:
			return "BUTTON_CONTEXT_1";
		case BUTTON_CONTEXT_2:
			return "BUTTON_CONTEXT_2";
		case BUTTON_CONTEXT_3:
			return "BUTTON_CONTEXT_3";
		case BUTTON_CONTEXT_4:
			return "BUTTON_CONTEXT_4";
		case BUTTON_SHOULDER_LEFT_1:
			return "BUTTON_SHOULDER_LEFT_1";
		case BUTTON_SHOULDER_LEFT_2:
			return "BUTTON_SHOULDER_LEFT_2";
		case BUTTON_SHOULDER_RIGHT_1:
			return "BUTTON_SHOULDER_RIGHT_1";
		case BUTTON_SHOULDER_RIGHT_2:
			return "BUTTON_SHOULDER_RIGHT_2";
		case VIRTBTN_STICK_LEFT_UP:
			return "VIRTBTN_STICK_LEFT_UP";
		case VIRTBTN_STICK_LEFT_RIGHT:
			return "VIRTBTN_STICK_LEFT_RIGHT";
		case VIRTBTN_STICK_LEFT_DOWN:
			return "VIRTBTN_STICK_LEFT_DOWN";
		case VIRTBTN_STICK_LEFT_LEFT:
			return "VIRTBTN_STICK_LEFT_LEFT";
		case VIRTBTN_STICK_RIGHT_UP:
			return "VIRTBTN_STICK_RIGHT_UP";
		case VIRTBTN_STICK_RIGHT_RIGHT:
			return "VIRTBTN_STICK_RIGHT_RIGHT";
		case VIRTBTN_STICK_RIGHT_DOWN:
			return "VIRTBTN_STICK_RIGHT_DOWN";
		case VIRTBTN_STICK_RIGHT_LEFT:
			return "VIRTBTN_STICK_RIGHT_LEFT";
		case STICK_LEFT:
			return "STICK_LEFT";
		case STICK_RIGHT:
			return "STICK_RIGHT";
		case VIRTSTK_DPAD:
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
		case VIRTBTN_STICK_LEFT_UP:
			return m_CurrentState[ 0 ];
		case VIRTBTN_STICK_LEFT_RIGHT:
			return m_CurrentState[ 1 ];
		case VIRTBTN_STICK_LEFT_DOWN:
			return m_CurrentState[ 2 ];
		case VIRTBTN_STICK_LEFT_LEFT:
			return m_CurrentState[ 3 ];
		case VIRTBTN_STICK_RIGHT_UP:
			return m_CurrentState[ 4 ];
		case VIRTBTN_STICK_RIGHT_RIGHT:
			return m_CurrentState[ 5 ];
		case VIRTBTN_STICK_RIGHT_DOWN:
			return m_CurrentState[ 6 ];
		case VIRTBTN_STICK_RIGHT_LEFT:
			return m_CurrentState[ 7 ];
		default:
			return TFALSE;
	}
}

void TInputDeviceController::SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue, TBOOL a_bUnknown )
{
	// DOES_NOTHING
}

// $Barnyard: FUNCTION 006c4a80
TBOOL TInputDeviceController::WasDown( Doodad a_iDoodad ) const
{
	switch ( a_iDoodad )
	{
		case VIRTBTN_STICK_LEFT_UP:
			return m_OldState[ 0 ];
		case VIRTBTN_STICK_LEFT_RIGHT:
			return m_OldState[ 1 ];
		case VIRTBTN_STICK_LEFT_DOWN:
			return m_OldState[ 2 ];
		case VIRTBTN_STICK_LEFT_LEFT:
			return m_OldState[ 3 ];
		case VIRTBTN_STICK_RIGHT_UP:
			return m_OldState[ 4 ];
		case VIRTBTN_STICK_RIGHT_RIGHT:
			return m_OldState[ 5 ];
		case VIRTBTN_STICK_RIGHT_DOWN:
			return m_OldState[ 6 ];
		case VIRTBTN_STICK_RIGHT_LEFT:
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
			        VIRTBTN_STICK_LEFT_UP + i,
			        m_CurrentState[ i ] == TTRUE ?
			            TInputInterface::EVENT_TYPE_GONE_DOWN :
			            TInputInterface::EVENT_TYPE_GONE_UP
			    )
			);

			iNumProcessed += 1;
		}
	}

	a_rEmitter.Throw(
	    TInputInterface::InputEvent(
	        this,
	        VIRTSTK_DPAD,
	        TInputInterface::EVENT_TYPE_VIRTUAL,
	        m_fMag1,
	        m_fMag2
	    )
	);

	return iNumProcessed + 1;
}

TOSHI_NAMESPACE_END
