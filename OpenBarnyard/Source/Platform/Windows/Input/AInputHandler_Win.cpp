#include "pch.h"
#include "Input/AInputHandler.h"
#include "GameInterface/AGameStateController.h"
#include "Tasks/ARootTask.h"
#include "Units/AControlComponentManager.h"
#include "GUI/AGUI2.h"
#include "AOptions.h"

#include <Input/TInputInterface.h>

#ifdef TRENDERINTERFACE_DX8
#  include <Platform/DX8/Input/TInputDeviceController_Win.h>
#  include <Platform/DX8/Input/TInputDeviceController_XInput.h>
#  include <Platform/DX8/Input/TInputDeviceMouse_Win.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00595930, CreateObject 00595ce0, CreateObjectInPlace 00595d40
TDEFINE_CLASS( AInputHandler );

// $Barnyard: FUNCTION 00595ae0
AInputHandler::AInputHandler()
{
	m_pMouseDevice           = TNULL;
	m_pKeyboardDevice        = TNULL;
	m_pMainController        = TNULL;
	m_pLostMainController    = TNULL;
	m_bIsPaused              = TFALSE;
	m_bHadInputLastFrame     = TFALSE;
	m_bRecordLostControllers = TFALSE;
	m_iNumLostControllers    = 0;
	m_iUnkF0                 = 0;
	m_bUnkF4                 = TFALSE;
	m_bRumblePaused          = TFALSE;
	m_pPositionalRumbles     = TNULL;
	m_pRumbles               = TNULL;
	m_pRumbleController      = TNULL;
	m_vRumblePosition        = TVector4::VEC_ZERO;

	auto pInputInterfaceClass = TClass::Find( "TInputDXInterface" );
	auto pInputInterface      = pInputInterfaceClass->CreateObject();
	TASSERT( pInputInterface && pInputInterface->IsA( &TGetClass( TInputInterface ) ) );
}

// $Barnyard: FUNCTION 00595c40
// $Barnyard: FUNCTION 00595d80
AInputHandler::~AInputHandler()
{
	if ( TInputInterface::GetSingleton() )
	{
		TInputInterface::DestroySingleton();
	}
}

// $Barnyard: FUNCTION 00596620
TBOOL AInputHandler::OnCreate()
{
	TInputInterface* pInputInterface = TInputInterface::GetSingleton();
	pInputInterface->Initialise();

	m_pMouseDevice    = pInputInterface->GetDeviceByIndex<TInputDeviceMouse>( 0 );
	m_pKeyboardDevice = pInputInterface->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

#ifdef TRENDERINTERFACE_DX8
	if ( AOptions::GetSingleton()->IsControllerEnabled() )
	{
		// Remember the controllers
		for ( auto it = pInputInterface->GetDeviceList().Begin(); it != pInputInterface->GetDeviceList().End(); it++ )
		{
#  ifdef BARNYARD_COMMUNITY_PATCH
			if ( it->GetClass()->IsA( &TGetClass( TInputDXDeviceController ) ) || it->GetClass()->IsA( &TGetClass( TInputXInputDeviceController ) ) )
#  else
			if ( it->GetClass()->IsA( &TGetClass( TInputDXDeviceController ) ) )
#  endif
				m_Devices.PushBack( it );
		}
	}
#endif

	m_InputListener.Connect(
	    pInputInterface->GetInputEventEmitter(),
	    this,
	    []( AInputHandler* a_pInputHandler, TInputInterface* a_pInputInterface, TInputInterface::InputEvent* a_pEvent ) {
		    // $Barnyard: FUNCTION 005965b0
		    return a_pInputHandler->ProcessInputEvent( a_pInputInterface, a_pEvent );
	    },
	    0
	);

	m_DeviceStatusListener.Connect(
	    pInputInterface->GetDeviceStatusEmitter(),
	    this,
	    []( AInputHandler* a_pInputHandler, TInputInterface* a_pInputInterface, TInputInterface::DeviceStatusEvent* a_pEvent ) {
		    // $Barnyard: FUNCTION 00596590
		    return a_pInputHandler->OnDeviceStatusChanged( a_pInputInterface, a_pEvent );
	    },
	    0
	);

	m_PauseListener.Connect(
	    g_oSystemManager.GetPauseEmitter(),
	    this,
	    []( AInputHandler* a_pInputHandler, TSystemManager* a_pSystemManager, TBOOL* a_pPause ) {
		    return a_pInputHandler->OnPause( a_pSystemManager, a_pPause );
	    },
	    0
	);

	// Create the rumble pools
	m_pRumbles = new Rumble[ MAX_NUM_RUMBLES ];

	for ( TINT i = 0; i < MAX_NUM_RUMBLES; i++ )
		m_llFreeRumbles.InsertTail( &m_pRumbles[ i ] );

	m_pPositionalRumbles = new PositionalRumble[ MAX_NUM_RUMBLES ];

	for ( TINT i = 0; i < MAX_NUM_RUMBLES; i++ )
		m_llFreePositionalRumbles.InsertTail( &m_pPositionalRumbles[ i ] );

	m_bRumbleEnabled = AOptions::GetSingleton()->IsRumbleEnabled();
	return TTRUE;
}

// $Barnyard: FUNCTION 00596540
TBOOL AInputHandler::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_bHadInputLastFrame = TFALSE;
	m_fDeltaTime         = a_fDeltaTime;

	TInputInterface::GetSingleton()->AcquireAll();
	TInputInterface::GetSingleton()->ProcessEvents( a_fDeltaTime );

	if ( !m_bRumblePaused )
		UpdateRumbles( a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 005964b0
void AInputHandler::OnDestroy()
{
	m_InputListener.Disconnect();
	m_DeviceStatusListener.Disconnect();
	m_PauseListener.Disconnect();

	StopAllRumbles();
	m_llFreeRumbles.RemoveAll();
	m_llFreePositionalRumbles.RemoveAll();

	if ( m_pRumbles )
		delete[] m_pRumbles;

	if ( m_pPositionalRumbles )
		delete[] m_pPositionalRumbles;
}

// $Barnyard: FUNCTION 00595970
TBOOL AInputHandler::Rumble::Update( TFLOAT a_fDeltaTime )
{
	if ( fDuration != -1.0f && fTime > fDuration )
		return TTRUE;

	if ( eFade == FADE_LINEAR )
	{
		fStrength = ( 1.0f - fTime / fDuration ) * fMaxStrength;
	}
	else if ( eFade == FADE_QUADRATIC )
	{
		TFLOAT fProgress = 1.0f - fTime / fDuration;
		fStrength        = fProgress * fProgress * fMaxStrength;
	}

	if ( fStrength < fMinStrength )
		fStrength = fMinStrength;

	if ( fDuration != -1.0f )
		fTime += a_fDeltaTime;

	return TFALSE;
}

// $Barnyard: FUNCTION 00596010
void AInputHandler::UpdateRumbles( TFLOAT a_fDeltaTime )
{
	// NOTE: m_pRumbleController is never set on PC retail (SetRumble is a stub)
	if ( !m_bRumbleEnabled || !m_pRumbleController )
		return;

	if ( m_llRumbles.IsEmpty() && m_llPositionalRumbles.IsEmpty() )
		return;

	TFLOAT            fStrongest           = 0.0f;
	Rumble*           pStrongest           = TNULL;
	PositionalRumble* pStrongestPositional = TNULL;

	for ( auto it = m_llRumbles.Begin(); it != m_llRumbles.End(); )
	{
		Rumble* pRumble = it;
		it++;

		if ( pRumble->Update( a_fDeltaTime ) )
		{
			// The rumble is over
			pRumble->Remove();
			m_llFreeRumbles.InsertTail( pRumble );
		}
#ifdef BARNYARD_COMMUNITY_PATCH
		else if ( pRumble->fStrength > fStrongest )
#else
		else if ( pRumble->fStrength > 0.0f )
#endif
		{
			// NOTE: retail doesn't compare it with the strongest one, the last active rumble is picked
			fStrongest = pRumble->fStrength;
			pStrongest = pRumble;
		}
	}

	for ( auto it = m_llPositionalRumbles.Begin(); it != m_llPositionalRumbles.End(); )
	{
		PositionalRumble* pRumble = it;
		it++;

		if ( pRumble->fDuration != -1.0f && pRumble->fTime > pRumble->fDuration )
		{
			// The rumble is over
			pRumble->Remove();
			m_llFreePositionalRumbles.InsertTail( pRumble );
			continue;
		}

		// NOTE: same as Rumble::Update but without the minimum strength
		if ( pRumble->eFade == Rumble::FADE_LINEAR )
		{
			pRumble->fStrength = ( 1.0f - pRumble->fTime / pRumble->fDuration ) * pRumble->fMaxStrength;
		}
		else if ( pRumble->eFade == Rumble::FADE_QUADRATIC )
		{
			TFLOAT fProgress   = 1.0f - pRumble->fTime / pRumble->fDuration;
			pRumble->fStrength = fProgress * fProgress * pRumble->fMaxStrength;
		}

		if ( pRumble->fDuration != -1.0f )
			pRumble->fTime += a_fDeltaTime;

		// Attenuate by the distance to the rumble position
		TFLOAT fInnerRadiusSq = pRumble->fInnerRadius * pRumble->fInnerRadius;
		TFLOAT fOuterRadiusSq = pRumble->fOuterRadius * pRumble->fOuterRadius;
		TFLOAT fDistanceSq    = TVector4::DistanceSq( m_vRumblePosition, pRumble->vPosition );
		TFLOAT fAttenuation;

		if ( fDistanceSq >= fOuterRadiusSq )
		{
			fAttenuation = 0.0f;
		}
		else if ( fDistanceSq <= fInnerRadiusSq )
		{
			fAttenuation = 1.0f;
		}
		else
		{
			fAttenuation = 1.0f - ( fDistanceSq - fInnerRadiusSq ) / ( fOuterRadiusSq - fInnerRadiusSq );
			TMath::Clip( fAttenuation, 0.0f, 1.0f );
		}

		TFLOAT fStrength = fAttenuation * pRumble->fStrength;

		if ( fStrength > fStrongest && fStrength > 0.0f )
		{
			fStrongest           = fStrength;
			pStrongestPositional = pRumble;
		}
	}

	if ( pStrongestPositional )
	{
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, fStrongest, TFALSE );
	}
	else if ( pStrongest )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, fStrongest, TFALSE );
#else
		// NOTE: the strongest rumble isn't positional, retail stops the vibration instead of applying it
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, 0.0f, TFALSE );
#endif
	}
	else
	{
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, 0.0f, TFALSE );
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_0, 0.0f, TFALSE );
	}
}

// $Barnyard: FUNCTION 00595940
TBOOL AInputHandler::IsValidDevice( Toshi::TInputDevice* a_pDevice ) const
{
	return a_pDevice && ( a_pDevice == m_pKeyboardDevice || a_pDevice == m_pMouseDevice || a_pDevice == m_pMainController );
}

// $Barnyard: FUNCTION 00595ec0
TBOOL AInputHandler::ProcessInputEvent( Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::InputEvent* a_pEvent )
{
	if ( m_bIsPaused )
	{
		return TFALSE;
	}

	auto pInputSource = a_pEvent->GetSource();

	if ( pInputSource != m_pMouseDevice && pInputSource != m_pKeyboardDevice )
	{
		if ( m_Devices.Find( pInputSource ) == m_Devices.End() )
		{
			return TFALSE;
		}
	}

	if ( m_pMainController == TNULL )
	{
		if ( pInputSource->GetClass()->IsA( &TGetClass( TInputDeviceController ) ) )
		{
			if ( a_pEvent->GetEventType() == Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN &&
			     pInputSource == m_Devices[ 0 ] )
			{
				m_pMainController = TSTATICCAST( TInputDeviceController, pInputSource );
			}
		}
	}


#ifdef BARNYARD_COMMUNITY_PATCH
	// Update whether controller is used or not
	if ( !IsControllerUsed() && pInputSource->GetClass()->IsA( &TGetClass( TInputDeviceController ) ) )
	{
		TINT   iDoodad    = a_pEvent->GetDoodad();
		TFLOAT flMagTotal = 0.0f;

		if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_VIRTUAL )
			flMagTotal = TMath::Abs( a_pEvent->GetMagnitudeFloat( 0 ) ) + TMath::Abs( a_pEvent->GetMagnitudeFloat( 1 ) );

		if ( flMagTotal >= 0.1f || a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			SetControllerUsed( TTRUE );

			TTRACE( "Controller is active\n" );
		}
	}
	else if ( IsControllerUsed() && a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_VIRTUAL )
	{
		if ( pInputSource->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) || pInputSource->GetClass()->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
		{
			SetControllerUsed( TFALSE );
			StopAllRumbles();

			TTRACE( "Controller is not active anymore due to %s\n", pInputSource->GetClass()->GetName() );
		}
	}
#endif // BARNYARD_COMMUNITY_PATCH

	// NOTE: the original first calls 006ce000 (RETURNS_FALSE stub) with the event
	if ( AGameStateController::GetSingleton()->ProcessInput( a_pEvent ) ||
	     ( ARootTask::GetSingleton()->IsGameSystemCreated() == TTRUE && AControlComponentManager::GetSingleton()->ProcessInput( a_pEvent ) ) )
	{
		m_bHadInputLastFrame = TTRUE;
		return TTRUE;
	}

	if ( pInputSource->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) )
		m_bHadInputLastFrame = TTRUE;

	return TFALSE;
}

// $Barnyard: FUNCTION 00595da0
// $Barnyard: FUNCTION 00595e51
TBOOL AInputHandler::OnDeviceStatusChanged( Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::DeviceStatusEvent* a_pEvent )
{
	TInputDevice* pDevice = a_pEvent->pDevice;

	if ( !pDevice->GetClass()->IsA( &TGetClass( TInputDeviceController ) ) )
		return TFALSE;

	if ( a_pEvent->eStatus == TInputInterface::DEVICESTATUS_CONNECTED )
	{
#if defined( BARNYARD_COMMUNITY_PATCH ) && defined( TRENDERINTERFACE_DX8 )
		if ( pDevice->GetClass()->IsA( &TGetClass( TInputXInputDeviceController ) ) &&
		     pDevice == m_pLostMainController && m_pMainController == TNULL &&
		     m_Devices.Find( pDevice ) == m_Devices.End() )
		{
			m_Devices.PushBack( pDevice );
			m_pMainController     = TSTATICCAST( TInputDeviceController, pDevice );
			m_pLostMainController = TNULL;
			return TTRUE;
		}
#endif

		return TFALSE;
	}

	if ( a_pEvent->eStatus == TInputInterface::DEVICESTATUS_DISCONNECTED )
	{
		if ( !m_bRecordLostControllers && pDevice == m_pMainController )
		{
			m_pLostMainController = m_pMainController;
			m_pMainController     = TNULL;
		}

		auto it = m_Devices.Find( pDevice );

		if ( it == m_Devices.End() )
			return TFALSE;

		m_Devices.Erase( it );

		if ( m_bRecordLostControllers == TTRUE )
		{
			LostController& rLostController = m_aLostControllers[ m_iNumLostControllers++ ];
			rLostController.iUnk            = -1;
			rLostController.iIndex          = TSTATICCAST( TInputDeviceController, pDevice )->GetPortIndex();
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005965d0
TBOOL AInputHandler::OnPause( Toshi::TSystemManager* a_pSystemManager, TBOOL* a_pPause )
{
	m_bIsPaused = *a_pPause;

#ifdef TRENDERINTERFACE_DX8
	if ( !m_bIsPaused )
	{
		// Move the cursor to where the mouse is now
		TInputDXDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );

		if ( pMouse )
			AGUI2::GetMouseCursor().SetMousePos( TVector2( TFLOAT( pMouse->GetPositionX() ), TFLOAT( pMouse->GetPositionY() ) ) );
	}
#endif

	return TTRUE;
}

// $Barnyard: FUNCTION 00595fd0
void AInputHandler::StopAllRumbles()
{
	while ( !m_llPositionalRumbles.IsEmpty() )
	{
		PositionalRumble* pRumble = m_llPositionalRumbles.Tail();
		pRumble->Remove();
		m_llFreePositionalRumbles.InsertTail( pRumble );
	}
}

// $Barnyard: FUNCTION 00596390
void AInputHandler::SetRumblePaused( TBOOL a_bPaused )
{
	if ( a_bPaused == TTRUE )
	{
		// NOTE: m_Devices only holds controllers
		for ( TINT i = 0; i < m_Devices.Size(); i++ )
		{
			TInputDeviceController* pController = TSTATICCAST( TInputDeviceController, m_Devices[ i ] );
			pController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_0, 0.0f, TTRUE );
			pController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, 0.0f, TTRUE );
		}
	}

	m_bRumblePaused = a_bPaused;
}

// $Barnyard: FUNCTION 00596420
void AInputHandler::SetRumbleEnabled( TBOOL a_bEnabled )
{
	if ( m_bRumbleEnabled != a_bEnabled )
	{
		m_bRumbleEnabled = a_bEnabled;

		if ( !a_bEnabled )
		{
			for ( TINT i = 0; i < m_Devices.Size(); i++ )
			{
				TInputDeviceController* pController = TSTATICCAST( TInputDeviceController, m_Devices[ i ] );
				pController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_0, 0.0f, TFALSE );
				pController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, 0.0f, TFALSE );
			}
		}
	}
}

// $Barnyard: FUNCTION 00595a20
void AInputHandler::StopRumble( TInputDeviceController* a_pController )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: like the PS2 build, also drop the rumbles started by SetRumble and stop the motors
	if ( !a_pController )
		return;

	while ( !m_llRumbles.IsEmpty() )
	{
		Rumble* pRumble = m_llRumbles.Head();
		pRumble->Remove();
		m_llFreeRumbles.InsertTail( pRumble );
	}

	if ( m_pRumbleController )
	{
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_0, 0.0f, TFALSE );
		m_pRumbleController->SetVibration( TInputDeviceController::VIBRATION_ACTUATOR_1, 0.0f, TFALSE );
	}
#endif

	if ( a_pController )
		m_pRumbleController = TNULL;
}

#ifdef BARNYARD_COMMUNITY_PATCH

TINT AInputHandler::SetRumble( TInputDeviceController* a_pController, const RumbleParams* a_pParams )
{
	if ( !a_pController || !a_pController->IsForceFeedbackDevice() || !m_bRumbleEnabled )
		return 0;

	if ( m_llFreeRumbles.IsEmpty() || !IsControllerUsed() )
		return 0;

	Rumble* pRumble = m_llFreeRumbles.Head();
	pRumble->Remove();

	pRumble->fMaxStrength = a_pParams->afUnk[ 0 ];
	pRumble->fMinStrength = a_pParams->afUnk[ 1 ];
	pRumble->fStrength    = a_pParams->afUnk[ 2 ];
	pRumble->fDuration    = a_pParams->afUnk[ 3 ];
	pRumble->fTime        = a_pParams->afUnk[ 4 ];
	pRumble->fUnk1        = a_pParams->afUnk[ 5 ];
	pRumble->fUnk2        = a_pParams->afUnk[ 6 ];
	pRumble->eFade        = Rumble::FADE( a_pParams->iUnk );
	pRumble->pController  = a_pController;

	if ( m_pRumbleController && m_pRumbleController != a_pController )
	{
		// The rumbles of the previous controller are over
		StopRumble( m_pRumbleController );
	}

	m_llRumbles.InsertTail( pRumble );
	m_pRumbleController = a_pController;

	return TINT( pRumble );
}

void AInputHandler::RemoveRumble( TINT a_iRumble, TInputDeviceController* a_pController )
{
	if ( a_iRumble == 0 )
		return;

	for ( auto it = m_llRumbles.Begin(); it != m_llRumbles.End(); it++ )
	{
		Rumble* pRumble = it;

		if ( TINT( pRumble ) == a_iRumble && ( !a_pController || pRumble->pController == a_pController ) )
		{
			pRumble->Remove();
			m_llFreeRumbles.InsertTail( pRumble );
			return;
		}
	}
}

#endif // BARNYARD_COMMUNITY_PATCH
