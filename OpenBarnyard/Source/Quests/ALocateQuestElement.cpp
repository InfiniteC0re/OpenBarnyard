#include "pch.h"
#include "ALocateQuestElement.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "Helpers/ABikeManager.h"
#include "Units/AUnit.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Player/APlayerManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d06d0, CreateObject 005d0ff0, CreateObjectInPlace 005d1050
TDEFINE_CLASS( ALocateQuestElement );

// $Barnyard: FUNCTION 005d07e0
// NOTE: retail leaves m_fRadiusSq, m_bSpawn and m_eMode uninitialised
ALocateQuestElement::ALocateQuestElement()
    : m_fRadiusSq( 0.0f )
    , m_bSpawn( TFALSE )
    , m_eMode( MODE_GOAL )
    , m_strDescription( GetGlobalAllocator() )
    , m_iUnitOffset( -1 )
{
}

// $Barnyard: FUNCTION 005d0850
// $Barnyard: FUNCTION 005d1090
ALocateQuestElement::~ALocateQuestElement()
{
}

// $Barnyard: FUNCTION 005d0950
TBOOL ALocateQuestElement::Create( const PBProperties* a_pProperties, TINT* a_pDataOffset )
{
	m_bSpawn = TFALSE;

	const PBPropertyValue* pUnitType = a_pProperties->GetProperty( "unittype" );

	if ( pUnitType )
	{
		// Locate a unit
		m_strUnitType = pUnitType->GetTPString8();
		m_eMode       = MODE_UNIT;

		for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
		{
			const TCHAR*           szName = it->GetName().GetString();
			const PBPropertyValue* pValue = it->GetValue();

			switch ( szName[ 0 ] )
			{
				case 'l':
					if ( TStringManager::String8CompareNoCase( szName, "location" ) == 0 )
						m_strLocation = pValue->GetTPString8();
					break;
				case 'r':
					if ( TStringManager::String8CompareNoCase( szName, "radius" ) == 0 )
						m_fRadiusSq = pValue->GetFloat();
					break;
				case 's':
					// NOTE: retail doesn't check the rest of the name ("spawn")
					m_bSpawn = pValue->GetRaw() != TNULL;
					break;
				case 'u':
					if ( TStringManager::String8CompareNoCase( szName, "unitname" ) == 0 )
						m_strUnitName = pValue->GetTPString8();
					break;
			}
		}
	}
	else
	{
		// Locate a goal
		m_eMode = MODE_GOAL;

		for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
		{
			const TCHAR*           szName = it->GetName().GetString();
			const PBPropertyValue* pValue = it->GetValue();

			switch ( szName[ 0 ] )
			{
				case 'g':
					if ( TStringManager::String8CompareNoCase( szName, "goal" ) == 0 )
						m_strGoal = pValue->GetTPString8();
					break;
				case 'n':
					if ( TStringManager::String8CompareNoCase( szName, "network" ) == 0 )
						m_strNetwork = pValue->GetTPString8();
					break;
				case 'r':
					if ( TStringManager::String8CompareNoCase( szName, "radius" ) == 0 )
						m_fRadiusSq = pValue->GetFloat();
					break;
			}
		}
	}

	m_fRadiusSq *= m_fRadiusSq;

	m_iUnitOffset = *a_pDataOffset;
	*a_pDataOffset += sizeof( AUnitRef );

	return AQuestElement::Create( a_pProperties, a_pDataOffset );
}

// $Barnyard: FUNCTION 005d0d10
AUnit* ALocateQuestElement::FindUnit( AUnitRef* a_pRef )
{
	AUnit* pUnit = a_pRef->GetUnit();

	if ( pUnit )
		return pUnit;

	if ( m_bSpawn )
	{
		pUnit = AUnitManager::GetSingleton()->SpawnUnit( m_strUnitType, TVector4::VEC_ZERO, TVector4::VEC_POSX );

		if ( !m_strUnitName.IsEmpty() )
			AUnitNameManager::GetSingleton()->SetUnitName( m_strUnitName, pUnit->GetId() );
	}
	else if ( !m_strUnitName.IsEmpty() )
	{
		pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( m_strUnitName );
	}
	else
	{
		pUnit = AUnitManager::GetSingleton()->FindUnitByTypeName( m_strUnitType );
	}

	if ( pUnit )
		a_pRef->oProxy = pUnit->GetSelfProxy();

	return pUnit;
}

// $Barnyard: FUNCTION 005d10b0
TBOOL ALocateQuestElement::InitData( AInteractionActivityData* a_pData )
{
	AUnitRef* pRef = GetUnitRef( a_pData );

	// NOTE: the instance data is raw memory, retail clears the reference without releasing anything
	if ( pRef )
		TUtil::MemClear( pRef, sizeof( AUnitRef ) );

	if ( m_eMode == MODE_UNIT )
	{
		pRef->oProxy = TNULL;
		pRef->pType  = TNULL;
		pRef->szName = TNULL;
		FindUnit( pRef );
	}

	return AQuestElement::InitData( a_pData );
}

// $Barnyard: FUNCTION 005d1140
void ALocateQuestElement::DeinitData( AInteractionActivityData* a_pData )
{
	if ( m_eMode == MODE_UNIT )
		GetUnitRef( a_pData )->oProxy = TNULL;

	AQuestElement::DeinitData( a_pData );
}

// $Barnyard: FUNCTION 005d06e0
TBOOL ALocateQuestElement::GetDescription( const TWCHAR** a_ppOutText )
{
	// NOTE: retail passes the TCHAR string to the wide %s
	m_strDescription.Format( L"Locate %s", m_strUnitType.GetString() );
	*a_ppOutText = m_strDescription.GetString( 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 005d0720
void ALocateQuestElement::Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	a_pBuffer->WriteString( "LocateQuestElem" );
	AUnitManager::GetSingleton()->SaveUnitRef( a_pBuffer, GetUnitRef( a_pData ) );
	AQuestElement::Save( a_pBuffer, a_pData );
}

// $Barnyard: FUNCTION 005d0760
TBOOL ALocateQuestElement::Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	a_pBuffer->CheckString( "LocateQuestElem" );

	AUnitRef* pRef = GetUnitRef( a_pData );

	if ( pRef )
		TUtil::MemClear( pRef, sizeof( AUnitRef ) );

	AUnitManager::GetSingleton()->LoadUnitRef( a_pBuffer, pRef );
	return AQuestElement::Load( a_pBuffer, a_pData );
}

// $Barnyard: FUNCTION 005d0e70
AQuestElement::PROGRESS ALocateQuestElement::CheckProgress( AInteractionActivityData* a_pData )
{
	// Not while the secondary launcher is selected
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayer( 0 );

	if ( pPlayer->IsA( &TGetClass( AUnitPlayer ) ) )
	{
		AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, pPlayer );

		// NOTE: inlined AUnitPlayer::GetSecondaryLauncher (0043cd80)
		if ( pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
			return PROGRESS_RUNNING;
	}

	AUnitRef* pRef        = GetUnitRef( a_pData );
	AUnit*    pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	// Not while riding a bike
	if ( pPlayerUnit->GetSteer()->GetClass()->IsA( &TGetClass( ABikeSteer ) ) ||
	     ( ABikeManager::GetSingleton() && ABikeManager::GetSingleton()->IsPlayerOnBike() ) )
	{
		return PROGRESS_RUNNING;
	}

	TFLOAT fDistanceSq = TMath::MAXFLOAT;

	if ( m_eMode == MODE_UNIT )
	{
		AUnit* pUnit = FindUnit( pRef );

		if ( pUnit && pUnit->GetSteer() )
		{
			const TVector4* pUnitPosition   = pUnit->GetSteer()->GetPosition();
			const TVector4* pPlayerPosition = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();
			fDistanceSq                     = TVector4::DistanceSq( *pPlayerPosition, *pUnitPosition );
		}
	}
	else if ( m_eMode == MODE_GOAL )
	{
		AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( m_strNetwork );

		if ( pNetwork )
		{
			TVector4 vGoalPosition;
			pNetwork->FindGoal( m_strGoal )->GetPosition( vGoalPosition );

			const TVector4* pPlayerPosition = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();
			fDistanceSq                     = TVector4::DistanceSq( *pPlayerPosition, vGoalPosition );
		}
	}

	return ( fDistanceSq >= m_fRadiusSq ) ? PROGRESS_RUNNING : PROGRESS_COMPLETE;
}
