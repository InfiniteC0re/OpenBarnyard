#pragma once
#include <Toshi/TNodeList.h>

namespace Toshi
{
class TGUIStyle;
class TGUIWidget;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the look of the items of a menu tree (PGUIMenuTree): the styles of
// the item buttons in the current menu (normal and selected) and in the next
// menu, and the dialog templates (top-level instances) of the previous,
// current and next menus.
//-----------------------------------------------------------------------------
class PGUIMenuTreeItemStyle : public Toshi::TNodeList<PGUIMenuTreeItemStyle>::TNode
{
public:
	// The level of a menu shown by a menu tree
	enum DisplayLevel : TINT
	{
		DisplayLevel_Previous = 0,
		DisplayLevel_Current  = 1,
		DisplayLevel_Next     = 2,
	};

public:
	PGUIMenuTreeItemStyle( Toshi::TGUIStyle* a_pCurrentItemStyle, Toshi::TGUIStyle* a_pCurrentItemSelectedStyle, Toshi::TGUIStyle* a_pNextItemStyle, Toshi::TGUIWidget* a_pPreviousMenuTemplate, Toshi::TGUIWidget* a_pCurrentMenuTemplate, Toshi::TGUIWidget* a_pNextMenuTemplate );
	~PGUIMenuTreeItemStyle();

	PGUIMenuTreeItemStyle& operator=( const PGUIMenuTreeItemStyle& a_rcOther );

	// The number of parameters of the current item style
	TINT GetParameterCount() const;

	// The current item style for the previous and the current menus, the next item style for the next one
	Toshi::TGUIStyle* GetItemStyle( DisplayLevel a_eLevel ) const;

	// The current item selected style for the previous and the current menus
	Toshi::TGUIStyle* GetSelectedItemStyle( DisplayLevel a_eLevel ) const;

	Toshi::TGUIStyle*  GetCurrentItemStyle() const;
	Toshi::TGUIStyle*  GetCurrentItemSelectedStyle() const;
	Toshi::TGUIStyle*  GetNextItemStyle() const;
	Toshi::TGUIWidget* GetPreviousMenuTemplate() const;
	Toshi::TGUIWidget* GetCurrentMenuTemplate() const;
	Toshi::TGUIWidget* GetNextMenuTemplate() const;

private:
	Toshi::TGUIStyle*  m_pCurrentItemStyle;         // 0x0C
	Toshi::TGUIStyle*  m_pCurrentItemSelectedStyle; // 0x10
	Toshi::TGUIStyle*  m_pNextItemStyle;            // 0x14
	Toshi::TGUIWidget* m_pPreviousMenuTemplate;     // 0x18
	Toshi::TGUIWidget* m_pCurrentMenuTemplate;      // 0x1C
	Toshi::TGUIWidget* m_pNextMenuTemplate;         // 0x20
};
