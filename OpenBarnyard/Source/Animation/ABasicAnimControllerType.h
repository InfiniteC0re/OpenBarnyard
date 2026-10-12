#pragma once
#include "ACharacterAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of ABasicAnimController.
// NOTE: retail derives it from ACharacterAnimControllerType (it shares its
// Create and UserRender) but registers AAnimControllerType as the TClass parent.
//-----------------------------------------------------------------------------
class ABasicAnimControllerType
    : public ACharacterAnimControllerType
{
public:
	TDECLARE_CLASS( ABasicAnimControllerType, AAnimControllerType );

public:
	ABasicAnimControllerType();
	~ABasicAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10
};
