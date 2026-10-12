#pragma once
#include "AHudElement.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// The jar of the Honey Collecting minigame (AHoneyCollectingGame) that fills
// up with the honey of the hives. Shows the royal jelly when it was found.
// TClass 00782050 (parent AHudElement), vtables 0071c2e8, 0071c2a0 (T2GUIElement),
// size 0x3a4.
//-----------------------------------------------------------------------------
class AHoneyCountHudElement
    : public AHudElement
    , public Toshi::T2GUIElement
{
public:
	TDECLARE_CLASS( AHoneyCountHudElement, AHudElement );

public:
	AHoneyCountHudElement();
	~AHoneyCountHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() OVERRIDE; // +0x14
	virtual void  OnDetach() OVERRIDE; // +0x18
	virtual void  Show() OVERRIDE;     // +0x1c
	virtual void  Hide() OVERRIDE;     // +0x20

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() OVERRIDE; // +0x0c

	// Sets the height of the honey in the jar
	void SetHoney( TFLOAT a_fHeight );

	// Raises the honey in the jar by a_fHeight (up to 85% of the jar)
	void AddHoney( TFLOAT a_fHeight );

	// Whether the honey reached 85% of the jar
	TBOOL IsFull();

	void SetRoyalJellyVisible( TBOOL a_bVisible ) { m_bRoyalJelly = a_bVisible; }

private:
	TUINT8                m_aUnk68[ 16 ]; // 0x068
	TINT                  m_iUnk78;       // 0x078
	Toshi::T2GUIElement   m_oJarRoot;     // 0x07c
	Toshi::T2GUIRectangle m_oShadow;      // 0x0d0, black
	Toshi::T2GUIRectangle m_oJar;         // 0x148, "Jar_Bkg"
	Toshi::T2GUIRectangle m_oJarMask;     // 0x1c0, "Jar_Mask"
	Toshi::T2GUIRectangle m_oHoney;       // 0x238, "Honey_Rectangle"
	Toshi::T2GUIRectangle m_oGlint;       // 0x2b0, "Jar_Glint"
	Toshi::T2GUIRectangle m_oRoyalJelly;  // 0x328, "Royal_Jelly"
	TBOOL                 m_bRoyalJelly;  // 0x3a0
};

TSTATICASSERT( sizeof( AHoneyCountHudElement ) == 0x3a4 );
