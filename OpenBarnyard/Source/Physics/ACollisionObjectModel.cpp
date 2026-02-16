#include "pch.h"
#include "ACollisionObjectModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061e810
ACollisionObjectModel::ACollisionObjectModel()
    : m_vUnk1( TVector4::VEC_NEGY )
    , m_iUnk2( 0 )
{
	m_eType = Type_Simple;
}

// $Barnyard: FUNCTION 0061e8a0
// $Barnyard: FUNCTION 0061edf0
// $Barnyard: FUNCTION 0061e8b0
ACollisionObjectModel::~ACollisionObjectModel()
{
}
