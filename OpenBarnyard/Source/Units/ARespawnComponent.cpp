#include "pch.h"
#include "ARespawnComponent.h"
#include "AUnit.h"
#include "AGeneralUnitComponentManager.h"
#include "Cameras/ACameraManager.h"
#include "Player/APlayerManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005918e0, CreateObject 00591fe0, CreateObjectInPlace 00592060
TDEFINE_CLASS( ARespawnComponent );

// NOTE: GetClass 005918d0, CreateObject 00591ee0, CreateObjectInPlace 00591cf0
TDEFINE_CLASS( ARespawnComponentData );

// NOTE: inlined into CreateObject / CreateObjectInPlace / ARespawnComponentData::OnUnitCreate
ARespawnComponent::ARespawnComponent()
    : m_eFlags( FLAGS_NONE )
    , m_fTimer( 0.0f )
    , m_iNumGoals( 0 )
{
}

// $Barnyard: FUNCTION 00592160
// $Barnyard: FUNCTION 00591df0
ARespawnComponent::~ARespawnComponent()
{
}

// NOTE: same code as AUnitProfileComponent::GetManager, folded in retail
AUnitComponentManager* ARespawnComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 00591e60
TBOOL ARespawnComponent::Create( AUnit* a_pOwner, ARespawnComponentData* a_pData )
{
	m_eFlags  = FLAGS_NONE;
	m_strGoal = a_pData->m_strGoal;
	m_eFlags |= FLAGS_COOLDOWN;
	m_fTimer    = 5.0f;
	m_iNumGoals = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->CountGoalsContaining( m_strGoal.GetString() );

	return TTRUE;
}

// $Barnyard: FUNCTION 00591b90
void ARespawnComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( m_eFlags & FLAGS_COOLDOWN )
	{
		m_fTimer -= a_fDeltaTime;

		if ( m_fTimer < 0.0f )
		{
			m_fTimer = 0.0f;
			m_eFlags = ( m_eFlags & ~( FLAGS_COOLDOWN | FLAGS_CHECKING ) ) | FLAGS_CHECKING;
		}

		return;
	}

	if ( !( m_eFlags & FLAGS_CHECKING ) )
		return;

	m_fTimer += a_fDeltaTime;
	if ( m_fTimer <= 3.0f )
		return;

	AUnit* pOwner  = GetOwner();
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayer( 0 );

	if ( pPlayer && pPlayer != pOwner )
	{
		const TVector4* pOwnerPos  = pOwner->GetSteer()->GetPosition();
		const TVector4* pPlayerPos = pPlayer->GetSteer()->GetPosition();

		TVector4 vecGoalPos;
		TVector4 vecGoalFacing;

		if ( FindClosestGoal( *pPlayerPos, vecGoalPos, vecGoalFacing ) )
		{
			if ( TVector4::DistanceSqXZ( *pOwnerPos, vecGoalPos ) > 0.09f )
			{
				if ( IsSeenByCamera( vecGoalPos, 2.0f, 15.0f ) || IsSeenByCamera( *pOwnerPos, 2.0f, 15.0f ) )
				{
					m_fTimer = 0.0f;
					return;
				}

				pOwner->GetSteer()->SetPosition( vecGoalPos );
				pOwner->GetSteer()->SetDirection( vecGoalFacing );
			}

			m_eFlags = ( m_eFlags & ~( FLAGS_COOLDOWN | FLAGS_CHECKING ) ) | FLAGS_COOLDOWN;
			m_fTimer = 5.0f;
			return;
		}
	}

	m_fTimer = 0.0f;
}

// $Barnyard: FUNCTION 005918f0
TBOOL ARespawnComponent::IsSeenByCamera( const TVector4& a_rcPosition, TFLOAT a_fMinDistance, TFLOAT a_fMaxDistance )
{
	const TMatrix44& rcCameraMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();

	TVector4 vecCameraPos     = rcCameraMatrix.GetTranslation();
	TVector4 vecCameraForward = rcCameraMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vecCameraForward.w        = 1.0f;

	TVector4 vecToPosition( a_rcPosition.x - vecCameraPos.x, a_rcPosition.y - vecCameraPos.y, a_rcPosition.z - vecCameraPos.z, a_rcPosition.w );
	TFLOAT   fDistanceSq = vecToPosition.MagnitudeSq();

	if ( a_fMaxDistance * a_fMaxDistance > fDistanceSq )
	{
		if ( a_fMinDistance * a_fMinDistance > fDistanceSq )
			return TTRUE;

		vecToPosition.Multiply( 1.0f / TMath::Sqrt( fDistanceSq ) );

		if ( TVector4::DotProduct3( vecToPosition, vecCameraForward ) > 0.3f )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00591a00
TBOOL ARespawnComponent::FindClosestGoal( const TVector4& a_rcPosition, TVector4& a_rOutPosition, TVector4& a_rOutFacing )
{
	AInteriorNetwork* pNetwork     = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	TINT              iClosest     = -1;
	TFLOAT            fClosestDist = TMath::MAXFLOAT;

	TCHAR szGoalName[ 64 ];
	T2String8::Copy( szGoalName, m_strGoal.GetString() );
	TCHAR* pszNumber = szGoalName + T2String8::Length( szGoalName );

	for ( TINT i = 1; i < m_iNumGoals + 1; i++ )
	{
		T2String8::Format( pszNumber, "%02d", i );

		AInteriorGoal* pGoal = pNetwork->FindGoalOrFirst( szGoalName );

		TVector4 vecGoalPos;
		pGoal->GetPosition( vecGoalPos );

		TFLOAT fDistance = TVector4::DistanceSqXZ( vecGoalPos, a_rcPosition );

		if ( fDistance < fClosestDist || iClosest < 0 )
		{
			fClosestDist   = fDistance;
			a_rOutPosition = vecGoalPos;
			iClosest       = i;

			if ( pGoal->m_vForward.MagnitudeSq() >= 0.01f )
				a_rOutFacing = pGoal->m_vForward;
			else
				a_rOutFacing = TVector4::VEC_NEGX;
		}
	}

	return iClosest != -1;
}

// NOTE: inlined into CreateObject / CreateObjectInPlace
ARespawnComponentData::ARespawnComponentData()
{
}

// $Barnyard: FUNCTION 00591f50
// $Barnyard: FUNCTION 00591f70
ARespawnComponentData::~ARespawnComponentData()
{
}

// $Barnyard: FUNCTION 00591d60
TBOOL ARespawnComponentData::Create( const PBProperties* a_pProperties )
{
	// NOTE: the properties aren't read, the goal prefix is always "goal"
	m_strGoal = TPString8( "goal" );

	return TTRUE;
}

// $Barnyard: FUNCTION 005920d0
void ARespawnComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	ARespawnComponent* pComponent = new ARespawnComponent();
	pComponent->Create( a_pUnit, this );

	a_pUnit->AddComponent( pComponent );
}
