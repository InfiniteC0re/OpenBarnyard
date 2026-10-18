#include "ToshiPCH.h"
#include "PGUIMenuTreeItemStyle.h"

#include <TGUI/TGUIStyle.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $JPOG: FUNCTION PGUICore.dll+00005f70
PGUIMenuTreeItemStyle::PGUIMenuTreeItemStyle( TGUIStyle* a_pCurrentItemStyle, TGUIStyle* a_pCurrentItemSelectedStyle, TGUIStyle* a_pNextItemStyle, TGUIWidget* a_pPreviousMenuTemplate, TGUIWidget* a_pCurrentMenuTemplate, TGUIWidget* a_pNextMenuTemplate )
    : m_pCurrentItemStyle( a_pCurrentItemStyle )
    , m_pCurrentItemSelectedStyle( a_pCurrentItemSelectedStyle )
    , m_pNextItemStyle( a_pNextItemStyle )
    , m_pPreviousMenuTemplate( a_pPreviousMenuTemplate )
    , m_pCurrentMenuTemplate( a_pCurrentMenuTemplate )
    , m_pNextMenuTemplate( a_pNextMenuTemplate )
{
}

// $JPOG: FUNCTION PGUICore.dll+00005ef0
PGUIMenuTreeItemStyle::~PGUIMenuTreeItemStyle()
{
}

// NOTE: the links of the node are copied too (as in the original)
// $JPOG: FUNCTION PGUICore.dll+00005eb0
PGUIMenuTreeItemStyle& PGUIMenuTreeItemStyle::operator=( const PGUIMenuTreeItemStyle& a_rcOther )
{
	m_pList                     = a_rcOther.m_pList;
	m_pNext                     = a_rcOther.m_pNext;
	m_pPrev                     = a_rcOther.m_pPrev;
	m_pCurrentItemStyle         = a_rcOther.m_pCurrentItemStyle;
	m_pCurrentItemSelectedStyle = a_rcOther.m_pCurrentItemSelectedStyle;
	m_pNextItemStyle            = a_rcOther.m_pNextItemStyle;
	m_pPreviousMenuTemplate     = a_rcOther.m_pPreviousMenuTemplate;
	m_pCurrentMenuTemplate      = a_rcOther.m_pCurrentMenuTemplate;
	m_pNextMenuTemplate         = a_rcOther.m_pNextMenuTemplate;

	return *this;
}

// $JPOG: FUNCTION PGUICore.dll+00005f00
TINT PGUIMenuTreeItemStyle::GetParameterCount() const
{
	return m_pCurrentItemStyle->GetParameterCount();
}

// $JPOG: FUNCTION PGUICore.dll+0000f880
TGUIStyle* PGUIMenuTreeItemStyle::GetItemStyle( DisplayLevel a_eLevel ) const
{
	if ( TUINT( a_eLevel ) < DisplayLevel_Next )
		return m_pCurrentItemStyle;

	if ( a_eLevel == DisplayLevel_Next )
		return m_pNextItemStyle;

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+0000f8b0
TGUIStyle* PGUIMenuTreeItemStyle::GetSelectedItemStyle( DisplayLevel a_eLevel ) const
{
	if ( TUINT( a_eLevel ) < DisplayLevel_Next )
		return m_pCurrentItemSelectedStyle;

	return TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00005f60
TGUIStyle* PGUIMenuTreeItemStyle::GetCurrentItemStyle() const
{
	return m_pCurrentItemStyle;
}

// $JPOG: FUNCTION PGUICore.dll+00005f50
TGUIStyle* PGUIMenuTreeItemStyle::GetCurrentItemSelectedStyle() const
{
	return m_pCurrentItemSelectedStyle;
}

// $JPOG: FUNCTION PGUICore.dll+00005f40
TGUIStyle* PGUIMenuTreeItemStyle::GetNextItemStyle() const
{
	return m_pNextItemStyle;
}

// $JPOG: FUNCTION PGUICore.dll+00005f30
TGUIWidget* PGUIMenuTreeItemStyle::GetPreviousMenuTemplate() const
{
	return m_pPreviousMenuTemplate;
}

// $JPOG: FUNCTION PGUICore.dll+00005f20
TGUIWidget* PGUIMenuTreeItemStyle::GetCurrentMenuTemplate() const
{
	return m_pCurrentMenuTemplate;
}

// $JPOG: FUNCTION PGUICore.dll+00005f10
TGUIWidget* PGUIMenuTreeItemStyle::GetNextMenuTemplate() const
{
	return m_pNextMenuTemplate;
}
