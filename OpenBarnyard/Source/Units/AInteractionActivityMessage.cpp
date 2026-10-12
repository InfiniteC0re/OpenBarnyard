#include "pch.h"
#include "AInteractionActivityMessage.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a0000
TDEFINE_CLASS_NORUNTIME( AInteractionActivityMessage );

// $Barnyard: FUNCTION 005a0040
AInteractionActivityMessage::AInteractionActivityMessage()
    : m_pSequence( TNULL )
{
}

// $Barnyard: FUNCTION 005a0090
// $Barnyard: FUNCTION 005a0070
AInteractionActivityMessage::~AInteractionActivityMessage()
{
}

// $Barnyard: FUNCTION 005a0100
void AInteractionActivityMessage::Reset()
{
	m_strName   = TPString8();
	m_pSequence = TNULL;
}

// $Barnyard: FUNCTION 005a0010
void AInteractionActivityMessage::SetName( const TPString8& a_rcName )
{
	m_strName = a_rcName;
}
