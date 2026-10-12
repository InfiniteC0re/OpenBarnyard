#include "pch.h"
#include "AGift.h"
#include "AUnit.h"
#include "AInteractionManager.h"

#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d1170, CreateObject 005d12f0, CreateObjectInPlace 005d1360
TDEFINE_CLASS( AGift, 2, 0 );

// NOTE: inlined into CreateObject / CreateObjectInPlace, m_fTime is left uninitialised
AGift::AGift()
{
}

// $Barnyard: FUNCTION 005d11c0
AGift::AGift( const TPString8& a_rcInteraction )
    : m_strInteraction( a_rcInteraction )
{
	m_fTime = g_oSystemManager.GetTimer().GetCurrentSeconds();
}

// $Barnyard: FUNCTION 005d13d0
// $Barnyard: FUNCTION 005d1230
AGift::~AGift()
{
}

// $Barnyard: FUNCTION 005d1180
TFLOAT AGift::GetAge()
{
	return g_oSystemManager.GetTimer().GetCurrentSeconds() - m_fTime;
}

// $Barnyard: FUNCTION 005d11a0
void AGift::ResetTime()
{
	m_fTime = g_oSystemManager.GetTimer().GetCurrentSeconds();
}

// $Barnyard: FUNCTION 005d12a0
TBOOL AGift::AssignInteraction( AUnit* a_pUnit, TFLOAT a_fUnused )
{
	if ( a_pUnit->HasInteraction() )
		return TFALSE;

	return AInteractionManager::GetSingleton()->AssignInteraction( a_pUnit, m_strInteraction ) != TNULL;
}
