#include "pch.h"
#include "AFXManager.h"
#include "ASplashManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00652fb0, CreateObject 00653070, CreateObjectInPlace 006530e0
TDEFINE_CLASS( AFXManager );

// NOTE: inlined in the create functions of the class (00653070, 006530e0)
AFXManager::AFXManager()
{
}

// $Barnyard: FUNCTION 00653010
// $Barnyard: FUNCTION 00653130
AFXManager::~AFXManager()
{
}

// $Barnyard: FUNCTION 00652fc0
TBOOL AFXManager::OnCreate()
{
	new ASplashManager();
	return TTRUE;
}

// $Barnyard: FUNCTION 00653030
TBOOL AFXManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	ASplashManager::GetSingleton()->Update( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 00653050
void AFXManager::OnDestroy()
{
	if ( ASplashManager::IsSingletonCreated() )
		delete ASplashManager::GetSingleton();
}
