#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "1x5 Horizontal Stretch" component: like the horizontal triplet, but the space
// left around the middle element (which keeps its width) is filled by the
// "leftish" and "rightish" elements.
//-----------------------------------------------------------------------------
class TGUIStretch1x5Component : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUIStretch1x5Component, TGUIComponent );

	enum Element
	{
		Element_Left,
		Element_Leftish,
		Element_Middle,
		Element_Rightish,
		Element_Right,
		Element_NUMOF
	};

public:
	TGUIStretch1x5Component();
	TGUIStretch1x5Component( const TGUIStretch1x5Component& a_rcOther );
	~TGUIStretch1x5Component();

	virtual TGUIStretch1x5Component* Clone() const override;
	virtual TBOOL                    GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo* GetComponentInfo() const override;
	virtual TGUIElement*             GetElements() override;
	virtual void                     Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;

protected:
	// Height of the component (the highest of the left, middle and right elements) and preferred widths of these elements
	void CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rHeight, TINT& a_rLeftWidth, TINT& a_rMiddleWidth, TINT& a_rRightWidth ) const;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
