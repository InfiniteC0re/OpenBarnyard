#include "pch.h"
#include "ALoadTimeProfiler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004024a0, CreateObject 00402670, CreateObjectInPlace 004026d0
TDEFINE_CLASS( ALoadTimeProfiler );

// $Barnyard: FUNCTION 00402520
ALoadTimeProfiler::ALoadTimeProfiler()
{
	m_pTimer = TNULL;
	m_iUnk1  = 0;
	m_iUnk2  = 0;
	m_pTimer = new THPTimer();
	m_bUnk4  = TFALSE;
}

// $Barnyard: FUNCTION 004025d0
// $Barnyard: FUNCTION 00402710
ALoadTimeProfiler::~ALoadTimeProfiler()
{
	if ( m_pTimer )
		delete m_pTimer;

	m_pTimer = TNULL;

	// NOTE: the original deletes the buffer of m_aProfiles here with delete[] (destroying the strings), TArray only frees it
}
