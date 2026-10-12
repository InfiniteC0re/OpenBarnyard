#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIScene.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUITabScene : AGUI2XUIScene (0x144 bytes)
// NOTE: only the TClass of this control is left in the Wii build. Its code
// (and vtable) was stripped by the linker and the TClass can't create objects.
//-----------------------------------------------------------------------------
class AGUI2XUITabScene
    : public AGUI2XUIScene
{
public:
	TDECLARE_CLASS( AGUI2XUITabScene, AGUI2XUIScene );

private:
	TUINT32 m_aUnk[ 3 ]; // 0x138
};

#endif // BARNYARD_LATEST
