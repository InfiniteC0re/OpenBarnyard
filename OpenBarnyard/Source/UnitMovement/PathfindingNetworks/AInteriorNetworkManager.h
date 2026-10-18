#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateInterface.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TDList.h>
#include <Toshi/T2ObjectPool.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class AInteriorNetworkManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AInteriorNetworkManager>
{
public:
	TDECLARE_CLASS( AInteriorNetworkManager, Toshi::TTask );

	static constexpr TUINT MAX_NUM_NETWORKS = 12;

	using NetworkPool = Toshi::T2ObjectPool<AInteriorNetwork, MAX_NUM_NETWORKS>;

public:
	AInteriorNetworkManager();
	~AInteriorNetworkManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Loads every network named in the string array a_pNetworkList, the first one becomes the default network
	// Persistent networks survive Reset (the ones loaded on startup are persistent)
	void LoadNetworksFromProperty( const PBPropertyValue* a_pNetworkList, TBOOL a_bPersistent );

	// Loads Data/Networks/<name>.trb unless a network with this name is already loaded
	AInteriorNetwork* LoadNetwork( const Toshi::TPString8& a_rcName );

	// Destroys the network unless it is persistent (a_bForce destroys persistent networks too)
	void DestroyNetwork( AInteriorNetwork* a_pNetwork, TBOOL a_bForce );

	// Makes the network the default one and destroys it if it isn't persistent
	void UnloadNetwork( const Toshi::TPString8& a_rcName );

	AInteriorNetwork* FindNetwork( const Toshi::TPString8& a_rcName );

	// Returns the first network whose name contains a_szSubString
	AInteriorNetwork* FindNetworkContaining( const TCHAR* a_szSubString );

	AInteriorArea* FindArea( const Toshi::TPString8& a_rcNetworkName, const Toshi::TPString8& a_rcAreaName, AInteriorNetwork*& a_rpOutNetwork );
	AInteriorGoal* FindRandomVisibleGoal( const Toshi::TPString8& a_rcNetworkName, const Toshi::TPString8& a_rcGoalName, AInteriorNetwork*& a_rpOutNetwork );

	// Called by the terrain once a section's collision is ready (00423720), see AInteriorNetwork::UpdateGoalCollisionGroup
	void UpdateGoalCollisionGroup( void* a_pUnused, TINT a_iGroupId );

	void RemoveGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface );

	// Returns the new default network
	AInteriorNetwork* SetDefaultNetwork( const Toshi::TPString8& a_rcName );

	// The network used by units that aren't connected to any other network
	AInteriorNetwork* GetDefaultNetwork() const { return m_pDefaultNetwork; }

	// NOTE: ours (the network editor): the default network, else the first loaded one
	AInteriorNetwork* GetAnyNetwork() { return m_pDefaultNetwork ? m_pDefaultNetwork : ( m_llNetworks.IsEmpty() ? TNULL : m_llNetworks.Head() ); }

	// NOTE: ours (the world debug drawing)
	Toshi::TDList<AInteriorNetwork>& GetNetworks() { return m_llNetworks; }

private:
	NetworkPool*                    m_pNetworkPool;    // 0x20
	Toshi::TDList<AInteriorNetwork> m_llNetworks;      // 0x24
	AInteriorNetwork*               m_pDefaultNetwork; // 0x2c
};

TSTATICASSERT( sizeof( AInteriorNetworkManager ) == 0x30 );
