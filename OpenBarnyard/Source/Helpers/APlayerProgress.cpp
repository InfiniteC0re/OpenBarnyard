#include "pch.h"
#include "APlayerProgress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

APlayerProgress::APlayerProgress()
    : m_eMiniGameFlags( 0x40380f )
{
}

APlayerProgress::~APlayerProgress()
{
}
