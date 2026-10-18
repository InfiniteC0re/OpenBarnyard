#pragma once
#include <TGUI/TGUIWindow.h>

//-----------------------------------------------------------------------------
// Purpose: a window (focus map, default focus) with the "background" and
// "border" components, sized to fit its children and the border. A click
// on it starts dragging (the drag itself isn't implemented).
//-----------------------------------------------------------------------------
class PGUIDialog : public Toshi::TGUIWindow
{
public:
	TDECLARE_CLASS( PGUIDialog, Toshi::TGUIWindow );

public:
	PGUIDialog();
	PGUIDialog( const PGUIDialog& a_rcOther );
	~PGUIDialog();

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
	virtual PGUIDialog*                  Clone() const override;
	virtual TBOOL                        AllowsChildren() const override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRelease( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnPointerMove( TINT a_iX, TINT a_iY ) override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	// $JPOG: FUNCTION PGUICore.dll+00004ea0
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

protected:
	TBOOL m_bDragging;     // 0xA4
	TINT  m_iDragPointerX; // 0xA8 the pointer position when the drag started
	TINT  m_iDragPointerY; // 0xAC
};
