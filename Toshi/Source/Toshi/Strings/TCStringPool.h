#pragma once
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
			TString8* m_szStrings;
			int count;
		};

		struct StringPools
		{
			StringPool* m_pStringPool;
			int m_iCountOfPools;
		};

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