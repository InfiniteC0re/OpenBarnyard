#pragma once
#include "AAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of ABikeAnimController.
//-----------------------------------------------------------------------------
class ABikeAnimControllerType
    : public AAnimControllerType
{
public:
	TDECLARE_CLASS( ABikeAnimControllerType, AAnimControllerType );

public:
	ABikeAnimControllerType();
	~ABikeAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
