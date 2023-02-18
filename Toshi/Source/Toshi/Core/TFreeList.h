#pragma once
namespace Toshi
{
	class TFreeList
	{
	public:

		struct Node
		{

		};

		TFreeList(int size, int a_iInitialSize, int a_iGrowSize);

		int GetCapacity() const { return m_iCapacity; }
		int GetGrowSize() const { return m_iGrowSize; }


	private:
		int m_iCapacity; // 0x0
		Node* m_pNodes;  // 0xC
		int m_iGrowSize; // 0x10
	};
}


