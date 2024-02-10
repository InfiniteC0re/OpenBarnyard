#include "ToshiPCH.h"
#include "TRandom.h"

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

namespace Toshi
{
	TRandom::TRandom()
	{
		THPTimer timer = THPTimer();
		SetSeed(timer.GetRaw32());
	}

	void TRandom::Isaac()
	{
		TUINT a, b, x, y, * m, * mm, * m2, * r, * mend;
		mm = m_pRandmem;
		r = m_pRandrsl;
		a = m_uiRandA;
		b = cut(m_uiRandB + (++m_uiRandC));

		m = mm;
		mend = m2 = m + (RANDSIZ / 2);
		while (m < mend) {
			shuffle(a, b, mm, m, m2, r, x);
		}

		m2 = mm;
		while (m2 < mend) {
			shuffle(a, b, mm, m, m2, r, x);
		}

		m_uiRandA = a;
		m_uiRandB = b;
	}

	void TRandom::RandInit()
	{
		TUINT32 a, b, c, d, e, f, g, h;
		TINT i;

		m_uiRandA = m_uiRandB = m_uiRandC = 0;

		/* Initialize a to h with the golden ratio and seed */
		a = b = c = d = e = f = g = h = 0x9e3779b9 + m_uiSeed;

		/* scramble it */
		for (i = 0; i < RANDSIZL; i++) {
			mix(a, b, c, d, e, f, g, h);
		}

		TUINT32* m = m_pRandmem;
		TUINT32* r = m_pRandrsl;

		/* initialize using the contents of r[] as the seed */
		for (i = 0; i < RANDSIZ; i += 8) {
			a += r[i];
			b += r[i + 1];
			c += r[i + 2];
			d += r[i + 3];
			e += r[i + 4];
			f += r[i + 5];
			g += r[i + 6];
			h += r[i + 7];

			mix(a, b, c, d, e, f, g, h);

			m[i] = a;
			m[i + 1] = b;
			m[i + 2] = c;
			m[i + 3] = d;
			m[i + 4] = e;
			m[i + 5] = f;
			m[i + 6] = g;
			m[i + 7] = h;
		}

		/* do a second pass to make all of the seed affect all of m */
		for (i = 0; i < RANDSIZ; i += 8) {
			a += m[i];
			b += m[i + 1];
			c += m[i + 2];
			d += m[i + 3];
			e += m[i + 4];
			f += m[i + 5];
			g += m[i + 6];
			h += m[i + 7];

			mix(a, b, c, d, e, f, g, h);

			m[i] = a;
			m[i + 1] = b;
			m[i + 2] = c;
			m[i + 3] = d;
			m[i + 4] = e;
			m[i + 5] = f;
			m[i + 6] = g;
			m[i + 7] = h;
		}

		Isaac();
		m_uiRndCnt = RANDSIZ;
	}

	TINT TRandom::GetInt(TUINT32 a_iLower, TUINT32 a_iUpper)
	{
		TASSERT(a_iUpper > a_iLower, "a_iLower can't be higher than a_iUpper");
		
		unsigned long long rand = RandRaw();
		unsigned long long range = a_iUpper - a_iLower;
		return a_iLower + (range * rand >> 0x20);
	}

	TINT TRandom::GetInt()
	{
		return RandRaw();
	}

	float TRandom::GetFloat()
	{
		return (RandRaw() >> 1) * 4.656613E-10f;
	}

	float TRandom::GetFloat2()
	{
		return (RandRaw() >> 1) * 9.313226E-10f * -1.0f;
	}

	void TRandom::SetSeed(TUINT a_uiSeed)
	{
		m_uiSeed = a_uiSeed;
		RandInit();
	}

	TUINT32 TRandom::RandRaw()
	{
		if (m_uiRndCnt-- == 0)
		{
			Isaac();
			m_uiRndCnt = RANDSIZ - 1;
			//return *(uint *)((TINT)this + 0x44);
		}
		return m_pRandrsl[m_uiRndCnt];
	}

}