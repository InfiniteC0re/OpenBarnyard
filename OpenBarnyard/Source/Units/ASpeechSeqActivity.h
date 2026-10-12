#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Step of a speech sequence ("action"): sets up the actors at a goal, switches
// to the conversation camera, shows or hides the speech bubbles...
//-----------------------------------------------------------------------------
class ASpeechSeqActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASpeechSeqActivity, AInteractionActivity );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_CAMERA        = BITFLAG( 0 ),  // "camera"
		FLAGS_SETUP_ACTORS  = BITFLAG( 1 ),  // "setupactors" and the variants below
		FLAGS_RESET_ACTORS  = BITFLAG( 3 ),  // "resetactors"
		FLAGS_BUBBLE_ON     = BITFLAG( 4 ),  // "bubble" with "value" TTRUE
		FLAGS_BUBBLE_OFF    = BITFLAG( 5 ),  // "bubble" with "value" TFALSE
		FLAGS_SKIP_POSITION = BITFLAG( 6 ),  // "setupactorsskipposition", "setupactorsfromlocatorskippos"
		FLAGS_SWAP          = BITFLAG( 7 ),  // "swapsetupactors"
		FLAGS_UNSET_ACTORS  = BITFLAG( 8 ),  // "unsetactors"
		FLAGS_FROM_LOCATOR  = BITFLAG( 9 ),  // "setupactorsfromlocator", "setupactorsfromlocatorskippos"
		FLAGS_LOADING       = BITFLAG( 10 ), // set while SetupActors waits for the terrain to load
	};

public:
	ASpeechSeqActivity();
	~ASpeechSeqActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	// Places the units of the category at the speaker spots of the speech camera rig, which is placed
	// at the goal (or locator, or quest locator) and makes them look at each other
	RESULT SetupActors( AInteractionSequence* a_pSequence );

private:
	FLAGS            m_uiFlags;  // 0x08, "action"
	Toshi::TPString8 m_strGoal;  // 0x0c, "goal"
	Toshi::TPString8 m_strModel; // 0x10, "model", "convpoint_01" by default
};

TSTATICASSERT( sizeof( ASpeechSeqActivity ) == 0x14 );
