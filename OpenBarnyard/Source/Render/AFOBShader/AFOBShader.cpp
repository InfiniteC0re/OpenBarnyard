#include "pch.h"
#include "AFOBShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: only the TClass object exists in PC retail (no GetClass, see AFOBShader.h)
TDEFINE_CLASS_NORUNTIME( AFOBShader );

TSTATICASSERT( sizeof( AFOBShader ) == 0x18 );
