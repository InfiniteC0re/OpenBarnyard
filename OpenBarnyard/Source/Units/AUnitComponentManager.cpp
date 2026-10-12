#include "pch.h"
#include "AUnitComponentManager.h"
#include "AGeneralUnitComponentManager.h"
#include "AControlComponentManager.h"
#include "AInteractionComponentManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AUnitComponentManager );
// NOTE: GetClass 0041b570, CreateObject 0041b720, CreateObjectInPlace 0041b780
TDEFINE_CLASS( AUnitComponentManagerList );

// $Barnyard: FUNCTION 0041b530
AUnitComponentManager::AUnitComponentManager()
{
}

// $Barnyard: FUNCTION 0041b550
// $Barnyard: FUNCTION 0041b520
AUnitComponentManager::~AUnitComponentManager()
{
}

// $Barnyard: FUNCTION 0041b580
AUnitComponentManagerList::AUnitComponentManagerList()
{
	m_llManagers.InsertTail( new AGeneralUnitComponentManager() );
	m_llManagers.InsertTail( new AInteractionComponentManager() );
	m_llManagers.InsertTail( new AControlComponentManager() );
}

// $Barnyard: FUNCTION 0041b820
// $Barnyard: FUNCTION 0041b7c0
AUnitComponentManagerList::~AUnitComponentManagerList()
{
	m_llManagers.DeleteAll();
}

// $Barnyard: FUNCTION 0041b6c0
void AUnitComponentManagerList::Update( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_llManagers.Begin(); it != m_llManagers.End(); it++ )
		it->Update( a_fDeltaTime );
}
