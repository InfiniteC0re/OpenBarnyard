#include "pch.h"
#include "AModEvents.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AModEvents::Event AModEvents::ms_aEvents[ AModEventId_NUMOF ];

TBOOL AModEvents::Subscribe( AModEventId a_eEvent, t_GenericCallback a_fnCallback, void* a_pUserData )
{
	TASSERT( a_eEvent >= 0 && a_eEvent < AModEventId_NUMOF );
	TASSERT( a_fnCallback != TNULL );

	Event& rEvent = ms_aEvents[ a_eEvent ];

	if ( rEvent.iNumListeners >= MAX_NUM_LISTENERS )
	{
		TERROR( "AModEvents: Event %d has too many listeners\n", a_eEvent );
		return TFALSE;
	}

	rEvent.aListeners[ rEvent.iNumListeners++ ] = { a_fnCallback, a_pUserData };
	return TTRUE;
}

TBOOL AModEvents::Unsubscribe( AModEventId a_eEvent, t_GenericCallback a_fnCallback, void* a_pUserData )
{
	TASSERT( a_eEvent >= 0 && a_eEvent < AModEventId_NUMOF );
	Event& rEvent = ms_aEvents[ a_eEvent ];

	for ( TINT i = 0; i < rEvent.iNumListeners; i++ )
	{
		Listener& rListener = rEvent.aListeners[ i ];

		if ( rListener.fnCallback == a_fnCallback && rListener.pUserData == a_pUserData )
		{
			// The list is compacted once the event is no longer being fired
			rListener.fnCallback        = TNULL;
			rEvent.bHasRemovedListeners = TTRUE;

			if ( rEvent.iFireDepth == 0 )
				RemoveDeadListeners( rEvent );

			return TTRUE;
		}
	}

	return TFALSE;
}

void AModEvents::Fire( AModEventId a_eEvent, const void* a_pEvent )
{
	Event& rEvent = ms_aEvents[ a_eEvent ];
	rEvent.iFireDepth++;

	// Listeners added while firing are called too
	for ( TINT i = 0; i < rEvent.iNumListeners; i++ )
	{
		const Listener& rListener = rEvent.aListeners[ i ];

		if ( rListener.fnCallback )
			rListener.fnCallback( a_pEvent, rListener.pUserData );
	}

	if ( --rEvent.iFireDepth == 0 && rEvent.bHasRemovedListeners )
		RemoveDeadListeners( rEvent );
}

void AModEvents::Reset()
{
	for ( TINT i = 0; i < AModEventId_NUMOF; i++ )
	{
		TASSERT( ms_aEvents[ i ].iFireDepth == 0 );

		ms_aEvents[ i ].iNumListeners        = 0;
		ms_aEvents[ i ].bHasRemovedListeners = TFALSE;
	}
}

void AModEvents::RemoveDeadListeners( Event& a_rEvent )
{
	TINT iNumAlive = 0;

	for ( TINT i = 0; i < a_rEvent.iNumListeners; i++ )
	{
		if ( a_rEvent.aListeners[ i ].fnCallback )
			a_rEvent.aListeners[ iNumAlive++ ] = a_rEvent.aListeners[ i ];
	}

	a_rEvent.iNumListeners        = iNumAlive;
	a_rEvent.bHasRemovedListeners = TFALSE;
}
