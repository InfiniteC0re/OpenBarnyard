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

#define RANDSIZL  (4)  /* 8 for crypto, 4 for simulations (aka Toshi uses 4) */
#define RANDSIZ   (1 << RANDSIZL)

#include "Toshi/THPTimer.h"

namespace Toshi
{
	class TRandom
	{
	public:
		TRandom();

		int GetInt();
		int GetInt(uint32_t a_iLower, uint32_t a_iUpper);
		inline int GetInt(uint32_t a_iUpper) { return GetInt(0, a_iUpper); };

		float GetFloat();
		float GetFloat2();

		void SetSeed(unsigned int a_uiSeed);

		uint32_t RandRaw();
		
		inline uint32_t GetSeed() { return m_uiSeed; }

	private:
		void Isaac();
		void RandInit();

	private:
		uint32_t m_uiSeed; // 0x0
		uint32_t m_uiRndCnt; // 0x4
		uint32_t m_pRandrsl[RANDSIZ]; //0x8

		uint32_t m_pRandmem[RANDSIZ]; // 0x48

		uint32_t m_uiRandA; // 0x88
		uint32_t m_uiRandB; // 0x8C
		uint32_t m_uiRandC; // 0x90
	};

}

