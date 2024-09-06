#include "pch.h"
#include "AGUI2MenuGrid.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUI2MenuGridItem::AGUI2MenuGridItem()
{
	m_pNextMenuGridItem = this;
	m_pPrevMenuGridItem = this;
	m_bEnabled          = TTRUE;
	m_pLeftElement      = TNULL;
	m_pRightElement     = TNULL;
	m_pTopElement       = TNULL;
	m_pBottomElement    = TNULL;
}

AGUI2MenuGridItem::~AGUI2MenuGridItem()
{
}

void AGUI2MenuGridItem::OnFocus()
{
}

void AGUI2MenuGridItem::OnFocusLost()
{
}

void AGUI2MenuGridItem::OnUpdate( TFLOAT a_fDeltaTime )
{
}

AGUI2MenuGridItem::COMMANDRESULT AGUI2MenuGridItem::OnInputCommand( AGUI2MenuGrid::ACTIONFLAGS& a_rActionFlags )
{
	if ( a_rActionFlags & AGUI2MenuGrid::ACTIONFLAGS_CANCEL )
		return COMMANDRESULT_CANCEL;

	return ( a_rActionFlags & AGUI2MenuGrid::ACTIONFLAGS_OK ) ? COMMANDRESULT_OK : COMMANDRESULT_NONE;
}

void AGUI2MenuGridItem::SetEnabled( TBOOL a_bEnabled )
{
	m_bEnabled = a_bEnabled;
}

TFLOAT AGUI2MenuGridItem::GetFlowVisualOffset()
{
	return 0.0f;
}

TFLOAT AGUI2MenuGridItem::GetFlowOffset()
{
	return 0.0f;
}

void AGUI2MenuGridItem::LinkMenuItemBefore( AGUI2MenuGridItem& a_rLinkAfter )
{
	m_pPrevMenuGridItem                      = &a_rLinkAfter;
	m_pNextMenuGridItem                      = a_rLinkAfter.m_pNextMenuGridItem;
	a_rLinkAfter.m_pNextMenuGridItem         = this;
	m_pNextMenuGridItem->m_pPrevMenuGridItem = this;
}
