#pragma once
#include <TGUI/TGUIWidget.h>

//-----------------------------------------------------------------------------
// Purpose: a box showing an editable text: the "background", the
// "foreground" (the text, the LABEL parameter of its styles) and the "cursor"
// components, and the "InsertMode" state.
//
// NOTE: reconstructed. JPOG (PGUICore.dll) doesn't have this class and
// Nicktoons/Barnyard don't contain the PGUI code. The root components and the
// state variable are the ones the Barnyard and Nicktoons styles and dialogs
// use; the layout follows PGUILabel (the components fill the box, the cursor
// at the end of the text). The text editing isn't known and isn't
// implemented: the box only takes the focus.
//-----------------------------------------------------------------------------
class PGUITextBox : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUITextBox, Toshi::TGUIWidget );

	enum State : TUINT
	{
		State_InsertMode = BITFLAG( 5 ),
	};

	enum Component : TINT
	{
		Component_Background,
		Component_Foreground,
		Component_Cursor,
	};

public:
	PGUITextBox();
	PGUITextBox( const PGUITextBox& a_rcOther );
	~PGUITextBox();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	//-----------------------------------------------------------------------------
	// Toshi::TGUIWidget
	//-----------------------------------------------------------------------------
	virtual const Toshi::TGUIWidgetInfo* GetWidgetInfo() const override;
	virtual PGUITextBox*                 Clone() const override;
	virtual TBOOL                        AcceptsFocus() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	void  SetInsertMode( TBOOL a_bInsertMode ) { SetState( State_InsertMode, a_bInsertMode ? TUINT( State_InsertMode ) : 0u ); }
	TBOOL IsInsertMode() const { return ( m_uiState & State_InsertMode ) != 0; }

	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;
};
