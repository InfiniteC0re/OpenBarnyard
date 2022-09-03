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

#ifdef USE_PORTABLE
#define cut(a)     ((a) & 0xffffffff) /* Cut the integer down to 32bits */
#else
#define cut(a)     (a) /* A no-op */
#endif

#define ind(mm,x)  ((mm)[(x>>2)&(RANDSIZ-1)])
/* the call to cut() is a macro defined in standard.h */
#define rngstep(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = cut((a^(mix)) + *(m2++)); \
  *(m++) = y = cut(ind(mm,x) + a + b); \
  *(r++) = b = cut(ind(mm,y>>RANDSIZL) + x); \
}
#define mix(a,b,c,d,e,f,g,h) \
{ \
  a^=b<<11; d+=a; b+=c; \
  b^=c>>2;  e+=b; c+=d; \
  c^=d<<8;  f+=c; d+=e; \
  d^=e>>16; g+=d; e+=f; \
  e^=f<<10; h+=e; f+=g; \
  f^=g>>4;  a+=f; g+=h; \
  g^=h<<8;  b+=g; h+=a; \
  h^=a>>9;  c+=h; a+=b; \
}
#define shuffle(a, b, mm, m, m2, r, x) \
{ \
  rngstep(a<<13, a, b, mm, m, m2, r, x); \
  rngstep(a>>6 , a, b, mm, m, m2, r, x); \
  rngstep(a<<2 , a, b, mm, m, m2, r, x); \
  rngstep(a>>16, a, b, mm, m, m2, r, x); \
}

#define RANDSIZL  (4)  /* 8 for crypto, 4 for simulations (aka Toshi uses 4) */
#define RANDSIZ   (1 << RANDSIZL)

#include "Toshi/Core/THPTimer.h"

namespace Toshi
{
	class TRandom
	{
	public:
		TRandom();

		int GetInt();
		int GetInt(uint32_t a_iUpper);
		int GetInt(uint32_t a_iLower, uint32_t a_iUpper);

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

