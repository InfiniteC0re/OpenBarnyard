#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Fades the screen "in" or "out" (the black overlay of the current game state)
// in "time" seconds.
//-----------------------------------------------------------------------------
class AFadeActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AFadeActivity, AInteractionActivity );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_FADE_IN  = BITFLAG( 0 ), // "fade" "in"
		FLAGS_FADE_OUT = BITFLAG( 1 ), // "fade" "out"
		FLAGS_FADED    = BITFLAG( 2 ), // the overlay reached its final alpha
		FLAGS_FINISHED = BITFLAG( 3 ), // the overlay has been shown for a frame with the final alpha
	};

public:
	AFadeActivity();
	~AFadeActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual void   GetDataSize( TINT* a_pSize ) OVERRIDE;                 // +0x10
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	// Sets the alpha of the overlay.
	// NOTE: inlined in OnStart and Update
	static void SetOverlayAlpha( TFLOAT a_fAlpha );

private:
	TUINT8 m_uiUnk08; // 0x08, unused
	FLAGS  m_uiFlags; // 0x09
	TFLOAT m_fRate;   // 0x0c, alpha change per second (1 / "time"), -1.0f by default
};

TSTATICASSERT( sizeof( AFadeActivity ) == 0x10 );
