#include "pch.h"
#include "ASpawnActivity.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "AUnitFactory.h"
#include "AUnitManager.h"
#include "AUnitNameManager.h"
#include "ARandom.h"
#include "Player/APlayerManager.h"
#include "Helpers/AGameLoader.h"
#include "Render/ARenderer.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#include <Render/TRenderContext.h>
#include <Render/TViewport.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bddb0, CreateObject 005bec90, CreateObjectInPlace 005becf0
TDEFINE_CLASS( ASpawnActivity );

TPSTRING8_DECLARE( unspawn );
TPSTRING8_DECLARE( teleport );
TPSTRING8_DECLARE( playerrespawngoal );
TPSTRING8_DECLARE( unittype );
TPSTRING8_DECLARE( network );
TPSTRING8_DECLARE( distancetrigger );
TPSTRING8_DECLARE( count );
TPSTRING8_DECLARE( offset );
TPSTRING8_DECLARE( area );
TPSTRING8_DECLARE( goal );
TPSTRING8_DECLARE( name );
TPSTRING8_DECLARE( steeringclass );
TPSTRING8_DECLARE( addunittointeraction );
TPSTRING8_DECLARE( setonground );
TPSTRING8_DECLARE( random );
TPSTRING8_DECLARE( randomy );
TPSTRING8_DECLARE( ignorefrustrum );
TPSTRING8_DECLARE( spawnupto );
TPSTRING8_DECLARE( player );

// Anything that isn't a float is read as an integer
static TFLOAT GetFloatOrInt( const PBPropertyValue* a_pValue )
{
	if ( a_pValue->GetType() == PBPropertyValue::Type::Float )
		return a_pValue->GetFloat();

	return TFLOAT( a_pValue->GetInteger() );
}

// $Barnyard: FUNCTION 005bde40
ASpawnActivity::ASpawnActivity()
    : m_pUnitType( TNULL )
    , m_iCount( 1 )
    , m_vecOffset( TVector4::VEC_ZERO )
    , m_vecRandomY( TVector2::VEC_ZERO )
    , m_uiFlags( FLAGS_NONE )
    , m_fDistanceTriggerSq( 0.0f )
{
}

// $Barnyard: FUNCTION 005bdec0
// $Barnyard: FUNCTION 005bed30
ASpawnActivity::~ASpawnActivity()
{
}

// $Barnyard: FUNCTION 005be000
TBOOL ASpawnActivity::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const PBPropertyValue* pValue = it->GetValue();
		TPString8              strName( it->GetName().GetString() );

		if ( strName == TPS8( unspawn ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_UNSPAWN;
		}
		else if ( strName == TPS8( teleport ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_TELEPORT;
		}
		else if ( strName == TPS8( playerrespawngoal ) )
		{
			m_strPlayerRespawnGoal = pValue->GetTPString8();
			m_uiFlags |= FLAGS_PLAYER_RESPAWN_GOAL;
		}
		else if ( strName == TPS8( unittype ) )
		{
			m_pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( pValue->GetTPString8() );
		}
		else if ( strName == TPS8( network ) )
		{
			m_strNetwork = pValue->GetTPString8();
		}
		else if ( strName == TPS8( distancetrigger ) )
		{
			TFLOAT fDistance = GetFloatOrInt( pValue );

			m_uiFlags |= FLAGS_DISTANCE_TRIGGER;
			m_fDistanceTriggerSq = fDistance * fDistance;
		}
		else if ( strName == TPS8( count ) )
		{
			m_iCount = TINT( TREINTERPRETCAST( TUINTPTR, pValue->GetRaw() ) );
		}
		else if ( strName == TPS8( offset ) )
		{
			if ( pValue->GetType() == PBPropertyValue::Type::Array )
				pValue->GetVector4( m_vecOffset );
		}
		else if ( strName == TPS8( area ) )
		{
			m_strArea = pValue->GetTPString8();
			m_uiFlags |= FLAGS_AREA;
		}
		else if ( strName == TPS8( goal ) )
		{
			m_strGoal = pValue->GetTPString8();
			m_uiFlags |= FLAGS_GOAL;
		}
		else if ( strName == TPS8D( "locator" ) )
		{
			m_strLocator = pValue->GetTPString8();
			m_uiFlags |= FLAGS_LOCATOR;
		}
		else if ( strName == TPS8( name ) )
		{
			m_strName = pValue->GetTPString8();

			// The name of a teleported unit is the unit to look for
			if ( !( m_uiFlags & FLAGS_TELEPORT ) )
				m_uiFlags |= FLAGS_NAME;
		}
		else if ( strName == TPS8( steeringclass ) )
		{
			m_strSteeringClass = pValue->GetTPString8();
			m_uiFlags |= FLAGS_STEERING_CLASS;
		}
		else if ( strName == TPS8( addunittointeraction ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_ADD_UNIT_TO_INTERACTION;
		}
		else if ( strName == TPS8( setonground ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_SET_ON_GROUND;
		}
		else if ( strName == TPS8( random ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_RANDOM;
		}
		else if ( strName == TPS8( randomy ) )
		{
			if ( pValue->GetType() == PBPropertyValue::Type::Array && pValue->GetVector2( m_vecRandomY ) )
				m_uiFlags |= FLAGS_RANDOM_Y;
		}
		else if ( strName == TPS8( ignorefrustrum ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_IGNORE_FRUSTUM;
		}
		else if ( strName == TPS8( spawnupto ) )
		{
			if ( pValue->GetRaw() )
				m_uiFlags |= FLAGS_SPAWN_UP_TO;
		}
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bddc0
TBOOL ASpawnActivity::IsUnitOutOfView( AUnit* a_pUnit )
{
	const TPlane* pPlanes = ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext()->GetWorldPlanes();
	ASteer*       pSteer  = a_pUnit->GetSteer();

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	for ( TINT i = 0; i < 6; i++ )
	{
		TFLOAT fDistance = TVector3::DotProduct( pPlanes[ i ].GetNormal(), vecPosition.AsVector3() ) - pPlanes[ i ].GetD();

		if ( fDistance > pSteer->GetTraits()->m_fRadius )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005be4f0
AInteractionActivity::RESULT ASpawnActivity::Update( AInteractionSequence* a_pSequence )
{
	AUnitManager*     pUnitManager = AUnitManager::GetSingleton();
	AInteriorNetwork* pNetwork     = TNULL;
	AInteriorArea*    pArea        = TNULL;
	AInteriorGoal*    pGoal        = TNULL;

	if ( m_uiFlags & FLAGS_PLAYER_RESPAWN_GOAL )
	{
		if ( !m_strPlayerRespawnGoal.IsEmpty() )
		{
			AGameLoader::SetPlayerStartGoal( m_strPlayerRespawnGoal );
			return RESULT_DONE;
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_UNSPAWN )
	{
		// Unspawns the named unit
		if ( !m_strName.IsEmpty() )
		{
			AUnit* pUnit = pUnitManager->FindUnitById( AUnitNameManager::GetSingleton()->GetUnitId( m_strName ) );
			pUnitManager->DestroyUnit( pUnit );

			return RESULT_DONE;
		}

		// Unspawns "count" units of the type
		TNodeList<AUnit>::Iterator it    = pUnitManager->GetUnits().Begin();
		AUnit*                     pUnit = pUnitManager->FindNextUnitOfType( m_pUnitType, it );

		for ( TINT i = 0; pUnit && i < m_iCount; i++ )
		{
			it = pUnitManager->GetUnits().Begin();
			pUnitManager->DestroyUnit( pUnit );
			pUnit = pUnitManager->FindNextUnitOfType( m_pUnitType, it );
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_TELEPORT )
	{
		if ( m_strName == TPS8( player ) )
		{
			// NOTE: retail calls APlayerManager::GetPlayer (0061faf0)
			AUnit* pPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerUnit( 0 );

			if ( !( m_uiFlags & FLAGS_GOAL ) )
				return RESULT_DONE;

			APlayerSteer* pPlayerSteer = TSTATICCAST( APlayerSteer, pPlayer->GetSteer() );

			if ( pPlayerSteer->IsAtGoal( m_strGoal ) )
				return RESULT_RUNNING;

			pPlayerSteer->PlaceAtNamedGoal( m_strGoal, TFALSE, TTRUE );
			return pPlayerSteer->IsAtGoal( m_strGoal ) ? RESULT_RUNNING : RESULT_DONE;
		}

		// Waits for the unit to be out of view (and far enough from the player)
		AUnit* pUnit = pUnitManager->FindUnitById( AUnitNameManager::GetSingleton()->GetUnitId( m_strName ) );

		if ( !IsUnitOutOfView( pUnit ) && !( m_uiFlags & FLAGS_IGNORE_FRUSTUM ) )
			return RESULT_RUNNING;

		if ( m_uiFlags & FLAGS_DISTANCE_TRIGGER )
		{
			AUnit* pPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerUnit( 0 );

			if ( TVector4::DistanceSq( *pPlayer->GetSteer()->GetPosition(), *pUnit->GetSteer()->GetPosition() ) < m_fDistanceTriggerSq )
				return RESULT_RUNNING;
		}

		// NOTE: the name can't be "player" here, retail checks again
		if ( m_strName == TPS8( player ) )
		{
			AUnit* pPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerUnit( 0 );

			if ( m_uiFlags & FLAGS_GOAL )
				TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->PlaceAtNamedGoal( m_strGoal, TFALSE, TTRUE );
		}
		else
		{
			if ( !IsUnitOutOfView( pUnit ) && !( m_uiFlags & FLAGS_IGNORE_FRUSTUM ) )
				return RESULT_RUNNING;

			pUnit = pUnitManager->FindUnitById( AUnitNameManager::GetSingleton()->GetUnitId( m_strName ) );

			if ( !IsUnitOutOfView( pUnit ) && !( m_uiFlags & FLAGS_IGNORE_FRUSTUM ) )
				return RESULT_RUNNING;

			if ( m_uiFlags & FLAGS_DISTANCE_TRIGGER )
			{
				AUnit* pPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerUnit( 0 );

				if ( TVector4::DistanceSq( *pPlayer->GetSteer()->GetPosition(), *pUnit->GetSteer()->GetPosition() ) < m_fDistanceTriggerSq )
					return RESULT_RUNNING;
			}
			else if ( m_uiFlags & FLAGS_UNSPAWN )
			{
				pUnitManager->DestroyUnit( pUnit );
				return RESULT_DONE;
			}
		}
	}

	if ( m_uiFlags & FLAGS_AREA )
		pArea = AInteriorNetworkManager::GetSingleton()->FindArea( m_strNetwork, m_strArea, pNetwork );

	TINT iNumUnits = m_iCount;

	if ( m_uiFlags & FLAGS_SPAWN_UP_TO )
		iNumUnits -= m_pUnitType->GetInstanceCount();

	TVector4 vecOffset = m_vecOffset;

	for ( ; iNumUnits > 0; iNumUnits-- )
	{
		if ( m_uiFlags & FLAGS_GOAL )
			pGoal = AInteriorNetworkManager::GetSingleton()->FindRandomVisibleGoal( m_strNetwork, m_strGoal, pNetwork );

		TVector4 vecPosition = TVector4::VEC_ZERO;
		AUnit*   pUnit       = TNULL;

		if ( m_uiFlags & FLAGS_AREA )
		{
			pArea->GetRandomPosition( vecPosition, 0.0f );
			vecPosition.x += m_vecOffset.x;
			vecPosition.y += m_vecOffset.y;
			vecPosition.z += m_vecOffset.z;
		}
		else if ( m_uiFlags & FLAGS_GOAL )
		{
			pGoal->GetPosition( vecPosition );

			vecPosition.x += m_vecOffset.x;
			vecPosition.y += m_vecOffset.y;
			vecPosition.z += m_vecOffset.z;
		}

		if ( m_uiFlags & FLAGS_RANDOM_Y )
		{
			// NOTE: the random offset adds up for every unit
			vecOffset.y += ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( m_vecRandomY.x, m_vecRandomY.y );
			vecPosition.y += vecOffset.y;
		}

		if ( m_uiFlags & FLAGS_TELEPORT )
		{
			// Moves the named unit
			pUnit          = pUnitManager->FindUnitById( AUnitNameManager::GetSingleton()->GetUnitId( m_strName ) );
			ASteer* pSteer = pUnit->GetSteer();
			pSteer->SetPosition( vecPosition );

			if ( pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
			{
				AInteriorSteer* pInteriorSteer = TSTATICCAST( AInteriorSteer, pUnit->GetSteer() );

				if ( pInteriorSteer->IsNetworkValid() == TTRUE )
				{
					// NOTE: retail uses the network FindArea / FindRandomVisibleGoal found, which is TNULL without "area" and "goal"
#ifdef BARNYARD_COMMUNITY_PATCH
					if ( pNetwork )
#endif
						pNetwork->DisconnectUnit( &pInteriorSteer->GetPath(), pUnit );
				}

				AInteriorNetwork* pSteerNetwork = m_strNetwork.IsEmpty() ? AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork() : AInteriorNetworkManager::GetSingleton()->FindNetwork( m_strNetwork );

				pInteriorSteer->ConnectToNetwork( pSteerNetwork );
				pInteriorSteer->Command_CancelCurrentCommand();
			}
		}
		else if ( m_uiFlags & FLAGS_GOAL )
		{
			AInteriorNetwork* pGoalNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( m_strNetwork );
			pUnit                          = pUnitManager->SpawnUnitAtGoal( m_pUnitType->GetName(), vecOffset, pGoalNetwork, pGoal, TNULL );
		}
		else if ( m_uiFlags & FLAGS_LOCATOR )
		{
			ATerrainLocator* pLocator = ATerrainInterface::GetSingleton()->FindLocator( m_strLocator.GetString() );

			if ( pLocator )
			{
				TMatrix44 matLocator;
				pLocator->GetMatrix( matLocator );

				// Converts the locator's axes
				TMatrix44 matRotation(
				    1.0f, 0.0f, 0.0f, 0.0f,
				    0.0f, 0.0f, 1.0f, 0.0f,
				    0.0f, -1.0f, 0.0f, 0.0f,
				    0.0f, 0.0f, 0.0f, 1.0f
				);

				matLocator = matRotation * matLocator;

				TVector4 vecLocatorPosition  = matLocator.AsBasisVector4( BASISVECTOR_TRANSLATION );
				TVector4 vecLocatorDirection = matLocator.AsBasisVector4( BASISVECTOR_UP );

				pUnit = pUnitManager->SpawnUnit( m_pUnitType->GetName(), vecLocatorPosition, vecLocatorDirection );
			}
		}
		else
		{
			pUnit = pUnitManager->SpawnUnit( m_pUnitType, vecPosition, TVector4::VEC_POSX );
		}

		if ( m_uiFlags & FLAGS_STEERING_CLASS )
		{
			TClass* pSteerClass = const_cast<TClass*>( TClass::Find( m_strSteeringClass.GetString(), &TGetClass( ASteer ) ) );
			pUnit->ChangeSteer( pSteerClass );

			if ( pSteerClass->IsA( &TGetClass( AInteriorSteer ) ) )
			{
				AInteriorNetwork* pSteerNetwork = m_strNetwork.IsEmpty() ? AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork() : AInteriorNetworkManager::GetSingleton()->FindNetwork( m_strNetwork );

				AInteriorSteer* pInteriorSteer = TSTATICCAST( AInteriorSteer, pUnit->GetSteer() );
				pInteriorSteer->ConnectToNetwork( pSteerNetwork );
				pInteriorSteer->Command_CancelCurrentCommand();
			}
		}

		if ( ( m_uiFlags & FLAGS_NAME ) && !( m_uiFlags & FLAGS_TELEPORT ) )
			AUnitNameManager::GetSingleton()->SetUnitName( m_strName, pUnit->GetId() );

		if ( ( m_uiFlags & FLAGS_ADD_UNIT_TO_INTERACTION ) && AInteractionComponent::IsUnitSuitableForInteraction( pUnit, TTRUE ) )
			a_pSequence->AddUnit( pUnit );
	}

	return RESULT_DONE;
}
