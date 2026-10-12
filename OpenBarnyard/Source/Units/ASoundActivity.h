#pragma once
#include "AInteractionActivity.h"
#include "Sound/ASoundLegacy.h"

//-----------------------------------------------------------------------------
// Plays, stops or fades a sound. The property that is set picks the type.
//-----------------------------------------------------------------------------
class ASoundActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASoundActivity, AInteractionActivity );

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_SFX          = 0, // "sfx"
		TYPE_SFX3D        = 1, // "sfx3d", at the position of the unit
		TYPE_STOP         = 2, // "stop"
		TYPE_FADEOUT      = 3, // "fadeout"
		TYPE_FADEIN       = 4, // "fadein"
		TYPE_FADESTOP     = 5, // "fadestop"
		TYPE_STREAMEDVO3D = 6, // "streamedvo3d", at the position of the unit
	};

public:
	ASoundActivity();
	~ASoundActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TYPE     m_eType;  // 0x08
	ASoundId m_iSound; // 0x0c, -1 if not set
	TINT     m_iCue;   // 0x10, cue (or stream) the activity plays, -1 if none
};

TSTATICASSERT( sizeof( ASoundActivity ) == 0x14 );
