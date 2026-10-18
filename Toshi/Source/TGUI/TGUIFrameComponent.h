#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "3x3 Frame" component: four corners, four edges and a middle. The
// corners keep their size, the edges and the middle fill the rest of the
// rectangle (repeated or stretched, see TGUIElement::Fill).
//-----------------------------------------------------------------------------
class TGUIFrameComponent : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUIFrameComponent, TGUIComponent );

	enum Element
	{
		Element_TopLeft,
		Element_TopRight,
		Element_BottomLeft,
		Element_BottomRight,
		Element_Top,
		Element_Bottom,
		Element_Left,
		Element_Right,
		Element_Middle,
		Element_NUMOF
	};

	// Dimensions of the rows and columns, kept in the TGUIComponentCache of the widget
	struct Cache
	{
		TUINT16 uiTopHeight;    // 0x0
		TUINT16 uiBottomHeight; // 0x2
		TUINT16 uiLeftWidth;    // 0x4
		TUINT16 uiRightWidth;   // 0x6
		TUINT16 uiMiddleWidth;  // 0x8
		TUINT16 uiMiddleHeight; // 0xA
	};

public:
	TGUIFrameComponent();
	TGUIFrameComponent( const TGUIFrameComponent& a_rcOther );
	~TGUIFrameComponent();

	virtual TGUIFrameComponent*      Clone() const override;
	virtual TBOOL                    GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo* GetComponentInfo() const override;
	virtual TGUIElement*             GetElements() override;
	virtual void                     Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;
	virtual void                     DestroyCache( void* a_pData ) const override;

protected:
	// Sizes of the rows and columns from the preferred sizes of the elements
	void CalculateDimensions( PPropertyValue* a_pParameters, Cache* a_pCache ) const;

	// Size that fits a whole number of the repeated top and left edges in a_rWidth and a_rHeight ("optimise_size")
	// NOTE: the name is unknown, not in JPOG
	void CalculateOptimisedSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight ) const;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
