#pragma once
#include "AUnitComponent.h"

#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Gets the activities ANPCBehaviourManager picks from the unit's schedule and
// starts their FSM (or "<unit type>_<FSM>" if there's one, "loiter_around" if
// the FSM doesn't exist) as the schedule of the unit's AUnitBehaviour, once the
// unit isn't following a command and isn't in an interaction.
//
// TClass 0x007b4f30 (version 0.1, size 0x18), vtable 0x00739758.
//-----------------------------------------------------------------------------
class ANPCBehaviourComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( ANPCBehaviourComponent, AUnitComponent );

public:
	// NOTE: inlined into CreateObject 006339a0 / CreateObjectInPlace 00633a00
	ANPCBehaviourComponent()
	    : m_iActivity( -1 )
	    , m_iLocation( -1 )
	{}

	~ANPCBehaviourComponent();

	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;                          // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;                                // +0x10
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14, writes the "NPC" save block
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18, reads the "NPC" save block
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;             // +0x2c

	// Gives the unit an activity of ANPCBehaviourManager to start.
	// NOTE: retail returns TFALSE
	TBOOL SetActivity( TINT a_iActivity );

	// FSM natives
	Toshi::TPString8 GetActivityLocationName();
	Toshi::TPString8 GetActivityNextLocationName();
	Toshi::TPString8 GetQuestLocationName();
	Toshi::TPString8 GetSleepGoal();
	void             ForceVisibility();

public:
	static AFunctionTable ms_oFunctionTable;

private:
	TINT m_iActivity; // 0x10, activity that has to be started, -1 if there's none
	TINT m_iLocation; // 0x14, location of the activity that was started last
};

TSTATICASSERT( sizeof( ANPCBehaviourComponent ) == 0x18 );
