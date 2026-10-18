#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "1x1 Icon" component: a single element in the rectangle.
//-----------------------------------------------------------------------------
class TGUIIconComponent : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUIIconComponent, TGUIComponent );

	enum Element
	{
		Element_Icon,
		Element_NUMOF
	};

public:
	TGUIIconComponent();
	TGUIIconComponent( const TGUIIconComponent& a_rcOther );
	~TGUIIconComponent();

	virtual TGUIIconComponent*       Clone() const override;
	virtual TBOOL                    GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo* GetComponentInfo() const override;
	virtual TGUIElement*             GetElements() override;
	virtual void                     Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
