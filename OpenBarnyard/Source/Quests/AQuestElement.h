#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>
#include <Plugins/PPropertyParser/PBProperties.h>

#include "GameInterface/AHackedQuickSave.h"
#include "Units/AInteractionSequence.h"

//-----------------------------------------------------------------------------
// Where an AQuestElement takes place ("worldlocation" of the element), shown
// as a marker on ASimMiniMap while the quest is assigned.
// NOTE: vtable 0x0072cd80, the only slot is a folded no-op destructor (005cb380).
//-----------------------------------------------------------------------------
class AQuestElementLocator
{
public:
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_UNIT = 0, // "unit(name)", the unit called name
		TYPE_GOAL = 1, // "goal(name)" or a plain name, a goal of the default interior network
		TYPE_NONE = 2, // no "worldlocation"
	};

public:
	AQuestElementLocator();
	virtual ~AQuestElementLocator() {}

	// Parses "unit(name)", "goal(name)" or "name"
	void SetLocation( const TCHAR* a_szLocation );

	// Adds the marker of the location to ASimMiniMap
	void AddMarker();

	// Removes the marker from ASimMiniMap
	void RemoveMarker();

	TYPE         GetType() const { return m_eType; }
	const TCHAR* GetName() const { return m_szName; }

private:
	TCHAR           m_szName[ 32 ]; // 0x04
	TYPE            m_eType;        // 0x24
	Toshi::TVector4 m_vPosition;    // 0x28
	TBOOL           m_bUnk38;       // 0x38
	TINT            m_iMarker;      // 0x3c, index of the marker on ASimMiniMap or -1
};

//-----------------------------------------------------------------------------
// A step of an AQuest ("elements" of the quest in Data/Quests/*.trb). The
// elements are shared by all the instances of the quest, the per instance
// state (Progress and the data of the subclasses) lives in the buffer of the
// AQuestInstance at m_iDataOffset.
// Created by name: "a%squestelement" with the "element_type" of the property.
//-----------------------------------------------------------------------------
class AQuestElement : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AQuestElement, Toshi::TObject );

	// The state of an element of an AQuestInstance
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_START    = 0, // starts the "startseq" interaction
		STATE_RUNNING  = 1, // waits for CheckProgress
		STATE_STARTSEQ = 2, // the "startseq" interaction is running
		STATE_COMPLETE = 3,
		STATE_ENDSEQ   = 4, // the "endseq" interaction is running
		STATE_FAILSEQ  = 5, // the "failseq" interaction is running
		STATE_FAILED   = 6,
	};

	// Result of CheckProgress
	using PROGRESS = TINT;
	enum PROGRESS_ : PROGRESS
	{
		PROGRESS_COMPLETE = 0,
		PROGRESS_RUNNING  = 1,
		PROGRESS_FAILED   = 2,
	};

	// Per instance state of an element (at m_iDataOffset of the instance data)
	struct Progress
	{
		STATE eState;         // 0x00
		TUINT uiStartSeqId;   // 0x04, AInteractionSequence::GetId of the "startseq" interaction
		TUINT uiEndSeqId;     // 0x08, same for "endseq"
		TUINT uiFailSeqId;    // 0x0c, same for "failseq"
		TBOOL bComplete;      // 0x10, forced completion (AQuestInstance::SetElementState)
		TBOOL bFailed;        // 0x11, forced failure
		TBOOL bFailedForGood; // 0x12, the element fails once the "failseq" interaction is over
	};

public:
	AQuestElement();
	~AQuestElement();

	// Reads the element from the quest file, a_pDataOffset is the size of the instance data so far
	virtual TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataOffset ); // +0x0c

	// Initialises the instance data of the element
	virtual TBOOL InitData( AInteractionActivityData* a_pData ); // +0x10

	// Releases the instance data of the element
	virtual void DeinitData( AInteractionActivityData* a_pData ) {} // +0x14, DOES_NOTHING (00628f40)

	// Debug description of the element
	virtual TBOOL GetDescription( const TWCHAR** a_ppOutText ) { return TFALSE; } // +0x18, RETURNS_FALSE (006ce000)

	virtual void  Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ); // +0x1c
	virtual TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ); // +0x20

	// The next elements can't start before this one is done ("block")
	virtual TBOOL IsBlocking() { return m_bBlock; } // +0x24, folded getter 006c3cc0

	// Checks if the goal of the element has been reached
	virtual PROGRESS CheckProgress( AInteractionActivityData* a_pData ) = 0; // +0x28

	// Runs the element, TRUE once it is complete
	TBOOL Update( AInteractionActivityData* a_pData, TFLOAT a_fDeltaTime );

	Progress* GetProgress( AInteractionActivityData* a_pData ) const { return TREINTERPRETCAST( Progress*, TSTATICCAST( TUINT8, a_pData->pData ) + m_iDataOffset ); }

	AQuestElementLocator&   GetLocator() { return m_oLocator; }
	const Toshi::TPString8& GetName() const { return m_strName; }

protected:
	TBOOL                m_bBlock;      // 0x04, "block"
	Toshi::TPString8     m_strStartSeq; // 0x08, "startseq"
	Toshi::TPString8     m_strEndSeq;   // 0x0c, "endseq"
	Toshi::TPString8     m_strFailSeq;  // 0x10, "failseq"
	Toshi::TPString8     m_strName;     // 0x14, "name"
	AQuestElementLocator m_oLocator;    // 0x18, "worldlocation"
	TINT                 m_iDataOffset; // 0x58, offset of Progress in the instance data
};

TSTATICASSERT( sizeof( AQuestElement::Progress ) == 0x14 );
TSTATICASSERT( sizeof( AQuestElementLocator ) == 0x40 );
TSTATICASSERT( sizeof( AQuestElement ) == 0x5c );
