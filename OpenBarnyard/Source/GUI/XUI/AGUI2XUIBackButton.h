#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIButton.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIBackButton : AGUI2XUIButton (0xC8 bytes)
// NOTE: only the TClass of this control is left in the Wii build. Its code
// (and vtable) was stripped by the linker and the TClass can't create objects.
//-----------------------------------------------------------------------------
class AGUI2XUIBackButton
    : public AGUI2XUIButton
{
public:
	TDECLARE_CLASS( AGUI2XUIBackButton, AGUI2XUIButton );
};

#endif // BARNYARD_LATEST
