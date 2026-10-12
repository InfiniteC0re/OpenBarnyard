#include "pch.h"
#include "AUnitRef.h"
#include "AUnit.h"
#include "AUnitNameManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0040e080
AUnit* AUnitRef::GetUnit()
{
	if ( oProxy.Get() && oProxy->GetUnit() )
		return oProxy->GetUnit();

	if ( !szName )
		return TNULL;

	AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( TPString8( szName ) );

	if ( pUnit )
		oProxy = pUnit->GetSelfProxy();

	return pUnit;
}
