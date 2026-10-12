#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>

// Breakpoint that throws an event to the listeners when the animation reaches it.
// Listeners are connected through AAnimatableObject::GetAnimationEmitter.
class AAnimationBreakpointCallback
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointCallback, AAnimationBreakpoint );

	// Data of the thrown event
	struct Event
	{
		Toshi::TPString8   strName; // 0x0, name of the breakpoint
		AAnimatableObject* pObject; // 0x4, object that plays the animation
	};

	using Emitter = Toshi::TEmitter<AAnimationBreakpointCallback, Event>;

public:
	AAnimationBreakpointCallback();
	~AAnimationBreakpointCallback();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) override;
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) override;

	Emitter& GetEmitter() { return m_oEmitter; }

	// $Barnyard: FUNCTION 00472cb0
	Toshi::TPString8 GetName() const { return m_strName; }

private:
	Emitter          m_oEmitter; // 0x8
	Toshi::TPString8 m_strName;  // 0x14
};
