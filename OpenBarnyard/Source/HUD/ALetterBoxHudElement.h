#pragma once
#include "AHudElement.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// The black bars above and below the screen while a scripted sequence plays (ASSeqState).
// TClass 0x007820e0, vtable 0x0071c40c.
//-----------------------------------------------------------------------------
class ALetterBoxHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ALetterBoxHudElement, AHudElement );

public:
	ALetterBoxHudElement();
	~ALetterBoxHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;

private:
	Toshi::T2GUIRectangle m_oTopBar;    // 0x14
	Toshi::T2GUIRectangle m_oBottomBar; // 0x8c
};

TSTATICASSERT( sizeof( ALetterBoxHudElement ) == 0x104 );
