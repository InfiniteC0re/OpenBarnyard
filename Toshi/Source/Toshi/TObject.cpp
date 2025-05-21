#include "ToshiPCH.h"
#include "TObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_COMPILETIME( TObject, 0, 2 );

// $Barnyard: FUNCTION 006bbee0
void TObject::Delete()
{
	delete this;
}

// $Barnyard: FUNCTION 006bbed0
TObject::~TObject()
{
}

TOSHI_NAMESPACE_END
