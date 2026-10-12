#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIControl.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIButton : AGUI2XUIControl
//-----------------------------------------------------------------------------
class AGUI2XUIButton
    : public AGUI2XUIControl
{
public:
	TDECLARE_CLASS( AGUI2XUIButton, AGUI2XUIControl );

public:
	AGUI2XUIButton();
	~AGUI2XUIButton();

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIControl
	//-----------------------------------------------------------------------------
	virtual void Press() override;
};

#endif // BARNYARD_LATEST
