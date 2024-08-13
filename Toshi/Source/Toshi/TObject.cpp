#include "ToshiPCH.h"
#include "TObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_COMPILETIME( TObject, 0, 2 );

void TObject::Delete()
{
	delete this;
}

TObject::~TObject()
{
}

TOSHI_NAMESPACE_END
