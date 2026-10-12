#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIControl.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIProgressBar : AGUI2XUIControl (0xC8 bytes)
// NOTE: only the TClass of this control is left in the Wii build. Its code
// (and vtable) was stripped by the linker and the TClass can't create objects.
//-----------------------------------------------------------------------------
class AGUI2XUIProgressBar
    : public AGUI2XUIControl
{
public:
	TDECLARE_CLASS( AGUI2XUIProgressBar, AGUI2XUIControl );
};

#endif // BARNYARD_LATEST
