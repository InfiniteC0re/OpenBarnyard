#include "ToshiPCH.h"
#include "TObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{

TDEFINE_CLASS_COMPILETIME( TObject, 0, 2 );

void TObject::Delete()
{
    delete this;
}

TObject::~TObject()
{
}

} // namespace Toshi
