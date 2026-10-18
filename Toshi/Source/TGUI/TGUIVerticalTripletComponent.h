#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "3x1 Vertical Stretch" component: a top and a bottom element that keep their
// height (they shrink proportionally when the rectangle is too short) and a
// middle element that fills the rest.
//-----------------------------------------------------------------------------
class TGUIVerticalTripletComponent : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUIVerticalTripletComponent, TGUIComponent );

	enum Element
	{
		Element_Top,
		Element_Middle,
		Element_Bottom,
		Element_NUMOF
	};

public:
	TGUIVerticalTripletComponent();
	TGUIVerticalTripletComponent( const TGUIVerticalTripletComponent& a_rcOther );
	~TGUIVerticalTripletComponent();

	virtual TGUIVerticalTripletComponent* Clone() const override;
	virtual TBOOL                         GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo*      GetComponentInfo() const override;
	virtual TGUIElement*                  GetElements() override;
	virtual void                          Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;

protected:
	// +0x38: Width of the component (the widest element) and preferred heights of the elements
	// NOTE: virtual in Nicktoons, not in JPOG
	virtual void CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rTopHeight, TINT& a_rMiddleHeight, TINT& a_rBottomHeight ) const;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
