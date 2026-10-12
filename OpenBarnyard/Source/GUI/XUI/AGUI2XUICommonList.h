#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIList.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUICommonList : AGUI2XUIList (0xC8 bytes)
// NOTE: only the TClass of this control is left in the Wii build. Its code
// (and vtable) was stripped by the linker and the TClass can't create objects.
//-----------------------------------------------------------------------------
class AGUI2XUICommonList
    : public AGUI2XUIList
{
public:
	TDECLARE_CLASS( AGUI2XUICommonList, AGUI2XUIList );
};

#endif // BARNYARD_LATEST
