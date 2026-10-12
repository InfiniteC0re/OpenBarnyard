#include "pch.h"
#include "AUnitTransform.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00412ee0
AUnitTransform::AUnitTransform( const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iUnk1, TINT a_iModelIndex, TINT a_iUnk3, TBOOL a_bCreateObstruction )
    : m_vecPosition( a_rcPosition )
    , m_vecDirection( a_rcDirection )
    , m_iUnk20( a_iUnk1 )
    , m_iModelIndex( a_iModelIndex )
    , m_iUnk28( a_iUnk3 )
    , m_bCreateObstruction( a_bCreateObstruction )
{
}
