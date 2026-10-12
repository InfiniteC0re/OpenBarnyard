#include "pch.h"
#include "APlaceholderRequirement.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005adb80
TDEFINE_CLASS_NORUNTIME( APlaceholderRequirement );

// $Barnyard: FUNCTION 005adbc0
APlaceholderRequirement::APlaceholderRequirement( TPString8 a_strName )
{
	m_strName = a_strName;
}

// $Barnyard: FUNCTION 005adb90
// $Barnyard: FUNCTION 005adba0
APlaceholderRequirement::~APlaceholderRequirement()
{
}
