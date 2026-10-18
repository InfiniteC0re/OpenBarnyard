#include "ToshiPCH.h"
#include "PGUIMenuTreeItem.h"
#include "PGUIPropertyValueFactory.h"

#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $JPOG: FUNCTION PGUICore.dll+0000f8d0
PGUIMenuTreeItem::PGUIMenuTreeItem( PGUIMenuTreeItemStyle* a_pStyle, TINT a_iNumValues, TUINT a_uiState )
    : PGUIItemData( a_iNumValues, a_uiState )
    , m_pStyle( a_pStyle )
    , m_pParent( TNULL )
{
}

// $JPOG: FUNCTION PGUICore.dll+0000f960
PGUIMenuTreeItem::PGUIMenuTreeItem( PGUIMenuTreeItemStyle* a_pStyle, const TWCHAR* a_wszLabel )
    : PGUIItemData( a_pStyle ? a_pStyle->GetParameterCount() : 0, State_Visible )
    , m_pStyle( a_pStyle )
    , m_pParent( TNULL )
{
	if ( a_pStyle )
		m_pValues[ 0 ] = a_wszLabel;
}

// Copies the values and clones the children
// $JPOG: FUNCTION PGUICore.dll+0000fcc0
PGUIMenuTreeItem::PGUIMenuTreeItem( const PGUIMenuTreeItem& a_rcOther )
    : PGUIItemData( 0, State_Enabled | State_Visible )
    , m_pStyle( a_rcOther.m_pStyle )
    , m_pParent( TNULL )
{
	if ( m_pStyle )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// Fixes the leak of the empty value array
		PGUIPropertyValueFactory::GetSingleton()->DeallocateValueArray( m_pValues );
#endif
		m_pValues = new PPropertyValue[ m_pStyle->GetParameterCount() ];
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// Fixes the copy of an item without style (the root item of the menu trees) dereferencing the style
	if ( m_pStyle )
#endif
	{
		for ( TINT i = 0; i < m_pStyle->GetParameterCount(); i++ )
			m_pValues[ i ] = a_rcOther.m_pValues[ i ];
	}

	for ( auto it = a_rcOther.m_oChildren.Begin(); it != a_rcOther.m_oChildren.End(); it++ )
	{
		PGUIMenuTreeItem* pChild = TSTATICCAST( PGUIMenuTreeItem, it->Clone() );
		pChild->m_pParent        = this;
		m_oChildren.InsertTail( pChild );
	}
}

// $JPOG: FUNCTION PGUICore.dll+00011cf0
PGUIMenuTreeItem::~PGUIMenuTreeItem()
{
	while ( !m_oChildren.IsEmpty() )
	{
		PGUIItemData* pChild = m_oChildren.Head().Get();
		m_oChildren.Remove( pChild );

		if ( pChild )
			delete pChild;
	}
}

// $JPOG: FUNCTION PGUICore.dll+00011cc0
PGUIMenuTreeItem* PGUIMenuTreeItem::Clone() const
{
	return new PGUIMenuTreeItem( *this );
}

// $JPOG: FUNCTION PGUICore.dll+0000fa10
PGUIMenuTreeItem* PGUIMenuTreeItem::AddItem( const TWCHAR* a_wszLabel, TUINT a_uiState )
{
	PGUIMenuTreeItem* pItem = FindChildItem( a_wszLabel, TFALSE );

	if ( !pItem )
	{
		pItem            = new PGUIMenuTreeItem( m_pStyle, a_wszLabel );
		pItem->m_uiState = a_uiState;
		pItem->m_pParent = this;
		m_oChildren.InsertTail( pItem );
	}

	return pItem;
}

// $JPOG: FUNCTION PGUICore.dll+00011c90
void PGUIMenuTreeItem::AddItem( PGUIMenuTreeItem* a_pItem )
{
	a_pItem->m_pParent = this;
	m_oChildren.InsertTail( a_pItem );
}

// $JPOG: FUNCTION PGUICore.dll+00011c60
void PGUIMenuTreeItem::RemoveItem( PGUIMenuTreeItem* a_pItem )
{
	a_pItem->m_pParent = TNULL;
	m_oChildren.Remove( a_pItem );
}

// $JPOG: FUNCTION PGUICore.dll+00011c20
void PGUIMenuTreeItem::DeleteItem( PGUIMenuTreeItem* a_pItem )
{
	a_pItem->m_pParent = TNULL;
	m_oChildren.Remove( a_pItem );

	if ( a_pItem )
		delete a_pItem;
}

// $JPOG: FUNCTION PGUICore.dll+0000fa80
PGUIMenuTreeItem* PGUIMenuTreeItem::FindChildItem( TINT a_iIndex )
{
	auto it = m_oChildren.Begin();

	for ( ; a_iIndex > 0 && it != m_oChildren.End(); a_iIndex-- )
		it++;

	return ( it != m_oChildren.End() ) ? TSTATICCAST( PGUIMenuTreeItem, it.Get() ) : TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000fad0
PGUIMenuTreeItem* PGUIMenuTreeItem::FindChildItem( const TWCHAR* a_wszLabel, TBOOL a_bRecurse )
{
	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); it++ )
	{
		PGUIMenuTreeItem* pChild = TSTATICCAST( PGUIMenuTreeItem, it.Get() );

		if ( pChild->m_pValues[ 0 ] == a_wszLabel )
			return pChild;

		if ( a_bRecurse )
		{
			if ( PGUIMenuTreeItem* pItem = pChild->FindChildItem( a_wszLabel, TTRUE ) )
				return pItem;
		}
	}

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000fb40
TBOOL PGUIMenuTreeItem::IsParentOf( PGUIMenuTreeItem* a_pItem )
{
	for ( PGUIMenuTreeItem* pParent = a_pItem->m_pParent; pParent; pParent = pParent->m_pParent )
	{
		if ( pParent == this )
			return TTRUE;
	}

	return TFALSE;
}

// $JPOG: FUNCTION PGUICore.dll+0000fb70
PGUIMenuTreeItem* PGUIMenuTreeItem::ChooseNextItem( TINT a_iDirection )
{
	PGUIMenuTreeItem* pItem = this;

	while ( TTRUE )
	{
		if ( a_iDirection > 0 )
			pItem = ( pItem == m_pParent->GetLastChild() ) ? m_pParent->GetFirstChild() : TSTATICCAST( PGUIMenuTreeItem, pItem->Next() );
		else if ( a_iDirection < 0 )
			pItem = ( pItem == m_pParent->GetFirstChild() ) ? m_pParent->GetLastChild() : TSTATICCAST( PGUIMenuTreeItem, pItem->Prev() );

		if ( pItem->CanSelect() )
			return pItem;

		if ( pItem == this )
			return TNULL;
	}
}

// $JPOG: FUNCTION PGUICore.dll+0000fbf0
PGUIMenuTreeItem* PGUIMenuTreeItem::ChooseChildItem()
{
	if ( m_oChildren.IsEmpty() )
		return TNULL;

	PGUIMenuTreeItem* pItem = GetFirstChild();

	if ( !pItem->CanSelect() )
		return pItem->ChooseNextItem( 1 );

	return pItem;
}

// $JPOG: FUNCTION PGUICore.dll+0000fc30
void PGUIMenuTreeItem::RecurseTree( t_RecurseCallback a_fnPre, t_RecurseCallback a_fnPost, t_RecurseCallback a_fnFilter, void* a_pUserData )
{
	if ( a_fnFilter && !a_fnFilter( this, a_pUserData ) )
		return;

	for ( auto it = m_oChildren.Begin(); it != m_oChildren.End(); )
	{
		PGUIMenuTreeItem* pChild = TSTATICCAST( PGUIMenuTreeItem, it.Get() );
		it++;

		if ( a_fnPre )
			a_fnPre( pChild, a_pUserData );

		pChild->RecurseTree( a_fnPre, a_fnPost, a_fnFilter, a_pUserData );

		if ( a_fnPost )
			a_fnPost( pChild, a_pUserData );
	}
}
