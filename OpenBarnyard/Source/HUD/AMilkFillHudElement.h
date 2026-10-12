#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// A glass (BarnTender) that fills up with milk: a back plate, the milk drawn
// through the glass mask and a glint on top, sized after the "Milk_Bottle"
// section. Plain T2GUIElement (no TClass), size 0x38c.
// vtable 0071c4a0: +0x00 dtor 00456510, +0x04 Tick 00456380, +0x0c Render 00456390,
// +0x48 Create 00456530, +0x4c Destroy 004562d0.
// Name from the Wii build (AMilkFillHudElement).
//-----------------------------------------------------------------------------
class AMilkFillHudElement
    : public Toshi::T2GUIElement
{
public:
	AMilkFillHudElement();
	~AMilkFillHudElement();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create();

	// Detaches the element and its parts
	virtual void Destroy();

	// Fills the glass to a_fValue / a_fMax (90% of the glass height when full)
	void SetFill( TFLOAT a_fValue, TFLOAT a_fMax );

public:
	Toshi::TGenericListener m_oListener;   // 0x54, unused
	Toshi::T2GUIElement     m_oGlass;      // 0x68, hidden, rendered by Render
	Toshi::T2GUIRectangle   m_oBackPlate;  // 0xbc, "Glass_Back_Plate"
	Toshi::T2GUIRectangle   m_oMask;       // 0x134, "Glass_Mask"
	Toshi::T2GUIRectangle   m_oMilk;       // 0x1ac, "Glass_Milk", child of m_oMask
	Toshi::T2GUIRectangle   m_oGlint;      // 0x224, "Glass_Glint"
	Toshi::T2GUIRectangle   m_oBackground; // 0x29c, transparent, rendered first by Render
	Toshi::T2GUIRectangle   m_oUnused;     // 0x314
};

TSTATICASSERT( sizeof( AMilkFillHudElement ) == 0x38c );
