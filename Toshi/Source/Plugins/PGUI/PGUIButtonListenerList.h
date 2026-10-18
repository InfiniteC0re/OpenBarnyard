#pragma once
#include "PGUIButton.h"

#include <Toshi/TDList.h>

//-----------------------------------------------------------------------------
// Purpose: the listeners of the action events of the item buttons a menu
// creates (PGUIMenuBar, PGUIMenuDialog): one listener per button, connected
// to the callback of the menu, deleted with the menu.
//
// NOTE: the name is unknown, JPOG inlines it in the menus.
//-----------------------------------------------------------------------------
template <class Owner>
class PGUIButtonListenerList
{
public:
	using t_Callback = TBOOL( __stdcall* )( Owner* a_pOwner, PGUIButton* a_pButton, Toshi::TGUIActionEvent* a_pEvent );

	struct Node : public Toshi::TDList<Node>::TNode
	{
		Toshi::TListener<PGUIButton, Toshi::TGUIActionEvent, Owner> oListener; // 0x08
		PGUIButton::ActionEmitter*                                  pEmitter;  // 0x1C
	};

public:
	PGUIButtonListenerList( Owner* a_pOwner, t_Callback a_fnCallback )
	    : m_pOwner( a_pOwner ), m_fnCallback( a_fnCallback )
	{}

	~PGUIButtonListenerList() { DeleteAll(); }

	// Disconnects and deletes the listeners
	void DeleteAll()
	{
		while ( !m_oNodes.IsEmpty() )
		{
			Node* pNode = m_oNodes.Head();
			pNode->Remove();
			delete pNode;
		}

		m_oNodes.RemoveAll();
	}

	// Connects a listener to the action emitter of a_pButton
	void Add( PGUIButton* a_pButton )
	{
		Node* pNode     = new Node();
		pNode->pEmitter = &a_pButton->GetActionEmitter();
		pNode->oListener.Connect( *pNode->pEmitter, m_pOwner, m_fnCallback, 0 );
		m_oNodes.InsertTail( pNode );
	}

private:
	Owner*                   m_pOwner;     // the menu
	t_Callback               m_fnCallback; // its OnButtonAction
	Toshi::TDList<Node>      m_oNodes;
};
