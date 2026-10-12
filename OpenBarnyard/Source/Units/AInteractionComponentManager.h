#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2ObjectPool.h>
#include <Toshi/T2DList.h>

#include "AUnitComponentManager.h"
#include "AInteractionComponent.h"

// Owns every AInteractionComponent (pooled) and keeps the ones that aren't in an
// interaction sequence in a list, assigning interactions to them once their units
// have been idle long enough.
class AInteractionComponentManager
    : public AUnitComponentManager
    , public Toshi::TSingleton<AInteractionComponentManager>
{
public:
	TDECLARE_CLASS( AInteractionComponentManager, AUnitComponentManager );

	friend AInteractionComponent;
	friend class AInteractionComponentData;

	static constexpr TINT MAX_NUM_COMPONENTS = 200;

public:
	AInteractionComponentManager();
	~AInteractionComponentManager();

	virtual void Update( TFLOAT a_fDeltaTime ) OVERRIDE;               // +0x0c
	virtual void ReleaseComponent( AUnitComponent* a_pComp ) OVERRIDE; // +0x10

	// Allocates a component from the pool.
	AInteractionComponent* CreateComponent();

private:
	// T2ObjectPool<AInteractionComponent, 200, 124, 4> methods (vtable 0x00729be4):
	// $Barnyard: FUNCTION 005a0500
	// $Barnyard: FUNCTION 005a0520
	// $Barnyard: FUNCTION 005a0530
	// $Barnyard: FUNCTION 005a0550
	Toshi::T2ObjectPool<AInteractionComponent, MAX_NUM_COMPONENTS> m_oPool;        // 0x0c
	Toshi::T2DList<AInteractionComponent>                          m_llComponents; // 0x60f4, components that aren't in a sequence
};

TSTATICASSERT( sizeof( AInteractionComponentManager ) == 0x60fc );
