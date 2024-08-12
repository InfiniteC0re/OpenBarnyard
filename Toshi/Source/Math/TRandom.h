#pragma once
/*
------------------------------------------------------------------------------
rand.c: By Bob Jenkins.  My random number generator, ISAAC.  Public Domain.
MODIFIED:
  960327: Creation (addition of randinit, really)
  970719: use context, not global variables, for internal state
  980324: added main (ifdef'ed out), also rearranged randinit()
  010626: Note that this is public domain
------------------------------------------------------------------------------
*/
// mix with source: https://github.com/jawnsy/Math-Random-ISAAC-XS/blob/master/src/rand.c

#define RANDSIZL ( 4 ) /* 8 for crypto, 4 for simulations (aka Toshi uses 4) */
#define RANDSIZ  ( 1 << RANDSIZL )

#include "Toshi/THPTimer.h"

namespace Toshi
{
class TRandom
{
public:
    TRandom();

    TINT GetInt();
    TINT GetInt( TUINT32 a_iLower, TUINT32 a_iUpper );
    TINT GetInt( TUINT32 a_iUpper ) { return GetInt( 0, a_iUpper ); };

    TFLOAT GetFloat();
    TFLOAT GetFloatMinMax( TFLOAT a_fLower, TFLOAT a_fUpper );
    TFLOAT GetFloat2();

    TUINT32 RandRaw();

    void    SetSeed( TUINT a_uiSeed );
    TUINT32 GetSeed() const { return m_uiSeed; }

private:
    void Isaac();
    void RandInit();

private:
    TUINT32 m_uiSeed;              // 0x0
    TUINT32 m_uiRndCnt;            // 0x4
    TUINT32 m_pRandrsl[ RANDSIZ ]; //0x8

    TUINT32 m_pRandmem[ RANDSIZ ]; // 0x48

    TUINT32 m_uiRandA; // 0x88
    TUINT32 m_uiRandB; // 0x8C
    TUINT32 m_uiRandC; // 0x90
};

} // namespace Toshi
