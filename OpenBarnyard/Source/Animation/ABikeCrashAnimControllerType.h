#pragma once
#include "AAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of ABikeCrashAnimController.
//-----------------------------------------------------------------------------
class ABikeCrashAnimControllerType
    : public AAnimControllerType
{
public:
	TDECLARE_CLASS( ABikeCrashAnimControllerType, AAnimControllerType );

public:
	ABikeCrashAnimControllerType();
	~ABikeCrashAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
