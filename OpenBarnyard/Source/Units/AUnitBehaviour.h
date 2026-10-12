#pragma once
#include "AUnitComponent.h"
#include "FSM/AFSMInstance.h"

#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

class ADrive;
class AFSM;
class AUnitBehaviourData;

//-----------------------------------------------------------------------------
// Runs the FSM script that controls a unit. Every few seconds the unit's
// ADriveComponent tells it which drive the unit wants most, and the FSM of that
// drive is started: the drive's own FSM, or for the "suggestion", "command" and
// "schedule" drives the FSM that was requested last (by interactions, quests and
// ANPCBehaviourManager's schedules).
//
// TClass 0x0078a368 (version 3.0, size 0x1f4), vtable 0x00728d14.
//-----------------------------------------------------------------------------
class AUnitBehaviour
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AUnitBehaviour, AUnitComponent );

	friend class AUnitBehaviourData;

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE              = 0,
		FLAGS_SUGGESTION_NEW    = BITFLAG( 0 ), // a new suggestion FSM was requested
		FLAGS_COMMAND_PENDING   = BITFLAG( 1 ), // the command FSM couldn't be started yet
		FLAGS_BEHAVIOUR_STOPPED = BITFLAG( 2 ), // UpdateAI does nothing, never set in retail
	};

	// An FSM to run with its arguments.
	// NOTE: The constructor is inlined into AUnitBehaviour's.
	struct FSMRequest
	{
		FSMRequest()
		    : pArgs( TNULL )
		{}

		~FSMRequest();

		// Copies the name and the arguments (TNULL for none).
		TBOOL Set( const Toshi::TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs );

		void Save( AHackedQuickSave::Buffer* a_pBuffer );
		void Load( AHackedQuickSave::Buffer* a_pBuffer );

		Toshi::TPString8     strFSM; // 0x00
		PPropertyValueArray* pArgs;  // 0x04, &oArgs or TNULL
		PPropertyValueArray  oArgs;  // 0x08
	};

public:
	AUnitBehaviour();
	~AUnitBehaviour();

	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;                          // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;                                // +0x10
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;             // +0x2c

	// TRUE if the FSM has finished.
	TBOOL IsFSMFinished();

	// Makes UpdateAI ask for the drive again on its next update.
	// $Barnyard: FUNCTION 00592e30
	void ResetDriveTimer() { m_fDriveTimer = 0.0f; }

	// $Barnyard: FUNCTION 00592e40
	TBOOL IsBehaviourStopped() const { return ( m_uiFlags & FLAGS_BEHAVIOUR_STOPPED ) == FLAGS_BEHAVIOUR_STOPPED; }

	// FSM native
	void SetCurrentFSMInterruptable( TBOOL a_bInterruptable );

	// Resets the command attribute so another drive takes over.
	void ClearCommand();

	// Requests a_rcFSM to run while the suggestion drive is the strongest.
	// Fails if the suggestion isn't the current drive of the unit.
	TBOOL SetSuggestion( const Toshi::TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, TBOOL a_bUnused );

	// Starts a_rcFSM as the unit's command (the command drive takes over), or keeps
	// it for later if the current FSM can't be interrupted.
	TBOOL SetCommand( const Toshi::TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, TBOOL a_bSetRequest );

	// Starts a_rcFSM as the unit's schedule unless the unit is following a command.
	TBOOL SetSchedule( const Toshi::TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, TBOOL a_bSetRequest );

	ADrive* GetCurrentDrive() const { return m_pCurrentDrive; }

	AFSMInstance* GetFSMInstance() { return &m_oFSMInstance; }

	// Returns the FSM called a_rcName, TNULL if there's none.
	static AFSM* FindStateMachine( const Toshi::TPString8& a_rcName );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	// Starts the FSM of a_pDrive if the current FSM can be changed.
	TBOOL SetDrive( ADrive* a_pDrive );

	// Starts a_pRequest's FSM (after setting it to a_rcFSM if a_bSetRequest) and makes the
	// drive called a_rcDrive the current one, if the current FSM can be changed.
	TBOOL StartRequest( FSMRequest* a_pRequest, const Toshi::TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, const Toshi::TPString8& a_rcDrive, TBOOL a_bSetRequest );

private:
	AUnitBehaviourData* m_pData;         // 0x10
	ADrive*             m_pCurrentDrive; // 0x14
	ADrive*             m_pPrevDrive;    // 0x18
	TFLOAT              m_fDriveTimer;   // 0x1c, time left until the drive is checked again

	AFSMInstance m_oFSMInstance; // 0x20

	FSMRequest m_oSuggestion; // 0x19c
	FSMRequest m_oCommand;    // 0x1b8
	FSMRequest m_oSchedule;   // 0x1d4
	FLAGS      m_uiFlags;     // 0x1f0
};

TSTATICASSERT( sizeof( AUnitBehaviour::FSMRequest ) == 0x1c );
TSTATICASSERT( sizeof( AUnitBehaviour ) == 0x1f4 );
