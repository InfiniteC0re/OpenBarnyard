#pragma once
namespace Toshi
{
	// Only used in JPOG
	class TCStringPool
	{
	public:
		TCStringPool()
		{
			TCStringPool(1024, 0);
		}

		TCStringPool(int unk, int unk2);

		struct StringPool
		{
			TString8* m_szStrings;
			int count;
		};


	private:
		int m_size;               // 0x0
		int m_unk;                // 0x4
		int m_iCount;             // 0x8
		StringPool* m_oStringPool;// 0xC
	};
}