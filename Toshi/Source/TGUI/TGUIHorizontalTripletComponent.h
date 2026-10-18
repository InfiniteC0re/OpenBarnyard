#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "1x3 Horizontal Stretch" component: a left and a right element that keep their
// width (they shrink proportionally when the rectangle is too narrow) and a
// middle element that fills the rest.
//-----------------------------------------------------------------------------
class TGUIHorizontalTripletComponent : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUIHorizontalTripletComponent, TGUIComponent );

	enum Element
	{
		Element_Left,
		Element_Middle,
		Element_Right,
		Element_NUMOF
	};

public:
	TGUIHorizontalTripletComponent();
	TGUIHorizontalTripletComponent( const TGUIHorizontalTripletComponent& a_rcOther );
	~TGUIHorizontalTripletComponent();

	virtual TGUIHorizontalTripletComponent* Clone() const override;
	virtual TBOOL                           GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo*        GetComponentInfo() const override;
	virtual TGUIElement*                    GetElements() override;
	virtual void                            Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;

protected:
	// Height of the component (the highest element) and preferred widths of the elements
	void CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rHeight, TINT& a_rLeftWidth, TINT& a_rMiddleWidth, TINT& a_rRightWidth ) const;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
