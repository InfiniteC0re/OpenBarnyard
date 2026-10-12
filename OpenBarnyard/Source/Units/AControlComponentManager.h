#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2ObjectPool.h>
#include <Toshi/T2DList.h>
#include <Toshi/TGlobalEmitter.h>
#include <Input/TInputInterface.h>

#include "AUnitComponentManager.h"
#include "AControlComponent.h"
#include "Input/AInputMapManager.h"
#include "GameInterface/AGameStateControllerEvent.h"

class AControlComponentData;

// Owns every AControlComponent (pooled), routes input events to the components of
// the units controlled by players and enables/disables components when game states
// get activated.
class AControlComponentManager
    : public AUnitComponentManager
    , public Toshi::TSingleton<AControlComponentManager>
{
public:
	TDECLARE_CLASS( AControlComponentManager, AUnitComponentManager );

	static constexpr TINT MAX_NUM_COMPONENTS = 16;

public:
	AControlComponentManager();
	~AControlComponentManager();

	virtual void  Update( TFLOAT a_fDeltaTime ) OVERRIDE {} // +0x0c, DOES_NOTHING
	virtual void  ReleaseComponent( AUnitComponent* a_pComp ) OVERRIDE;
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent ); // +0x14

	// Creates a component of a_pClass (an AControlComponent subclass) in the pool.
	AControlComponent* CreateComponent( const Toshi::TClass* a_pClass, AControlComponentData* a_pData );

private:
	void OnGameStateControllerEvent( const AGameStateControllerEvent& a_rEvent );

	static void OnGameStateControllerEventCallback( void* a_pReceiver, const AGameStateControllerEvent& a_rEvent );

private:
	Toshi::T2DerivedDynamicObjectPool<AControlComponent>                        m_oPool;              // 0x0c
	Toshi::T2DList<AControlComponent>                                           m_llComponents;       // 0x24
	AInputCommandArray                                                          m_oCommands;          // 0x2c
	Toshi::TGlobalListener<AControlComponentManager, AGameStateControllerEvent> m_oGameStateListener; // 0xd0
};

TSTATICASSERT( sizeof( AControlComponentManager ) == 0xe0 );
