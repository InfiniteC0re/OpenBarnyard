#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2ObjectPool.h>
#include <Toshi/T2Vector.h>

#include "AKinematicsObstruction.h"

//-----------------------------------------------------------------------------
// Purpose: Pool and list of the active obstructions units have to steer around.
// Created by AGameSystemManager::OnCreate through its TClass.
// NOTE: retail's pool is a class derived from T2DerivedDynamicObjectPool (the
// pool's vtable is replaced by 007275f0 after the base ctor ran), with no
// behaviour of its own besides the destructor.
//-----------------------------------------------------------------------------
class AKinematicsObstructionManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AKinematicsObstructionManager>
{
public:
	TDECLARE_CLASS( AKinematicsObstructionManager, Toshi::TObject );

	static constexpr TINT MAX_NUM_POOLED_OBSTRUCTIONS = 20;
	static constexpr TINT MAX_NUM_OBSTRUCTIONS        = 82;

public:
	AKinematicsObstructionManager();
	~AKinematicsObstructionManager();

	// Adds the obstruction to the list of the active ones (if it's not there yet)
	TBOOL AddObstruction( AKinematicsObstruction* a_pObstruction );

	// Removes the obstruction from the list of the active ones.
	void RemoveObstruction( AKinematicsObstruction* a_pObstruction );

	// Allocates an obstruction of the specified AKinematicsObstruction subclass from the pool.
	AKinematicsObstruction* CreateObstruction( Toshi::TClass* a_pClass );

	// Destroys the obstruction and returns it to the pool.
	void DeleteObstruction( AKinematicsObstruction* a_pObstruction );

	Toshi::T2Vector<AKinematicsObstruction*, MAX_NUM_OBSTRUCTIONS>& GetObstructions() { return m_vecObstructions; }

private:
	Toshi::T2DerivedDynamicObjectPool<AKinematicsObstruction>      m_oPool;           // 0x04
	Toshi::T2Vector<AKinematicsObstruction*, MAX_NUM_OBSTRUCTIONS> m_vecObstructions; // 0x1c
};

TSTATICASSERT( sizeof( AKinematicsObstructionManager ) == 0x168 );
