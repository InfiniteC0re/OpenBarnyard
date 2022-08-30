#pragma once
namespace Toshi
{
	class TModel
	{
		unsigned int m_Flags; // 0x4
		int m_iLODCount; // 0xC
		void* m_pSkeleton; // 0x14

		inline bool IsCreated() { return (m_Flags & 1) != 0; }

		inline int GetLODCount() { return m_iLODCount; }
	};
}


