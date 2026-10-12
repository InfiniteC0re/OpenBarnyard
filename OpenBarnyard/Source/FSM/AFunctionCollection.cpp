#include "pch.h"
#include "AFunctionCollection.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c8f80, CreateObject 005c9020, CreateObjectInPlace 005c9080
TDEFINE_CLASS( AFunctionCollection );

// $Barnyard: FUNCTION 005c8fa0
AFunctionCollection::AFunctionCollection()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apUnknown ); i++ )
		m_apUnknown[ i ] = TNULL;
}

// $Barnyard: FUNCTION 005c8f90
// $Barnyard: FUNCTION 005c9000
AFunctionCollection::~AFunctionCollection()
{
}
