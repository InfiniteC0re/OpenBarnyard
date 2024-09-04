#include "pch.h"
#include "AGUI2Menu.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUI2MenuItem::AGUI2MenuItem()
{
	m_pNextMenuItem = this;
	m_pPrevMenuItem = this;
	m_bEnabled      = TTRUE;
}

AGUI2MenuItem::~AGUI2MenuItem()
{
}

void AGUI2MenuItem::SetAlpha( TFLOAT a_fAlpha )
{
	for ( AGUI2Element* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		it->SetAlpha( a_fAlpha );
	}

	AGUI2Element::SetAlpha( a_fAlpha );
}

void AGUI2MenuItem::OnFocus()
{
}

void AGUI2MenuItem::OnFocusLost()
{
}

void AGUI2MenuItem::OnUpdate( TFLOAT a_fDeltaTime )
{
}

AGUI2MenuItem::COMMANDRESULT AGUI2MenuItem::OnInputCommand( AGUI2Menu::ACTIONFLAGS& a_rActionFlags )
{
	if ( a_rActionFlags & AGUI2Menu::ACTIONFLAGS_CANCEL )
		return COMMANDRESULT_CANCEL;

	return ( a_rActionFlags & AGUI2Menu::ACTIONFLAGS_OK ) ? COMMANDRESULT_OK : COMMANDRESULT_NONE;
}

void AGUI2MenuItem::SetEnabled( TBOOL a_bEnabled )
{
	m_bEnabled = a_bEnabled;
}

TFLOAT AGUI2MenuItem::GetFlowVisualOffset()
{
	return 0.0f;
}

TFLOAT AGUI2MenuItem::GetFlowOffset()
{
	return 0.0f;
}

void AGUI2MenuItem::LinkMenuItemBefore( AGUI2MenuItem& a_rLinkAfter )
{
	m_pPrevMenuItem                  = &a_rLinkAfter;
	m_pNextMenuItem                  = a_rLinkAfter.m_pNextMenuItem;
	a_rLinkAfter.m_pNextMenuItem     = this;
	m_pNextMenuItem->m_pPrevMenuItem = this;
}
