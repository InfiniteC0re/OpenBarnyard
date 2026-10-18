#include "ToshiPCH.h"
#include "TGUIWidgetAction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: the free list is created by the static initialiser of TGUIWidget in Nicktoons (003769e0)
TDEFINE_FREELIST_ALLOCATOR( TGUIWidgetAction );

// $JPOG: FUNCTION TGuiInterface.dll+00007a80
TGUIWidgetAction::TGUIWidgetAction( TINT a_iDoodadID, const TWCHAR* a_wszLabel, TINT a_iCommandID, TBOOL a_bEnabled )
    : m_oEmitter( this )
    , m_iDoodadID( a_iDoodadID )
    , m_wszLabel( a_wszLabel )
    , m_iCommandID( a_iCommandID )
    , m_bEnabled( a_bEnabled )
{
}

// NOTE: the links of the node and the emitter are copied too (as in the original)
// $JPOG: FUNCTION TGuiInterface.dll+00007970
TGUIWidgetAction::TGUIWidgetAction( const TGUIWidgetAction& a_rcOther )
    : TDList<TGUIWidgetAction>::TNode( a_rcOther )
    , m_oEmitter( a_rcOther.m_oEmitter )
    , m_iDoodadID( a_rcOther.m_iDoodadID )
    , m_wszLabel( a_rcOther.m_wszLabel )
    , m_iCommandID( a_rcOther.m_iCommandID )
    , m_bEnabled( a_rcOther.m_bEnabled )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00007940
TGUIWidgetAction::~TGUIWidgetAction()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+000078c0
TGUIWidgetAction& TGUIWidgetAction::operator=( const TGUIWidgetAction& a_rcOther )
{
	m_pNext      = a_rcOther.m_pNext;
	m_pPrev      = a_rcOther.m_pPrev;
	m_oEmitter   = a_rcOther.m_oEmitter;
	m_iDoodadID  = a_rcOther.m_iDoodadID;
	m_wszLabel   = a_rcOther.m_wszLabel;
	m_iCommandID = a_rcOther.m_iCommandID;
	m_bEnabled   = a_rcOther.m_bEnabled;

	return *this;
}

// $JPOG: FUNCTION TGuiInterface.dll+000079e0
void TGUIWidgetAction::Throw( TGUIWidget* a_pWidget )
{
	m_oEmitter.Throw( &a_pWidget );
}

TOSHI_NAMESPACE_END
