#pragma once
#include "AInteractionActivity.h"

class AGameState;

//-----------------------------------------------------------------------------
// Plays the scripted sequence (cutscene) "name" in an ASSeqState, or with
// "pushstate" not set, only queues it and sets the game state to go back to
// ("return": "barn" or "sim").
//-----------------------------------------------------------------------------
class AScriptedSequenceActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AScriptedSequenceActivity, AInteractionActivity );

	using RETURN = TINT;
	enum RETURN_ : RETURN
	{
		RETURN_DEFAULT = 0, // the barn if the barn explore minigame is loaded, the sim otherwise
		RETURN_BARN    = 1, // "barn", ABarnExploreMinigameState
		RETURN_SIM     = 2, // "sim", ASimState
	};

	// A scripted sequence of the game (0x007791f0, also used by the debug menu AFrontEndDebugSSeqState::OnInsertion
	// and by the game loader).
	struct SequenceInfo
	{
		const TWCHAR* wszName;           // 0x00, shown by AFrontEndDebugSSeqState
		const TCHAR*  szName;            // 0x04, "name" of the activity, ASSeqState, also the library ScriptedSequence/<szName>
		const TCHAR*  szScript;          // 0x08, ASSeqState
		const TCHAR*  szCamera;          // 0x0c, ASSeqState
		TBOOL         bCanSkip;          // 0x10, copied to the ASSeqState (+0x1e9)
		TINT          eTerrain;          // 0x14, AGameLoader::Terrain the game loader loads for it
		TINT          iSyncType;         // 0x18, ASSeqState
		TINT          iTimeFX;           // 0x1c, forced AGameTimeFX (day phase), -1 if none
		TINT          iTimeFXGroup;      // 0x20, weather of the forced AGameTimeFX
		TINT          iGate;             // 0x24, AGateManager gate, -1 if none
		TBOOL         bGateState;        // 0x28, the gates are opened (or closed)
		TBOOL         bRestoreGate;      // 0x29, the gate gets its state back once the sequence is over
		TINT          iInstanceModels;   // 0x2c, model list of the game loader (s_aInstanceModelLists)
		TBOOL         bShowChapterTitle; // 0x30, ASSeqState
	};

#ifdef BARNYARD_LATEST
	static constexpr TINT NUM_SEQUENCES = 33;

	static constexpr TINT SEQUENCE_SSCUOC_MAIN      = 29;
	static constexpr TINT SEQUENCE_SSBFF_MAIN       = 30;
	static constexpr TINT SEQUENCE_SS06_STORYMERGED = 31;
	static constexpr TINT SEQUENCE_SSUL_MAIN        = 32;
#else
	static constexpr TINT NUM_SEQUENCES = 29;
#endif

public:
	AScriptedSequenceActivity();
	~AScriptedSequenceActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

	static const SequenceInfo ms_aSequences[ NUM_SEQUENCES ];

private:
	TBOOL       m_bPushState; // 0x08, "pushstate"
	TINT        m_iSequence;  // 0x0c, index in ms_aSequences, -1 if unknown
	RETURN      m_eReturn;    // 0x10, "return"
	AGameState* m_pState;     // 0x14, the ASSeqState while it's played
	TINT        m_iGate;      // 0x18, gate to restore once the sequence is over, -1 if none
	TBOOL       m_bGateState; // 0x1c, state of m_iGate before the sequence
};

TSTATICASSERT( sizeof( AScriptedSequenceActivity::SequenceInfo ) == 0x34 );
TSTATICASSERT( sizeof( AScriptedSequenceActivity ) == 0x20 );
