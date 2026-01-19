#include "pch.h"
#include "AFSMKeyWord.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006c1a20
AFSMKeyWord::AFSMKeyWord( const TCHAR* a_pchName )
    : pchName( a_pchName )
    , pNext( TNULL )
{
	if ( ms_pHead == TNULL )
	{
		ms_pHead = this;
		ms_pTail = this;
	}
	else
	{
		ms_pTail->pNext = this;
		ms_pTail        = this;
	}
}

AFSMKeyWord::~AFSMKeyWord()
{
}
