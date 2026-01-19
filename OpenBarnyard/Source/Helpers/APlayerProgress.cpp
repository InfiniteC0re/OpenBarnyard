#include "pch.h"
#include "APlayerProgress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

APlayerProgress::APlayerProgress( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4 )
    : m_eMiniGameFlags( 0b00000000010000000011100000001111 )
{
}

APlayerProgress::~APlayerProgress()
{
}
