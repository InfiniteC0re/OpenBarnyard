#include "pch.h"
#include "ASquirtActivity.h"
#include "Player/APlayerProfileManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c19f0, CreateObject 005c1a10, CreateObjectInPlace 005c1a70
TDEFINE_CLASS( ASquirtActivity );

// NOTE: inlined in CreateObject
ASquirtActivity::ASquirtActivity()
{
}

// $Barnyard: FUNCTION 005c1a00
// $Barnyard: FUNCTION 005c1ac0
ASquirtActivity::~ASquirtActivity()
{
}

// $Barnyard: FUNCTION 005c1ae0
AInteractionActivity::RESULT ASquirtActivity::Update( AInteractionSequence* a_pSequence )
{
	APlayerProfileManager::GetSingleton()->GetMainProfile()->ClearSquirtedUnits();
	return RESULT_DONE;
}
