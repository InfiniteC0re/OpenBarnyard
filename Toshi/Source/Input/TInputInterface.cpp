#include "ToshiPCH.h"
#include "TInputInterface.h"
#include "TInputDeviceKeyboard.h"
#include "TInputDeviceMouse.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TInputInterface );

// $Barnyard: FUNCTION 006c3fd0
TInputDevice* TInputInterface::GetDeviceByIndex( TClass* a_pClass, TUINT a_uiIndex )
{
	if ( m_DeviceList.Count() == 0 )
	{
		return TNULL;
	}

	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		if ( it->GetClass()->IsA( a_pClass ) )
		{
			if ( a_uiIndex == 0 )
			{
				return it;
			}
			else
			{
				a_uiIndex -= 1;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006c3f80
void TInputInterface::AddDevice( TInputDevice* device )
{
	if ( device != TNULL )
	{
		m_DeviceList.InsertTail( device );
		device->SetInputInterface( this );
	}
}

// $Barnyard: FUNCTION 006c3d50
void TInputInterface::RemoveDevice( TInputDevice* device )
{
	if ( device != TNULL )
	{
		m_DeviceList.Remove( device );
		device->SetInputInterface( TNULL );
	}
}

// $Barnyard: FUNCTION 006c3e10
TBOOL TInputInterface::AcquireAll()
{
	TBOOL bResult = TTRUE;

	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		bResult &= it->Acquire();
	}

	return bResult;
}

// $Barnyard: FUNCTION 006c3e50
TBOOL TInputInterface::UnacquireAll()
{
	TBOOL bResult = TTRUE;

	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		bResult &= it->Unacquire();
	}

	return bResult;
}

// $Barnyard: FUNCTION 006c3e90
TBOOL TInputInterface::FlushAll()
{
	TBOOL bResult = TTRUE;

	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		bResult &= it->Flush();
	}

	return bResult;
}

// $Barnyard: FUNCTION 006c3cb0
void TInputInterface::SetExclusiveMode( TBOOL a_bIsExclusive )
{
	m_bIsExclusiveMode = a_bIsExclusive;
}

// $Barnyard: FUNCTION 006c3cc0
TBOOL TInputInterface::GetExclusiveMode() const
{
	return m_bIsExclusiveMode;
}

// $Barnyard: FUNCTION 006c3ed0
TINT TInputInterface::ProcessEvents( TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = 0;

	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		if ( it->IsAcquired() )
		{
			it->Update( a_fDeltaTime );
		}
	}

	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		if ( it->IsAcquired() )
		{
			iNumProcessed += it->ProcessEvents( m_InputEventEmitter, a_fDeltaTime );
		}
	}

	return iNumProcessed;
}

// $Barnyard: FUNCTION 006c3f50
void TInputInterface::StopAllRepeats()
{
	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		it->StopAllRepeats();
	}
}

// $Barnyard: FUNCTION 006c3d90
void TInputInterface::ReleaseAllDevices()
{
	for ( auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it )
	{
		it->Unacquire();
		it->Release();
		it->Remove();
		delete it.Get();
	}
}

// $Barnyard: FUNCTION 006c4030
TInputInterface::TInputInterface()
    : m_InputEventEmitter( this )
{
	m_bIsExclusiveMode = TFALSE;
}

// $Barnyard: FUNCTION 006c4080
TInputInterface::~TInputInterface()
{
}

// $Barnyard: FUNCTION 006c3ba0
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType )
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 0;
	m_wszString[ 0 ]    = L'\0';
}

TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TWCHAR* a_wszString )
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 0;
	TStringManager::String16Copy( m_wszString, a_wszString, 3 );
	m_Magnitude.Ints[ 0 ] = 0;
}

TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1 )
{
	m_pSource             = a_pDevice;
	m_iDoodad             = a_iDoodad;
	m_eEventType          = a_eEventType;
	m_Magnitude.Ints[ 0 ] = a_iMagnitude1;
	m_bIsMagnitudeFloat   = TFALSE;
	m_iAxisCount          = 1;
	m_wszString[ 0 ]      = L'\0';
}

// $Barnyard: FUNCTION 006c3c50
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2 )
{
	m_pSource             = a_pDevice;
	m_iDoodad             = a_iDoodad;
	m_eEventType          = a_eEventType;
	m_Magnitude.Ints[ 0 ] = a_iMagnitude1;
	m_Magnitude.Ints[ 1 ] = a_iMagnitude2;
	m_bIsMagnitudeFloat   = TFALSE;
	m_iAxisCount          = 2;
	m_wszString[ 0 ]      = L'\0';
}

TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1 )
{
	m_pSource               = a_pDevice;
	m_iDoodad               = a_iDoodad;
	m_eEventType            = a_eEventType;
	m_Magnitude.Floats[ 0 ] = a_fMagnitude1;
	m_bIsMagnitudeFloat     = TTRUE;
	m_iAxisCount            = 1;
	m_wszString[ 0 ]        = L'\0';
}

// $Barnyard: FUNCTION 006c3c10
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2 )
{
	m_pSource               = a_pDevice;
	m_iDoodad               = a_iDoodad;
	m_eEventType            = a_eEventType;
	m_Magnitude.Floats[ 0 ] = a_fMagnitude1;
	m_Magnitude.Floats[ 1 ] = a_fMagnitude2;
	m_bIsMagnitudeFloat     = TTRUE;
	m_iAxisCount            = 2;
	m_wszString[ 0 ]        = L'\0';
}

TINT TInputInterface::InputEvent::GetMagnitudeInt( TINT a_iAxis ) const
{
	TASSERT( a_iAxis >= 0 && a_iAxis < GetAxisCount() );

	if ( IsMagnitudeInt() )
	{
		return m_Magnitude.Ints[ a_iAxis ];
	}
	else
	{
		if ( m_Magnitude.Floats[ a_iAxis ] < -0.5f )
		{
			return -1;
		}

		return 0.5f < m_Magnitude.Floats[ a_iAxis ];
	}
}

// $Barnyard: FUNCTION 006c3c90
TFLOAT TInputInterface::InputEvent::GetMagnitudeFloat( TINT a_iAxis ) const
{
	TASSERT( a_iAxis >= 0 && a_iAxis < GetAxisCount() );

	if ( IsMagnitudeFloat() )
	{
		return m_Magnitude.Floats[ a_iAxis ];
	}

	return (TFLOAT)m_Magnitude.Ints[ a_iAxis ];
}

// $Barnyard: FUNCTION 006c3cd0
TBOOL TInputInterface::InputEvent::StartRepeat( TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime ) const
{
	return m_pSource->StartRepeat( m_iDoodad, a_fFirstRepeatTime, a_fRepeatTime );
}

TOSHI_NAMESPACE_END
