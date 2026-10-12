#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIButton.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUINavButton : AGUI2XUIButton (0xCC bytes)
// NOTE: only the TClass of this control is left in the Wii build. Its code
// (and vtable) was stripped by the linker and the TClass can't create objects.
//-----------------------------------------------------------------------------
class AGUI2XUINavButton
    : public AGUI2XUIButton
{
public:
	TDECLARE_CLASS( AGUI2XUINavButton, AGUI2XUIButton );

private:
	TUINT32 m_aUnk[ 1 ]; // 0xC8
};

#endif // BARNYARD_LATEST
