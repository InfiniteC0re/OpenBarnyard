#include "pch.h"
#include "AHashedObjectAuto.h"
#include "AObjectHashMain.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061f0b0
AHashedObjectAuto::AHashedObjectAuto()
{
}

// $Barnyard: FUNCTION 0061f740
// $Barnyard: FUNCTION 0061f710
AHashedObjectAuto::~AHashedObjectAuto()
{
	AHashedObjectAuto::RemoveHash();
}

// $Barnyard: FUNCTION 0061f6e0
void AHashedObjectAuto::RemoveHash()
{
	if ( AObjectHashMain::IsSingletonCreated() )
	{
		AObjectHashMain::GetSingleton()->RemoveHash( this );
	}
}
