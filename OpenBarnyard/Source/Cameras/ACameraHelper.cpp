#include "pch.h"
#include "ACameraHelper.h"
#include "ACameraManager.h"

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

// $Barnyard: FUNCTION 0045b660
void ACameraHelper::UpdateCurrentCamera()
{
	m_pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
}

// $Barnyard: FUNCTION 0045b6a0
void ACameraHelper::GetCameraRotation( Toshi::TVector4& a_rOutRotation ) const
{
	a_rOutRotation   = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	a_rOutRotation.w = 1.0f;
}

void ACameraHelper::Unk7( void* )
{
}

void ACameraHelper::OnDetach()
{
}

TFLOAT ACameraHelper::GetFOV() const
{
	return ACamera::sm_fCurrentFOV;
}
