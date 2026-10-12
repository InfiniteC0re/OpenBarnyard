#include "ToshiPCH.h"
#include "TObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006bbcc0, CreateObject 006bbef0, CreateObjectInPlace 006bbf10
TDEFINE_CLASS_COMPILETIME( TObject, 0, 2 );

// $Barnyard: FUNCTION 006bbee0
void TObject::Delete()
{
	delete this;
}

// $Barnyard: FUNCTION 006bbed0
// $Barnyard: FUNCTION 00570620
TObject::~TObject()
{
}

TOSHI_NAMESPACE_END
