#pragma once
#include <Toshi/TNodeList.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

namespace Toshi
{
class TGUIWidget;
}

//-----------------------------------------------------------------------------
// Purpose: an item of a list box or a menu: the values of the parameters of
// the style of the widget showing it and its state (the bits of the widget
// state it sets: Enabled, Visible and Selected).
//
// NOTE: the copy constructor and Clone share the values with the original (as
// in the original), both delete them.
//-----------------------------------------------------------------------------
class PGUIItemData : public Toshi::TNodeList<PGUIItemData>::TNode
{
public:
	enum State : TUINT
	{
		State_Enabled  = BITFLAG( 0 ),
		State_Visible  = BITFLAG( 2 ),
		State_Selected = BITFLAG( 4 ),

		// The widget state bits UpdateWidget sets
		State_WidgetMask = State_Enabled | State_Visible | State_Selected,
	};

public:
	PGUIItemData( TINT a_iNumValues, TUINT a_uiState );
	PGUIItemData( const PPropertyValue& a_rcValue, TUINT a_uiState );
	virtual ~PGUIItemData();

	virtual PGUIItemData* Clone() const;

	// Enabled and visible
	TBOOL CanSelect() const;

	void  SetState( TUINT a_uiMask, TUINT a_uiValue );
	void  SetState( TUINT a_uiMask, TBOOL a_bSet );
	void  SetState( TUINT a_uiState );
	void  AddState( TUINT a_uiState );
	void  RemoveState( TUINT a_uiState );
	TUINT GetState() const;

	PPropertyValue* GetParameterValues();

	// Sets the values to the parameters of the style of a_pWidget and the state bits to its state
	void UpdateWidget( Toshi::TGUIWidget* a_pWidget );

protected:
	PGUIItemData( const PGUIItemData& a_rcOther );

protected:
	PPropertyValue* m_pValues;  // 0x10
	TUINT           m_uiState;  // 0x14
};
