#include "pch.h"
#include "AGUIGameHudChange.h"
#include "Animation/AAnimationBreakpointUnitAction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0059ca70, CreateObject 0059ca80 and CreateObjectInPlace 0059cb00 of the TClass (the constructor is inlined in both)
// $Barnyard: FUNCTION 0059ca80
// $Barnyard: FUNCTION 0059cb00
TDEFINE_CLASS( AGUIGameHudChange );

AGUIGameHudChange::Listener AGUIGameHudChange::ms_oListeners = {
	&AGUIGameHudChange::ms_oListeners,
	&AGUIGameHudChange::ms_oListeners,
	TNULL,
	TNULL
};

AGUIGameHudChange::AGUIGameHudChange()
{
	m_iUnk8 = 0;
}

// NOTE: 0059cb80 is reached through the AThrowableEvent vtable (this adjusted by -4)
// $Barnyard: FUNCTION 0059cb80
void AGUIGameHudChange::Throw()
{
	ThrowEvent( this );

	// NOTE: the original passes the AThrowableEvent part of the event to the same list AAnimationBreakpointUnitAction::Event::Throw uses
	AAnimationBreakpointUnitAction::ThrowOtherEvent( reinterpret_cast<AAnimationBreakpointUnitAction::Event*>( static_cast<AThrowableEvent*>( this ) ) );
}

// $Barnyard: FUNCTION 0059cbc0
void AGUIGameHudChange::ThrowEvent( AGUIGameHudChange* a_pEvent )
{
	Listener* pListener = ms_oListeners.pNext;

	while ( pListener != &ms_oListeners )
	{
		Listener* pNext = pListener->pNext;
		pListener->pCallback( pListener->pCaller, a_pEvent );
		pListener = pNext;
	}
}
