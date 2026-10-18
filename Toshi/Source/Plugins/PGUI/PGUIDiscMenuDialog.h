#pragma once
#include "PGUIMenuDialog.h"

//-----------------------------------------------------------------------------
// Purpose: a menu whose buttons are around a disc (its menu bucket has a
// PGUIDiscLayout): the stick selects the item in its direction, the
// navigation actions are ignored.
//-----------------------------------------------------------------------------
class PGUIDiscMenuDialog : public PGUIMenuDialog
{
public:
	TDECLARE_CLASS( PGUIDiscMenuDialog, PGUIMenuDialog );

public:
	PGUIDiscMenuDialog();
	PGUIDiscMenuDialog( const PGUIDiscMenuDialog& a_rcOther );
	~PGUIDiscMenuDialog();

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
	virtual PGUIDiscMenuDialog*          Clone() const override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRelease( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnAxisMove( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUIMenuDialog
	//-----------------------------------------------------------------------------

	// Centred on a_rX, a_rY (within the screen margins of the tree), the next menu is centred on the selected button
	virtual void UpdatePosition( TINT& a_rX, TINT& a_rY ) override;
	virtual void UpdateSelection() override;

	// $JPOG: FUNCTION PGUICore.dll+00005d50
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

private:
	TINT m_iUnknown; // 0xF4 cleared when the stick is released, nothing reads it
};
