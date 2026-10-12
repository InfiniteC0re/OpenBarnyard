#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00555210, CreateObject 005556a0, CreateObjectInPlace 00555700
TDEFINE_CLASS( AInteriorNetworkManager );

// $Barnyard: FUNCTION 00555360
AInteriorNetworkManager::AInteriorNetworkManager()
{
	m_pNetworkPool    = TNULL;
	m_pDefaultNetwork = TNULL;
	m_pNetworkPool    = new NetworkPool();
}

// $Barnyard: FUNCTION 00555420
// $Barnyard: FUNCTION 00555740
AInteriorNetworkManager::~AInteriorNetworkManager()
{
	while ( m_llNetworks.Begin() != m_llNetworks.End() )
		DestroyNetwork( m_llNetworks.Begin(), TTRUE );

	AInteriorNetwork::ClearAreaSearchNodes();
	m_pDefaultNetwork = TNULL;

	if ( m_pNetworkPool )
		delete m_pNetworkPool;
}

// $Barnyard: FUNCTION 005554e0
TBOOL AInteriorNetworkManager::Reset()
{
	// Destroy every network that isn't persistent
	for ( auto it = m_llNetworks.Begin(); it != m_llNetworks.End(); )
	{
		AInteriorNetwork* pNetwork = it;
		it++;

		DestroyNetwork( pNetwork, TFALSE );
	}

	m_pDefaultNetwork = TNULL;
	return TTRUE;
}

// $Barnyard: FUNCTION 005559a0
void AInteriorNetworkManager::LoadNetworksFromProperty( const PBPropertyValue* a_pNetworkList, TBOOL a_bPersistent )
{
	if ( !a_pNetworkList )
		return;

	const PBPropertyValueArray* pArray = a_pNetworkList->GetArray();

	for ( TUINT i = 0; i < pArray->GetSize(); i++ )
	{
		TPString8 strNetworkName = pArray->GetValue( i )->GetTPString8();

		LoadNetwork( strNetworkName )->SetPersistent( a_bPersistent );

		if ( i == 0 )
			m_pDefaultNetwork = FindNetwork( strNetworkName );
	}
}

// $Barnyard: FUNCTION 005558e0
AInteriorNetwork* AInteriorNetworkManager::LoadNetwork( const Toshi::TPString8& a_rcName )
{
	AInteriorNetwork* pNetwork = FindNetwork( a_rcName );

	if ( !pNetwork )
	{
		pNetwork = m_pNetworkPool->NewObject();

		TCHAR szFilePath[ 52 ];
		T2String8::Format( szFilePath, "Data/Networks/%s.trb", a_rcName.GetString() );

		pNetwork->LoadFromFile( a_rcName, szFilePath );
		m_llNetworks.InsertTail( pNetwork );
	}

	return pNetwork;
}

// $Barnyard: FUNCTION 00555290
void AInteriorNetworkManager::DestroyNetwork( AInteriorNetwork* a_pNetwork, TBOOL a_bForce )
{
	if ( a_pNetwork && ( !a_pNetwork->IsPersistent() || a_bForce ) )
	{
		if ( m_pDefaultNetwork == a_pNetwork )
			m_pDefaultNetwork = TNULL;

		a_pNetwork->Remove();
		m_pNetworkPool->DeleteObject( a_pNetwork );
	}
}

// $Barnyard: FUNCTION 00555a60
void AInteriorNetworkManager::UnloadNetwork( const Toshi::TPString8& a_rcName )
{
	m_pDefaultNetwork = FindNetwork( a_rcName );
	DestroyNetwork( m_pDefaultNetwork, TFALSE );
}

// $Barnyard: FUNCTION 00555760
AInteriorNetwork* AInteriorNetworkManager::FindNetwork( const Toshi::TPString8& a_rcName )
{
	T2_FOREACH( m_llNetworks, it )
	{
		TPString8 strNetworkName = it->GetName();

		// NOTE: retail uses TPString8::Compare (006bc110), which compares the pooled pointers too
		if ( a_rcName == strNetworkName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00555560
AInteriorNetwork* AInteriorNetworkManager::FindNetworkContaining( const TCHAR* a_szSubString )
{
	T2_FOREACH( m_llNetworks, it )
	{
		if ( T2String8::FindString( it->GetName().GetString(), a_szSubString ) )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00555840
AInteriorArea* AInteriorNetworkManager::FindArea( const Toshi::TPString8& a_rcNetworkName, const Toshi::TPString8& a_rcAreaName, AInteriorNetwork*& a_rpOutNetwork )
{
	AInteriorNetwork* pNetwork = FindNetwork( a_rcNetworkName );

	if ( pNetwork )
	{
		TINT iAreaIndex = pNetwork->FindAreaIndex( a_rcAreaName );

		if ( iAreaIndex >= 0 )
		{
			a_rpOutNetwork = pNetwork;
			return &pNetwork->m_pINetwork->m_pAreas[ iAreaIndex ];
		}
	}

	a_rpOutNetwork = TNULL;
	return TNULL;
}

// $Barnyard: FUNCTION 00555890
AInteriorGoal* AInteriorNetworkManager::FindRandomVisibleGoal( const Toshi::TPString8& a_rcNetworkName, const Toshi::TPString8& a_rcGoalName, AInteriorNetwork*& a_rpOutNetwork )
{
	AInteriorNetwork* pNetwork = FindNetwork( a_rcNetworkName );

	if ( !pNetwork )
	{
		a_rpOutNetwork = TNULL;
		return TNULL;
	}

	a_rpOutNetwork = pNetwork;

	if ( pNetwork->HasGoal( a_rcGoalName ) == TTRUE )
		return pNetwork->FindRandomVisibleGoal( a_rcGoalName );

	return TNULL;
}

// $Barnyard: FUNCTION 005555e0
void AInteriorNetworkManager::UpdateGoalCollisionGroup( void* a_pUnused, TINT a_iGroupId )
{
	T2_FOREACH( m_llNetworks, it )
	{
		it->UpdateGoalCollisionGroup( a_pUnused, a_iGroupId );
	}
}

// $Barnyard: FUNCTION 00555640
void AInteriorNetworkManager::RemoveGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface )
{
	if ( a_pGoalInterface->IsLinked() )
	{
		T2_FOREACH( m_llNetworks, it )
		{
			it->EraseGoalInterface( a_pGoalInterface );
		}
	}
}

// $Barnyard: FUNCTION 00555980
AInteriorNetwork* AInteriorNetworkManager::SetDefaultNetwork( const Toshi::TPString8& a_rcName )
{
	m_pDefaultNetwork = FindNetwork( a_rcName );
	return m_pDefaultNetwork;
}
