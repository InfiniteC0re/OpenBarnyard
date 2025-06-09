#pragma once

class ATerrainPhysicsManager
{
public:
	static void Create();

private:
	ATerrainPhysicsManager();
	~ATerrainPhysicsManager();
};

extern ATerrainPhysicsManager* g_pTerrainPhysicsManager;
