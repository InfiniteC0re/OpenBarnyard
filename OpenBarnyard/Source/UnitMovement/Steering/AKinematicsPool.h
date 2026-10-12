#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2ObjectPool.h>
#include <Toshi/T2DList.h>

#include "ASteerKinematics.h"

//-----------------------------------------------------------------------------
// Purpose: Allocates the movement integrators (ASteerKinematics2D/3D) of the
// unit steers. Created by AGameSystemManager::OnCreate through its TClass.
// NOTE: retail's pool is a class derived from T2DerivedDynamicObjectPool (the
// pool's vtable is replaced by 007276b4 after the base ctor ran), with no
// behaviour of its own besides the destructor. The Wii build names the list
// node of ASteerKinematics Toshi::TQList<ASteerKinematics>::Node, but it links
// node pointers like T2DList does.
//-----------------------------------------------------------------------------
class AKinematicsPool
    : public Toshi::TObject
    , public Toshi::TSingleton<AKinematicsPool>
{
public:
	TDECLARE_CLASS( AKinematicsPool, Toshi::TObject );

	static constexpr TINT MAX_NUM_KINEMATICS = 200;

public:
	AKinematicsPool();
	~AKinematicsPool();

	// Allocates a kinematics of the specified ASteerKinematics subclass
	ASteerKinematics* CreateKinematics( Toshi::TClass* a_pClass );

	// Unlinks and destroys a kinematics created by CreateKinematics
	void DestroyKinematics( ASteerKinematics* a_pKinematics );

private:
	Toshi::T2DList<ASteerKinematics>                    m_llKinematics; // 0x04
	Toshi::T2DerivedDynamicObjectPool<ASteerKinematics> m_oPool;        // 0x0c
};

TSTATICASSERT( sizeof( AKinematicsPool ) == 0x24 );
