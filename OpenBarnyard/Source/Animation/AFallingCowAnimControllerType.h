#pragma once
#include "AAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of AFallingCowAnimController.
//-----------------------------------------------------------------------------
class AFallingCowAnimControllerType
    : public AAnimControllerType
{
public:
	TDECLARE_CLASS( AFallingCowAnimControllerType, AAnimControllerType );

public:
	AFallingCowAnimControllerType();
	~AFallingCowAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
