#include "pch.h"
#include "AWorldPositionActivityMessage.h"
#include "AInteractionActivityMessageGroups.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c8880, CreateObject 005c8aa0, CreateObjectInPlace 005c8b00
TDEFINE_CLASS( AWorldPositionActivityMessage );

// $Barnyard: FUNCTION 005c8910
AWorldPositionActivityMessage::AWorldPositionActivityMessage()
    : m_vPosition( TVector3::VEC_ZERO ) // NOTE: retail copies the zero vector at 0x007d2f44
    , m_vDirection( TVector3::VEC_ZERO )
{
}

// $Barnyard: FUNCTION 005c8960
// $Barnyard: FUNCTION 005c8890
AWorldPositionActivityMessage::~AWorldPositionActivityMessage()
{
}

// $Barnyard: FUNCTION 005c8d00
void AWorldPositionActivityMessage::CreateGroup( AInteractionActivityMessageGroups* a_pGroups )
{
	a_pGroups->CreateGroup<AWorldPositionActivityMessage>();
}

// AInteractionActivityMessageGroup<AWorldPositionActivityMessage> (template instance):
// pool ctor (message array + free list)
// $Barnyard: FUNCTION 005c8980
// pool dtor
// $Barnyard: FUNCTION 005c88a0
// ctor
// $Barnyard: FUNCTION 005c8b40
// Allocate
// $Barnyard: FUNCTION 005c8bb0
// Free
// $Barnyard: FUNCTION 005c8c00
// dtor, scalar deleting dtor
// $Barnyard: FUNCTION 005c8c30
// $Barnyard: FUNCTION 005c8c10
// AInteractionActivityMessageGroups::CreateGroup<AWorldPositionActivityMessage>
// $Barnyard: FUNCTION 005c8c80
