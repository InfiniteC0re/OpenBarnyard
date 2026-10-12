#include "pch.h"
#include "AUnitProxy.h"
#include "AUIDMap.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040d800, CreateObject 0040db30, CreateObjectInPlace 0040d960 (the constructor is inlined in both)
TDEFINE_CLASS( AUnitProxy );

// $Barnyard: FUNCTION 0040d920
AUnitProxy::AUnitProxy()
{
	// m_pData/m_iId/m_pOwner are filled in when the proxy is bound to an id.
}

// $Barnyard: FUNCTION 0040d810
// $Barnyard: FUNCTION 0040d830
AUnitProxy::~AUnitProxy()
{
}

// $Barnyard: FUNCTION 0040df00
void AUnitProxy::Delete()
{
	m_pOwner->DeleteProxy( this );
}
