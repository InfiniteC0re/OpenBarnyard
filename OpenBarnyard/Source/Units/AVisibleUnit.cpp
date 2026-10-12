#include "pch.h"
#include "AVisibleUnit.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "AUnitType.h"
#include "AUnitComponent.h"
#include "AUnitCollisionComponentData.h"
#include "AUnitTransform.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Physics/ACollisionObjectMesh.h"
#include "Physics/ACollisionObjectUnit.h"
#include "AObjectTraits.h"
#include "AKinematicsObstructionUnit.h"
#include "Sound/ASoundManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/Steering/ASteer.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail registers it with size 0x94 and alignment 1
TDEFINE_CLASS_INIT_NORUNTIME( AVisibleUnit, 1, 1 );

AFunctionTable AVisibleUnit::ms_oFunctionTable( &TGetClass( AVisibleUnit ), &AUnit::ms_oFunctionTable );

// $Barnyard: FUNCTION 00415540
void AVisibleUnit::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AVisibleUnit>::Create( "IsInCameraFrustum", &AVisibleUnit::IsInCameraFrustum ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AVisibleUnit, TFLOAT>::Create( "IsInCameraFrustumDistance", &AVisibleUnit::IsInCameraFrustumDistance ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AVisibleUnit, TBOOL>::Create( "SetVisible", &AVisibleUnit::SetVisible ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AVisibleUnit>::Create( "IsVisible", &AVisibleUnit::IsVisible ) );
}

// $Barnyard: FUNCTION 00414e60
void AVisibleUnit::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00415860
AVisibleUnit::AVisibleUnit()
    : m_pCollisionObject( TNULL )
    , m_fThinkTimer( 0.0f )
    , m_fGoalYOffset( 0.0f )
    , m_iLOD( 0 )
{
	m_eFlags &= ~FLAGS_MILK_TARGET;
}

// $Barnyard: FUNCTION 004159c0
// $Barnyard: FUNCTION 00415910
// $Barnyard: FUNCTION 00415900
AVisibleUnit::~AVisibleUnit()
{
	// NOTE: 00415900 is the this-adjusting thunk of the secondary (+0x40) vtable.
	// m_oLoopSounds, m_oModelInstanceListener (disconnects) and the goal interface
	// (removes itself from AInteriorNetworkManager) are torn down by their destructors.
}

// $Barnyard: FUNCTION 004158f0
AFunctionTable* AVisibleUnit::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00414eb0
void AVisibleUnit::SetSimLOD( TINT a_iLOD )
{
	TINT iOldLOD = m_iLOD;

	if ( a_iLOD != iOldLOD )
	{
		m_iLOD = a_iLOD;
		m_pSteer->SetLOD( iOldLOD, a_iLOD );
		OnLODChange();
	}
}

// $Barnyard: FUNCTION 00414ee0
TINT AVisibleUnit::GetSimLOD()
{
	return m_iLOD;
}

// $Barnyard: FUNCTION 004155d0
TBOOL AVisibleUnit::Create( AUnitTransform* a_pTransform )
{
	if ( a_pTransform->m_bCreateObstruction == TTRUE )
	{
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_pType->FindStaticData( &TGetClass( AObjectTraits ) ) );

		if ( !pTraits || pTraits->IsObstruction() )
			m_pObstruction = new AKinematicsObstructionUnit( GetSelfProxy() );
	}

	if ( !AUnit::Create( a_pTransform ) )
		return TFALSE;

	AModelInstance* pModelInstance = GetModelInstance();

	// NOTE: same code as ConnectModelInstance, inlined in retail
	m_oModelInstanceListener.Disconnect();
	m_oModelInstanceListener.Connect( pModelInstance->GetChangeEmitter(), this, OnModelInstanceChangeCallback, 0 );

	AUnitCollisionComponentData* pCollisionData = TSTATICCAST( AUnitCollisionComponentData, m_pType->FindStaticData( &TGetClass( AUnitCollisionComponentData ) ) );

	if ( pCollisionData )
	{
		if ( pCollisionData->m_iCollideFlags == ACollisionObject::TYPE_MESH )
		{
			m_pCollisionObject = new ACollisionObjectMesh();

			TTransformObject& rTransform = GetModelInstance()->GetTransform();
			rTransform.SetMode( TTransformObject::Mode::Matrix );

			TMatrix44 matIdentity;
			matIdentity.Identity();
			rTransform.SetMatrix( matIdentity );

			TSTATICCAST( ACollisionObjectMesh, m_pCollisionObject )->Create( GetModelInstance()->GetSceneObject() );
		}
		else
		{
			ACollisionObjectUnit* pCollisionObject = new ACollisionObjectUnit();
			m_pCollisionObject                     = pCollisionObject;

			pCollisionObject->Create( this, pModelInstance, pCollisionData->m_bIgnoreObjectCollision );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00415990
void AVisibleUnit::OnDestroy()
{
	m_oModelInstanceListener.Disconnect();
	StopAllLoopSounds();

	if ( m_pCollisionObject )
	{
		delete m_pCollisionObject;
		m_pCollisionObject = TNULL;
	}

	AUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00414ef0
void AVisibleUnit::UpdateAI( TFLOAT a_fDeltaTime )
{
	AUnit::UpdateAI( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00415050
TBOOL AVisibleUnit::Think( TFLOAT a_fDeltaTime )
{
	ASteer*  pSteer  = m_pSteer;
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( TVector4::DistanceSq( pCamera->GetMatrix().GetTranslation(), *pSteer->GetPosition() ) >= 10000.0f && m_pSteer )
	{
		// Interior steers far away from the camera only think every 1.5 seconds
		if ( m_pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
		{
			m_fThinkTimer += a_fDeltaTime;

			if ( m_fThinkTimer > 1.5f )
			{
				TBOOL bResult = m_pSteer->Update( m_fThinkTimer );
				m_fThinkTimer = 0.0f;

				return bResult;
			}

			return TFALSE;
		}
	}

	return m_pSteer->Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00414e90
void AVisibleUnit::UpdateComponents( TFLOAT a_fDeltaTime )
{
	AUnit::UpdateComponents( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00414f00
void AVisibleUnit::OnGoalCollisionUpdate( AGoalCollisionUpdateInterface* a_pInterface )
{
	TVector4 vecPosition;

	if ( a_pInterface->GetGoal()->GetPosition( vecPosition ) )
	{
		vecPosition.y += m_fGoalYOffset;

		// NOTE: retail sets the steer position twice (directly and through AUnit)
		m_pSteer->SetPosition( vecPosition );
		SetSteerPosition( vecPosition );
	}
}

// $Barnyard: FUNCTION 00414ea0
TBOOL AVisibleUnit::IsInCameraFrustum()
{
	// The model instance is flagged visible when it passed the render frustum test
	return GetModelInstance()->IsVisible();
}

// $Barnyard: FUNCTION 00414fe0
TBOOL AVisibleUnit::IsInCameraFrustumDistance( TFLOAT a_fDistance )
{
	ASteer*         pSteer    = m_pSteer;
	ACamera*        pCamera   = ACameraManager::GetSingleton()->GetCurrentCamera();
	const TVector4* pPosition = pSteer->GetPosition();

	if ( a_fDistance * a_fDistance > TVector4::DistanceSqXZ( pCamera->GetMatrix().GetTranslation(), *pPosition ) )
		return GetModelInstance()->IsVisible(); // NOTE: IsInCameraFrustum, inlined

	return TFALSE;
}

// $Barnyard: FUNCTION 00414f50
void AVisibleUnit::SetOpacity( TFLOAT a_fOpacity )
{
	// NOTE: retail calls AModelInstance::SetOpacity (00610710), which does this
	TSceneObject* pSceneObject = GetModelInstance()->GetSceneObject();
	pSceneObject->EnableOpacity();
	pSceneObject->GetFadeParams().y = a_fOpacity;
}

// $Barnyard: FUNCTION 00415510
void AVisibleUnit::ConnectModelInstance( AModelInstance* a_pModelInstance )
{
	m_oModelInstanceListener.Disconnect();
	m_oModelInstanceListener.Connect( a_pModelInstance->GetChangeEmitter(), this, OnModelInstanceChangeCallback, 0 );
}

// $Barnyard: FUNCTION 00414e70
TBOOL __stdcall AVisibleUnit::OnModelInstanceChangeCallback( AVisibleUnit* a_pCaller, AModelInstance* a_pModelInstance, AModelInstance::ChangeEvent* a_pEvent )
{
	return a_pCaller->OnModelInstanceChange( a_pModelInstance, a_pEvent );
}

// $Barnyard: FUNCTION 00415790
TBOOL AVisibleUnit::OnModelInstanceChange( AModelInstance* a_pModelInstance, AModelInstance::ChangeEvent* a_pEvent )
{
	switch ( *a_pEvent )
	{
		case AModelInstance::ChangeEvent_Visibility: // became visible
			NotifyComponentsVisibility( TTRUE );
			return TTRUE;
		case AModelInstance::ChangeEvent_UNK2: // became invisible
			NotifyComponentsVisibility( TFALSE );
			return TTRUE;
		case AModelInstance::ChangeEvent_EnabledSkeletonUpdate:
			NotifyComponentsSkeletonUpdate( TTRUE );
			return TTRUE;
		case AModelInstance::ChangeEvent_DisabledSkeletonUpdate:
			NotifyComponentsSkeletonUpdate( TFALSE );
			break;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004154b0
void AVisibleUnit::NotifyComponentsVisibility( TBOOL a_bVisible )
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
		it->OnVisibilityChanged( a_bVisible );
}

// $Barnyard: FUNCTION 00415450
void AVisibleUnit::NotifyComponentsSkeletonUpdate( TBOOL a_bUpdating )
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
		it->OnSkeletonUpdateChanged( a_bUpdating );
}

// $Barnyard: FUNCTION 004157f0
void AVisibleUnit::StopAllLoopSounds()
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	while ( !m_oLoopSounds.IsEmpty() )
	{
		pSoundManager->StopCue( m_oLoopSounds.Tail()->iCueIndex );
		m_oLoopSounds.DeleteTail();
	}
}
