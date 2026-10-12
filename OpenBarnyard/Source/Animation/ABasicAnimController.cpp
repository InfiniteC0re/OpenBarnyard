#include "pch.h"
#include "ABasicAnimController.h"
#include "ABasicAnimControllerType.h"
#include "ACharacterAnimController.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00586580, CreateObject 00586660, CreateObjectInPlace 005866d0
TDEFINE_CLASS( ABasicAnimController );

ABasicAnimController::ABasicAnimController()
{
}

// $Barnyard: FUNCTION 00586740
// $Barnyard: FUNCTION 00586590
ABasicAnimController::~ABasicAnimController()
{
}

// $Barnyard: FUNCTION 00586760
TBOOL ABasicAnimController::OnCreate()
{
	if ( TSTATICCAST( ABasicAnimControllerType, m_pControllerType )->IsUserRender() )
		m_pAnimatableObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetPreRenderCallback( ACharacterAnimController::UserRenderCallback );

	return AAnimController::OnCreate();
}

// $Barnyard: FUNCTION 005867b0
// $Barnyard: FUNCTION 00586980
void ABasicAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_pOwner || m_pAnimatableObject->GetParentObject() )
		return;

	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	if ( !pSteer )
		return;

	TVector4 vecDirection;
	pSteer->CopyDirection( vecDirection );

	if ( m_pAnimatableObject->GetModelInstance()->IsFacingDifferent( vecDirection ) )
	{
		TVector4 vecPosition;
		pSteer->CopyPosition( vecPosition );

		// The default up vector can't be used when facing straight up or down
		if ( vecDirection.y <= 0.999f && vecDirection.y >= -0.999f )
			m_pAnimatableObject->GetModelInstance()->SetTransform( vecPosition, vecDirection );
		else
			m_pAnimatableObject->GetModelInstance()->SetTransformKeepRight( vecPosition, vecDirection );
	}
	else
	{
		// Same direction, only update the position
		TVector4 vecPosition;
		pSteer->CopyPosition( vecPosition );

		m_pAnimatableObject->GetModelInstance()->SetTranslation( vecPosition.AsVector3() );
	}
}
