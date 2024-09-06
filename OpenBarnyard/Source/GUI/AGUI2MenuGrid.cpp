#include "pch.h"
#include "AGUI2MenuGrid.h"
#include "AGUI2.h"

#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING


void AGUI2MenuGridItem::SetSideElements( AGUI2MenuGridItem* a_pLeft, AGUI2MenuGridItem* a_pRight, AGUI2MenuGridItem* a_pTop, AGUI2MenuGridItem* a_pBottom )
{
	m_pLeftElement   = a_pLeft;
	m_pRightElement  = a_pRight;
	m_pTopElement    = a_pTop;
	m_pBottomElement = a_pBottom;
}
