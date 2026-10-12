#include "pch.h"
#include "AInventoryComponentInterface.h"
#include "AInventoryComponent.h"
#include "AInventoryGroupComponent.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPString8 AInventoryComponentInterface::ms_strDefaultInventory;

// $Barnyard: FUNCTION 00622d80
// $Barnyard: FUNCTION 00622d90
AInventoryComponentInterface::~AInventoryComponentInterface()
{
}

// NOTE: folded getter of AGeneralUnitComponentManager::ms_pSingleton (005d2300)
AUnitComponentManager* AInventoryComponentInterface::GetManager()
{
	return AGeneralUnitComponentManager::GetSingleton();
}

// $Barnyard: FUNCTION 00625a30
const TPString8& AInventoryComponentInterface::GetName()
{
	static TPString8 s_strName;
	return s_strName;
}

// $Barnyard: FUNCTION 00623490
AInventoryComponentInterface* AInventoryComponentInterface::GetInventory( AUnit* a_pUnit )
{
	AUnitComponent* pInventory = a_pUnit->FindComponentExact( &TGetClass( AInventoryGroupComponent ) );

	if ( !pInventory )
		pInventory = a_pUnit->FindComponentExact( &TGetClass( AInventoryComponent ) );

	return TSTATICCAST( AInventoryComponentInterface, pInventory );
}
