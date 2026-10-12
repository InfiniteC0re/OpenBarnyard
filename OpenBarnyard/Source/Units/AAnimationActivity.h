#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Plays "animation" on the units of the category (from "namedanimationset" if
// it's set). With "block" it waits for the animation to finish, with
// "breakpoint" until the animation reaches the breakpoint. "rand_dance" picks
// one of the dancemove animations. With "run" set to false the animation isn't
// played, the activity only waits for it.
//-----------------------------------------------------------------------------
class AAnimationActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AAnimationActivity, AInteractionActivity );

	// Breakpoints the activity can wait for ("breakpoint"), order of the name table at 0x007740f0
	using BREAKPOINT = TINT;
	enum BREAKPOINT_ : BREAKPOINT
	{
		BREAKPOINT_PICKUP,             // "pickup_bp"
		BREAKPOINT_HD_THROW,           // "hd_throw_bp"
		BREAKPOINT_START,              // "start_bp"
		BREAKPOINT_END,                // "end_bp"
		BREAKPOINT_START_CLIMB_GETOFF, // "start_climb_getoff_bp"
		BREAKPOINT_END_CLIMB_GETOFF,   // "end_climb_getoff_bp"
		BREAKPOINT_ATTACH,             // "attach_bp"
		BREAKPOINT_DETACH,             // "detach_bp"
		BREAKPOINT_PUTDOWN,            // "putdown_bp"
		BREAKPOINT_NUMOF,
		BREAKPOINT_NONE = BREAKPOINT_NUMOF,
	};

	// Animation event callback (005b2800), calls OnAnimEvent
	using t_AnimEventCallback = void( __stdcall* )( AAnimationActivity* a_pActivity, void* a_pEvent );

	//-----------------------------------------------------------------------------
	// Listener of the global animation event list (head at 0x0077e35c), same as
	// APlayerSteer::AnimEventListener.
	// NOTE: The list and its event aren't reconstructed.
	//-----------------------------------------------------------------------------
	struct AnimEventListener
	{
		AnimEventListener()
		    : pNext( this ), pPrev( this ), pCaller( TNULL ), pCallback( TNULL )
		{}

		~AnimEventListener() { Disconnect(); }

		void Disconnect();

		AnimEventListener*  pNext;     // 0x00
		AnimEventListener*  pPrev;     // 0x04
		AAnimationActivity* pCaller;   // 0x08
		t_AnimEventCallback pCallback; // 0x0c
	};

public:
	AAnimationActivity();
	~AAnimationActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

	// The event's +0x04 is the breakpoint that has been reached.
	// NOTE: a_pCaller is the activity itself (see OnAnimEventCallback)
	virtual void OnAnimEvent( void* a_pCaller, void* a_pEvent ); // +0x3c

private:
	static void __stdcall OnAnimEventCallback( AAnimationActivity* a_pActivity, void* a_pEvent );

private:
	Toshi::TPString8  m_strAnimation;         // 0x08, "animation"
	Toshi::TPString8  m_strPlayingAnim;       // 0x0c, the animation picked by OnStart
	Toshi::TPString8  m_strNamedAnimationSet; // 0x10, "namedanimationset"
	TBOOL             m_bBlock;               // 0x14, "block"
	TBOOL             m_bRun;                 // 0x15, "run", TTRUE by default
	BREAKPOINT        m_eBreakpoint;          // 0x18, "breakpoint"
	AnimEventListener m_oAnimEventListener;   // 0x1c
};

TSTATICASSERT( sizeof( AAnimationActivity ) == 0x2c );
