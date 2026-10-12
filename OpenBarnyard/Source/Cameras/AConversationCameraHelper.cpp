#include "pch.h"
#include "AConversationCameraHelper.h"
#include "ACameraManager.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "Units/AInteraction.h"
#include "Units/AInteractionComponent.h"
#include "Units/AInteractionSequence.h"
#include "Units/AWalkActivity.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AConversationCameraHelper );

// Spline settings, only used by AConversationCameraHelper::OnCreate
static TFLOAT s_fSplineTangentScale = 3.0f;  // 0x007737e8
static TFLOAT s_fSplineSpeed        = 20.0f; // 0x007737ec

// $Barnyard: FUNCTION 004618c0
AConversationCameraHelper::AConversationCameraHelper()
{
}

// $Barnyard: FUNCTION 00461100
// $Barnyard: FUNCTION 00461920
AConversationCameraHelper::~AConversationCameraHelper()
{
	m_pOtherTarget = TNULL;
}

// $Barnyard: FUNCTION 004610e0
TBOOL AConversationCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_oSpline.Update( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 00461940
// $Barnyard: FUNCTION 00461190
void AConversationCameraHelper::OnCreate()
{
	AUnit*   pPlayer    = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	AUnit*   pOtherUnit = TNULL;
	ACamera* pCamera    = ACameraManager::GetSingleton()->GetCurrentCamera();
	TFLOAT   fRadius    = 1.7f;

	if ( pPlayer->HasInteraction() )
	{
		AInteractionSequence* pSequence = TSTATICCAST( AInteractionComponent, pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) )->GetSequence();

		if ( pSequence && pSequence->GetInteraction() )
		{
			// Find the first unit of the sequence that isn't the player
			for ( auto it = pSequence->GetComponents().Begin(); it != pSequence->GetComponents().End(); it++ )
			{
				AUnit* pOwner = it->GetOwner();

				if ( pOwner && pOwner != pPlayer )
				{
					pOtherUnit = pOwner;
					break;
				}
			}

			AInteractionActivity* pActivity = pSequence->GetInteraction()->GetActivity( 0 );

			if ( pActivity->GetClass()->IsA( &TGetClass( AWalkActivity ) ) == TTRUE )
			{
				TFLOAT fMaxRadius = TSTATICCAST( AWalkActivity, pActivity )->GetMaxRadius();
				fRadius           = fMaxRadius + fMaxRadius;
			}
		}
	}

	TVector3 vPlayerPos;
	TVector3 vOtherPos;
	pPlayer->GetSteer()->CopyPosition3( vPlayerPos );

	if ( pOtherUnit )
		pOtherUnit->GetSteer()->CopyPosition3( vOtherPos );
	else
		vOtherPos = vPlayerPos;

	TVector4 vDown( 0.0f, -1.0f, 0.0f, 1.0f );
	TFLOAT   fDistance = fRadius * 4.0f;

	TVector4 vToOther( vOtherPos.x - vPlayerPos.x, vOtherPos.y - vPlayerPos.y, vOtherPos.z - vPlayerPos.z, 1.0f );

	// The point between both units, one unit lower
	TVector4 vCenter( vOtherPos.x + vPlayerPos.x, vPlayerPos.y + vOtherPos.y, vPlayerPos.z + vOtherPos.z, 1.0f );
	vCenter.Multiply( 0.5f );
	vCenter.y = vCenter.y - 1.0f;

	// Side of the line between the units
	TVector4 vSide;
	vSide.CrossProduct( vToOther, vDown );
	vSide.Normalise();

	TVector4 vOffset;
	vOffset.Multiply( vSide, fDistance );

	TVector4 vCameraPos( vOffset.x + vCenter.x, vOffset.y + vCenter.y, vOffset.z + vCenter.z, vCenter.w );

	TVector4 vRayDir1( vCameraPos.x - vCenter.x, ( vCameraPos.y - 2.0f ) - vCenter.y, vCameraPos.z - vCenter.z, vCenter.w );
	vRayDir1.Normalise();

	TVector4 vDestination;
	TFLOAT   fHitDistance;
	ARayData oRay1( vCenter, vRayDir1, fDistance );

	if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay1, &fHitDistance, 0 ) )
	{
		// Something is in the way, try the other side
		vOffset.Multiply( vSide, fDistance );
		vCameraPos = TVector4( vCenter.x - vOffset.x, vCenter.y - vOffset.y, vCenter.z - vOffset.z, vCenter.w );

		TVector4 vRayDir2( vCameraPos.x - vCenter.x, ( vCameraPos.y - 2.0f ) - vCenter.y, vCameraPos.z - vCenter.z, vCenter.w );
		vRayDir2.Normalise();

		ARayData oRay2( vCenter, vRayDir2, fDistance );

		if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( pPlayer, oRay2, fHitDistance, 0 ) )
		{
			// Both sides are blocked, place the camera behind the player
			TVector4 vDirection = *pPlayer->GetSteer()->GetDirection();
			vDestination        = *pPlayer->GetSteer()->GetPosition();
			vDirection.AsVector3().RotateY( 10.0f );

			vOffset.Multiply( vDirection, fDistance );
			vDestination.x = vDestination.x - vOffset.x;
			vDestination.y = vDestination.y - vOffset.y;
			vDestination.z = vDestination.z - vOffset.z;
		}
		else
		{
			vOffset.Multiply( vRayDir2, fDistance );
			vDestination = TVector4( vOffset.x + vCenter.x, vOffset.y + vCenter.y, vOffset.z + vCenter.z, vCenter.w );
		}
	}
	else
	{
		vOffset.Multiply( vRayDir1, fDistance );
		vDestination = TVector4( vOffset.x + vCenter.x, vOffset.y + vCenter.y, vOffset.z + vCenter.z, vCenter.w );
	}

	TVector4 vLookDir( vCenter.x - vDestination.x, vCenter.y - vDestination.y, vCenter.z - vDestination.z, vCenter.w );
	vLookDir.Normalise();

	m_oSpline.Setup( pCamera, vDestination, vLookDir, s_fSplineTangentScale, s_fSplineSpeed );
}
