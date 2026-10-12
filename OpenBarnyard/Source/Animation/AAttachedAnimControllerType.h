#pragma once
#include "ACharacterAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of AAttachedAnimController.
// NOTE: retail derives it from ACharacterAnimControllerType but registers
// AAnimControllerType as the TClass parent.
//-----------------------------------------------------------------------------
class AAttachedAnimControllerType
    : public ACharacterAnimControllerType
{
public:
	TDECLARE_CLASS( AAttachedAnimControllerType, AAnimControllerType );

public:
	AAttachedAnimControllerType();
	~AAttachedAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual Toshi::TClass* GetControllerClass() OVERRIDE; // +0x10

private:
	TUINT32 m_uiUnk10; // 0x10, never accessed
};
