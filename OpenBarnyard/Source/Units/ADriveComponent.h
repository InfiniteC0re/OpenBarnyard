#pragma once
#include "AUnitComponent.h"
#include "AInteractionSequence.h"

class ADrive;
class ADriveSystem;

//-----------------------------------------------------------------------------
// Gives a unit the drives of its type's ADriveSystem and keeps the data the
// drive functions need per unit (see AHysteresisDriveFunction).
//
// TClass 0x00783f18 (version 0.1, size 0x18), vtable 0x00724844.
//-----------------------------------------------------------------------------
class ADriveComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( ADriveComponent, AUnitComponent );

public:
	ADriveComponent();
	~ADriveComponent();

	virtual AUnitComponentManager* GetManager() OVERRIDE; // +0x10

	// Attaches the drive system and allocates the data its functions need.
	AInteractionActivityData* Initialise( AUnit* a_pUnit, ADriveSystem* a_pDriveSystem );

	// Returns the drive the unit wants most right now.
	ADrive* GetCurrentDrive();

	ADriveSystem* GetDriveSystem() const { return m_pDriveSystem; }

private:
	ADriveSystem*            m_pDriveSystem; // 0x10
	AInteractionActivityData m_oData;        // 0x14
};

TSTATICASSERT( sizeof( ADriveComponent ) == 0x18 );
