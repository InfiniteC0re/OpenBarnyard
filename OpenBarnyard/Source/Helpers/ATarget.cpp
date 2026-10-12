#include "pch.h"
#include "ATarget.h"
#include "GUI/AGUI2.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0042c2a0, CreateObject 0042c430, CreateObjectInPlace 0042c490 (the constructor is inlined in both)
TDEFINE_CLASS( ATarget );

// $Barnyard: FUNCTION 0042c2c0
ATarget::ATarget()
    : m_vRayOrigin( 0.0f, 0.0f, 0.0f, 1.0f )
    , m_vRayDirection( 1.0f, 0.0f, 0.0f, 1.0f )
    , m_vUnk40( 0.0f, 0.0f, 0.0f, 1.0f )
    , m_bUpdateRay( TFALSE )
    , m_iRayValid( 0 )
{
}

// $Barnyard: FUNCTION 0042c360
// $Barnyard: FUNCTION 0042c4d0
ATarget::~ATarget()
{
}

// $Barnyard: FUNCTION 0042c2b0
TBOOL ATarget::OnCreate()
{
	return TTask::OnCreate();
}

// $Barnyard: FUNCTION 0042c4f0
TBOOL ATarget::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bUpdateRay == TTRUE )
		UpdateRay();

	// RETURNS_TRUE
	return TTRUE;
}

// $Barnyard: FUNCTION 0042c380
void ATarget::UpdateRay()
{
	TVector2 vCursorPos = AGUI2::GetMouseCursor().GetCursorPos();

	TVector2 vScreenPos;
	ARenderer::GetSingleton()->GUIViewportToScreen( vCursorPos, vScreenPos );

	TVector4 vOrigin;
	TVector4 vDirection;
	ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), vScreenPos, vOrigin, vDirection );

	m_vRayOrigin    = vOrigin;
	m_vRayDirection = vDirection;
	m_iRayValid     = 1;
	m_bUpdateRay    = TFALSE;
}
