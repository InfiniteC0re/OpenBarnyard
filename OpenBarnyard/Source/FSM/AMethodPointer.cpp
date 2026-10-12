#include "pch.h"
#include "AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005c9240
AMethodPointer::AMethodPointer( const TCHAR* a_szName )
    : AFunction( a_szName )
{
}

// $Barnyard: FUNCTION 005c9260
AMethodPointer::AMethodPointer( const AMethodPointer& a_rOther )
    : AFunction( a_rOther )
{
}

// $Barnyard: FUNCTION 005c92f0
// $Barnyard: FUNCTION 005c9280
AMethodPointer::~AMethodPointer()
{
}

// NOTE: the TArray<const TClass*> methods used by AFunction::GetSignature (GrowBy 00410ad0, Resize 00410b10) are marked in TArray.h
