#pragma once
#include <Toshi/T2DList.h>
#include <Input/TInputInterface.h>

#include "AUnitComponent.h"
#include "Input/AInputMap.h"

class AControlComponentData;
class AControlComponentManager;

// Base for components that drive a unit from input (player, bike, ...). Instances are
// allocated from AControlComponentManager's pool and are only enabled while one of the
// game states listed in their AControlComponentData is active.
class AControlComponent
    : public AUnitComponent
    , public Toshi::T2DList<AControlComponent>::Node
{
public:
	TDECLARE_CLASS( AControlComponent, AUnitComponent );

	friend AControlComponentManager;

public:
	AControlComponent();
	~AControlComponent();

	virtual AUnitComponentManager* GetManager() OVERRIDE; // +0x10

	virtual AControlComponentData* GetData() { return m_pData; }                                                                                     // +0x38, folded getter 005f7a30
	virtual TBOOL                  ProcessCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent ) { return TFALSE; } // +0x3c, RETURNS_ZERO (name from the original source)
	virtual void                   OnDisable() {}                                                                                                    // +0x40, DOES_NOTHING
	virtual void                   OnEnable() {}                                                                                                     // +0x44, DOES_NOTHING

	// Updates the enabled flag, calling OnEnable/OnDisable when it changes.
	void SetEnabled( TBOOL a_bEnabled );

private:
	AControlComponentData* m_pData; // 0x18
};

TSTATICASSERT( sizeof( AControlComponent ) == 0x1c );
