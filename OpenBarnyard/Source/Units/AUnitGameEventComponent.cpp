#include "pch.h"
#include "AUnitGameEventComponent.h"
#include "AGeneralUnitComponentManager.h"
#include "AInteractionEvent.h"
#include "AInteractionEventManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c78c0, CreateObject 005c7f60, CreateObjectInPlace 005c7fc0
TDEFINE_CLASS( AUnitGameEventComponent, 2, 0 );

// $Barnyard: FUNCTION 005c7a10
AUnitGameEventComponent::AUnitGameEventComponent()
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		m_apHeardEvents[ i ] = TNULL;
		m_auiHeardData[ i ]  = 0;
	}

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		m_apWitnessedEvents[ i ] = TNULL;
		m_auiWitnessedData[ i ]  = 0;
	}
}

// $Barnyard: FUNCTION 005c78d0
// $Barnyard: FUNCTION 005c7a60
AUnitGameEventComponent::~AUnitGameEventComponent()
{
}

// NOTE: only checks the first heard event and the first witnessed event
// $Barnyard: FUNCTION 005c7c70
TBOOL AUnitGameEventComponent::HasEventWithAllTopics( TUINT a_uiTopics )
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] )
		{
			if ( ( a_uiTopics & m_apHeardEvents[ i ]->GetTopicFlags( GetOwner() ) ) == a_uiTopics )
				return TTRUE;

			break;
		}
	}

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] )
			return ( a_uiTopics & m_apWitnessedEvents[ i ]->GetTopicFlags( GetOwner() ) ) == a_uiTopics;
	}

	return TFALSE;
}

// NOTE: only checks the first heard event and the first witnessed event
// $Barnyard: FUNCTION 005c7cf0
TBOOL AUnitGameEventComponent::HasEventWithAnyTopic( TUINT a_uiTopics )
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] )
		{
			if ( ( m_apHeardEvents[ i ]->GetTopicFlags( GetOwner() ) & a_uiTopics ) != 0 )
				return TTRUE;

			break;
		}
	}

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] )
			return ( m_apWitnessedEvents[ i ]->GetTopicFlags( GetOwner() ) & a_uiTopics ) != 0;
	}

	return TFALSE;
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* AUnitGameEventComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005c7ae0
TBOOL AUnitGameEventComponent::AddHeardEvent( AInteractionEvent* a_pEvent, TUINT8 a_uiUnk )
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] == a_pEvent )
			return TFALSE;
	}

	return AddEvent( a_pEvent, m_apHeardEvents, m_auiHeardData, MAX_NUM_EVENTS, a_uiUnk );
}

// $Barnyard: FUNCTION 005c7b20
TBOOL AUnitGameEventComponent::AddWitnessedEvent( AInteractionEvent* a_pEvent, TUINT8 a_uiUnk )
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] == a_pEvent )
			return TFALSE;
	}

	return AddEvent( a_pEvent, m_apWitnessedEvents, m_auiWitnessedData, MAX_NUM_EVENTS, a_uiUnk );
}

// $Barnyard: FUNCTION 005c7ba0
TINT8 AUnitGameEventComponent::GetEventKnowledge( AInteractionEvent* a_pEvent ) const
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] == a_pEvent )
		{
			if ( TINT8( m_auiHeardData[ i ] ) >= 0 )
				return TINT8( m_auiHeardData[ i ] );

			break;
		}
	}

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] == a_pEvent )
			return ( TINT8( m_auiWitnessedData[ i ] ) < 0 ) ? 0 : TINT8( m_auiWitnessedData[ i ] );
	}

	return 0;
}

// $Barnyard: FUNCTION 005c7b60
TBOOL AUnitGameEventComponent::KnowsEvent( AInteractionEvent* a_pEvent ) const
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] == a_pEvent )
			return TTRUE;
	}

	return HasWitnessedEvent( a_pEvent );
}

// $Barnyard: FUNCTION 005c7c00
TBOOL AUnitGameEventComponent::HasWitnessedEvent( AInteractionEvent* a_pEvent ) const
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] == a_pEvent )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005c7c30
void AUnitGameEventComponent::RemoveEvent( AInteractionEvent* a_pEvent )
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] == a_pEvent )
		{
			m_apHeardEvents[ i ] = TNULL;
			return;
		}
	}

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] == a_pEvent )
		{
			m_apWitnessedEvents[ i ] = TNULL;
			return;
		}
	}
}

// $Barnyard: FUNCTION 005c7d60
AInteractionEvent* AUnitGameEventComponent::FindEventOfExactClass( TClass* a_pClass ) const
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] && m_apHeardEvents[ i ]->GetClass() == a_pClass )
			return m_apHeardEvents[ i ];
	}

	return FindEventOfExactClass( a_pClass, m_apWitnessedEvents, MAX_NUM_EVENTS );
}

// $Barnyard: FUNCTION 005c7960
AInteractionEvent* AUnitGameEventComponent::FindEventOfExactClass( TClass* a_pClass, AInteractionEvent* const* a_apEvents, TINT a_iNumSlots )
{
	for ( TINT i = 0; i < a_iNumSlots; i++ )
	{
		if ( a_apEvents[ i ] && a_apEvents[ i ]->GetClass() == a_pClass )
			return a_apEvents[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005c78e0
TBOOL AUnitGameEventComponent::AddEvent( AInteractionEvent* a_pEvent, AInteractionEvent** a_apEvents, TUINT8* a_auiData, TINT a_iNumSlots, TUINT8 a_uiData )
{
	TINT iReplace = -1;

	for ( TINT i = 0; i < a_iNumSlots; i++ )
	{
		if ( a_apEvents[ i ] == TNULL )
		{
			a_apEvents[ i ] = a_pEvent;
			a_auiData[ i ]  = a_uiData;
			return TTRUE;
		}

		// Replace the least important event if all the slots are used
		if ( iReplace < 0 || a_apEvents[ iReplace ]->GetPriority() < a_apEvents[ i ]->GetPriority() )
			iReplace = i;
	}

	if ( iReplace >= 0 )
	{
		a_apEvents[ iReplace ] = a_pEvent;
		a_auiData[ iReplace ]  = a_uiData;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005c7a80
TINT AUnitGameEventComponent::GetNumWitnessedEvents() const
{
	TINT iNumEvents = 0;

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] )
			iNumEvents++;
	}

	return iNumEvents;
}

// $Barnyard: FUNCTION 005c7dc0
void AUnitGameEventComponent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "UnitGameEventComponent" );

	TINT iNumHeard = 0;

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] ) iNumHeard++;
	}

	a_pBuffer->WriteInt( iNumHeard );

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apHeardEvents[ i ] )
		{
			a_pBuffer->WriteInt( m_apHeardEvents[ i ]->GetId() );
			a_pBuffer->WriteUInt8( m_auiHeardData[ i ] );
		}
	}

	TINT iNumWitnessed = 0;

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] ) iNumWitnessed++;
	}

	a_pBuffer->WriteInt( iNumWitnessed );

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		if ( m_apWitnessedEvents[ i ] )
		{
			a_pBuffer->WriteInt( m_apWitnessedEvents[ i ]->GetId() );
			a_pBuffer->WriteUInt8( m_auiWitnessedData[ i ] );
		}
	}
}

// $Barnyard: FUNCTION 005c8070
void AUnitGameEventComponent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		m_apHeardEvents[ i ] = TNULL;
		m_auiHeardData[ i ]  = 0;
	}

	for ( TINT i = 0; i < MAX_NUM_EVENTS; i++ )
	{
		m_apWitnessedEvents[ i ] = TNULL;
		m_auiWitnessedData[ i ]  = 0;
	}

	a_pBuffer->CheckString( "UnitGameEventComponent" );
	LoadEvents( a_pBuffer, m_apHeardEvents, m_auiHeardData, MAX_NUM_EVENTS );
	LoadEvents( a_pBuffer, m_apWitnessedEvents, m_auiWitnessedData, MAX_NUM_EVENTS );
}

// $Barnyard: FUNCTION 005c7ef0
void AUnitGameEventComponent::LoadEvents( AHackedQuickSave::Buffer* a_pBuffer, AInteractionEvent** a_apEvents, TUINT8* a_auiData, TINT a_iNumSlots )
{
	TINT iNumEvents = 0;
	a_pBuffer->ReadInt( iNumEvents );

	// NOTE: the original doesn't check a_iNumSlots
	for ( TINT i = 0; i < iNumEvents; i++ )
	{
		TUINT uiId;
		a_pBuffer->ReadUInt( uiId );
		a_apEvents[ i ] = AInteractionEventManager::GetSingleton()->FindEventById( uiId );
		a_pBuffer->ReadUInt8( a_auiData[ i ] );
	}
}
