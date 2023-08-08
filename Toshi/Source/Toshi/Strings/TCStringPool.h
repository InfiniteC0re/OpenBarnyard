#pragma once

#ifdef TOSHI_ENABLE_DEPRECATED

#include "Toshi/Strings/TPooledCString.h"


namespace Toshi
{
	// Only used in JPOG
	// it's also used in Barnyard but it was renamed to TString8Pool
	// as well as TCString was renamed to TString8 and they probably have
	// made some changes to the code too

	class TCStringPool
	{
	public:
		TCStringPool()
		{
			TCStringPool(1024, 0);
		}

		TCStringPool(int unk, int unk2);
		TCStringPool(const char* a_szFileName, int unk, int unk2);

		struct StringPool
		{
			int count;
			TString8* m_szStrings;
		};

		struct StringPools
		{
			StringPool* m_pCStringPool;
			int m_iCountOfPools;
		};

	public:

		void Remove(TPooledCString& a_pcString);
	protected:
		TBOOL ReadFile(const char* a_szFileName);
		void InitStringPool(int a_iStringCount);

	private:
		int m_iMaxSize;           // 0x0
		int m_iStringCount;       // 0x4
		int m_iCapacity;          // 0x8
		StringPools m_oStringPool;// 0xC
	};
}
#endif // TOSHI_ENABLE_DEPRECATED