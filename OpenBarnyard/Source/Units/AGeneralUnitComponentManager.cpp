#include "pch.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnitComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0041a650, CreateObject 0041a6c0, CreateObjectInPlace 0041a730 (the constructor is inlined in both)
TDEFINE_CLASS( AGeneralUnitComponentManager );

// $Barnyard: FUNCTION 0041a680
AGeneralUnitComponentManager::AGeneralUnitComponentManager()
{
}

// $Barnyard: FUNCTION 0041a780
// $Barnyard: FUNCTION 0041a6a0
AGeneralUnitComponentManager::~AGeneralUnitComponentManager()
{
}

// $Barnyard: FUNCTION 0041a660
void AGeneralUnitComponentManager::ReleaseComponent( AUnitComponent* a_pComp )
{
	if ( a_pComp )
		delete a_pComp;
}
