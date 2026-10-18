#include "ToshiPCH.h"
#include "TGUISoundFactory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (00369750)
TDEFINE_CLASS_NORUNTIME( TGUISoundFactory );

// $NicktoonsPS2: FUNCTION 00369770
// $JPOG: FUNCTION TGuiInterface.dll+000158e0
TGUISoundFactory::TGUISoundFactory()
{
}

// $NicktoonsPS2: FUNCTION 003697a8
// $JPOG: FUNCTION TGuiInterface.dll+000158d0
TGUISoundFactory::~TGUISoundFactory()
{
}

TOSHI_NAMESPACE_END
