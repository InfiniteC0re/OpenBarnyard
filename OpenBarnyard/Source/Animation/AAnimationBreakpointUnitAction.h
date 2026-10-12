#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TPString8.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>

class AUnitProxy;

//-----------------------------------------------------------------------------
// Tells the listeners of the global animation event list (0x0077e35c) that the
// unit owning the object has reached an action ("Mode") of its animation.
// AAnimationActivity and APlayerSteer listen to it.
//-----------------------------------------------------------------------------
class AAnimationBreakpointUnitAction
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointUnitAction, AAnimationBreakpoint );

	// "Mode", order of the name table at 0x007740f0 (shared with AAnimationActivity::BREAKPOINT)
	using ACTION = TINT;
	enum ACTION_ : ACTION
	{
		ACTION_PICKUP,             // "pickup_bp"
		ACTION_HD_THROW,           // "hd_throw_bp"
		ACTION_START,              // "start_bp"
		ACTION_END,                // "end_bp"
		ACTION_START_CLIMB_GETOFF, // "start_climb_getoff_bp"
		ACTION_END_CLIMB_GETOFF,   // "end_climb_getoff_bp"
		ACTION_ATTACH,             // "attach_bp"
		ACTION_DETACH,             // "detach_bp"
		ACTION_PUTDOWN,            // "putdown_bp"
		ACTION_NUMOF,
	};

	// The event thrown to the listeners. Vtable 0x00728090.
	class Event
	{
	public:
		Event( ACTION a_eAction, Toshi::TRef<AUnitProxy> a_pUnitProxy );

		// Throws the event to the animation event list and the list at 0x0077d10c
		virtual void Throw(); // +0x00
		virtual ~Event();     // +0x04

		Toshi::TRef<AUnitProxy> GetUnitProxy() const;

	public:
		ACTION                  eAction;    // 0x04
		Toshi::TRef<AUnitProxy> pUnitProxy; // 0x08, the unit owning the object (can be TNULL)
	};

	// Node of the global animation event list
	// NOTE: AAnimationActivity::AnimEventListener and APlayerSteer::AnimEventListener have the same layout
	struct Listener
	{
		using t_Callback = void( __stdcall* )( void* a_pCaller, Event* a_pEvent );

		Listener*  pNext;     // 0x00
		Listener*  pPrev;     // 0x04
		void*      pCaller;   // 0x08
		t_Callback pCallback; // 0x0c
	};

public:
	AAnimationBreakpointUnitAction();
	~AAnimationBreakpointUnitAction();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;             // +0x10
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) OVERRIDE; // +0x14

	// Calls every listener of the global animation event list
	static void ThrowEvent( Event* a_pEvent );

	static Listener& GetListeners() { return ms_oListeners; }

	// Calls every listener of the second event list (0x0077d10c), nothing registers to it in retail
	static void ThrowOtherEvent( Event* a_pEvent );

private:
	// Head of the global animation event list (0x0077e35c)
	static Listener ms_oListeners;

	// Head of the second event list (0x0077d10c)
	static Listener ms_oOtherListeners;

private:
	// NOTE: same layout as AAnimationBreakpointAttachment, these are never set
	Toshi::TVector4    m_vecUnused;  // 0x08
	Toshi::TQuaternion m_quatUnused; // 0x18
	Toshi::TPString8   m_strUnused1; // 0x28
	Toshi::TPString8   m_strUnused2; // 0x2c
	Toshi::TPString8   m_strUnused3; // 0x30
	Toshi::TPString8   m_strUnused4; // 0x34
	ACTION             m_eAction;    // 0x38
};

TSTATICASSERT( sizeof( AAnimationBreakpointUnitAction ) == 0x3c );
