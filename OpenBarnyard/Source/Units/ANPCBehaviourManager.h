#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>

#include "Helpers/AGameTimeManager.h"

class AUnit;

//-----------------------------------------------------------------------------
// Runs the daily routines of the named NPCs (Data/NPCBehaviour/NPCSchedules.trb,
// in lib_startup). Every time the phase of the day changes, an activity is picked
// from each unit's schedule (for the phase, the week day and the quest tier) and
// handed to the unit's ANPCBehaviourComponent, which starts its FSM as the unit's
// schedule. Activities can also hide or show their units, which is done once the
// units are out of view.
//
// TClass 0x007b4fc4 (version 0.1, size 0x1f0), vtable 0x007398c0, singleton 0x007b4fc0.
//-----------------------------------------------------------------------------
class ANPCBehaviourManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ANPCBehaviourManager>
{
public:
	TDECLARE_CLASS( ANPCBehaviourManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_VISIBILITY_REQUESTS = 52;

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE             = 0,
		FLAGS_UPDATE_SCHEDULES = BITFLAG( 0 ), // the schedules from m_iCurrentSchedule on need an activity
	};

	// Flags of the activities of a schedule, also used by the visibility requests
	using ACTIVITYFLAGS = TUINT8;
	enum ACTIVITYFLAGS_ : ACTIVITYFLAGS
	{
		ACTIVITYFLAGS_NONE     = 0,
		ACTIVITYFLAGS_DISABLED = BITFLAG( 0 ), // the activity can't be picked
		ACTIVITYFLAGS_SHOW     = BITFLAG( 1 ), // the unit is shown during the activity
		ACTIVITYFLAGS_HIDE     = BITFLAG( 2 ), // the unit is hidden during the activity
		ACTIVITYFLAGS_FORCE    = BITFLAG( 3 ), // visibility requests only, done even if the unit is in view (ForceVisibility)
	};

	// NOTE: The structures of NPCSchedules.trb.

	// "NPCActivities"
	struct Activity
	{
		TINT          iLocation;  // 0x00, index into StringMappingTables::ppLocationNames, -1 for none
		TINT          iFSM;       // 0x04, index into StringMappingTables::ppFSMNames
		TUINT8        uiDayPhase; // 0x08, AGameTimeManager::DAYPHASE
		TUINT8        uiWeekDay;  // 0x09, 7 for any day
		TUINT8        uiTierMask; // 0x0a, a bit for every quest tier (from tier 1)
		ACTIVITYFLAGS uiFlags;    // 0x0b, copied to the schedules by ResetActivityFlags
	};

	struct ActivityList
	{
		Activity* pActivities; // 0x00
		TINT      iUnk;        // 0x04
	};

	// "NPCSchedules"
	struct Schedule
	{
		TINT           iUnitName;        // 0x00, index into StringMappingTables::ppUnitNames
		TINT*          piActivities;     // 0x04, indices into ActivityList::pActivities
		ACTIVITYFLAGS* puiActivityFlags; // 0x08
		TINT           iNumActivities;   // 0x0c
	};

	struct ScheduleList
	{
		Schedule* pSchedules;    // 0x00
		TINT      iNumSchedules; // 0x04
	};

	// "StringMappingTables"
	struct StringMappingTables
	{
		const TCHAR** ppUnitNames;     // 0x00, "generic_" names stand for every unit of the type
		TINT          iUnk1;           // 0x04
		const TCHAR** ppLocationNames; // 0x08
		TINT          iUnk2;           // 0x0c
		const TCHAR** ppFSMNames;      // 0x10, "none" for no FSM
		TINT          iUnk3;           // 0x14
	};

	// A unit to hide or show once it's out of view.
	struct VisibilityRequest
	{
		AUnit*        pUnit;   // 0x00
		ACTIVITYFLAGS uiFlags; // 0x04
	};

	// NOTE: retail uses a fixed-capacity vector template here, only its Erase isn't inlined
	struct VisibilityRequestList
	{
		// Erases a request keeping the order of the others, returns the index of the next one.
		TINT Erase( TINT a_iIndex );

		TINT              iNumRequests;                             // 0x00
		VisibilityRequest aRequests[ MAX_NUM_VISIBILITY_REQUESTS ]; // 0x04
	};

public:
	ANPCBehaviourManager();
	~ANPCBehaviourManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Create() OVERRIDE;                        // +0x0c
	virtual TBOOL Reset() OVERRIDE;                         // +0x14
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x1c

	// Unknown virtuals of the class, no override known
	virtual TBOOL Unknown1( void* ) { return TTRUE; } // +0x38, RETURNS_TRUE
	virtual TBOOL Unknown2( void* ) { return TTRUE; } // +0x3c, RETURNS_TRUE
	virtual TBOOL Unknown3( void* ) { return TTRUE; } // +0x40, RETURNS_TRUE

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Makes the next update pick new activities for every schedule.
	void RequestScheduleUpdate();

	// Hides or shows a_pUnit once it's out of view (a_uiFlags: ACTIVITYFLAGS_HIDE or ACTIVITYFLAGS_SHOW).
	// Replaces the earlier requests of the unit.
	void AddVisibilityRequest( AUnit* a_pUnit, ACTIVITYFLAGS a_uiFlags );

	// Makes the request of a_pUnit be done even if the unit is in view.
	void ForceVisibility( AUnit* a_pUnit );

	Activity*    GetActivity( TINT a_iActivity );
	const TCHAR* GetFSMName( TINT a_iFSM );
	const TCHAR* GetLocationName( TINT a_iLocation );

	// Hides or shows a unit right away, with everything that makes it take part in the world.
	static void SetUnitVisible( AUnit* a_pUnit, TBOOL a_bVisible );

private:
	// Copies the flags of the activities to the schedules.
	void ResetActivityFlags();

	// Picks a random enabled activity of the schedule for the current phase of the day, -1 if there's none.
	TINT PickActivity( TINT a_iSchedule );

	// Returns the visibility flags of the schedule's first activity for the current phase that hides or shows its unit.
	ACTIVITYFLAGS GetVisibilityFlags( TINT a_iSchedule );

	// Gives the units of the schedules from m_iCurrentSchedule on their activity.
	void UpdateSchedules();

	// Hides or shows the units of the requests that are out of view.
	void UpdateVisibility();

	static TBOOL __stdcall OnDateChangeCallback( ANPCBehaviourManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent );

private:
	Toshi::TListener<AGameTimeManager, AGameTimeManager::DateChangeEvent, ANPCBehaviourManager> m_oDateChangeListener; // 0x20
	FLAGS                                                                                       m_uiFlags;             // 0x34
	TINT                                                                                        m_iUnk38;              // 0x38, only ever zeroed
	VisibilityRequestList                                                                       m_oVisibilityRequests; // 0x3c
	ScheduleList*                                                                               m_pSchedules;          // 0x1e0, "NPCSchedules"
	ActivityList*                                                                               m_pActivities;         // 0x1e4, "NPCActivities"
	StringMappingTables*                                                                        m_pStringTables;       // 0x1e8, "StringMappingTables"
	TINT                                                                                        m_iCurrentSchedule;    // 0x1ec
};

TSTATICASSERT( sizeof( ANPCBehaviourManager::Activity ) == 0xc );
TSTATICASSERT( sizeof( ANPCBehaviourManager::Schedule ) == 0x10 );
TSTATICASSERT( sizeof( ANPCBehaviourManager ) == 0x1f0 );
