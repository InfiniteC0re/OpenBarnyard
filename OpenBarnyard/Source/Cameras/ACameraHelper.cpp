#include "pch.h"
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ACameraHelper );

ACameraHelper::ACameraHelper()
{
}

ACameraHelper::~ACameraHelper()
{
}

void ACameraHelper::UpdateCurrentCamera()
{
    TIMPLEMENT();
}

void ACameraHelper::GetCameraPosition( Toshi::TVector4& a_rPosition ) const
{
    TIMPLEMENT();
}

void ACameraHelper::Unk7( void* )
{
}

TFLOAT ACameraHelper::GetFOV() const
{
    return ACamera::sm_fCurrentFOV;
}
