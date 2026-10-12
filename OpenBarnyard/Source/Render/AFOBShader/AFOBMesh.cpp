#include "pch.h"
#include "AFOBMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: only the TClass object exists in PC retail (no GetClass, see AFOBShader.h)
TDEFINE_CLASS_NORUNTIME( AFOBMesh );

TSTATICASSERT( sizeof( AFOBMesh ) == 0x2C );
