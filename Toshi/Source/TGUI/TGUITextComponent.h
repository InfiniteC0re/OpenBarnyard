#pragma once
#include "TGUIComponent.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: "Multiline Text" component: the label of its element broken into lines that
// fit the rectangle (a TGUIFormattedText kept in the TGUIComponentCache).
//-----------------------------------------------------------------------------
class TGUITextComponent : public TGUIComponent
{
public:
	TDECLARE_CLASS( TGUITextComponent, TGUIComponent );

	enum Element
	{
		Element_Text,
		Element_NUMOF
	};

public:
	TGUITextComponent();
	TGUITextComponent( const TGUITextComponent& a_rcOther );
	~TGUITextComponent();

	virtual TGUITextComponent*       Clone() const override;
	virtual TBOOL                    GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const override;
	virtual const TGUIComponentInfo* GetComponentInfo() const override;
	virtual TGUIElement*             GetElements() override;
	virtual void                     Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const override;
	virtual void                     DestroyCache( void* a_pData ) const override;

protected:
	// Formats the text in the rectangle (inside the margins)
	TGUIFormattedText* BuildCache( PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect ) const;

public:
	static TGUIComponentInfo ms_component_info;

private:
	TGUIElement m_aElements[ Element_NUMOF ]; // 0x20
};

TOSHI_NAMESPACE_END
