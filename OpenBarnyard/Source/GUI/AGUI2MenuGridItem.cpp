#include "pch.h"
#include "AGUI2MenuGrid.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0063f710
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

// $Barnyard: FUNCTION 0063f6d0
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

// $Barnyard: FUNCTION 0063f6e0
void AGUI2MenuGridItem::SetSideElements( AGUI2MenuGridItem* a_pLeft, AGUI2MenuGridItem* a_pRight, AGUI2MenuGridItem* a_pTop, AGUI2MenuGridItem* a_pBottom )
{
	m_pLeftElement   = a_pLeft;
	m_pRightElement  = a_pRight;
	m_pTopElement    = a_pTop;
	m_pBottomElement = a_pBottom;
}
