#pragma once
#include "ACharacterAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of AAlienAnimController.
//-----------------------------------------------------------------------------
class AAlienAnimControllerType
    : public ACharacterAnimControllerType
{
public:
	TDECLARE_CLASS( AAlienAnimControllerType, ACharacterAnimControllerType );

public:
	AAlienAnimControllerType();
	~AAlienAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
