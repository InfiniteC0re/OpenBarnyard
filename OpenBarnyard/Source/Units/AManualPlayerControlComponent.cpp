#include "pch.h"
#include "AManualPlayerControlComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058cff0, CreateObject 0058d000, CreateObjectInPlace 0058d060
TDEFINE_CLASS( AManualPlayerControlComponent );

AManualPlayerControlComponent::AManualPlayerControlComponent()
{
}

// $Barnyard: FUNCTION 0058cfc0
// $Barnyard: FUNCTION 0058cfe0
AManualPlayerControlComponent::~AManualPlayerControlComponent()
{
}

// $Barnyard: FUNCTION 0058d0b0
void AManualPlayerControlComponent::GetCameraForward( TVector4& a_rOutForward )
{
	a_rOutForward = TVector4::VEC_POSZ;
}
