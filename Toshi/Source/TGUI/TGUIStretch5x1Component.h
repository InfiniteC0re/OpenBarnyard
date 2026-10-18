#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "5x1 Vertical Stretch" component: like the vertical triplet, but the space left
// around the middle element (which keeps its height) is filled by the "upper"
// and "lower" elements.
//-----------------------------------------------------------------------------
class TGUIStretch5x1Component : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUIStretch5x1Component, TGUIComponent );

	enum Element
	{
		Element_Top,
		Element_Upper,
		Element_Middle,
		Element_Lower,
		Element_Bottom,
		Element_NUMOF
	};

public:
	TGUIStretch5x1Component();
	TGUIStretch5x1Component( const TGUIStretch5x1Component& a_rcOther );
	~TGUIStretch5x1Component();

	virtual TGUIStretch5x1Component* Clone() const override;
	virtual TBOOL                    GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo* GetComponentInfo() const override;
	virtual TGUIElement*             GetElements() override;
	virtual void                     Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;

protected:
	// +0x38: Width of the component (the widest of the top, middle and bottom elements) and preferred heights of these elements
	virtual void CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rTopHeight, TINT& a_rMiddleHeight, TINT& a_rBottomHeight ) const;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
