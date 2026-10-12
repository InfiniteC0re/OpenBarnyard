#pragma once
#include "ACharacterAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of AChasingChicksAnimController.
// NOTE: retail derives it from ACharacterAnimControllerType (it shares its
// Create and UserRender) but registers AAnimControllerType as the TClass parent.
//-----------------------------------------------------------------------------
class AChasingChicksAnimControllerType
    : public ACharacterAnimControllerType
{
public:
	TDECLARE_CLASS( AChasingChicksAnimControllerType, AAnimControllerType );

public:
	AChasingChicksAnimControllerType();
	~AChasingChicksAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
