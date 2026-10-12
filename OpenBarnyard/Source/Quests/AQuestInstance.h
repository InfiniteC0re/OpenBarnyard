#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TGlobalEmitter.h>

#include "GameInterface/AHackedQuickSave.h"
#include "MiniGame/AMiniGameState.h"
#include "Units/AInteractionManager.h"
#include "Units/AUnitRef.h"

class AQuest;
class AUnit;

//-----------------------------------------------------------------------------
// A quest the player has been assigned (see AQuestManager): the state of the
// quest and of its elements, the unit that gave it and the time spent on it.
// Lives in the T2ObjectPool of AQuestManager.
//-----------------------------------------------------------------------------
class AQuestInstance : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AQuestInstance, Toshi::TObject );

	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_START    = 0, // starts the "startseq" interaction of the quest
		STATE_RUNNING  = 1, // runs the elements
		STATE_COMPLETE = 2,
		STATE_ENDSEQ   = 3, // the "endseq" interaction of the quest is running
		STATE_FAILED   = 4,
		STATE_STARTSEQ = 5, // the "startseq" interaction of the quest is running
	};

	// Values of SetElementState / SetElementsState (see AQuestActivity)
	using ELEMENTSTATE = TINT;
	enum ELEMENTSTATE_ : ELEMENTSTATE
	{
		ELEMENTSTATE_COMPLETE      = 0,
		ELEMENTSTATE_FAILED        = 1,
		ELEMENTSTATE_FAILEDFORGOOD = 2,
	};

	using Listener = Toshi::TGlobalListener<AQuestInstance, AInteractionManagerEvent>;

public:
	AQuestInstance();
	~AQuestInstance();

	// Releases the data of the elements and stops listening to the interactions
	virtual void Reset(); // +0x0c

	virtual AQuest* GetQuest(); // +0x10

	virtual TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer );                      // +0x14
	virtual TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer, TBOOL a_bSpawnUnits ); // +0x18

	// Starts the quest a_iQuest of AQuestManager given by a_pQuestGiver (can be TNULL)
	void Create( TINT a_iQuest, TINT a_iId, AUnit* a_pQuestGiver );

	// Runs the quest, the state changes to STATE_COMPLETE or STATE_FAILED once it's over
	TBOOL Update( TFLOAT a_fDeltaTime );

	// Completes or fails the element a_iElement, a_bCheckBlocking ignores the call while a blocking element before it is running
	void SetElementState( TINT a_iElement, ELEMENTSTATE a_eState, TBOOL a_bCheckBlocking );

	// SetElementState on every element
	void SetElementsState( ELEMENTSTATE a_eState );

	// TRUE if the element a_iElement doesn't run anymore
	TBOOL IsElementDone( TINT a_iElement ) const;

	// Removes the markers of the elements from ASimMiniMap
	void RemoveMarkers();

	// Passes the results of a minigame to the AGameEventQuestElement elements
	void OnMiniGameResult( AMiniGameState::S1* a_pResults );

	// Lets the AGameEventQuestElement elements set up the players of the minigame a_rcName
	void OnMiniGameStart( const Toshi::TPString8& a_rcName );

	// The unit that gave the quest
	AUnit*    GetQuestGiver();
	AUnitRef* GetQuestGiverRef() { return &m_oQuestGiver; }

	TINT                      GetQuestIndex() const { return m_iQuest; }
	TINT                      GetId() const { return m_iId; }
	STATE                     GetState() const { return m_eState; }
	TFLOAT                    GetTime() const { return m_fTime; }
	AInteractionActivityData* GetData() { return &m_oData; }

private:
	// Calls DeinitData of the elements
	void DeinitElements();

	// Follows the "startseq" and "endseq" interactions of the quest and of the elements
	void OnInteractionEvent( const AInteractionManagerEvent& a_rcEvent );

	static void OnInteractionEventCallback( void* a_pInstance, const AInteractionManagerEvent& a_rcEvent );

private:
	TINT                     m_iQuest;           // 0x04, index of the quest in AQuestManager
	TINT                     m_iId;              // 0x08
	STATE                    m_eState;           // 0x0c
	AUnitRef                 m_oQuestGiver;      // 0x10
	TFLOAT                   m_fTime;            // 0x1c, time spent on the quest
	TUINT16                  m_uiActiveElements; // 0x20, a bit for every element that still runs
	AInteractionActivityData m_oData;            // 0x24, data of the elements (AQuest::GetDataSize bytes)
	AInteractionSequence*    m_pEndSeq;          // 0x28
	AInteractionSequence*    m_pStartSeq;        // 0x2c
	Listener                 m_oListener;        // 0x30
};

TSTATICASSERT( sizeof( AQuestInstance ) == 0x40 );
