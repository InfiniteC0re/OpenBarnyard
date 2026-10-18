#include "ToshiPCH.h"
#include "TGUIAudioContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (00363510)
TDEFINE_CLASS_NORUNTIME( TGUIAudioContext );

// $NicktoonsPS2: FUNCTION 00363530
// $JPOG: FUNCTION TGuiInterface.dll+000088a0
TGUIAudioContext::TGUIAudioContext()
{
}

// $NicktoonsPS2: FUNCTION 00363568
// $JPOG: FUNCTION TGuiInterface.dll+00008890
TGUIAudioContext::~TGUIAudioContext()
{
}

TOSHI_NAMESPACE_END
