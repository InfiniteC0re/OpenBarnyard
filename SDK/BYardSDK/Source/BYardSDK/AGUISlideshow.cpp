#include "pch.h"
#include "AGUISlideshow.h"
#include "AGUI2.h"
#include "SDKHooks.h"

AGUISlideshow::AGUISlideshow() :
    m_Images( Toshi::GetGlobalAllocator(), 64, 64 )
{
	CALL_THIS( 0x0059d810, AGUISlideshow*, void, this );
}

void AGUISlideshow::SwitchToNextSlide( TBOOL a_bUnused )
{
	CALL_THIS( 0x0059d810, AGUISlideshow*, void, this, TBOOL, a_bUnused );
}

TBOOL AGUISlideshow::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent )
{
	if ( a_pEvent->GetEventType() == Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN )
	{
		if ( m_eFlags & Flags_InstaSkippable )
		{
			m_ImageIterator = m_Images.End();
			m_eFlags |= Flags_Ended;
			return TTRUE;
		}

		if ( m_eFlags & Flags_Skippable )
		{
			SwitchToNextSlide( TTRUE );
			return TTRUE;
		}
	}

	return TFALSE;
}

TBOOL AGUISlideshow::ProcessCommand( AInputCommand a_eCommand )
{
	if ( ( ( a_eCommand != AInputCommand_Back && a_eCommand != AInputCommand_Cancel && a_eCommand != AInputCommand_Quit ) || !HASANYFLAG( m_eFlags, Flags_Unk1 ) ) && !HASANYFLAG( m_eFlags, Flags_InstaSkippable ) )
	{
		if ( m_eFlags & Flags_Skippable )
		{
			SwitchToNextSlide( TTRUE );
			return TTRUE;
		}

		return TFALSE;
	}

	m_ImageIterator = m_Images.End();
	m_eFlags |= Flags_Ended;
	return TTRUE;
}
