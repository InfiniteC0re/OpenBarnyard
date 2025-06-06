#include "pch.h"
#include "Input/AInputHandler.h"
#include "GameInterface/AGameStateController.h"
#include "Tasks/ARootTask.h"

#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AInputHandler );

// $Barnyard: FUNCTION 00595ae0
AInputHandler::AInputHandler()
{
	TIMPLEMENT();

	auto pInputInterfaceClass = TClass::Find( "TInputDXInterface" );
	auto pInputInterface      = pInputInterfaceClass->CreateObject();
	TASSERT( pInputInterface && pInputInterface->IsA( &TGetClass( TInputInterface ) ) );

	m_pMouseDevice    = TNULL;
	m_pKeyboardDevice = TNULL;
	m_pMainController = TNULL;

	m_bIsPaused          = TFALSE;
	m_bHadInputLastFrame = TFALSE;
}

// $Barnyard: FUNCTION 00595c40
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
	TIMPLEMENT();
	TInputInterface::GetSingleton()->Initialise();
	m_pMouseDevice    = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );
	m_pKeyboardDevice = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	m_InputListener.Connect(
	    TInputInterface::GetSingleton()->GetInputEventEmitter(),
	    this,
	    []( AInputHandler* a_pInputHandler, TInputInterface* a_pInputInterface, TInputInterface::InputEvent* a_pEvent ) {
		    return a_pInputHandler->ProcessInputEvent( a_pInputInterface, a_pEvent );
	    },
	    0
	);

	return TTRUE;
}

// $Barnyard: FUNCTION 00596540
TBOOL AInputHandler::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();
	m_bHadInputLastFrame = TFALSE;
	m_fDeltaTime         = a_fDeltaTime;

	TInputInterface::GetSingleton()->AcquireAll();
	TInputInterface::GetSingleton()->ProcessEvents( a_fDeltaTime );
	return TTRUE;
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

	if ( !AGameStateController::GetSingleton()->ProcessInput( a_pEvent ) )
	{
		TFIXME( "Do some check with AControlComponentManager" );
		if ( !ARootTask::GetSingleton()->IsGameSystemCreated() && TTRUE )
		{
			if ( pInputSource->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				m_bHadInputLastFrame = TTRUE;
			}

			return TFALSE;
		}
	}

	m_bHadInputLastFrame = TTRUE;
	return TTRUE;
}
