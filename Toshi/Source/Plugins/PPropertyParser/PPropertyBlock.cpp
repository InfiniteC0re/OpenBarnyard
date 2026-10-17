#include "ToshiPCH.h"
#include "PPropertyBlock.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// NOTE: abstract, CreateObject returns TNULL in Nicktoons
TDEFINE_CLASS_NORUNTIME( PPropertyBlock );

// $NicktoonsPS2: FUNCTION 0038a360
// $JPOG: FUNCTION PPropertyParser.dll+00002ae0
PPropertyBlock::PPropertyBlock( const TPString8& a_rcName, Priority a_ePriority )
    : m_strName( a_rcName )
    , m_ePriority( a_ePriority )
{
}

// $NicktoonsPS2: FUNCTION 0038a3d8
// $JPOG: FUNCTION PPropertyParser.dll+00002b80
PPropertyBlock::~PPropertyBlock()
{
}
