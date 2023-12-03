#pragma once

namespace Toshi
{
	class TRefCounted
	{
		int m_iRefCount;

	public:
		TRefCounted() { m_iRefCount = 0; }
		TRefCounted(TRefCounted const&) { m_iRefCount = 0; }
		~TRefCounted() { m_iRefCount = -1; }

		inline int DecRefCount() { return m_iRefCount--; }
		inline int GetRefCount() { return m_iRefCount; }
		inline int IncRefCount() { return m_iRefCount++; }
		inline TRefCounted& operator=(TRefCounted const&) { return *this; }
	};
}


