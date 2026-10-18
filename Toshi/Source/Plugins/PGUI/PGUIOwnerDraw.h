#pragma once
#include "PGUIPanel.h"

//-----------------------------------------------------------------------------
// Purpose: a panel the game lays out and paints itself through a callback.
//-----------------------------------------------------------------------------
class PGUIOwnerDraw : public PGUIPanel
{
public:
	TDECLARE_CLASS( PGUIOwnerDraw, PGUIPanel );

	// Implemented by the game
	class CallBack
	{
	public:
		// +0x00
		virtual void Pack( PGUIOwnerDraw* a_pOwnerDraw ) = 0;

		// +0x04: Sizes the widget (the sizes that aren't fixed)
		virtual void Layout( PGUIOwnerDraw* a_pOwnerDraw ) = 0;

		// +0x08
		virtual void Paint( PGUIOwnerDraw* a_pOwnerDraw, Toshi::TGUIDisplayContext* a_pDisplayContext ) = 0;
	};

public:
	PGUIOwnerDraw();
	PGUIOwnerDraw( const PGUIOwnerDraw& a_rcOther );
	~PGUIOwnerDraw();

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
	virtual PGUIOwnerDraw*               Clone() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUIOwnerDraw
	//-----------------------------------------------------------------------------

	// +0xB4: Paints the widget with the callback
	virtual void Paint( Toshi::TGUIDisplayContext* a_pDisplayContext );

	using TGUIWidget::Paint;

	// Returns the previous callback
	CallBack* SetCallBack( CallBack* a_pCallBack );

	// $JPOG: FUNCTION PGUICore.dll+00012670
	CallBack* GetCallBack() const { return m_pCallBack; }

	// $JPOG: FUNCTION PGUICore.dll+00012680
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

private:
	CallBack* m_pCallBack; // 0x8C
};
