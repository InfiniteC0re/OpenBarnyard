#pragma once
#include "AAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of ADartThrowAnimController.
//-----------------------------------------------------------------------------
class ADartThrowAnimControllerType
    : public AAnimControllerType
{
public:
	TDECLARE_CLASS( ADartThrowAnimControllerType, AAnimControllerType );

public:
	ADartThrowAnimControllerType();
	~ADartThrowAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
