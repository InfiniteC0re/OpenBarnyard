#pragma once
#include <Opcode.h>

class AOpcodeCache
{
public:
	static void Create();

	friend class ACollisionModel;

private:
	AOpcodeCache();
	~AOpcodeCache();

public:
	Opcode::RayCollider    m_RayCollider;
	Opcode::CollisionFaces m_StabbedFaces;
	udword                 m_uiRayColliderCache;
	Opcode::SphereCollider m_SphereCollider;
	Opcode::SphereCache    m_SphereCache;
	// ...
	Opcode::AABBCollider     m_AABBCollider;
	Opcode::AABBCache        m_AABBCache;
	Opcode::LSSCollider      m_LSSCollider;
	Opcode::LSSCache         m_LSSCache;
	Opcode::OBBCollider      m_OBBCollider;
	Opcode::OBBCache         m_OBBCache;
	Opcode::AABBTreeCollider m_AABBTreeCollider;
	// ...
};

