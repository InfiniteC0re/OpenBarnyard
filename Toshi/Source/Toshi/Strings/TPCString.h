#pragma once
#include <Toshi/Strings/TCString.h>
#include <Toshi/Strings/TCStringPool.h>

namespace Toshi
{
	class TPCString
	{
		TCString* m_sStr; //0x0
		mutable TCString m_sStr2; //0x4
		TCStringPool* m_pStringPool; // 0xC

	public:
		inline TCStringPool* GetStringPool() const { return m_pStringPool; }
		inline TCString& GetVolatileCString() const { return m_sStr2; }
		inline const TCString& GetCString() const { return m_sStr == TNULL ? NullString : m_sStr2; }
		inline bool IsEmpty() const { return m_sStr == TNULL ? true : m_sStr2.IsEmpty();; }

		TPCString() { m_sStr = TNULL; }
	};
}


