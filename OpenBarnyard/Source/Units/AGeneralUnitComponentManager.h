#pragma once
#include <Toshi/TSingleton.h>

#include "AUnitComponentManager.h"

// Owns components that need no central update; released components are simply deleted.
class AGeneralUnitComponentManager
    : public AUnitComponentManager
    , public Toshi::TSingleton<AGeneralUnitComponentManager>
{
public:
	TDECLARE_CLASS( AGeneralUnitComponentManager, AUnitComponentManager );

public:
	AGeneralUnitComponentManager();
	~AGeneralUnitComponentManager();

	virtual void Update( TFLOAT a_fDeltaTime ) OVERRIDE {} // +0x0c, DOES_NOTHING
	virtual void ReleaseComponent( AUnitComponent* a_pComp ) OVERRIDE;
};
