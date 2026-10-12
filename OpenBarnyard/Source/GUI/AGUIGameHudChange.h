#pragma once
#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// NOTE: The name is made up. Base of the events that throw themselves to a
// list of listeners (vtable 0x007297b4 in AGUIGameHudChange).
//-----------------------------------------------------------------------------
class AThrowableEvent
{
public:
	virtual void Throw() = 0; // +0x00
};

//-----------------------------------------------------------------------------
// Purpose: Event telling its listeners (the list at 0x0077dd80) that the game
// HUD changed. Nothing creates it but its TClass and nothing listens to it in
// retail. TClass "AGUIGameHudChange" (size 0xc), vtable 0x007297b8.
//-----------------------------------------------------------------------------
class AGUIGameHudChange
    : public Toshi::TObject
    , public AThrowableEvent
{
public:
	TDECLARE_CLASS( AGUIGameHudChange, Toshi::TObject );

	// Node of the listener list
	struct Listener
	{
		using t_Callback = void( __stdcall* )( void* a_pCaller, AGUIGameHudChange* a_pEvent );

		Listener*  pNext;     // 0x00
		Listener*  pPrev;     // 0x04
		void*      pCaller;   // 0x08
		t_Callback pCallback; // 0x0c
	};

public:
	AGUIGameHudChange();

	//-----------------------------------------------------------------------------
	// AThrowableEvent
	//-----------------------------------------------------------------------------
	virtual void Throw() override;

	// Calls every listener of the list
	static void ThrowEvent( AGUIGameHudChange* a_pEvent );

	static Listener& GetListeners() { return ms_oListeners; }

private:
	// Head of the listener list (0x0077dd80)
	static Listener ms_oListeners;

private:
	TINT m_iUnk8; // 0x08, never set but by the constructor
};
